#include "Prompt.h"

JNIEXPORT jstring JNICALL Java_Prompt_getLine(JNIEnv *env, jobject self, jstring prompt) {
    const jbyte *str;

    str = (*env)->GetStringUTFChars(env, prompt, NULL);
    if (str == NULL)
        return NULL; /* OutOfMemoryError already thrown */
    printf("%s", str);
    (*env)->ReleaseStringUTFChars(env, prompt, str);

    return (*env)->NewStringUTF(env, "Hello, World!");
}

