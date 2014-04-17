#include "Prompt.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
	printf("[SANDBOX: %s says Hi!\n", __func__);
	return JNI_VERSION_1_2;
}

JNIEXPORT jstring JNICALL Java_Prompt_getLine(JNIEnv *env, jobject self, jstring prompt) {
    const jbyte *str;

    str = (*env)->GetStringUTFChars(env, prompt, NULL);
    if (str == NULL)
        return NULL; /* OutOfMemoryError already thrown */
    printf("%s", str);
    (*env)->ReleaseStringUTFChars(env, prompt, str);

    return (*env)->NewStringUTF(env, "Hello, World!");
}

JNIEXPORT void JNICALL Java_gnu_java_nio_VMChannel_initIDs(JNIEnv *env, jclass clazz) {
	printf("[SANDBOX: %s says Hi!\n", __func__);
//  jclass bufferClass = JCL_FindClass(env, "java/nio/Buffer");
//  jclass byteBufferClass = JCL_FindClass(env, "java/nio/ByteBuffer");
//
///*   NIODBG("%s", "..."); */
//
//  address_fid = (*env)->GetFieldID(env, bufferClass, "address",
//                                   "Lgnu/classpath/Pointer;");
//  if (address_fid == NULL)
//    {
//  	  JCL_ThrowException(env, "java/lang/InternalError",
//  	  	"Unable to find internal field");
//      return;
//    }
//
//  get_position_mid = get_method_id(env, bufferClass, "position", "()I");
//  set_position_mid = get_method_id(env, bufferClass, "position",
//                                   "(I)Ljava/nio/Buffer;");
//  get_limit_mid = get_method_id(env, bufferClass, "limit", "()I");
//  set_limit_mid = get_method_id(env, bufferClass, "limit",
//                                "(I)Ljava/nio/Buffer;");
//  has_array_mid = get_method_id(env, byteBufferClass, "hasArray", "()Z");
//  array_mid = get_method_id(env, byteBufferClass, "array", "()[B");
//  array_offset_mid = get_method_id(env, byteBufferClass, "arrayOffset", "()I");
//
//  vm_channel_class = clazz;
//  thread_interrupted_mid = (*env)->GetStaticMethodID(env, clazz,
//                                                  "isThreadInterrupted",
//                                                  "()Z");
}
