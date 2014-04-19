#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

extern jboolean Java_gnu_java_nio_KqueueSelectorImpl_check_1eof (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate);
extern  void Java_gnu_java_nio_KqueueSelectorImpl_implClose (JNIEnv *env, jclass clazz __attribute__((unused)), jint kq);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_implOpen (JNIEnv *env, jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_KqueueSelectorImpl_kevent (JNIEnv *env, jobject this __attribute__((unused)), jint kq, jobject nstate, jint nevents, jint maxevents, jlong timeout);
extern  void Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate, jint i, jint fd, jint ops, jint active, jint key);
extern jboolean Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported (JNIEnv *env __attribute__((unused)), jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate, jint interest);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent (JNIEnv *env __attribute__((unused)), jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_accept (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_available (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern  void Java_gnu_java_nio_VMChannel_close (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jboolean Java_gnu_java_nio_VMChannel_connect (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr, jint port, jint timeout);
extern jboolean Java_gnu_java_nio_VMChannel_connect6 (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr, jint port, int timeout);
extern  void Java_gnu_java_nio_VMChannel_disconnect (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jboolean Java_gnu_java_nio_VMChannel_flush (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jboolean metadata __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_getpeername (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jobject name);
extern jint Java_gnu_java_nio_VMChannel_getsockname (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jobject name);
extern  void Java_gnu_java_nio_VMChannel_initIDs (JNIEnv *env, jclass clazz);
extern jboolean Java_gnu_java_nio_VMChannel_lock (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos, jlong len, jboolean shared, jboolean wait);
extern jobject Java_gnu_java_nio_VMChannel_map (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jchar mode, jlong position, jint size);
extern jint Java_gnu_java_nio_VMChannel_open (JNIEnv *env, jclass c __attribute__((unused)), jstring path, jint mode);
extern jlong Java_gnu_java_nio_VMChannel_position (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jlong Java_gnu_java_nio_VMChannel_readScattering (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobjectArray bbufs, jint offset, jint length);
extern jint Java_gnu_java_nio_VMChannel_read__I (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2 (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobject bbuf);
extern jint Java_gnu_java_nio_VMChannel_receive (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jobject dst, jobject addrPort);
extern  void Java_gnu_java_nio_VMChannel_seek (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos);
extern jint Java_gnu_java_nio_VMChannel_send (JNIEnv *env, jclass c __attribute__((unused)), int fd, jobject src, jbyteArray addr, jint port);
extern jint Java_gnu_java_nio_VMChannel_send6 (JNIEnv *env, jclass c __attribute__((unused)), int fd, jobject src, jbyteArray addr, jint port);
extern  void Java_gnu_java_nio_VMChannel_setBlocking (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jboolean blocking);
extern jlong Java_gnu_java_nio_VMChannel_size (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_socket (JNIEnv *env, jclass clazz __attribute__((unused)), jboolean stream);
extern jint Java_gnu_java_nio_VMChannel_stderr_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_stdin_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_stdout_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern  void Java_gnu_java_nio_VMChannel_truncate (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong len);
extern  void Java_gnu_java_nio_VMChannel_unlock (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos, jlong len);
extern jlong Java_gnu_java_nio_VMChannel_writeGathering (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobjectArray bbufs, jint offset, jint length);
extern  void Java_gnu_java_nio_VMChannel_write__II (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint data);
extern jint Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2 (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobject bbuf);
extern jintArray Java_gnu_java_nio_VMPipe_pipe0 (JNIEnv *env, jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMSelector_select (JNIEnv * env, jclass obj __attribute__ ((__unused__)), jintArray read, jintArray write, jintArray except, jlong timeout);
extern  void Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv (JNIEnv * env UNUSED, jobject obj UNUSED);
extern jint Java_gnu_java_nio_charset_iconv_IconvDecoder_decode (JNIEnv * env UNUSED, jobject obj UNUSED, jbyteArray inArr UNUSED, jcharArray outArr UNUSED, jint posIn UNUSED, jint remIn UNUSED, jint posOut UNUSED, jint remOut UNUSED);
extern  void Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv (JNIEnv * env UNUSED, jobject obj UNUSED, jstring jname UNUSED);
extern  void Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv (JNIEnv * env UNUSED, jobject obj UNUSED);
extern jint Java_gnu_java_nio_charset_iconv_IconvEncoder_encode (JNIEnv * env UNUSED, jobject obj UNUSED, jcharArray inArr UNUSED, jbyteArray outArr UNUSED, jint posIn UNUSED, jint remIn UNUSED, jint posOut UNUSED, jint remOut UNUSED);
extern  void Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv (JNIEnv * env UNUSED, jobject obj UNUSED, jstring jname UNUSED);
extern  void Java_java_nio_MappedByteBufferImpl_forceImpl (JNIEnv *env, jobject this);
extern jboolean Java_java_nio_MappedByteBufferImpl_isLoadedImpl (JNIEnv * env, jobject this);
extern  void Java_java_nio_MappedByteBufferImpl_loadImpl (JNIEnv *env, jobject this);
extern  void Java_java_nio_MappedByteBufferImpl_unmapImpl (JNIEnv *env, jobject this);
extern jobject Java_java_nio_VMDirectByteBuffer_adjustAddress (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint offset);
extern jobject Java_java_nio_VMDirectByteBuffer_allocate (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jint capacity);
extern  void Java_java_nio_VMDirectByteBuffer_free (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address);
extern jbyte Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index);
extern  void Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyteArray dst, jint dst_offset, jint dst_len);
extern  void Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyte value);
extern  void Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyteArray src, jint src_offset, jint src_len);
extern  void Java_java_nio_VMDirectByteBuffer_shiftDown (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint dst_offset, jint src_offset, jint count);

typedef struct method_entry {
	char *name;
	void *func;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{ "Java_gnu_java_nio_KqueueSelectorImpl_check_1eof", &Java_gnu_java_nio_KqueueSelectorImpl_check_1eof },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key", &Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_implClose", &Java_gnu_java_nio_KqueueSelectorImpl_implClose },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_implOpen", &Java_gnu_java_nio_KqueueSelectorImpl_implOpen },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kevent", &Java_gnu_java_nio_KqueueSelectorImpl_kevent },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set", &Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported", &Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops", &Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent", &Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent },
	{ "Java_gnu_java_nio_VMChannel_accept", &Java_gnu_java_nio_VMChannel_accept },
	{ "Java_gnu_java_nio_VMChannel_available", &Java_gnu_java_nio_VMChannel_available },
	{ "Java_gnu_java_nio_VMChannel_close", &Java_gnu_java_nio_VMChannel_close },
	{ "Java_gnu_java_nio_VMChannel_connect", &Java_gnu_java_nio_VMChannel_connect },
	{ "Java_gnu_java_nio_VMChannel_connect6", &Java_gnu_java_nio_VMChannel_connect6 },
	{ "Java_gnu_java_nio_VMChannel_disconnect", &Java_gnu_java_nio_VMChannel_disconnect },
	{ "Java_gnu_java_nio_VMChannel_flush", &Java_gnu_java_nio_VMChannel_flush },
	{ "Java_gnu_java_nio_VMChannel_getpeername", &Java_gnu_java_nio_VMChannel_getpeername },
	{ "Java_gnu_java_nio_VMChannel_getsockname", &Java_gnu_java_nio_VMChannel_getsockname },
	{ "Java_gnu_java_nio_VMChannel_initIDs", &Java_gnu_java_nio_VMChannel_initIDs },
	{ "Java_gnu_java_nio_VMChannel_lock", &Java_gnu_java_nio_VMChannel_lock },
	{ "Java_gnu_java_nio_VMChannel_map", &Java_gnu_java_nio_VMChannel_map },
	{ "Java_gnu_java_nio_VMChannel_open", &Java_gnu_java_nio_VMChannel_open },
	{ "Java_gnu_java_nio_VMChannel_position", &Java_gnu_java_nio_VMChannel_position },
	{ "Java_gnu_java_nio_VMChannel_readScattering", &Java_gnu_java_nio_VMChannel_readScattering },
	{ "Java_gnu_java_nio_VMChannel_read__I", &Java_gnu_java_nio_VMChannel_read__I },
	{ "Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2", &Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2 },
	{ "Java_gnu_java_nio_VMChannel_receive", &Java_gnu_java_nio_VMChannel_receive },
	{ "Java_gnu_java_nio_VMChannel_seek", &Java_gnu_java_nio_VMChannel_seek },
	{ "Java_gnu_java_nio_VMChannel_send", &Java_gnu_java_nio_VMChannel_send },
	{ "Java_gnu_java_nio_VMChannel_send6", &Java_gnu_java_nio_VMChannel_send6 },
	{ "Java_gnu_java_nio_VMChannel_setBlocking", &Java_gnu_java_nio_VMChannel_setBlocking },
	{ "Java_gnu_java_nio_VMChannel_size", &Java_gnu_java_nio_VMChannel_size },
	{ "Java_gnu_java_nio_VMChannel_socket", &Java_gnu_java_nio_VMChannel_socket },
	{ "Java_gnu_java_nio_VMChannel_stderr_1fd", &Java_gnu_java_nio_VMChannel_stderr_1fd },
	{ "Java_gnu_java_nio_VMChannel_stdin_1fd", &Java_gnu_java_nio_VMChannel_stdin_1fd },
	{ "Java_gnu_java_nio_VMChannel_stdout_1fd", &Java_gnu_java_nio_VMChannel_stdout_1fd },
	{ "Java_gnu_java_nio_VMChannel_truncate", &Java_gnu_java_nio_VMChannel_truncate },
	{ "Java_gnu_java_nio_VMChannel_unlock", &Java_gnu_java_nio_VMChannel_unlock },
	{ "Java_gnu_java_nio_VMChannel_writeGathering", &Java_gnu_java_nio_VMChannel_writeGathering },
	{ "Java_gnu_java_nio_VMChannel_write__II", &Java_gnu_java_nio_VMChannel_write__II },
	{ "Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2", &Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2 },
	{ "Java_gnu_java_nio_VMPipe_pipe0", &Java_gnu_java_nio_VMPipe_pipe0 },
	{ "Java_gnu_java_nio_VMSelector_select", &Java_gnu_java_nio_VMSelector_select },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv", &Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_decode", &Java_gnu_java_nio_charset_iconv_IconvDecoder_decode },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv", &Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv", &Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_encode", &Java_gnu_java_nio_charset_iconv_IconvEncoder_encode },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv", &Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv },
	{ "Java_java_nio_MappedByteBufferImpl_forceImpl", &Java_java_nio_MappedByteBufferImpl_forceImpl },
	{ "Java_java_nio_MappedByteBufferImpl_isLoadedImpl", &Java_java_nio_MappedByteBufferImpl_isLoadedImpl },
	{ "Java_java_nio_MappedByteBufferImpl_loadImpl", &Java_java_nio_MappedByteBufferImpl_loadImpl },
	{ "Java_java_nio_MappedByteBufferImpl_unmapImpl", &Java_java_nio_MappedByteBufferImpl_unmapImpl },
	{ "Java_java_nio_VMDirectByteBuffer_adjustAddress", &Java_java_nio_VMDirectByteBuffer_adjustAddress },
	{ "Java_java_nio_VMDirectByteBuffer_allocate", &Java_java_nio_VMDirectByteBuffer_allocate },
	{ "Java_java_nio_VMDirectByteBuffer_free", &Java_java_nio_VMDirectByteBuffer_free },
	{ "Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I", &Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I },
	{ "Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII", &Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII },
	{ "Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB", &Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB },
	{ "Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII", &Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII },
	{ "Java_java_nio_VMDirectByteBuffer_shiftDown", &Java_java_nio_VMDirectByteBuffer_shiftDown },
	{NULL, NULL}
};
