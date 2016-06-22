#include <jni.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>


JNIEXPORT void JNICALL
#ifdef __CHERI_PURE_CAPABILITY__
Java_sandbox_BenchmarkMultiply_multiplyNative
#else
Java_sandbox_BenchmarkMultiply_multiplyNativeUnsafe
#endif
  (JNIEnv *env, jclass cls, jintArray array, jint factor)
{
	jint *buffer = (*env)->GetIntArrayElements(env, array, NULL);
	jsize len = (*env)->GetArrayLength(env, array);
	for (jsize i=0 ; i<len ; i++)
	{
		for (jsize j=0 ; j<len ; j++)
		{
			buffer[i] += buffer[i] * factor;
		}
	}
	(*env)->ReleaseIntArrayElements(env, array, buffer, 0);
}

__attribute__((noinline))
static void compress_bytes(void *in, void *out, size_t in_len, size_t out_len)
{
#if 0
	z_stream zs;
	memset(&zs, 0, sizeof(zs));
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	deflateInit(&zs, Z_DEFAULT_COMPRESSION);
	zs.next_in = in;
	zs.avail_in = in_len;
	zs.next_out = out;
	zs.avail_out = out_len;
	deflate(&zs, Z_FINISH);
	deflateEnd(&zs);
#endif
	z_stream *zs = calloc(1, sizeof(z_stream));
	zs->zalloc = Z_NULL;
	zs->zfree = Z_NULL;
	deflateInit(zs, Z_DEFAULT_COMPRESSION);
	zs->next_in = in;
	zs->avail_in = in_len;
	zs->next_out = out;
	zs->avail_out = out_len;
	deflate(zs, Z_FINISH);
	deflateEnd(zs);
	free(zs);
}

#ifdef __CHERI_PURE_CAPABILITY__
void print_cap(const void *c)
{
	printf("b:0x%llx l:0x%llx o:0x%llx t:%d s:%d\n",
		(unsigned long long)__builtin_memcap_base_get(c),
		(unsigned long long)__builtin_memcap_length_get(c),
		(unsigned long long)__builtin_memcap_offset_get(c),
		(int)__builtin_memcap_type_get(c),
		(int)__builtin_memcap_sealed_get(c));
}
#endif


JNIEXPORT void JNICALL
#ifdef __CHERI_PURE_CAPABILITY__
Java_sandbox_BenchmarkZlib_compress
#else
Java_sandbox_BenchmarkZlib_compressUnsafe
#endif
  (JNIEnv *env, jclass this, jobject input, jobject output)
{
	void *in = (*env)->GetDirectBufferAddress(env, input);
	void *out = (*env)->GetDirectBufferAddress(env, output);
	size_t in_len = (*env)->GetDirectBufferCapacity(env, input);
	size_t out_len = (*env)->GetDirectBufferCapacity(env, output);
	//printf("Compressing  %lx bytes from %p to %p\n", (unsigned long)in_len, in, out);
#ifdef __CHERI_PURE_CAPABILITY__
	//print_cap(in);
	//print_cap(out);
	assert(__builtin_memcap_length_get(in) >= in_len);
	assert(__builtin_memcap_length_get(out) >= out_len);
#endif
	compress_bytes(in, out, in_len, out_len);
}

#ifndef __CHERI_PURE_CAPABILITY__
JNIEXPORT void JNICALL Java_sandbox_BenchmarkZlib_compressUnsafeCopy
  (JNIEnv *env, jclass this, jobject input, jobject output)
{
	void *in = (*env)->GetDirectBufferAddress(env, input);
	void *out = (*env)->GetDirectBufferAddress(env, output);
	size_t in_len = (*env)->GetDirectBufferCapacity(env, input);
	size_t out_len = (*env)->GetDirectBufferCapacity(env, output);
	void *in_copy = calloc(1, in_len);
	void *out_copy = calloc(1, out_len);
	//fprintf(stderr, "Copying %lx bytes from %p to %p\n", (unsigned long)in_len, in, in_copy);
	memcpy(in_copy, in, in_len);
	compress_bytes(in_copy, out_copy, in_len, out_len);
	//fprintf(stderr, "Copying %lx bytes from %p to %p\n", (unsigned long)out_len, out_copy, out);
	memcpy(out, out_copy, out_len);
	free(in_copy);
	free(out_copy);
}
#endif
