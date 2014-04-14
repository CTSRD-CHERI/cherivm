#include "Prompt.h"

JNIEXPORT jstring JNICALL Java_Prompt_getLine(JNIEnv *env, jobject self, jstring prompt) {
    char buf[128];
    const jbyte *str;

    str = (*env)->GetStringUTFChars(env, prompt, NULL);
    if (str == NULL)
        return NULL; /* OutOfMemoryError already thrown */
    printf("%s", str);
    (*env)->ReleaseStringUTFChars(env, prompt, str);

    /* We assume here that the user does not type more than
     * 127 characters */
    scanf("%s", buf);
    return (*env)->NewStringUTF(env, buf);
}

