#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <unistd.h>
#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>
#include <machine/regnum.h>
#include <machine/sysarch.h>
#include <cheri/sandbox.h>
#include <cheri/cheric.h>
#include <cheri/cheri_type.h>
#include <cheri/cheri_system.h>
#include <ucontext.h>
#include <cheri/cheri_stack.h>
#include <sys/mman.h>
#include <sys/syscall.h>
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
#include "alloc.h"



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
     * The number of times that this sandbox has been invoked since the last
     * time that it was scanned to revoke capabilities.
     */
    int invokes_since_sweep;
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
 * Structure used to associate a sandbox with an object.
 */
struct jni_per_object_sandbox
{
    /**
     * The Java object that this is associated with.  FIXME: This will
     * currently leak: we should use a zeroing weak reference and collect later.
     */
    pObject java_object;
    /**
     * Weak reference to the Java object.  We use this to check whether the
     * `java_object` field actually points to the right object.  If the weak
     * reference refers to an invalid object then this is stale and can be
     * safely deleted.
     */
    jweak   weak_ref;
    /**
     * The sandbox object associated with this Java object;
     */
    struct jni_sandbox_object *object;
    /**
     * Lock protecting this sandbox during invocation.
     */
    pthread_mutex_t lock;
    /**
     * Hash handle.  Used to map from {cheri class, java object} pairs to sandboxes.
     */
    UT_hash_handle hh;
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
     * Lock used to protect per-object sandboxes.
     */
    pthread_mutex_t per_object_lock;
    /**
     * Hash table of per-object sandboxes.
     */
    struct jni_per_object_sandbox *objects;
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
static struct jni_sandbox *sandboxes __pt_guarded_by(sandboxes_lock);

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
 * Macro for declaring and defining things that happen for all array types.
 */
#define ALL_PRIMITIVE_TYPES(x) \
    x(Boolean, jboolean, "Z")  \
    x(Byte, jbyte, "B")        \
    x(Char, jchar, "C")        \
    x(Short, jshort, "S")      \
    x(Int, jint, "I")          \
    x(Long, jlong, "J")        \
    x(Float, jfloat, "F")      \
    x(Double, jdouble, "D")


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

static void revoke_caps(struct jni_sandbox_object *pool);

/**
 * Thread for collecting stale per-object sandboxes.  This runs in the
 * background and periodically scans through the list of sandboxes, returning
 * sandbox objects to the pool if required.
 */
static void cleanup_per_object(Thread *unused)
{
    const int invokes_per_sweep = 10;
    while (true)
    {
        sleep(1);
        pthread_mutex_lock(&sandboxes_lock);
        struct jni_sandbox *s, *tmp;
        HASH_ITER(hh, sandboxes, s, tmp)
        {
            if (s->objects != NULL)
            {
                // Drop the sandboxes lock while we do something expensive.  This
                // is safe because we don't care if we miss a sandbox while
                // iterating, we'll get it on the next pass
                pthread_mutex_unlock(&sandboxes_lock);
                struct jni_per_object_sandbox *o, *otmp;
                pthread_mutex_lock(&s->per_object_lock);
                HASH_ITER(hh, s->objects, o, otmp)
                {
                    if (o->weak_ref != NULL)
                    {
                        jobject weak_obj = REF_TO_OBJ_WEAK_NULL_CHECK(o->weak_ref);
                        if (weak_obj == NULL)
                        {
                            // Return this object to the pool
                            pthread_mutex_lock(&s->pool_lock);
                            o->object->next = s->pool_head;
                            s->pool_head = o->object;
                            pthread_mutex_unlock(&s->pool_lock);
                            env->DeleteWeakGlobalRef(&env, o->weak_ref);
                            HASH_DEL(s->objects, o);
                            free(o);
                        }
                        else if (o->object->invokes_since_sweep > invokes_per_sweep)
                        {
                            pthread_mutex_lock(&o->lock);
                            revoke_caps(o->object);
                            pthread_mutex_unlock(&o->lock);
                            o->object->invokes_since_sweep = 0;
                        }
                    }
                }
                pthread_mutex_unlock(&s->per_object_lock);
                pthread_mutex_lock(&sandboxes_lock);
            }
            if (s->global_object)
            {
                if (s->global_object->invokes_since_sweep > 10)
                {
                    pthread_mutex_unlock(&sandboxes_lock);
                    pthread_mutex_lock(&s->global_lock);
                    revoke_caps(s->global_object);
                    s->global_object->invokes_since_sweep = 0;
                    pthread_mutex_unlock(&s->global_lock);
                    pthread_mutex_lock(&sandboxes_lock);
                }
            }
        }
        pthread_mutex_unlock(&sandboxes_lock);
    }
}


/**
 * Helper function, pops the return frame from the trusted stack.
 *
 * This walks up the trusted stack until it finds a frame that points back into
 * the JVM, so that we can be sure of unwinding all of the way.
 */
static void pop_trusted_stack(void)
{
    // FIXME: Move this into libcheri (with suitable error return instead of
    // asserts)
    __capability void *pcc = __builtin_cheri_program_counter_get();
    size_t base = __builtin_cheri_base_get(pcc);
    size_t length = __builtin_cheri_length_get(pcc);
    struct cheri_stack cs;
    sysarch(CHERI_GET_STACK, &cs);
    assert(cs.cs_tsp > (2 * CHERI_FRAME_SIZE));
    int frames = cs.cs_tsp / CHERI_FRAME_SIZE;
    for (frames-- ; frames > 0 ; frames--)
    {
        struct cheri_stack_frame *frame = &cs.cs_frames[CHERI_STACK_DEPTH-frames];
        if ((__builtin_cheri_base_get(frame->csf_pcc) == base) &&
            (__builtin_cheri_length_get(frame->csf_pcc) == length))
        {
            break;
        }
    }
    assert(frames > 0);
    cs.cs_tsp = (CHERI_STACK_DEPTH-frames) * CHERI_FRAME_SIZE;
    sysarch(CHERI_SET_STACK, &cs);
}
/**
 * Class for java.nio.Buffer.
 */
static pClass bufferClass;
/**
 * Method block for java.nio.buffer::isReadOnly.
 */
int isReadOnlyMethodIdx;
/**
 * The class that's used to perform the `SecurityManager` checks.
 */
static pClass checks_class;
/**
 * The method for checking the generic syscall permission.
 */
static pMethodBlock syscall_perm_check_method;
/**
 * The method for checking the open() syscall.
 */
static pMethodBlock syscall_open_check_method;
/**
 * The method for checking the readFileDescriptor permission.
 */
static pMethodBlock syscall_readfd_check_method;
/**
 * The method for checking the writeFileDescriptor permission.
 */
static pMethodBlock syscall_writefd_check_method;
/**
 * The method for checking both the readFileDescriptor and writeFileDescriptor permissions.
 */
static pMethodBlock syscall_readwritefd_check_method;
#define DECLARE_ARRAY_CLASS(type, ctype, x) \
    pClass type##ArrayClass;
pClass string_class;
ALL_PRIMITIVE_TYPES(DECLARE_ARRAY_CLASS)

int __sys_open(const char *path, int oflags, int mode);

/**
 * Function to call into the java checker class to check the general syscall
 * permission.
 */
static int syscallPermCheck(int *retp, __capability int *stub_errno)
{
    executeMethod(checks_class, syscall_perm_check_method);
    if (exceptionOccurred())
    {
        //fprintf(stderr, "exception occurred\n");
        cherierrno = -2;
        pop_trusted_stack();
        return -2;
    }
    return 0;
}

/**
 * Function to call into the java checker class to check the open() syscall.
 */
static int syscallOpenCheck(int *retp, __capability int *stub_errno, __capability const char *path, int oflags, int mode)
{
    //printf("syscallOpenCheck...\n");
    jvalue args[2];
    char *path_copy;
    size_t path_len;
        
    if (__builtin_cheri_offset_get(path) > __builtin_cheri_length_get(path)) {
        *retp = -1;
        *stub_errno = ENOMEM;
        return -1;
    }
    path_len = __builtin_cheri_length_get(path) - __builtin_cheri_offset_get(path);
    path_copy = strndup((char *)path, path_len);
    if (path_copy == NULL) {
        *retp = -1;
        *stub_errno = ENOMEM;
        return -1;
    }   
    path_copy[path_len - 1] = '\0';
    
    // Translate oflags to java permission actions (a comma-separated list of
    // actions from the set (read, write, execute, readlink, delete)). All but
    // the delete action are relevant here.
    bool read = oflags & ~(O_WRONLY | O_EXEC);
    bool write = oflags & (O_WRONLY | O_RDWR | O_APPEND | O_CREAT | O_TRUNC);
    bool execute = oflags & O_EXEC;
    bool readlink = oflags & O_NOFOLLOW;
    
    char actions[28]; // maximum-size buffer for the actions string
    int currIdx = 0;
    if (read)
    {
        strcpy(actions, "read");
        currIdx = 4;
    }
    if (write)
    {
        if (currIdx > 0)
        {
            strcpy(actions+currIdx, ",");
            currIdx++;
        }
        strcpy(actions+currIdx, "write");
        currIdx += 5;
    }
    if (execute)
    {
        if (currIdx > 0)
        {
            strcpy(actions+currIdx, ",");
            currIdx++;
        }
        strcpy(actions+currIdx, "execute");
        currIdx += 7;
    }
    if (readlink)
    {
        if (currIdx > 0)
        {
            strcpy(actions+currIdx, ",");
            currIdx++;
        }
        strcpy(actions+currIdx, "readlink");
        currIdx += 8;
    }
    actions[currIdx] = '\0';

    args[0].l = createString(path_copy);
    args[1].l = createString(actions);
    executeMethodList(NULL, checks_class, syscall_open_check_method, (u8*)args);
    if (exceptionOccurred())
    {
        //fprintf(stderr, "exception occurred\n");
        cherierrno = -2;
        pop_trusted_stack();
        return -2;
    }

    errno = *stub_errno;
    *retp = __sys_open(path_copy, oflags, mode);
    *stub_errno = errno;

    free(path_copy);
    return -1;
}

/**
 * Function to call into the java checker class to check the readFileDescriptor
 * permission.
 */
static int syscallReadFDCheck(int *retp, __capability int *stub_errno, int fd, __capability void *buf, size_t nbytes)
{
    if (__builtin_cheri_offset_get(buf) > __builtin_cheri_length_get(buf)) {
        *retp = -1;
        return -1;
    }
    else {
        int buf_len = __builtin_cheri_length_get(buf) - __builtin_cheri_offset_get(buf);
        if (nbytes > buf_len)
        {
            *retp = -1;
            return -1;
        }
        if ((__builtin_cheri_perms_get(buf) & __CHERI_CAP_PERMISSION_PERMIT_STORE__) == 0)
        {
            *retp = -1;
            return -1;
        }
    }
    executeMethod(checks_class, syscall_readfd_check_method);
    if (exceptionOccurred())
    {
        //fprintf(stderr, "exception occurred\n");
        cherierrno = -2;
        pop_trusted_stack();
        return -2;
    }
    return 0;
}

/**
 * Function to call into the java checker class to check the writeFileDescriptor
 * permission.
 */
static int syscallWriteFDCheck(int *retp, __capability int *stub_errno, int fd, __capability const void *buf, size_t nbytes)
{
    if (__builtin_cheri_offset_get(buf) > __builtin_cheri_length_get(buf)) {
        *retp = -1;
        return -1;
    }
    else {
        int buf_len = __builtin_cheri_length_get(buf) - __builtin_cheri_offset_get(buf);
        if (nbytes > buf_len)
        {
            *retp = -1;
            return -1;
        }
        if ((__builtin_cheri_perms_get(buf) & __CHERI_CAP_PERMISSION_PERMIT_LOAD__) == 0)
        {
            *retp = -1;
            return -1;
        }
    }
    executeMethod(checks_class, syscall_writefd_check_method);
    if (exceptionOccurred())
    {
        //fprintf(stderr, "exception occurred\n");
        cherierrno = -2;
        pop_trusted_stack();
        return -2;
    }
    return 0;
}

/**
 * Function to call into the java checker class to check for both the readFileDescriptor
 * and writeFileDescriptor permissions.
 */
static int syscallReadWriteFDCheck(int *retp, __capability int *stub_errno)
{
    executeMethod(checks_class, syscall_readwritefd_check_method);
    if (exceptionOccurred())
    {
        //fprintf(stderr, "exception occurred\n");
        cherierrno = -2;
        pop_trusted_stack();
        return -2;
    }
    return 0;
}

/**
 * Function called by `pthread_once` to set up global state.
 */
static void initGlobalSandboxState(void)
{
#define INIT_SEALING_TYPE(name, ignored) \
    name##_type = cheri_type_alloc();
SEALING_TYPES(INIT_SEALING_TYPE)
    createVMThread("sandbox cleanup thread", cleanup_per_object);
    bufferClass = findClassFromClassLoader("java/nio/Buffer", getSystemClassLoader());
    assert(bufferClass);
    isReadOnlyMethodIdx = lookupMethod(bufferClass, SYMBOL(isReadOnly), SYMBOL(___Z))->method_table_index;
    pClass tmp;
#define GET_ARRAY_CLASS(type, ctype, x) \
    tmp = findArrayClass("[" x); \
    registerStaticClassRefLocked(&type##ArrayClass, tmp); \
    assert(type##ArrayClass);
ALL_PRIMITIVE_TYPES(GET_ARRAY_CLASS)
    string_class = findClassFromClassLoader("java/lang/String", getSystemClassLoader());
    checks_class = findClassFromClassLoader("java/lang/VMSandboxedNative", getSystemClassLoader());
    assert(checks_class);
    syscall_perm_check_method = findMethod(checks_class, SYMBOL(checkSyscallPerm), SYMBOL(___V));
    syscall_open_check_method = findMethod(checks_class, SYMBOL(checkSyscallOpen), SYMBOL(_java_lang_String_java_lang_String__V));
    syscall_readfd_check_method = findMethod(checks_class, SYMBOL(checkSyscallReadFD), SYMBOL(___V));
    syscall_writefd_check_method = findMethod(checks_class, SYMBOL(checkSyscallWriteFD), SYMBOL(___V));
    syscall_readwritefd_check_method = findMethod(checks_class, SYMBOL(checkSyscallReadWriteFD), SYMBOL(___V));
    assert(syscall_perm_check_method);
    assert(syscall_open_check_method);
    assert(syscall_readfd_check_method);
    assert(syscall_writefd_check_method);
    assert(syscall_readwritefd_check_method);
    // Set the system call checking functions.
    syscall_checks[SYS_getpid] = syscallPermCheck;
    syscall_checks[SYS_open] = (syscall_check_t)syscallOpenCheck;
    syscall_checks[SYS_read] = (syscall_check_t)syscallReadFDCheck;
    syscall_checks[SYS_write] = (syscall_check_t)syscallWriteFDCheck;
    syscall_checks[SYS_kqueue] = syscallReadWriteFDCheck;
	// Do for remaining Capsicum-safe syscalls
	syscall_checks[SYS___acl_aclcheck_fd] = syscallPermCheck;
	syscall_checks[SYS___acl_delete_fd] = syscallPermCheck;
	syscall_checks[SYS___acl_get_fd] = syscallPermCheck;
	syscall_checks[SYS___acl_set_fd] = syscallPermCheck;
	syscall_checks[SYS___mac_get_fd] = syscallPermCheck;
	syscall_checks[SYS___mac_get_proc] = syscallPermCheck;
	syscall_checks[SYS___mac_set_fd] = syscallPermCheck;
	syscall_checks[SYS___mac_set_proc] = syscallPermCheck;
	syscall_checks[SYS___sysctl] = syscallPermCheck;
	syscall_checks[SYS__umtx_op] = syscallPermCheck;
	syscall_checks[SYS_abort2] = syscallPermCheck;
	syscall_checks[SYS_accept] = syscallPermCheck;
	syscall_checks[SYS_accept4] = syscallPermCheck;
	syscall_checks[SYS_aio_cancel] = syscallPermCheck;
	syscall_checks[SYS_aio_error] = syscallPermCheck;
	syscall_checks[SYS_aio_fsync] = syscallPermCheck;
	syscall_checks[SYS_aio_read] = syscallPermCheck;
	syscall_checks[SYS_aio_return] = syscallPermCheck;
	syscall_checks[SYS_aio_suspend] = syscallPermCheck;
	syscall_checks[SYS_aio_waitcomplete] = syscallPermCheck;
	syscall_checks[SYS_aio_write] = syscallPermCheck;
	syscall_checks[SYS_bindat] = syscallPermCheck;
	syscall_checks[SYS_cap_enter] = syscallPermCheck;
	syscall_checks[SYS_cap_fcntls_get] = syscallPermCheck;
	syscall_checks[SYS_cap_fcntls_limit] = syscallPermCheck;
	syscall_checks[SYS_cap_getmode] = syscallPermCheck;
	syscall_checks[SYS_cap_ioctls_get] = syscallPermCheck;
	syscall_checks[SYS_cap_ioctls_limit] = syscallPermCheck;
	syscall_checks[SYS___cap_rights_get] = syscallPermCheck;
	syscall_checks[SYS_cap_rights_limit] = syscallPermCheck;
	syscall_checks[SYS_clock_getres] = syscallPermCheck;
	syscall_checks[SYS_clock_gettime] = syscallPermCheck;
	syscall_checks[SYS_close] = syscallPermCheck;
	syscall_checks[SYS_closefrom] = syscallPermCheck;
	syscall_checks[SYS_connectat] = syscallPermCheck;
	syscall_checks[SYS_dup] = syscallPermCheck;
	syscall_checks[SYS_dup2] = syscallPermCheck;
	syscall_checks[SYS_extattr_delete_fd] = syscallPermCheck;
	syscall_checks[SYS_extattr_get_fd] = syscallPermCheck;
	syscall_checks[SYS_extattr_list_fd] = syscallPermCheck;
	syscall_checks[SYS_extattr_set_fd] = syscallPermCheck;
	syscall_checks[SYS_fchflags] = syscallPermCheck;
	syscall_checks[SYS_fchmod] = syscallPermCheck;
	syscall_checks[SYS_fchown] = syscallPermCheck;
	syscall_checks[SYS_fcntl] = syscallPermCheck;
	syscall_checks[SYS_fexecve] = syscallPermCheck;
	syscall_checks[SYS_flock] = syscallPermCheck;
	syscall_checks[SYS_fork] = syscallPermCheck;
	syscall_checks[SYS_fpathconf] = syscallPermCheck;
	syscall_checks[SYS_fstat] = syscallPermCheck;
	syscall_checks[SYS_fstatfs] = syscallPermCheck;
	syscall_checks[SYS_fsync] = syscallPermCheck;
	syscall_checks[SYS_ftruncate] = syscallPermCheck;
	syscall_checks[SYS_futimens] = syscallPermCheck;
	syscall_checks[SYS_futimes] = syscallPermCheck;
	syscall_checks[SYS_getaudit] = syscallPermCheck;
	syscall_checks[SYS_getaudit_addr] = syscallPermCheck;
	syscall_checks[SYS_getauid] = syscallPermCheck;
	syscall_checks[SYS_getcontext] = syscallPermCheck;
	syscall_checks[SYS_getdents] = syscallPermCheck;
	syscall_checks[SYS_getdirentries] = syscallPermCheck;
	syscall_checks[SYS_getegid] = syscallPermCheck;
	syscall_checks[SYS_geteuid] = syscallPermCheck;
	syscall_checks[SYS_getitimer] = syscallPermCheck;
	syscall_checks[SYS_getgid] = syscallPermCheck;
	syscall_checks[SYS_getgroups] = syscallPermCheck;
	syscall_checks[SYS_getlogin] = syscallPermCheck;
	syscall_checks[SYS_getpeername] = syscallPermCheck;
	syscall_checks[SYS_getpgid] = syscallPermCheck;
	syscall_checks[SYS_getpgrp] = syscallPermCheck;
	syscall_checks[SYS_getppid] = syscallPermCheck;
	syscall_checks[SYS_getpriority] = syscallPermCheck;
	syscall_checks[SYS_getresgid] = syscallPermCheck;
	syscall_checks[SYS_getresuid] = syscallPermCheck;
	syscall_checks[SYS_getrlimit] = syscallPermCheck;
	syscall_checks[SYS_getrusage] = syscallPermCheck;
	syscall_checks[SYS_getsid] = syscallPermCheck;
	syscall_checks[SYS_getsockname] = syscallPermCheck;
	syscall_checks[SYS_getsockopt] = syscallPermCheck;
	syscall_checks[SYS_gettimeofday] = syscallPermCheck;
	syscall_checks[SYS_getuid] = syscallPermCheck;
	syscall_checks[SYS_issetugid] = syscallPermCheck;
	syscall_checks[SYS_kevent] = syscallPermCheck;
	syscall_checks[SYS_kill] = syscallPermCheck;
	syscall_checks[SYS_kmq_notify] = syscallPermCheck;
	syscall_checks[SYS_kmq_setattr] = syscallPermCheck;
	syscall_checks[SYS_kmq_timedreceive] = syscallPermCheck;
	syscall_checks[SYS_kmq_timedsend] = syscallPermCheck;
	syscall_checks[SYS_ktimer_create] = syscallPermCheck;
	syscall_checks[SYS_ktimer_delete] = syscallPermCheck;
	syscall_checks[SYS_ktimer_getoverrun] = syscallPermCheck;
	syscall_checks[SYS_ktimer_gettime] = syscallPermCheck;
	syscall_checks[SYS_ktimer_settime] = syscallPermCheck;
	syscall_checks[SYS_lio_listio] = syscallPermCheck;
	syscall_checks[SYS_listen] = syscallPermCheck;
	syscall_checks[SYS_lseek] = syscallPermCheck;
	syscall_checks[SYS_madvise] = syscallPermCheck;
	syscall_checks[SYS_mincore] = syscallPermCheck;
	syscall_checks[SYS_minherit] = syscallPermCheck;
	syscall_checks[SYS_mlock] = syscallPermCheck;
	syscall_checks[SYS_mlockall] = syscallPermCheck;
	syscall_checks[SYS_mmap] = syscallPermCheck;
	syscall_checks[SYS_mprotect] = syscallPermCheck;
	syscall_checks[SYS_msync] = syscallPermCheck;
	syscall_checks[SYS_munlock] = syscallPermCheck;
	syscall_checks[SYS_munlockall] = syscallPermCheck;
	syscall_checks[SYS_munmap] = syscallPermCheck;
	syscall_checks[SYS_nanosleep] = syscallPermCheck;
	syscall_checks[SYS_ntp_gettime] = syscallPermCheck;
	syscall_checks[SYS_chflagsat] = syscallPermCheck;
	syscall_checks[SYS_faccessat] = syscallPermCheck;
	syscall_checks[SYS_fchmodat] = syscallPermCheck;
	syscall_checks[SYS_fchownat] = syscallPermCheck;
	syscall_checks[SYS_fstatat] = syscallPermCheck;
	syscall_checks[SYS_futimesat] = syscallPermCheck;
	syscall_checks[SYS_linkat] = syscallPermCheck;
	syscall_checks[SYS_mkdirat] = syscallPermCheck;
	syscall_checks[SYS_mkfifoat] = syscallPermCheck;
	syscall_checks[SYS_mknodat] = syscallPermCheck;
	syscall_checks[SYS_openat] = syscallPermCheck;
	syscall_checks[SYS_readlinkat] = syscallPermCheck;
	syscall_checks[SYS_renameat] = syscallPermCheck;
	syscall_checks[SYS_symlinkat] = syscallPermCheck;
	syscall_checks[SYS_unlinkat] = syscallPermCheck;
	syscall_checks[SYS_utimensat] = syscallPermCheck;
	syscall_checks[SYS_pdfork] = syscallPermCheck;
	syscall_checks[SYS_pdgetpid] = syscallPermCheck;
	syscall_checks[SYS_pdkill] = syscallPermCheck;
	syscall_checks[SYS_pipe2] = syscallPermCheck;
	syscall_checks[SYS_poll] = syscallPermCheck;
	syscall_checks[SYS_pread] = syscallPermCheck;
	syscall_checks[SYS_preadv] = syscallPermCheck;
	syscall_checks[SYS_profil] = syscallPermCheck;
	syscall_checks[SYS_pwrite] = syscallPermCheck;
	syscall_checks[SYS_pwritev] = syscallPermCheck;
	syscall_checks[SYS_readv] = syscallPermCheck;
	syscall_checks[SYS_recvfrom] = syscallPermCheck;
	syscall_checks[SYS_recvmsg] = syscallPermCheck;
	syscall_checks[SYS_rtprio] = syscallPermCheck;
	syscall_checks[SYS_rtprio_thread] = syscallPermCheck;
	syscall_checks[SYS_sbrk] = syscallPermCheck;
	syscall_checks[SYS_sched_get_priority_max] = syscallPermCheck;
	syscall_checks[SYS_sched_get_priority_min] = syscallPermCheck;
	syscall_checks[SYS_sched_getparam] = syscallPermCheck;
	syscall_checks[SYS_sched_getscheduler] = syscallPermCheck;
	syscall_checks[SYS_sched_rr_get_interval] = syscallPermCheck;
	syscall_checks[SYS_sched_setparam] = syscallPermCheck;
	syscall_checks[SYS_sched_setscheduler] = syscallPermCheck;
	syscall_checks[SYS_sched_yield] = syscallPermCheck;
	syscall_checks[SYS_sctp_generic_recvmsg] = syscallPermCheck;
	syscall_checks[SYS_sctp_generic_sendmsg] = syscallPermCheck;
	syscall_checks[SYS_sctp_generic_sendmsg_iov] = syscallPermCheck;
	syscall_checks[SYS_sctp_peeloff] = syscallPermCheck;
	syscall_checks[SYS_pselect] = syscallPermCheck;
	syscall_checks[SYS_select] = syscallPermCheck;
	syscall_checks[SYS_sendfile] = syscallPermCheck;
	syscall_checks[SYS_sendmsg] = syscallPermCheck;
	syscall_checks[SYS_sendto] = syscallPermCheck;
	syscall_checks[SYS_setaudit] = syscallPermCheck;
	syscall_checks[SYS_setaudit_addr] = syscallPermCheck;
	syscall_checks[SYS_setauid] = syscallPermCheck;
	syscall_checks[SYS_setcontext] = syscallPermCheck;
	syscall_checks[SYS_setegid] = syscallPermCheck;
	syscall_checks[SYS_seteuid] = syscallPermCheck;
	syscall_checks[SYS_setgid] = syscallPermCheck;
	syscall_checks[SYS_setitimer] = syscallPermCheck;
	syscall_checks[SYS_setpriority] = syscallPermCheck;
	syscall_checks[SYS_setregid] = syscallPermCheck;
	syscall_checks[SYS_setresgid] = syscallPermCheck;
	syscall_checks[SYS_setresuid] = syscallPermCheck;
	syscall_checks[SYS_setreuid] = syscallPermCheck;
	syscall_checks[SYS_setrlimit] = syscallPermCheck;
	syscall_checks[SYS_setsid] = syscallPermCheck;
	syscall_checks[SYS_setsockopt] = syscallPermCheck;
	syscall_checks[SYS_setuid] = syscallPermCheck;
	syscall_checks[SYS_shm_open] = syscallPermCheck;
	syscall_checks[SYS_shutdown] = syscallPermCheck;
	syscall_checks[SYS_sigaction] = syscallPermCheck;
	syscall_checks[SYS_sigaltstack] = syscallPermCheck;
	syscall_checks[SYS_sigpending] = syscallPermCheck;
	syscall_checks[SYS_sigprocmask] = syscallPermCheck;
	syscall_checks[SYS_sigqueue] = syscallPermCheck;
	syscall_checks[SYS_sigreturn] = syscallPermCheck;
	syscall_checks[SYS_sigsuspend] = syscallPermCheck;
	syscall_checks[SYS_sigtimedwait] = syscallPermCheck;
	syscall_checks[SYS_sigwaitinfo] = syscallPermCheck;
	syscall_checks[SYS_sigwait] = syscallPermCheck;
	syscall_checks[SYS_socket] = syscallPermCheck;
	syscall_checks[SYS_socketpair] = syscallPermCheck;
	syscall_checks[SYS_sstk] = syscallPermCheck;
	syscall_checks[SYS_sync] = syscallPermCheck;
	syscall_checks[SYS_sysarch] = syscallPermCheck;
	syscall_checks[SYS_thr_create] = syscallPermCheck;
	syscall_checks[SYS_thr_exit] = syscallPermCheck;
	syscall_checks[SYS_thr_kill] = syscallPermCheck;
	syscall_checks[SYS_thr_new] = syscallPermCheck;
	syscall_checks[SYS_thr_self] = syscallPermCheck;
	syscall_checks[SYS_thr_set_name] = syscallPermCheck;
	syscall_checks[SYS_thr_suspend] = syscallPermCheck;
	syscall_checks[SYS_thr_wake] = syscallPermCheck;
	syscall_checks[SYS_umask] = syscallPermCheck;
	syscall_checks[SYS_utrace] = syscallPermCheck;
	syscall_checks[SYS_uuidgen] = syscallPermCheck;
	syscall_checks[SYS_writev] = syscallPermCheck;
	syscall_checks[SYS_yield] = syscallPermCheck;

}

/**
 * Debugging function to dump a capability.
 */
static void print_cap(__capability void *c)
{
    fprintf(stderr, "b:0x%llx l:0x%llx o:0x%llx p:0x%d t:%d s:%d\n",
        (unsigned long long)__builtin_cheri_base_get(c),
        (unsigned long long)__builtin_cheri_length_get(c),
        (unsigned long long)__builtin_cheri_offset_get(c),
        (int)__builtin_cheri_perms_get(c),
        (int)__builtin_cheri_type_get(c),
        (int)__builtin_cheri_sealed_get(c));
}

/**
 * Construct a capability from an integer pointer and seal it with the provided
 * type.
 */
static __capability void *seal(void* obj, __capability void *type)
{
    if (obj == NULL)
    {
        return 0;
    }
    return __builtin_cheri_seal((__capability void*)obj, type);
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
    if (!__builtin_cheri_sealed_get(obj) ||
        (__builtin_cheri_type_get(obj) != __builtin_cheri_base_get(type) +
         __builtin_cheri_offset_get(type)))
    {
        return NULL;
    }
    return (void*)__builtin_cheri_unseal(obj, type);
}

/**
 * Unwind the trusted stack by two frames.  THis function is called when one of
 * the JNI functions encounters an error.
 */
__attribute__((cold))
static void jni_error(void)
{
    cherierrno = -1;
    pop_trusted_stack();
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
 * Helper function that returns true if `obj` is something that can be cast to
 * an instance of `cls`, false otherwise.
 */
static inline bool isKindOfClass(pObject obj, pClass cls)
{
    assert(obj);
    assert(obj->class);
    assert(cls);
    for (pClass objCls = obj->class ; objCls != NULL ; objCls = CLASS_CB(objCls)->super)
    {
        if (objCls == cls)
        {
            return true;
        }
    }
    return false;
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

#define GET_PRIM_ARRAY_ELEMENTS(type, native_type, x)                        \
SJNI_CALLBACK                                                                \
__capability native_type *sjni_Get##type##ArrayElements(JNIEnvType ptr,      \
                                          __capability void *array_cap,      \
                                          __capability jboolean *isCopy) {   \
    void *array_ref = unseal_object(array_cap);                              \
    REQUIRE(array_ref, NULL);                                                \
    REQUIRE(isKindOfClass(array_ref, type##ArrayClass), NULL);               \
    jboolean isCopyLocal;                                                    \
    __capability native_type *buffer = (__capability native_type *)          \
        env->Get##type##ArrayElements(&env,                                  \
            (native_type##Array)array_ref, &isCopyLocal);                    \
    if(isCopy != NULL)                                                       \
        *isCopy = isCopyLocal;                                               \
    size_t length = ARRAY_LEN(REF_TO_OBJ(array_ref)) * sizeof(native_type);  \
    insert_unsealed_cap(ptr, array_ref, (void*)buffer, length);              \
    buffer = __builtin_cheri_bounds_set(buffer, length);                    \
    buffer = __builtin_cheri_perms_and(buffer,                              \
            ~__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ &              \
            ~__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__);               \
    return buffer;                                                           \
}

#define RELEASE_PRIM_ARRAY_ELEMENTS(type, native_type, x)                    \
SJNI_CALLBACK                                                                \
void sjni_Release##type##ArrayElements(JNIEnvType ptr, native_type##Array_c  \
        array_cap, __capability native_type *elems, jint mode) {             \
    void *array_ref = unseal_object(array_cap);                              \
    REQUIRE(array_ref, );                                                    \
    REQUIRE(isKindOfClass(array_ref, type##ArrayClass), );                   \
    struct shared_unsealed_cap search;                                       \
    struct jni_sandbox_object *pool =                                        \
       (void*)unseal_jnienv((*ptr)->reserved1);                              \
    size_t base = __builtin_cheri_base_get(elems);                          \
    search.base = base;                                                      \
    if (pool->scope != SandboxScopeMethod)                                   \
    {                                                                        \
        struct shared_unsealed_cap *original =                               \
            RB_NFIND(unsealed_cap_tree, &pool->unsealed_caps, &search);      \
        if (base <= original->base + original->length)                       \
        {                                                                    \
           original->refcount--;                                             \
        }                                                                    \
    }                                                                        \
    env->Release##type##ArrayElements(&env, array_ref, (native_type*)elems,  \
            mode);                                                           \
}

SJNI_CALLBACK  jobject_c sjni_GetObjectField(JNIEnvType ptr, jobject_c obj, jfieldID_c fieldID)
{
    jobject object = unseal_object(obj);
    REQUIRE(object, NULL);
    pFieldBlock f = unseal_fieldID(fieldID);
    REQUIRE(f, NULL);
    jobject field = env->GetObjectField(&env, object, f);
    REQUIRE(field, NULL);
    jobject_c fieldObject = seal_object(field);
    REQUIRE(isKindOfClass(object, f->class), 0);
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
    pFieldBlock f = unseal_fieldID(fieldID);
    REQUIRE(f, );
    jobject value = unseal_object(val);
    REQUIRE(object, );
    REQUIRE(isKindOfClass(object, f->class), );
    env->SetObjectField(&env, object, f, value);
}

#define FIELD_ACCESSOR(type, native_type, x)                                   \
SJNI_CALLBACK  native_type sjni_Get##type##Field(JNIEnvType ptr,               \
        jobject_c obj, jfieldID_c fieldID)                                     \
{                                                                              \
    jobject object = unseal_object(obj);                                       \
    REQUIRE(object, 0);                                                        \
    pFieldBlock f = unseal_fieldID(fieldID);                                   \
    REQUIRE(f, 0);                                                             \
    REQUIRE(isKindOfClass(object, f->class), 0);                               \
    return env->Get##type##Field(&env, unseal_object(obj), f);                 \
}

#define FIELD_SETTER(type, native_type, x)                                     \
SJNI_CALLBACK  void  sjni_Set##type##Field(JNIEnvType ptr,                     \
        jobject_c obj, jfieldID_c fieldID, native_type val)                    \
{                                                                              \
    jobject object = unseal_object(obj);                                       \
    REQUIRE(object, );                                                         \
    pFieldBlock f = unseal_fieldID(fieldID);                                   \
    REQUIRE(f, );                                                              \
    REQUIRE(isKindOfClass(object, f->class), );                                \
    env->Set##type##Field(&env, unseal_object(obj), f, val);                   \
}

SJNI_CALLBACK
__capability const char *sjni_GetStringUTFChars(JNIEnvType ptr, __capability void *string, __capability jboolean *isCopy) {
    //printf("GetStringUTFChars()...");
    jobject string_ref = unseal_object(string);
    REQUIRE(string_ref, NULL);
    REQUIRE(isKindOfClass(string_ref, string_class), NULL);
    //printf("calling env->GetStringUTFChars()...");
    jboolean isCopyLocal;
    __capability char *buffer = (__capability char *)
        env->GetStringUTFChars(&env, string_ref, &isCopyLocal);
    if (isCopy != NULL)
        *isCopy = isCopyLocal;
    //printf("returned, ");
    size_t length = env->GetStringUTFLength(&env, string_ref);
    uint64_t base = (uint64_t)buffer;
    //printf("string: %s, length: %zu, setting permissions...", (char*)base, length);
    insert_unsealed_cap(ptr, string_ref, (void*)buffer, length);
    buffer = __builtin_cheri_bounds_set(buffer, length + 1);
    buffer = __builtin_cheri_perms_and(buffer, __CHERI_CAP_PERMISSION_PERMIT_LOAD__ |
                                                __CHERI_CAP_PERMISSION_GLOBAL__);
    //printf("done, returning...\n");
    return buffer;
}

SJNI_CALLBACK
void sjni_ReleaseStringUTFChars(JNIEnvType ptr, __capability void *string, __capability const char *native_string) {
    jobject string_ref = unseal_object(string);
    REQUIRE(string_ref, );
    REQUIRE(isKindOfClass(string_ref, string_class), );
    struct shared_unsealed_cap search;
    struct jni_sandbox_object *pool = 
       (void*)unseal_jnienv((*ptr)->reserved1);
    if (pool->scope != SandboxScopeMethod)
    {
        size_t base = __builtin_cheri_base_get(native_string);
        search.base = base;
        struct shared_unsealed_cap *original =
            RB_NFIND(unsealed_cap_tree, &pool->unsealed_caps, &search);
        if (base <= original->base + original->length)
        {
           original->refcount--;
        }
    }
    env->ReleaseStringUTFChars(&env, string_ref, (const char *)native_string);
}

/**
 * Helper function for native-to-Java calls.  Unwraps all of the arguments
 * provided in an array.
 */
// FIXME: This function triggers a bug in GVN, which elides the store of the
// non-capability value.
__attribute__((optnone))
static void unwrapArguments(__capability jvalue_c *capargs, jvalue *args, int argc)
{
    for (int i=0 ; i<argc; i++)
    {
        if (__builtin_cheri_tag_get(capargs[i].l))
        {
            args[i].l = unseal_object(capargs[i].l);
        }
        else
        {
            args[i].j = capargs[i].j;
        }
    }
}

// FIXME: zero-argument methods
#define CALL_METHOD_A(type, native_type, x)                                    \
SJNI_CALLBACK                                                                  \
native_type sjni_Call##type##MethodA(JNIEnvType ptr,                           \
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
    REQUIRE((argc <= __builtin_cheri_length_get(capargs) / sizeof(jvalue_c)), 0);\
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
    REQUIRE((argc <= __builtin_cheri_length_get(capargs) / sizeof(jvalue_c)), 0);
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
    REQUIRE((argc <= __builtin_cheri_length_get(capargs) / sizeof(jvalue_c)), 0);
    jvalue args[argc];
    unwrapArguments(capargs, args, argc);
    jobject obj = env->NewObjectA(&env, cls, methodID, args);
    return seal_object(obj);
}

ALL_PRIMITIVE_TYPES(GET_PRIM_ARRAY_ELEMENTS)
ALL_PRIMITIVE_TYPES(RELEASE_PRIM_ARRAY_ELEMENTS)
ALL_PRIMITIVE_TYPES(FIELD_ACCESSOR)
ALL_PRIMITIVE_TYPES(FIELD_SETTER)
ALL_PRIMITIVE_TYPES(CALL_METHOD_A)

SJNI_CALLBACK __capability void*
sjni_GetDirectBufferAddress(JNIEnvType ptr, jobject_c buf)
{
    jobject buffer = unseal_object(buf);
    REQUIRE(buffer, NULL);
    REQUIRE(isKindOfClass(buffer, bufferClass), NULL);
    size_t length = env->GetDirectBufferCapacity(&env, buffer);
    void *raw_buffer = env->GetDirectBufferAddress(&env, buffer);
    if (raw_buffer)
    {
        __capability void *cap = (__capability void *)raw_buffer;
        // Note that this will add a global reference to the object, preventing
        // it from being collected until the last reference to the buffer has
        // gone away.
        insert_unsealed_cap(ptr, buffer, raw_buffer, length);
        int perms = ~__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ &
            ~__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__;
        // If this is a read-only buffer, don't allow write access
        pObject obj = REF_TO_OBJ(buffer);
        pMethodBlock mb = CLASS_CB(obj->class)->method_table[isReadOnlyMethodIdx];
        if (*(jboolean*)executeMethod(obj, mb))
        {
            perms &= ~__CHERI_CAP_PERMISSION_PERMIT_STORE__;
        }
        cap = __builtin_cheri_bounds_set(cap, length);
        cap = __builtin_cheri_perms_and(cap, perms);
        return cap;
    }
    return NULL;
}

SJNI_CALLBACK jlong sjni_GetDirectBufferCapacity(JNIEnvType ptr, jobject_c buf)
{
    jobject buffer = unseal_object(buf);
    REQUIRE(buffer, -1);
    REQUIRE(isKindOfClass(buffer, bufferClass), -1);
    return env->GetDirectBufferCapacity(&env, buffer);
}

/**
 * Check that a C string passed from native code contains a null terminator
 * within its length.
 */
bool checkString(__capability const char *str)
{
    // Not a valid string if we can't read it!
    if ((__builtin_cheri_perms_get(str) & __CHERI_CAP_PERMISSION_PERMIT_LOAD__) == 0)
    {
        return false;
    }
    size_t len = __builtin_cheri_length_get(str);
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
    callbacks = __builtin_cheri_bounds_set(callbacks, sizeof(struct
                _JNISandboxedNativeInterface));
#define SET_CALLBACK(x) \
    callbacks->x = __builtin_cheri_callback_create("_cheri_system_object", system, sjni_ ## x)
    SET_CALLBACK(GetVersion);
#define ADD_ARRAY_ACCESSOR(type, ctype, x) SET_CALLBACK(Get##type##ArrayElements);
#define ADD_ARRAY_RELEASE(type, ctype, x) SET_CALLBACK(Release##type##ArrayElements);
#define ADD_FIELD_ACCESSOR(type, ctype, x) SET_CALLBACK(Get##type##Field);
#define ADD_FIELD_SETTER(type, ctype, x) SET_CALLBACK(Set##type##Field);
#define ADD_CALL_METHOD_A(type, ctype, x) SET_CALLBACK(Call##type##MethodA);
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
    SET_CALLBACK(GetDirectBufferAddress);
    SET_CALLBACK(GetDirectBufferCapacity);
    SET_CALLBACK(GetStringUTFChars);
    SET_CALLBACK(ReleaseStringUTFChars);
    ALL_PRIMITIVE_TYPES(ADD_FIELD_ACCESSOR);
    ALL_PRIMITIVE_TYPES(ADD_FIELD_SETTER);
    ALL_PRIMITIVE_TYPES(ADD_CALL_METHOD_A);
    // Remove write permission from the callbacks capability
    // Note: We might want to do this just before passing it in, if we want to
    // use the reserved fields for anything.
    callbacks->reserved0 = seal_jnienv((SJNIEnvPtr)callbacks);
    callbacks->reserved1 = seal_jnienv((void*)pool);
    callbacks = __builtin_cheri_perms_and(callbacks,
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

struct jni_sandbox_object *get_or_create_sandbox_object(struct jni_sandbox *sandbox)
{
    pthread_mutex_lock(&sandbox->pool_lock);
    struct jni_sandbox_object *pool = sandbox->pool_head;
    if (pool)
    {
        sandbox->pool_head = pool->next;
    }
    pthread_mutex_unlock(&sandbox->pool_lock);
    if (pool)
    {
        return pool;
    }
    return create_sandbox_object(sandbox);
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
    const size_t heap_base = __builtin_cheri_base_get(heap);
    const size_t size = __builtin_cheri_length_get(heap);
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
        memset(buffer, 0, size);
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
            if (!__builtin_cheri_tag_get(cap))
            {
                continue;
            }
            // If it's sealed, then it can be passed back and it might be
            // something that we care about.  If it's not sealed and its length
            // is zero, then it doesn't allow any access, so ignore it.
            if (!__builtin_cheri_sealed_get(cap) && (__builtin_cheri_length_get(cap) == 0))
            {
                continue;
            }
            size_t base = __builtin_cheri_base_get(cap);
            if ((base >= heap_base) && (base <= heap_end))
            {
                continue;
            }
            //fprintf(stderr, "Found escaped cap: ");
            //print_cap(cap);
            if (!__builtin_cheri_sealed_get(cap))
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
    //revoke_from_range(pool, stack);
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

int sandbox_object_stack_reset(struct sandbox_object *sbop);
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
        sandbox_object_stack_reset(sandbox->global_object->obj);
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
    bool is_static = mb->access_flags & ACC_STATIC;
    void *receiver = is_static ? class : ((void*)*(ostack_args++));
    cap_args[1] = seal_object(receiver);

    struct jni_sandbox_object *pool = NULL;
    struct jni_per_object_sandbox *h;
    assert(metadata->sandbox);
    switch (metadata->scope)
    {
        case SandboxScopeGlobal:
            pthread_mutex_lock(&metadata->sandbox->global_lock);
            if (metadata->sandbox->global_object == NULL)
            {
                metadata->sandbox->global_object =
                    get_or_create_sandbox_object(metadata->sandbox);
                metadata->sandbox->global_object->scope = SandboxScopeGlobal;
            }
            pool = metadata->sandbox->global_object;
            break;
        case SandboxScopeObject:
        {
            pthread_mutex_lock(&metadata->sandbox->per_object_lock);
            HASH_FIND_PTR(metadata->sandbox->objects, &receiver, h);
            // If we've found a sandbox, check that it refers to this object
            // and not a dead one that happens to have the same address.  Skip
            // this check for static methods.
            if ((h != NULL) && (h->weak_ref != NULL))
            {
                jobject weak_obj = env->NewLocalRef(&env, h->weak_ref);
                if (weak_obj != receiver)
                {
                    // Return this object to the pool
                    pthread_mutex_lock(&metadata->sandbox->pool_lock);
                    h->object->next = metadata->sandbox->pool_head;
                    metadata->sandbox->pool_head = h->object;
                    pthread_mutex_unlock(&metadata->sandbox->pool_lock);
                    env->DeleteWeakGlobalRef(&env, h->weak_ref);
                    HASH_DEL(metadata->sandbox->objects, h);
                    free(h);
                    h = NULL;
                }
            }
            // If we still haven't found a valid sandbox, then allocate one.
            if (h == NULL)
            {
                pool = get_or_create_sandbox_object(metadata->sandbox);
                pool->scope = SandboxScopeGlobal;
                h = calloc(1, sizeof(struct jni_per_object_sandbox));
                h->lock = PTHREAD_MUTEX_INITIALIZER;
                h->java_object = receiver;
                h->weak_ref = is_static ? NULL : env->NewWeakGlobalRef(&env, receiver);
                h->object = pool;
                HASH_ADD_PTR(metadata->sandbox->objects, java_object, h);
            }
            else
            {
                pool = h->object;
            }
            pthread_mutex_unlock(&metadata->sandbox->per_object_lock);
            pthread_mutex_lock(&h->lock);
            break;
        }
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
    cap_args[0] = __builtin_cheri_bounds_set((__capability void*)&pool->env, sizeof(__capability void*));

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
                // Skip the type of the array.
                signature++;
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
            if (cherierrno != 0)
            {
                // If an exception occurred in an object sandbox, assume that it
                // is in an undefined state and reset it.
                sandbox_object_reset(pool->obj);
                release_sandbox_resources(pool, true);
            }
            else
            {
                sandbox_object_stack_reset(pool->obj);
                pool->invokes_since_sweep++;
            }
            pthread_mutex_unlock(&h->lock);
            break;
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
                sandbox_object_stack_reset(pool->obj);
                pool->invokes_since_sweep++;
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
    if (cherierrno == -1) {
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
        char *sandbox_class_name = "uk/ac/cam/cheri/Sandbox";
        static pClass sandbox_class;
        if (sandbox_class == NULL) {
            sandbox_class = findClassFromClassLoader(sandbox_class_name, loader);
        }
        assert(sandbox_class);
        pObject *annotation_data = ARRAY_DATA(annotations, pObject);
        for (int i=0 ; i<annotation_count ; i++) {
            pObject annot = annotation_data[i];

            pMethodBlock method = findMethod(annot->class, SYMBOL(annotationType), SYMBOL(___java_lang_Class));
            pClass annot_cls = *(pClass*)executeMethod(annot, method);
            if (annot_cls == sandbox_class) {
                struct jni_sandbox_method *metadata = calloc(sizeof(struct jni_sandbox_method), 1);
                pMethodBlock class_method = findMethod(annot->class, SYMBOL(SandboxClass), SYMBOL(___java_lang_String));
                pMethodBlock scope_method = findMethod(annot->class, SYMBOL(scope), SYMBOL(___uk_ac_cam_cheri_Sandbox_Scope));
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
