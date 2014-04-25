#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

extern jboolean Java_java_io_VMFile_canExecute (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_canRead (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_canWrite (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_canWriteDirectory (JNIEnv *env, jclass clazz, jstring path);
extern jboolean Java_java_io_VMFile_create (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_delete (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_exists (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jlong Java_java_io_VMFile_getFreeSpace (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring path);
extern jlong Java_java_io_VMFile_getTotalSpace (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring path);
extern jlong Java_java_io_VMFile_getUsableSpace (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring path);
extern jboolean Java_java_io_VMFile_isDirectory (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_isFile (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jlong Java_java_io_VMFile_lastModified (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jlong Java_java_io_VMFile_length (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jobjectArray Java_java_io_VMFile_list (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_mkdir (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_renameTo (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring t, jstring d);
extern jboolean Java_java_io_VMFile_setExecutable (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring name, jboolean executable, jboolean ownerOnly);
extern jboolean Java_java_io_VMFile_setLastModified (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name, jlong newtime);
extern jboolean Java_java_io_VMFile_setReadOnly (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jstring name);
extern jboolean Java_java_io_VMFile_setReadable (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring name, jboolean readable, jboolean ownerOnly);
extern jboolean Java_java_io_VMFile_setWritable (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jstring name, jboolean writable, jboolean ownerOnly);
extern jstring Java_java_io_VMFile_toCanonicalForm (JNIEnv *env, jclass class __attribute__ ((__unused__)), jstring jpath);
extern jobject Java_java_io_VMObjectInputStream_allocateObject (JNIEnv * env, jclass clazz __attribute__((__unused__)), jclass target_clazz, jclass constr_clazz, jobject constructor);
extern jboolean Java_java_io_VMObjectStreamClass_hasClassInitializer (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jclass klass);
extern void Java_java_io_VMObjectStreamClass_setBooleanNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jboolean value);
extern void Java_java_io_VMObjectStreamClass_setByteNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jbyte value);
extern void Java_java_io_VMObjectStreamClass_setCharNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jchar value);
extern void Java_java_io_VMObjectStreamClass_setDoubleNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jdouble value);
extern void Java_java_io_VMObjectStreamClass_setFloatNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jfloat value);
extern void Java_java_io_VMObjectStreamClass_setIntNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jint value);
extern void Java_java_io_VMObjectStreamClass_setLongNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jlong value);
extern void Java_java_io_VMObjectStreamClass_setObjectNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jobject value);
extern void Java_java_io_VMObjectStreamClass_setShortNative (JNIEnv * env, jclass vmosklass __attribute__ ((__unused__)), jobject field, jobject object, jshort value);

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{ "Java_java_io_VMFile_canExecute", &Java_java_io_VMFile_canExecute, 2 },
	{ "Java_java_io_VMFile_canRead", &Java_java_io_VMFile_canRead, 2 },
	{ "Java_java_io_VMFile_canWrite", &Java_java_io_VMFile_canWrite, 2 },
	{ "Java_java_io_VMFile_canWriteDirectory", &Java_java_io_VMFile_canWriteDirectory, 2 },
	{ "Java_java_io_VMFile_create", &Java_java_io_VMFile_create, 2 },
	{ "Java_java_io_VMFile_delete", &Java_java_io_VMFile_delete, 2 },
	{ "Java_java_io_VMFile_exists", &Java_java_io_VMFile_exists, 2 },
	{ "Java_java_io_VMFile_getFreeSpace", &Java_java_io_VMFile_getFreeSpace, 2 },
	{ "Java_java_io_VMFile_getTotalSpace", &Java_java_io_VMFile_getTotalSpace, 2 },
	{ "Java_java_io_VMFile_getUsableSpace", &Java_java_io_VMFile_getUsableSpace, 2 },
	{ "Java_java_io_VMFile_isDirectory", &Java_java_io_VMFile_isDirectory, 2 },
	{ "Java_java_io_VMFile_isFile", &Java_java_io_VMFile_isFile, 2 },
	{ "Java_java_io_VMFile_lastModified", &Java_java_io_VMFile_lastModified, 2 },
	{ "Java_java_io_VMFile_length", &Java_java_io_VMFile_length, 2 },
	{ "Java_java_io_VMFile_list", &Java_java_io_VMFile_list, 3 },
	{ "Java_java_io_VMFile_mkdir", &Java_java_io_VMFile_mkdir, 2 },
	{ "Java_java_io_VMFile_renameTo", &Java_java_io_VMFile_renameTo, 2 },
	{ "Java_java_io_VMFile_setExecutable", &Java_java_io_VMFile_setExecutable, 2 },
	{ "Java_java_io_VMFile_setLastModified", &Java_java_io_VMFile_setLastModified, 2 },
	{ "Java_java_io_VMFile_setReadOnly", &Java_java_io_VMFile_setReadOnly, 2 },
	{ "Java_java_io_VMFile_setReadable", &Java_java_io_VMFile_setReadable, 2 },
	{ "Java_java_io_VMFile_setWritable", &Java_java_io_VMFile_setWritable, 2 },
	{ "Java_java_io_VMFile_toCanonicalForm", &Java_java_io_VMFile_toCanonicalForm, 3 },
	{ "Java_java_io_VMObjectInputStream_allocateObject", &Java_java_io_VMObjectInputStream_allocateObject, 3 },
	{ "Java_java_io_VMObjectStreamClass_hasClassInitializer", &Java_java_io_VMObjectStreamClass_hasClassInitializer, 2 },
	{ "Java_java_io_VMObjectStreamClass_setBooleanNative", &Java_java_io_VMObjectStreamClass_setBooleanNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setByteNative", &Java_java_io_VMObjectStreamClass_setByteNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setCharNative", &Java_java_io_VMObjectStreamClass_setCharNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setDoubleNative", &Java_java_io_VMObjectStreamClass_setDoubleNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setFloatNative", &Java_java_io_VMObjectStreamClass_setFloatNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setIntNative", &Java_java_io_VMObjectStreamClass_setIntNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setLongNative", &Java_java_io_VMObjectStreamClass_setLongNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setObjectNative", &Java_java_io_VMObjectStreamClass_setObjectNative, 1 },
	{ "Java_java_io_VMObjectStreamClass_setShortNative", &Java_java_io_VMObjectStreamClass_setShortNative, 1 },
	{NULL, NULL, 0}
};
