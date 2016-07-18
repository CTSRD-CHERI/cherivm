#include <jni.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

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

__attribute__((always_inline))
static inline int idx(int x, int y, jint size)
{
	return (size * x) + y;
}

static inline void multiply_matrix(jint *out, jint *in1, jint *in2, jint size)
{
	int i = 0;
	for (jsize x=0 ; x<size ; x++)
	{
		for (jsize y=0 ; y<size ; y++)
		{
			jint a = 0;
			for (jsize f=0 ; f<size ; f++)
			{
				a += in1[idx(x,f, size)] * in2[idx(f,y, size)];
			}
			out[idx(x,y,size)] = a;
		}
	}
}

JNIEXPORT void JNICALL
#ifdef __CHERI_PURE_CAPABILITY__
Java_sandbox_BenchmarkMultiply_multiplyNative
#else
Java_sandbox_BenchmarkMultiply_multiplyNativeUnsafe
#endif
  (JNIEnv *env, jclass cls, jintArray outArr, jintArray inArr1, jintArray inArr2, jint size)
{
	jint *out = (*env)->GetIntArrayElements(env, outArr, NULL);
	jint *in1 = (*env)->GetIntArrayElements(env, inArr1, NULL);
	jint *in2 = (*env)->GetIntArrayElements(env, inArr2, NULL);
	multiply_matrix(out, in1, in2, size);
	(*env)->ReleaseIntArrayElements(env, outArr, out, 0);
	(*env)->ReleaseIntArrayElements(env, inArr1, in1, 0);
	(*env)->ReleaseIntArrayElements(env, inArr2, in2, 0);
}

#ifndef __CHERI_PURE_CAPABILITY__
JNIEXPORT void JNICALL
Java_sandbox_BenchmarkMultiply_multiplyNativeUnsafeCopy
  (JNIEnv *env, jclass cls, jintArray outArr, jintArray inArr1, jintArray inArr2, jint size)
{
	jint *out = (*env)->GetIntArrayElements(env, outArr, NULL);
	jint *in1 = (*env)->GetIntArrayElements(env, inArr1, NULL);
	jint *in2 = (*env)->GetIntArrayElements(env, inArr2, NULL);

	jint array_size = size*size;
	jint *out_copy = calloc(sizeof(jint), array_size);
	jint *in1_copy = calloc(sizeof(jint), array_size);
	jint *in2_copy = calloc(sizeof(jint), array_size);
	memcpy(in1_copy, in1, array_size * sizeof(jint));
	memcpy(in2_copy, in2, array_size * sizeof(jint));

	multiply_matrix(out_copy, in1_copy, in2_copy, size);

	memcpy(out, out_copy, array_size * sizeof(jint));
	
	free(out_copy);
	free(in1_copy);
	free(in2_copy);

	(*env)->ReleaseIntArrayElements(env, outArr, out, 0);
	(*env)->ReleaseIntArrayElements(env, inArr1, in1, 0);
	(*env)->ReleaseIntArrayElements(env, inArr2, in2, 0);
}
#endif

static size_t compress_bytes(void *in, void *out, size_t in_len, size_t out_len)
{
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
	size_t size = out_len - zs->avail_out;
	free(zs);
	return size;
}

static size_t decompress_bytes(void *in, void *out, size_t in_len, size_t out_len)
{
	z_stream *zs = calloc(1, sizeof(z_stream));
	zs->zalloc = Z_NULL;
	zs->zfree = Z_NULL;
	inflateInit(zs);
	zs->next_in = in;
	zs->avail_in = in_len;
	zs->next_out = out;
	zs->avail_out = out_len;
	inflate(zs, Z_FINISH);
	inflateEnd(zs);
	size_t size = out_len - zs->avail_out;
	free(zs);
	return size;
}


JNIEXPORT jlong JNICALL
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
	//assert(__builtin_memcap_length_get(in) >= in_len);
	//assert(__builtin_memcap_length_get(out) >= out_len);
#endif
	return compress_bytes(in, out, in_len, out_len);
}

#ifndef __CHERI_PURE_CAPABILITY__
JNIEXPORT jlong JNICALL Java_sandbox_BenchmarkZlib_compressUnsafeCopy
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
	jlong ret = compress_bytes(in_copy, out_copy, in_len, out_len);
	//fprintf(stderr, "Copying %lx bytes from %p to %p\n", (unsigned long)out_len, out_copy, out);
	memcpy(out, out_copy, out_len);
	free(in_copy);
	free(out_copy);
	return ret;
}
#endif

JNIEXPORT jlong JNICALL
#ifdef __CHERI_PURE_CAPABILITY__
Java_sandbox_BenchmarkZlib_decompress
#else
Java_sandbox_BenchmarkZlib_decompressUnsafe
#endif
  (JNIEnv *env, jclass this, jobject input, jobject output, jint in_len)
{
	void *in = (*env)->GetDirectBufferAddress(env, input);
	void *out = (*env)->GetDirectBufferAddress(env, output);
	size_t out_len = (*env)->GetDirectBufferCapacity(env, output);
#ifdef __CHERI_PURE_CAPABILITY__
	//print_cap(in);
	//print_cap(out);
	//assert(__builtin_memcap_length_get(in) >= in_len);
	//assert(__builtin_memcap_length_get(out) >= out_len);
#endif
	return decompress_bytes(in, out, in_len, out_len);
}

#ifndef __CHERI_PURE_CAPABILITY__
JNIEXPORT jlong JNICALL Java_sandbox_BenchmarkZlib_decompressUnsafeCopy
  (JNIEnv *env, jclass this, jobject input, jobject output, jint in_len)
{
	void *in = (*env)->GetDirectBufferAddress(env, input);
	void *out = (*env)->GetDirectBufferAddress(env, output);
	size_t out_len = (*env)->GetDirectBufferCapacity(env, output);
	void *in_copy = calloc(1, in_len);
	void *out_copy = calloc(1, out_len);
	//fprintf(stderr, "Copying %lx bytes from %p to %p\n", (unsigned long)in_len, in, in_copy);
	memcpy(in_copy, in, in_len);
	jlong ret = decompress_bytes(in_copy, out_copy, in_len, out_len);
	//fprintf(stderr, "Copying %lx bytes from %p to %p\n", (unsigned long)out_len, out_copy, out);
	memcpy(out, out_copy, out_len);
	free(in_copy);
	free(out_copy);
	return ret;
}
#endif
