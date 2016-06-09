#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>

#include <unistd.h>
#include <machine/cheri.h>
#include <machine/cheric.h>
#include <cheri/sandbox.h>
#include <cheri/cheri_type.h>
#include <cheri/cheri_system.h>
#include <ucontext.h>
#include <cheri/cheri_stack.h>
#include <sys/mman.h>
#include <sys/tree.h>


#include "jam.h"

#include "uthash.h"
#include "hash.h"
#include "jni.h"
#include "jni-internal.h"
#include "natives.h"
#include "symbol.h"
#include "excep.h"
#include "reflect.h"



extern char *mangleClassAndMethodName(pMethodBlock mb);
extern int nativeExtraArg(pMethodBlock mb);

extern uintptr_t *callSandboxedJNIMethod(struct cheri_object sandbox,
                                void __capability** cap_args, int cap_arg_count,
                                int64_t *int_args, int int_arg_count,
                                uintptr_t *ostack, int method_num, int ret);
extern struct _JNINativeInterface Jam_JNINativeInterface;
static const struct _JNINativeInterface *env = &Jam_JNINativeInterface;

/**
 * Enumerated type defining the scope of the sandbox.
 */
typedef enum {
    /**
     * Global - one sandbox per CHERI class.
     */
    SandboxScopeGlobal,
    /**
     * One sandbox per CHERI class, Java object pair.
     */
    SandboxScopeObject,
    /**
     * Ephemeral sandbox that exists for the duration of the method invocation.
     */
    SandboxScopeMethod
} SandboxScope;

/**
 * Array of names for mapping SandboxScope values to human-readable names.
 */
static const char *SandboxScopeNames[] = {
    "Global", "Object", "Method"
};

/**
 * Capability version of the sandboxed JNI env pointer.
 */
typedef __capability struct _JNISandboxedNativeInterface* SJNIEnvPtr_c;
/**
 * Non-capability version of the sandboxed JNI env pointer.
 */
typedef struct _JNISandboxedNativeInterface* SJNIEnvPtr;
/**
 * Capability pointer to a capability pointer to the sandboxed JNI type.  This
 * is the type of the argument to all JNI methods.
 */
typedef const __capability struct _JNISandboxedNativeInterface *__capability * JNIEnvType;

/**
 * Structure describing a Java object pointer that has been delegated to the sandbox.
 */
struct shared_sealed_cap
{
    /**
     * The (sealed) capability representing the object.
     */
    __capability void *cap;
    /**
     * The number of global references to this object.  When this reaches zero,
     * all remaining references are local and the JVM may zero the remaining
     * pointers.
     */
    int refcount;
    /**
     * Hash handle, used for storing these in a hash table.
     */
    UT_hash_handle hh;
};

/**
 * Structure describing a range of Java heap memory that has been passed to the
 * sandbox.  Sandboxed code may subset delegated unsealed capabilities and so
 * we must look for any capability within this range when revoking.
 *
 * FIXME: This currently assumes arrays, but should be extended to support
 * direct buffers and strings.
 */
struct shared_unsealed_cap
{
    /**
     * The Java object that owns this buffer.
     */
    jarray array;
    /**
     * The base of the buffer.
     */
    size_t base;
    /**
     * The length of the buffer.
     */
    size_t length;
    /**
     * The number of references to this object.  Note that the refcount applies
     * to the entire array and some references within the sandbox may be to
     * shorter regions.
     */
    int refcount;
    /**
     * When we find a global during a sweep, we mark it as seen.  We can then
     * delete any for which no references remain.
     */
    bool marked;
    /**
     * The handle for storing this structure in a red-black tree.
     */
    RB_ENTRY(shared_unsealed_cap) entry;
};

/**
 * A single instance of a sandbox object, including all of the metadata related
 * to the capabilities that it holds.
 *
 * This structure is used to implement a pool so that sandboxes can be reused.
 */
struct jni_sandbox_object
{
    /**
     * The libcheri object that is used to implement this JNI sandbox.
     */
    struct sandbox_object *obj;
    /**
     * The CHERI object used to invoke this sandbox (the code and data
     * capability).  This can be extracted from the sandbox libcheri object,
     * but it is more efficient to keep a cached copy here as this is needed on
     * every invocation.
     */
    struct cheri_object cheri_obj;
    /**
     * The pointer to the JNI environment that will be passed into the sandbox
     * with each method invocation.  These are relatively expensive to create
     * and reusing them means that we never need to garbage collect them.
     */
    SJNIEnvPtr_c env;
    /**
     * Hash table containing all of the sealed capabilities.  Sealed
     * capabilities can not be modified and so we can perform exact
     * comparisons to find members here.
     */
    struct shared_sealed_cap *sealed_caps;
    /**
     * What is the current scope of this sandbox?  This is used to determine
     * whether we need to track lifetime of objects (for method-scoped
     * invocations, we do not because we implicitly collect everything at the
     * end).
     */
    SandboxScope scope;
    /**
     * Red-black tree containing all of the unsealed capabilities delegated to
     * this sandbox.  The untrusted code can derive restricted capabilities
     * from this and so we must perform comparison based on the 
     */
    RB_HEAD(unsealed_cap_tree, shared_unsealed_cap) unsealed_caps;
    /**
     * Next pointer, used to implement a linked list when we need a pool of
     * these.  We always push and pop from the head, so there is never a need
     * to chase this pointer chain.
     */
    struct jni_sandbox_object *next;
};

/**
 * Metadata for a CHERI object used as a JNI compartment.
 */
struct jni_sandbox
{
    /**
     * The sandbox name.
     *
     * Immutable after creation
     */
    const char *sandbox_name;
    /**
     * The libcheri class for this compartment.
     *
     * Immutable after creation
     */
    struct sandbox_class *cls;
    /**
     * The lock protecting the `global_object` field.  This is a recursive
     * mutex that must be held for as long as native code is executing.
     */
    pthread_mutex_t global_lock;
    /**
     * The global instance of this class, used for globally scoped sandbox
     * objects.  This is created lazily, the first time that a method with a
     * persistent sandbox is created.
     */
    struct jni_sandbox_object *global_object;
    /**
     * Lock protecting modifications to the `pool_head` list.  This is a linked
     * list of sandbox objects that can be reset, rather than allocating a new
     * one each time.
     */
    pthread_mutex_t pool_lock;
    /**
     * The first sandbox in the pool.
     */
    struct jni_sandbox_object *pool_head;
    /**
     * Hash handle, used for a hash table keyed on the name to allow name to
     * sandbox mappings.
     */
    UT_hash_handle hh;
};

/**
 * Compare function used by the red-black tree implementation.
 */
int unsealed_cmp(struct shared_unsealed_cap *r, struct shared_unsealed_cap *l)
{
    return (l->base < r->base ? -1 : l->base > r->base);
}

RB_GENERATE_STATIC(unsealed_cap_tree, shared_unsealed_cap, entry, unsealed_cmp);

/**
 * Lock used to protect the `sandboxes` variable.
 */
static pthread_mutex_t sandboxes_lock = PTHREAD_MUTEX_INITIALIZER;
/**
 * Hash table mapping from class names to `jni_sandbox` structures.
 */
static struct jni_sandbox *sandboxes;

/**
 * Metadata for a sandbox method, extracted from the Java annotations
 * associated with that method the first time that it is invoked.
 */
struct jni_sandbox_method {
    /**
     * The sandbox that this method will use.
     */
    struct jni_sandbox *sandbox;
    /**
     * The method number for the function that implements this method.
     */
    int method_num;
    /**
     * The scope of the sandbo method invocation.
     */
    SandboxScope scope;
};

/**
 * Macro for applying another macro to each of the sealing types.  The macro
 * provided as the `x` argument is invoked with the name and the C type of each
 * sealing type.
 */
#define SEALING_TYPES(x) \
    x(object,jobject)    \
    x(fieldID,jfieldID)  \
    x(methodID,jfieldID) \
    x(class,jclass)      \
    x(jnienv,SJNIEnvPtr)

/**
 * Macro for declaring each of the capability type variables.
 */
#define DECLARE_SEALING_TYPE(name, ignored) \
static __capability void *name##_type;
SEALING_TYPES(DECLARE_SEALING_TYPE)

/**
 * Key protecting global initialization from running more than once.
 */
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

/**
 * Function called by `pthread_once` to set up global state.
 */
static void initGlobalSandboxState(void)
{
#define INIT_SEALING_TYPE(name, ignored) \
    name##_type = cheri_type_alloc();
SEALING_TYPES(INIT_SEALING_TYPE)
}

/**
 * Debugging function to dump a capability.
 */
static void print_cap(__capability void *c)
{
    fprintf(stderr, "b:0x%llx l:0x%llx o:0x%llx p:0x%d t:%d s:%d\n",
        (unsigned long long)__builtin_memcap_base_get(c),
        (unsigned long long)__builtin_memcap_length_get(c),
        (unsigned long long)__builtin_memcap_offset_get(c),
        (int)__builtin_memcap_perms_get(c),
        (int)__builtin_memcap_type_get(c),
        (int)__builtin_memcap_sealed_get(c));
}

/**
 * Construct a capability from an integer pointer and seal it with the provided
 * type.
 */
static __capability void *seal(void* obj, __capability void *type)
{
    return __builtin_memcap_seal((__capability void*)obj, type);
}

/**
 * Unseal a capability and return the pointer that it corresponds to.  This
 * will return `NULL` if called with a capability that is not sealed with the
 * expected type.
 */
static void *unseal(__capability void* obj, __capability void* type)
{
    if (obj == NULL)
    {
        return NULL;
    }
    if (!__builtin_memcap_sealed_get(obj) ||
        (__builtin_memcap_type_get(obj) != __builtin_memcap_base_get(type) +
         __builtin_memcap_offset_get(type)))
    {
        return NULL;
    }
    return (void*)__builtin_memcap_unseal(obj, type);
}

/**
 * Unwind the trusted stack by two frames.  THis function is called when one of
 * the JNI functions encounters an error.
 */
__attribute__((cold))
static void jni_error(void)
{
    cherierrno = -1;
    ucontext_t context;
    getcontext(&context);
    cheri_stack_unwind(&context, -1, CHERI_STACK_UNWIND_OP_N, 2);
}

/**
 * Macro used for argument type checking in JNI functions.  Takes the expected
 * condition and error return value as arguments.
 */
#define REQUIRE(x, ret) \
    if (!x)\
    {\
        jni_error();\
        return ret;\
    }

/**
 * Macros used with `SEALING_TYPES` to generate a pair of functions for sealing
 * and unsealing all of the types that are passed into native code.
 */
#define SEAL_FUNCTIONS(name, type)             \
static inline type##_c  seal_##name(type obj)  \
{                                              \
    return seal(obj, name##_type);             \
}                                              \
static inline type unseal_##name(type##_c obj) \
{                                              \
    return unseal(obj, name ##_type);          \
}
SEALING_TYPES(SEAL_FUNCTIONS)

/**
 * Function attributes for all of the callback functions.
 */
#define SJNI_CALLBACK \
    __attribute__((cheri_ccallee))\
    __attribute__((cheri_method_class(_cheri_system_object)))

/**
 * Helper function to record the fact that an array has been delegated to
 * native code.
 */
static void insert_unsealed_cap(JNIEnvType ptr, jarray array, void *base,
        size_t length)
{
    struct jni_sandbox_object *pool =
       (void*)unseal_jnienv((*ptr)->reserved1);
    if (pool->scope == SandboxScopeMethod)
    {
        return;
    }
    struct shared_unsealed_cap *new = calloc(1, sizeof(struct shared_unsealed_cap));
    new->base = (size_t)base;
    new->length = length;
    new->refcount = 1;
    new->array = array;
    struct shared_unsealed_cap *old = RB_INSERT(unsealed_cap_tree,
            &pool->unsealed_caps, new);
    if (old)
    {
        free(new);
        old->refcount++;
    }
    else
    {
        env->NewGlobalRef(&env, array);
    }
}
/**
 * Helper function to record the fact that an object has been delegated to
 * native code.  The `isLocal` argument indicates whether this is a local
 * reference (and so can be collected when the invocation ends) or a global
 * reference (and so must be collected later).
 */
static int insert_object_reference(struct jni_sandbox_object *pool,
        jobject_c cap, bool isLocal)
{
    if (pool->scope == SandboxScopeMethod)
    {
        return 0;
    }
    struct shared_sealed_cap *original;
    HASH_FIND(hh, pool->sealed_caps, &cap, sizeof(cap), original);
    if (!original)
    {
        original = calloc(1, sizeof(struct shared_sealed_cap));
        original->cap = cap;
        HASH_ADD(hh, pool->sealed_caps, cap, sizeof(cap), original);
    }
    if (!isLocal)
    {
        original->refcount++;
    }
    return original->refcount;
}

/**
 * Helper function to record the fact that a global object reference has been
 * delegated to native code.
 */
static int remove_object_reference(struct jni_sandbox_object *pool,
        jobject_c cap)
{
    struct shared_sealed_cap *original;
    HASH_FIND(hh, pool->sealed_caps, &cap, sizeof(cap), original);
    if (!original)
    {
        return -1;
    }
    return --(original->refcount);
}

SJNI_CALLBACK jint sjni_GetVersion(JNIEnvType ptr)
{
  return env->GetVersion(&env);
}

#define GET_PRIM_ARRAY_ELEMENTS(type, native_type)                           \
SJNI_CALLBACK                                                                \
__capability native_type *sjni_Get##type##ArrayElements(JNIEnvType ptr,     \
                                          __capability void *array_cap,      \
                                          __capability jboolean *isCopy) {   \
    void *array_ref = unseal_object(array_cap);                              \
    if (!array_ref)                                                          \
    {                                                                        \
      return NULL;                                                           \
    }                                                                        \
    jboolean isCopyLocal;                                                    \
    __capability native_type *buffer = (__capability native_type *)          \
        env->Get##type##ArrayElements(&env,                                  \
            (native_type##Array)array_ref, &isCopyLocal);                    \
    if(isCopy != NULL)                                                       \
        *isCopy = isCopyLocal;                                               \
    size_t length = ARRAY_LEN(REF_TO_OBJ(array_ref)) * sizeof(native_type);  \
    insert_unsealed_cap(ptr, array_ref, (void*)buffer, length);              \
    buffer = __builtin_memcap_bounds_set(buffer, length);                    \
    buffer = __builtin_memcap_perms_and(buffer,                              \
            ~__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ &              \
            ~__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__);               \
    return buffer;                                                           \
}

// FIXME: Actually release the array until we've done the revocation
#define RELEASE_PRIM_ARRAY_ELEMENTS(type, native_type)                       \
SJNI_CALLBACK                                                                \
void sjni_Release##type##ArrayElements(JNIEnvType ptr, native_type##Array_c \
        array_cap, __capability native_type *elems, jint mode) {             \
    void *array_ref = unseal_object(array_cap);                              \
    if (!array_ref)                                                          \
    {                                                                        \
      return;                                                                \
    }                                                                        \
    struct shared_unsealed_cap search;                                       \
    struct jni_sandbox_object *pool =                             \
       (void*)unseal_jnienv((*ptr)->reserved1);                                 \
    size_t base = __builtin_memcap_base_get(elems);                          \
    search.base = base;                                                      \
    struct shared_unsealed_cap *original =                                   \
        RB_NFIND(unsealed_cap_tree, &pool->unsealed_caps, &search);          \
    if (base <= original->base + original->length)                           \
    {                                                                        \
       original->refcount--;                                                 \
    }                                                                        \
}

SJNI_CALLBACK  jobject_c sjni_GetObjectField(JNIEnvType ptr, jobject_c obj, jfieldID_c fieldID)
{
    jobject object = unseal_object(obj);
    REQUIRE(object, NULL);
    jfieldID f = unseal_fieldID(fieldID);
    REQUIRE(f, NULL);
    jobject field = env->GetObjectField(&env, object, f);
    REQUIRE(field, NULL);
    jobject_c fieldObject = seal_object(field);
    struct jni_sandbox_object *pool =
       (void*)unseal_jnienv((*ptr)->reserved1);
    insert_object_reference(pool, fieldObject, true);
    return fieldObject;
}

SJNI_CALLBACK  void sjni_SetObjectField(JNIEnvType ptr, jobject_c obj,
        jfieldID_c fieldID, jobject_c val)
{
    jobject object = unseal_object(obj);
    REQUIRE(object, );
    jfieldID f = unseal_fieldID(fieldID);
    REQUIRE(f, );
    jobject value = unseal_object(val);
    REQUIRE(object, );
    env->SetObjectField(&env, object, f, value);
}

#define FIELD_ACCESSOR(type, native_type) \
SJNI_CALLBACK  native_type sjni_Get##type##Field(JNIEnvType ptr,              \
        jobject_c obj, jfieldID_c fieldID)                                     \
{                                                                              \
    jobject object = unseal_object(obj);                                       \
    if (object == NULL)                                                        \
    {                                                                          \
        jni_error();                                                           \
        return 0;                                                              \
    }                                                                          \
    jfieldID f = unseal_fieldID(fieldID);                                      \
    if (f == NULL)                                                             \
    {                                                                          \
        jni_error();                                                           \
        return 0;                                                              \
    }                                                                          \
    return env->Get##type##Field(&env, unseal_object(obj), f);                  \
}

#define FIELD_SETTER(type, native_type) \
SJNI_CALLBACK  void  sjni_Set##type##Field(JNIEnvType ptr,                    \
        jobject_c obj, jfieldID_c fieldID, native_type val)                    \
{                                                                              \
    jobject object = unseal_object(obj);                                       \
    if (object == NULL)                                                        \
    {                                                                          \
        jni_error();                                                           \
        return;                                                                \
    }                                                                          \
    jfieldID f = unseal_fieldID(fieldID);                                      \
    if (f == NULL)                                                             \
    {                                                                          \
        jni_error();                                                           \
        return;                                                                \
    }                                                                          \
    env->Set##type##Field(&env, unseal_object(obj), f, val);                    \
}

/**
 * Helper function for native-to-Java calls.  Unwraps all of the arguments
 * provided in an array.
 */
static void unwrapArguments(__capability jvalue_c *capargs, jvalue *args, int argc)
{
    for (int i=0 ; i<argc; i++)
    {
        if (__builtin_memcap_tag_get(capargs[i].l))
        {
            args[i].l = unseal_object(capargs[i].l);
        }
        else
        {
            memcpy(&args[i], (void*)&capargs[i], sizeof(jvalue));
        }
    }
}

// FIXME: zero-argument methods
#define CALL_METHOD_A(type, native_type)                                       \
SJNI_CALLBACK                                                                  \
native_type sjni_Call##type##MethodA(JNIEnvType ptr,                          \
                                     jobject_c objcap,                         \
                                     jmethodID_c method,                       \
                                     __capability jvalue_c *capargs)           \
{                                                                              \
    jobject obj = unseal_object(objcap);                                       \
    REQUIRE(obj, 0);                                                           \
    jmethodID methodID = unseal_methodID(method);                              \
    REQUIRE(methodID, 0);                                                      \
    pObject ob = REF_TO_OBJ(obj);                                              \
    pMethodBlock mb = lookupVirtualMethod(ob, methodID);                       \
    REQUIRE(mb, 0);                                                            \
    int argc = mb->args_count -1;                                              \
    REQUIRE((argc <= __builtin_memcap_length_get(capargs) / sizeof(jvalue_c)), 0);\
    jvalue args[argc];                                                         \
    unwrapArguments(capargs, args, argc);                                      \
    return *(native_type*)executeMethodList(ob, ob->class, mb, (u8*)args);     \
}

SJNI_CALLBACK
jobject_c sjni_CallObjectMethodA(JNIEnvType ptr,
                                 jobject_c objcap,
                                 jmethodID_c method,
                                 __capability jvalue_c *capargs)
{
    jobject obj = unseal_object(objcap);
    REQUIRE(obj, 0);
    jmethodID methodID = unseal_methodID(method);
    REQUIRE(methodID, 0);
    pObject ob = REF_TO_OBJ(obj);
    pMethodBlock mb = lookupVirtualMethod(ob, methodID);
    REQUIRE(mb, 0);
    int argc = mb->args_count -1;
    REQUIRE((argc <= __builtin_memcap_length_get(capargs) / sizeof(jvalue_c)), 0);
    jvalue args[argc];
    unwrapArguments(capargs, args, argc);
    jobject retobj = *(jobject*)executeMethodList(ob, ob->class, mb, (u8*)args);
    return seal_object(retobj);
}


SJNI_CALLBACK
jobject_c sjni_NewObjectA(JNIEnvType ptr,
                          jclass_c clscap,
                          jmethodID_c method,
                          __capability jvalue_c *capargs)
{
    jobject cls = unseal_object(clscap);
    REQUIRE(cls, 0);
    jmethodID methodID = unseal_methodID(method);
    REQUIRE(methodID, 0);
    pMethodBlock mb = methodID;
    int argc = mb->args_count -1;
    REQUIRE((argc <= __builtin_memcap_length_get(capargs) / sizeof(jvalue_c)), 0);
    jvalue args[argc];
    unwrapArguments(capargs, args, argc);
    jobject obj = env->NewObjectA(&env, cls, methodID, args);
    return seal_object(obj);
}

#define ALL_PRIMITIVE_TYPES(x) \
    x(Boolean, jboolean)  \
    x(Byte, jbyte)        \
    x(Char, jchar)        \
    x(Short, jshort)      \
    x(Int, jint)          \
    x(Long, jlong)        \
    x(Float, jfloat)      \
    x(Double, jdouble)

ALL_PRIMITIVE_TYPES(GET_PRIM_ARRAY_ELEMENTS)
ALL_PRIMITIVE_TYPES(RELEASE_PRIM_ARRAY_ELEMENTS)
ALL_PRIMITIVE_TYPES(FIELD_ACCESSOR)
ALL_PRIMITIVE_TYPES(FIELD_SETTER)
ALL_PRIMITIVE_TYPES(CALL_METHOD_A)

/**
 * Check that a C string passed from native code contains a null terminator
 * within its length.
 */
bool checkString(__capability const char *str)
{
    // Not a valid string if we can't read it!
    if ((__builtin_memcap_perms_get(str) & __CHERI_CAP_PERMISSION_PERMIT_LOAD__) == 0)
    {
        return false;
    }
    size_t len = __builtin_memcap_length_get(str);
    return memchr((const void*)str, 0, len) != NULL;
}

SJNI_CALLBACK jclass_c sjni_GetObjectClass(JNIEnvType ptr, jobject_c objcap)
{
    jobject obj = unseal_object(objcap);
    if (obj == NULL)
    {
        return NULL;
    }
    jclass cls = env->GetObjectClass(&env, obj);
    return seal_class(cls);
}

SJNI_CALLBACK jfieldID_c sjni_GetFieldID(JNIEnvType ptr, jclass_c clazz,
    __capability const char *name, __capability const char *sig)
{
    jclass cls = unseal_class(clazz);
    if ((cls == NULL) ||
        !checkString(name) ||
        !checkString(sig))
    {
        return NULL;
    }
    jfieldID f = env->GetFieldID(&env, cls, (const char*)name,
            (const char*)sig);
    return seal_fieldID(f);
}

SJNI_CALLBACK jmethodID_c sjni_GetMethodID(JNIEnvType ptr, jclass_c clazz,
    __capability const char *name, __capability const char *sig)
{
    jclass cls = unseal_class(clazz);
    if ((cls == NULL) ||
        !checkString(name) ||
        !checkString(sig))
    {
        return NULL;
    }
    jmethodID f = env->GetMethodID(&env, cls, (const char*)name,
            (const char*)sig);
    return seal_methodID(f);
}

SJNI_CALLBACK jobject_c sjni_NewGlobalRef(JNIEnvType ptr, jobject_c obj)
{
    jobject object = unseal_object(obj);
    struct jni_sandbox_object *pool =
       (void*)unseal_jnienv((*ptr)->reserved1);
    REQUIRE(obj, NULL);
    REQUIRE(pool, NULL);
    int refcount = insert_object_reference(pool, obj, false);
    // If this is the *first* global ref to this object from this sandbox, then
    // register with the GC.
    if (refcount == 1)
    {
        env->NewGlobalRef(&env, object);
    }
    return obj;
}

SJNI_CALLBACK void sjni_DeleteGlobalRef(JNIEnvType ptr, jobject_c obj)
{
    jobject object = unseal_object(obj);
    struct jni_sandbox_object *pool =
       (void*)unseal_jnienv((*ptr)->reserved1);
    REQUIRE(obj, );
    REQUIRE(pool, );
    remove_object_reference(pool, obj);
}

SJNI_CALLBACK jobject_c sjni_AllocObject(JNIEnvType ptr, jclass_c clazz)
{
    jclass cls = unseal_class(clazz);
    REQUIRE(cls, 0);
    return seal_object(env->AllocObject(&env, cls));
}

SJNI_CALLBACK jsize sjni_GetArrayLength(JNIEnvType ptr, jarray_c array)
{
    jarray arr = unseal_object(array);
    REQUIRE(arr, 0);
    return env->GetArrayLength(&env, arr);
}

/**
 * Construct the sandbox JNI environment structure.
 */
static __capability struct _JNISandboxedNativeInterface *
createSandboxCallbacks(struct jni_sandbox_object *pool)
{
    struct cheri_object system = sandbox_object_getsystemobject(pool->obj);
    __capability struct _JNISandboxedNativeInterface *callbacks =
        (__capability struct _JNISandboxedNativeInterface *)calloc(1,
                sizeof(struct _JNISandboxedNativeInterface));
    callbacks = __builtin_memcap_bounds_set(callbacks, sizeof(struct
                _JNISandboxedNativeInterface));
#define SET_CALLBACK(x) \
    callbacks->x = __builtin_memcap_callback_create("_cheri_system_object", system, sjni_ ## x)
    SET_CALLBACK(GetVersion);
#define ADD_ARRAY_ACCESSOR(type, ctype) SET_CALLBACK(Get##type##ArrayElements);
#define ADD_ARRAY_RELEASE(type, ctype) SET_CALLBACK(Release##type##ArrayElements);
#define ADD_FIELD_ACCESSOR(type, ctype) SET_CALLBACK(Get##type##Field);
#define ADD_FIELD_SETTER(type, ctype) SET_CALLBACK(Set##type##Field);
#define ADD_CALL_METHOD_A(type, ctype) SET_CALLBACK(Call##type##MethodA);
    ALL_PRIMITIVE_TYPES(ADD_ARRAY_ACCESSOR);
    ALL_PRIMITIVE_TYPES(ADD_ARRAY_RELEASE);
    SET_CALLBACK(GetObjectField);
    SET_CALLBACK(SetObjectField);
    SET_CALLBACK(GetObjectClass);
    SET_CALLBACK(NewGlobalRef);
    SET_CALLBACK(GetFieldID);
    SET_CALLBACK(GetMethodID);
    SET_CALLBACK(GetArrayLength);
    SET_CALLBACK(CallObjectMethodA);
    SET_CALLBACK(NewObjectA);
    ALL_PRIMITIVE_TYPES(ADD_FIELD_ACCESSOR);
    ALL_PRIMITIVE_TYPES(ADD_FIELD_SETTER);
    ALL_PRIMITIVE_TYPES(ADD_CALL_METHOD_A);
    // Remove write permission from the callbacks capability
    // Note: We might want to do this just before passing it in, if we want to
    // use the reserved fields for anything.
    callbacks->reserved0 = seal_jnienv((SJNIEnvPtr)callbacks);
    callbacks->reserved1 = seal_jnienv((void*)pool);
    callbacks = __builtin_memcap_perms_and(callbacks,
            ~__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ &
            ~__CHERI_CAP_PERMISSION_PERMIT_STORE__);
    return callbacks;
}

/**
 * Construct a new sandbox object.
 */
struct jni_sandbox_object *create_sandbox_object(struct jni_sandbox *sandbox)
{
    struct sandbox_class *cls = sandbox->cls;
    struct jni_sandbox_object *pool =
        calloc(sizeof(struct jni_sandbox_object), 1);
    // FIXME: Make size configurable somehow.  For now, 16MB ought to be enough for anyone.
    sandbox_object_new(cls, 1024*1024*16, &pool->obj);
    // FIXME: Error checking
    pool->cheri_obj = sandbox_object_getobject(pool->obj);
    pool->env = createSandboxCallbacks(pool);
    RB_INIT(&pool->unsealed_caps);
    return pool;
}

/**
 * Helper function used with `__attribute__((cleanup))` to free a heap-allocated buffer.
 */
static void freeptr(char **ptr)
{
    free(*ptr);
}

/**
 * Revokes all of the capabilities from a specific range.
 */
static void revoke_from_range(struct jni_sandbox_object *pool,
        __capability void *__capability *heap)
{
    static int page_size;
    if (page_size == 0)
    {
        page_size = getpagesize();
    }
    const size_t heap_base = __builtin_memcap_base_get(heap);
    const size_t size = __builtin_memcap_length_get(heap);
    const size_t heap_end = heap_base+size;
    const size_t pages = size / page_size;
    const size_t caps_per_page = page_size / sizeof(__capability void*);
    char stack_buffer[32];
    __attribute__((cleanup(freeptr)))
    char *heap_buffer = NULL;
    char *buffer;
    if (pages <= 32)
    {
        buffer = stack_buffer;
    }
    else
    {
        heap_buffer = malloc(pages);
        buffer = heap_buffer;
    }
    if (mincore((void*)heap, size, buffer) != 0)
    {
        // FIXME: This will currently break because libcheri doesn't give us
        // read permissions to all of the pages.
        fprintf(stderr, "mincore failed\n");
        memset(buffer, size, MINCORE_MODIFIED);
    }
    for (int p=0 ; p<pages ; p++)
    {
        if ((buffer[p] | (MINCORE_MODIFIED & MINCORE_MODIFIED_OTHER)) == 0)
        {
            continue;
        }
        __capability void *__capability *page = heap + (p * caps_per_page);
        for (int c=0 ; c<caps_per_page ; c++)
        {
            __capability void *cap = page[c];
            // If the tag bit is clear, it's not a capability, so ignore it
            if (!__builtin_memcap_tag_get(cap))
            {
                continue;
            }
            // If it's sealed, then it can be passed back and it might be
            // something that we care about.  If it's not sealed and its length
            // is zero, then it doesn't allow any access, so ignore it.
            if (!__builtin_memcap_sealed_get(cap) && (__builtin_memcap_length_get(cap) == 0))
            {
                continue;
            }
            size_t base = __builtin_memcap_base_get(cap);
            if ((base >= heap_base) && (base <= heap_end))
            {
                continue;
            }
            //fprintf(stderr, "Found escaped cap: ");
            //print_cap(cap);
            if (!__builtin_memcap_sealed_get(cap))
            {
                struct shared_unsealed_cap search;
                search.base = base;
                struct shared_unsealed_cap *original =
                    RB_NFIND(unsealed_cap_tree, &pool->unsealed_caps, &search);
                if (!original)
                {
                    continue;
                }

                if (base < original->base)
                {
                    continue;
                }
                if (base > original->base + original->length)
                {
                    continue;
                }
                if (original->refcount == 0)
                {
                    page[c] = 0;
                }
                else
                {
                    original->marked = true;
                }
                //fprintf(stderr, "Found escaped cap from array: (0x%lx-0x%lx)", (unsigned long)original->base, (unsigned long)original->base + original->length);
                //print_cap(cap);
            }
            else
            {
                // If this is a sealed capability, then ignore it unless it's a
                // sealed object.
                if (!unseal_object(cap))
                {
                    continue;
                }
                struct shared_sealed_cap *original;
                HASH_FIND(hh, pool->sealed_caps, &cap, sizeof(cap), original);
                if (original && original->refcount == 0)
                {
                    page[c] = 0;
                }
            }
        }
    }
}

/**
 * Release all of the objects that were locked into memory because we could not
 * guarantee that they were not revoked.
 */
static void release_sandbox_resources(struct jni_sandbox_object *pool,
        bool force)
{
    {
        struct shared_unsealed_cap *u, *tmp;
        RB_FOREACH_SAFE(u, unsealed_cap_tree, &pool->unsealed_caps, tmp)
        {
            if (force || (u->refcount == 0) || (!u->marked))
            {
                env->DeleteGlobalRef(&env, u->array);
                free(RB_REMOVE(unsealed_cap_tree, &pool->unsealed_caps, u));
            }
        }
    }
    {
        struct shared_sealed_cap *s, *tmp;
        HASH_ITER(hh, pool->sealed_caps, s, tmp)
        {
            if (force || s->refcount == 0)
            {
                jobject obj = unseal_object(s->cap);
                env->DeleteGlobalRef(&env, obj);
                HASH_DEL(pool->sealed_caps, s);
                free(s);
            }
        }
    }
}

/**
 * Revoke all of the capabilities (temporarily) delegated to a sandbox.
 */
static void revoke_caps(struct jni_sandbox_object *pool)
{
    struct sandbox_object *obj = pool->obj;
    __capability void *__capability *heap = sandbox_object_getsandboxdata(obj);
    __capability void *__capability *stack = sandbox_object_getsandboxstack(obj);
    // Clear the mark bits for all of these values.
    struct shared_unsealed_cap *u, *tmp;
    RB_FOREACH_SAFE(u, unsealed_cap_tree, &pool->unsealed_caps, tmp)
    {
        u->marked = false;
    }
    revoke_from_range(pool, heap);
    release_sandbox_resources(pool, false);
}

/**
 * Completely reset a global sandbox.
 */
uintptr_t *resetGlobalSandbox(pClass class, pMethodBlock mb, uintptr_t *ostack)
{
    char *class_name = String2Cstr((pObject)ostack[0]);
    struct jni_sandbox *sandbox;
    pthread_mutex_lock(&sandboxes_lock);
    HASH_FIND_STR(sandboxes, class_name, sandbox);
    pthread_mutex_unlock(&sandboxes_lock);
    if (sandbox == NULL)
    {
        return ostack;
    }
    pthread_mutex_lock(&sandbox->global_lock);
    if (sandbox->global_object != NULL)
    {
        sandbox_object_reset(sandbox->global_object->obj);
        release_sandbox_resources(sandbox->global_object, true);
    }
    pthread_mutex_unlock(&sandbox->global_lock);
    return ostack;
}

/**
 * Revoke all capabilities delegated to a global sandbox, but do not perform a
 * complete reset.
 */
uintptr_t *revokeGlobalSandbox(pClass class, pMethodBlock mb, uintptr_t *ostack)
{
    char *class_name = String2Cstr((pObject)ostack[0]);
    struct jni_sandbox *sandbox;
    pthread_mutex_lock(&sandboxes_lock);
    HASH_FIND_STR(sandboxes, class_name, sandbox);
    pthread_mutex_unlock(&sandboxes_lock);
    if (sandbox == NULL)
    {
        return ostack;
    }
    pthread_mutex_lock(&sandbox->global_lock);
    if (sandbox->global_object != NULL)
    {
        revoke_caps(sandbox->global_object);
    }
    pthread_mutex_unlock(&sandbox->global_lock);
    return ostack;
}

/**
 * Wrapper function that invokes the assembly function that calls a function in
 * a sandbox.
 */
uintptr_t *callJNISandboxWrapper(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    struct jni_sandbox_method *metadata = (struct jni_sandbox_method*)mb->code;

    if(!initJNILrefs())
        return NULL;

    if (!object_type)
    {
        pthread_once(&once_control, initGlobalSandboxState);
    }
    // FIXME: The calling convention currently only permits passing arguments
    // in registers.  Eventually we should permit on-stack arguments.
    __capability void *cap_args[8] = {0};
    int64_t int_args[8] = {1,2,3,4,5,6,7,8};
    int cap_arg_count = 2;
    int int_arg_count = 0;


    struct cheri_object sandbox;
    uintptr_t *ostack_args = ostack;
    cap_args[1] = seal_object((mb->access_flags & ACC_STATIC) ? class : ((void*)*(ostack_args++)));

    struct jni_sandbox_object *pool = NULL;
    assert(metadata->sandbox);
    switch (metadata->scope)
    {
        case SandboxScopeGlobal:
            pthread_mutex_lock(&metadata->sandbox->global_lock);
            // FIXME: Promote from the method pool if one exists.
            if (metadata->sandbox->global_object == NULL)
            {
                metadata->sandbox->global_object =
                    create_sandbox_object(metadata->sandbox);
                metadata->sandbox->global_object->scope = SandboxScopeGlobal;
            }
            pool = metadata->sandbox->global_object;
            break;
        case SandboxScopeObject:
            // FIXME: Implement object-scoped sandboxes.
            assert(0 && "Object-scoped sandboxes are not yet supported");
            break;
        case SandboxScopeMethod:
            if (metadata->sandbox->pool_head)
            {
                pthread_mutex_lock(&metadata->sandbox->pool_lock);
                pool = metadata->sandbox->pool_head;
                if (pool)
                {
                    metadata->sandbox->pool_head = pool->next;
                }
                pthread_mutex_unlock(&metadata->sandbox->pool_lock);
            }
            if (pool)
            {
                sandbox_object_reset(pool->obj);
            }
            else
            {
                pool = create_sandbox_object(metadata->sandbox);
            }
            pool->scope = SandboxScopeMethod;
    }
    assert(pool);
    // FIXME: Check error, throw exception if sandbox creation failed
    char *signature;
    cap_args[0] = __builtin_memcap_bounds_set((__capability void*)&pool->env, sizeof(__capability void*));

    for (signature = mb->type+1 ; *signature != ')' ; signature++)
    {
        switch (*signature)
        {
            case 'F':
            case 'D':
                // Fallthrough on softfloat
                // FIXME: hard-float ABIs
            case 'Z':
            case 'B':
            case 'C':
            case 'S':
            case 'I':
            case 'J':
                int_args[int_arg_count++] = *(ostack_args++);
                break;
            case 'L':
                cap_args[cap_arg_count++] = seal_object((void*)*(ostack_args++));
                do { signature++; } while (*signature != ';');
                break;
            case '[':
                cap_args[cap_arg_count++] = seal_object((void*)*(ostack_args++));
                if (*signature == 'L')
                {
                    do { signature++; } while (*signature != ';');
                }
                break;
        }
        // Fix passing more arguments
        assert(cap_arg_count < 8);
        assert(int_arg_count < 8);
    }
    signature++;
    if (metadata->scope != SandboxScopeMethod)
    {
        for (int i=1 ; i<cap_arg_count ; i++)
        {
            insert_object_reference(pool, cap_args[i], true);
        }
    }

    cherierrno = 0;
    uintptr_t *ret = callSandboxedJNIMethod(pool->cheri_obj, cap_args, cap_arg_count,
            int_args, int_arg_count, ostack, metadata->method_num, *signature);
    switch (metadata->scope)
    {
        case SandboxScopeObject:
        case SandboxScopeGlobal:
            if (cherierrno != 0)
            {
                // If an exception occurred in a global sandbox, assume that it
                // is in an undefined state and reset it.
                sandbox_object_reset(pool->obj);
                release_sandbox_resources(pool, true);
            }
            else
            {
                revoke_caps(pool);
            }
            pthread_mutex_unlock(&metadata->sandbox->global_lock);
            break;
        case SandboxScopeMethod:
            pthread_mutex_lock(&metadata->sandbox->pool_lock);
            pool->next = metadata->sandbox->pool_head;
            metadata->sandbox->pool_head = pool;
            pthread_mutex_unlock(&metadata->sandbox->pool_lock);
            break;
    }
    if (cherierrno != 0) {
        const char *msg = cherierrno == -1 ? "Invalid JNI API use" :
            sys_siglist[cherierrno];
        signalException(java_lang_NullPointerException, (char*)msg);
    }
    return ret;
}

/**
 * Load a sandbox with a given class name.
 */
static bool loadSandbox(const char *class_name, struct sandbox_class **cls)
{
    if (class_name[0] == '/')
    {
        return sandbox_class_new(class_name, -1, cls) == 0;
    }
    char *paths = getenv("LIBCHERI_LIBRARY_PATH");
    bool found = false;
    if (paths)
    {
        paths = strdup(paths);
        char *next = paths;
        char *path;
        while (!found && (path = strsep(&next, ":")))
        {
            char *loc;
            asprintf(&loc, "%s/%s.co", path, class_name);
            if (sandbox_class_new(loc, -1, cls) == 0)
            {
                found = true;
            }
            free(loc);
        }
    }
    return found;
}

/**
 * Look up whether a specified method is implemented in any sandbox that we
 * either have loaded or can load.
 */
void *lookupLoadedSandboxes(pMethodBlock mb) {

    pObject loader = (CLASS_CB(mb->class))->class_loader;
    char *mangled = mangleClassAndMethodName(mb);

    pObject annotations = getMethodAnnotations(mb);
    int annotation_count = ARRAY_LEN(annotations);
    if (annotation_count) {
        char *sandbox_class_name = "sandbox/Sandbox";
        static pClass sandbox_class;
        if (sandbox_class == NULL) {
            sandbox_class = findClassFromClassLoader(sandbox_class_name, loader);
        }
        pObject *annotation_data = ARRAY_DATA(annotations, pObject);
        for (int i=0 ; i<annotation_count ; i++) {
            pObject annot = annotation_data[i];

            pMethodBlock method = findMethod(annot->class, SYMBOL(annotationType), SYMBOL(___java_lang_Class));
            pClass annot_cls = *(pClass*)executeMethod(annot, method);
            if (annot_cls == sandbox_class) {
                struct jni_sandbox_method *metadata = calloc(sizeof(struct jni_sandbox_method), 1);
                pMethodBlock class_method = findMethod(annot->class, SYMBOL(SandboxClass), SYMBOL(___java_lang_String));
                pMethodBlock scope_method = findMethod(annot->class, SYMBOL(scope), SYMBOL(___sandbox_Sandbox_Scope));
                pObject scope_enum = *(pObject*)executeMethod(annot, scope_method);
                pMethodBlock ordinal_method = lookupMethod(scope_enum->class, SYMBOL(ordinal), SYMBOL(___I));
                metadata->scope = *(SandboxScope*)executeMethod(scope_enum, ordinal_method);
                pObject sandbox_class = *(pObject*)executeMethod(annot, class_method);
                char *class_name = String2Cstr(sandbox_class);
                struct jni_sandbox *sandbox;
                pthread_mutex_lock(&sandboxes_lock);
                HASH_FIND_STR(sandboxes, class_name, sandbox);
                if (!sandbox)
                {
                    sandbox = calloc(sizeof(struct jni_sandbox), 1);
                    if (!loadSandbox(class_name, &sandbox->cls))
                    {
                        free(sandbox);
                        return NULL;
                    }
                    sandbox->sandbox_name = class_name;
                    sandbox->pool_lock = PTHREAD_MUTEX_INITIALIZER;
                    HASH_ADD_STR(sandboxes, sandbox_name, sandbox);
                }
                else
                {
                    sysFree(class_name);
                }
                pthread_mutex_unlock(&sandboxes_lock);
                metadata->sandbox = sandbox;
                metadata->method_num = sandbox_class_method_get_number(sandbox->cls, mangled);
                mb->code = (unsigned char *)metadata;
                mb->native_extra_arg = nativeExtraArg(mb);
                return mb->native_invoker = &callJNISandboxWrapper;
            }
        }
    }
    return NULL;
}
