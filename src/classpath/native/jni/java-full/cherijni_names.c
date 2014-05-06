#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

extern jint JNI_OnLoad (JavaVM *vm, void *reserved);

extern void Java_gnu_java_nio_EpollSelectorImpl_epoll_1add (JNIEnv *env, jclass c __attribute__((unused)), jint efd, jint fd, jint ops);
extern jint Java_gnu_java_nio_EpollSelectorImpl_epoll_1create (JNIEnv *env, jclass c __attribute__((unused)), jint size);
extern void Java_gnu_java_nio_EpollSelectorImpl_epoll_1delete (JNIEnv *env, jclass c __attribute__((unused)), jint efd, jint fd);
extern void Java_gnu_java_nio_EpollSelectorImpl_epoll_1modify (JNIEnv *env, jclass c __attribute__((unused)), jint efd, jint fd, jint ops);
extern jboolean Java_gnu_java_nio_EpollSelectorImpl_epoll_1supported (JNIEnv *e __attribute__((unused)), jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_EpollSelectorImpl_epoll_1wait (JNIEnv *env, jclass c __attribute__((unused)), jint efd, jobject nstate, jint num_events, jint timeout);
extern jint Java_gnu_java_nio_EpollSelectorImpl_selected_1fd (JNIEnv *env, jclass c __attribute__((unused)), jobject value);
extern jint Java_gnu_java_nio_EpollSelectorImpl_selected_1ops (JNIEnv *env, jclass c __attribute__((unused)), jobject value);
extern jint Java_gnu_java_nio_EpollSelectorImpl_sizeof_1struct (JNIEnv *env, jclass c __attribute__((unused)));
extern jboolean Java_gnu_java_nio_KqueueSelectorImpl_check_1eof (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate);
extern void Java_gnu_java_nio_KqueueSelectorImpl_implClose (JNIEnv *env, jclass clazz __attribute__((unused)), jint kq);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_implOpen (JNIEnv *env, jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_KqueueSelectorImpl_kevent (JNIEnv *env, jobject this __attribute__((unused)), jint kq, jobject nstate, jint nevents, jint maxevents, jlong timeout);
extern void Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate, jint i, jint fd, jint ops, jint active, jint key);
extern jboolean Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported (JNIEnv *env __attribute__((unused)), jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops (JNIEnv *env, jclass clazz __attribute__((unused)), jobject nstate, jint interest);
extern jint Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent (JNIEnv *env __attribute__((unused)), jclass clazz __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_accept (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_available (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern void Java_gnu_java_nio_VMChannel_close (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jboolean Java_gnu_java_nio_VMChannel_connect (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr, jint port, jint timeout);
extern jboolean Java_gnu_java_nio_VMChannel_connect6 (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr, jint port, int timeout);
extern void Java_gnu_java_nio_VMChannel_disconnect (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jboolean Java_gnu_java_nio_VMChannel_flush (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jboolean metadata __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_getpeername (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jobject name);
extern jint Java_gnu_java_nio_VMChannel_getsockname (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jobject name);
extern void Java_gnu_java_nio_VMChannel_initIDs (JNIEnv *env, jclass clazz);
extern jboolean Java_gnu_java_nio_VMChannel_lock (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos, jlong len, jboolean shared, jboolean wait);
extern jobject Java_gnu_java_nio_VMChannel_map (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jchar mode, jlong position, jint size);
extern jint Java_gnu_java_nio_VMChannel_open (JNIEnv *env, jclass c __attribute__((unused)), jstring path, jint mode);
extern jlong Java_gnu_java_nio_VMChannel_position (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jlong Java_gnu_java_nio_VMChannel_readScattering (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobjectArray bbufs, jint offset, jint length);
extern jint Java_gnu_java_nio_VMChannel_read__I (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2 (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobject bbuf);
extern jint Java_gnu_java_nio_VMChannel_receive (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jobject dst, jobject addrPort);
extern void Java_gnu_java_nio_VMChannel_seek (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos);
extern jint Java_gnu_java_nio_VMChannel_send (JNIEnv *env, jclass c __attribute__((unused)), int fd, jobject src, jbyteArray addr, jint port);
extern jint Java_gnu_java_nio_VMChannel_send6 (JNIEnv *env, jclass c __attribute__((unused)), int fd, jobject src, jbyteArray addr, jint port);
extern void Java_gnu_java_nio_VMChannel_setBlocking (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jboolean blocking);
extern jlong Java_gnu_java_nio_VMChannel_size (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern jint Java_gnu_java_nio_VMChannel_socket (JNIEnv *env, jclass clazz __attribute__((unused)), jboolean stream);
extern jint Java_gnu_java_nio_VMChannel_stderr_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_stdin_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMChannel_stdout_1fd (JNIEnv *env __attribute__((unused)), jclass c __attribute__((unused)));
extern void Java_gnu_java_nio_VMChannel_truncate (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong len);
extern void Java_gnu_java_nio_VMChannel_unlock (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jlong pos, jlong len);
extern jlong Java_gnu_java_nio_VMChannel_writeGathering (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobjectArray bbufs, jint offset, jint length);
extern void Java_gnu_java_nio_VMChannel_write__II (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint data);
extern jint Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2 (JNIEnv *env, jobject o __attribute__ ((__unused__)), jint fd, jobject bbuf);
extern jintArray Java_gnu_java_nio_VMPipe_pipe0 (JNIEnv *env, jclass c __attribute__((unused)));
extern jint Java_gnu_java_nio_VMSelector_select (JNIEnv * env, jclass obj __attribute__ ((__unused__)), jintArray read, jintArray write, jintArray except, jlong timeout);
extern void Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv (JNIEnv * env UNUSED, jobject obj UNUSED);
extern jint Java_gnu_java_nio_charset_iconv_IconvDecoder_decode (JNIEnv * env UNUSED, jobject obj UNUSED, jbyteArray inArr UNUSED, jcharArray outArr UNUSED, jint posIn UNUSED, jint remIn UNUSED, jint posOut UNUSED, jint remOut UNUSED);
extern void Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv (JNIEnv * env UNUSED, jobject obj UNUSED, jstring jname UNUSED);
extern void Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv (JNIEnv * env UNUSED, jobject obj UNUSED);
extern jint Java_gnu_java_nio_charset_iconv_IconvEncoder_encode (JNIEnv * env UNUSED, jobject obj UNUSED, jcharArray inArr UNUSED, jbyteArray outArr UNUSED, jint posIn UNUSED, jint remIn UNUSED, jint posOut UNUSED, jint remOut UNUSED);
extern void Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv (JNIEnv * env UNUSED, jobject obj UNUSED, jstring jname UNUSED);
extern void Java_java_nio_MappedByteBufferImpl_forceImpl (JNIEnv *env, jobject this);
extern jboolean Java_java_nio_MappedByteBufferImpl_isLoadedImpl (JNIEnv * env, jobject this);
extern void Java_java_nio_MappedByteBufferImpl_loadImpl (JNIEnv *env, jobject this);
extern void Java_java_nio_MappedByteBufferImpl_unmapImpl (JNIEnv *env, jobject this);
extern jobject Java_java_nio_VMDirectByteBuffer_adjustAddress (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint offset);
extern jobject Java_java_nio_VMDirectByteBuffer_allocate (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jint capacity);
extern void Java_java_nio_VMDirectByteBuffer_free (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address);
extern jbyte Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index);
extern void Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyteArray dst, jint dst_offset, jint dst_len);
extern void Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyte value);
extern void Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII (JNIEnv *env, jclass clazz __attribute__ ((__unused__)), jobject address, jint index, jbyteArray src, jint src_offset, jint src_len);
extern void Java_java_nio_VMDirectByteBuffer_shiftDown (JNIEnv * env, jclass clazz __attribute__ ((__unused__)), jobject address, jint dst_offset, jint src_offset, jint count);

extern jlong Java_java_lang_VMDouble_doubleToRawLongBits (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble doubleValue);
extern void Java_java_lang_VMDouble_initIDs (JNIEnv * env, jclass cls __attribute__ ((__unused__)));
extern jdouble Java_java_lang_VMDouble_longBitsToDouble (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jlong longValue);
extern jdouble Java_java_lang_VMDouble_parseDouble (JNIEnv * env, jclass cls __attribute__ ((__unused__)), jstring str);
extern jstring Java_java_lang_VMDouble_toString (JNIEnv * env, jclass cls __attribute__ ((__unused__)), jdouble value, jboolean isFloat);
extern jint Java_java_lang_VMFloat_floatToRawIntBits (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jfloat value);
extern jfloat Java_java_lang_VMFloat_intBitsToFloat (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jint bits);
extern jdouble Java_java_lang_VMMath_IEEEremainder (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_acos (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_asin (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_atan (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_atan2 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble y, jdouble x);
extern jdouble Java_java_lang_VMMath_cbrt (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_ceil (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_cos (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_cosh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_exp (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_expm1 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_floor (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_hypot (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_log (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_log10 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_log1p (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_pow (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_rint (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sin (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sinh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sqrt (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_tan (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_tanh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern void Java_java_lang_VMProcess_nativeKill (JNIEnv * env, jclass clazz, jlong pid);
extern jboolean Java_java_lang_VMProcess_nativeReap (JNIEnv * env, jclass clazz);
extern void Java_java_lang_VMProcess_nativeSpawn (JNIEnv * env, jobject this, jobjectArray cmdArray, jobjectArray envArray, jobject dirFile, jboolean redirect);
extern jlong Java_java_lang_VMSystem_currentTimeMillis (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)));
extern jobject Java_java_lang_VMSystem_environ (JNIEnv *env, jclass klass __attribute__((__unused__)));
extern jstring Java_java_lang_VMSystem_getenv (JNIEnv * env, jclass klass __attribute__ ((__unused__)), jstring jname);
extern jlong Java_java_lang_VMSystem_nanoTime (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)));
extern void Java_java_lang_VMSystem_setErr (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);
extern void Java_java_lang_VMSystem_setIn (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);
extern void Java_java_lang_VMSystem_setOut (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);

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

extern void Java_gnu_java_net_VMPlainSocketImpl_bind (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr, jint port);
extern void Java_gnu_java_net_VMPlainSocketImpl_bind6 (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr, jint port);
extern jobject Java_gnu_java_net_VMPlainSocketImpl_getMulticastInterface (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint optionId __attribute__((unused)));
extern jint Java_gnu_java_net_VMPlainSocketImpl_getOption (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint option);
extern void Java_gnu_java_net_VMPlainSocketImpl_join (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr);
extern void Java_gnu_java_net_VMPlainSocketImpl_join6 (JNIEnv *env, jclass clazz __attribute__((unused)), jint fd, jbyteArray addr);
extern void Java_gnu_java_net_VMPlainSocketImpl_joinGroup (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr, jstring ifname);
extern void Java_gnu_java_net_VMPlainSocketImpl_joinGroup6 (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr, jstring ifname);
extern void Java_gnu_java_net_VMPlainSocketImpl_leave (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr);
extern void Java_gnu_java_net_VMPlainSocketImpl_leave6 (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr);
extern void Java_gnu_java_net_VMPlainSocketImpl_leaveGroup (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr, jstring ifname);
extern void Java_gnu_java_net_VMPlainSocketImpl_leaveGroup6 (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jbyteArray addr, jstring ifname);
extern void Java_gnu_java_net_VMPlainSocketImpl_listen (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint backlog);
extern void Java_gnu_java_net_VMPlainSocketImpl_sendUrgentData (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint data);
extern void Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint optionId __attribute__((unused)), jobject addr);
extern void Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface6 (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint optionId __attribute__((unused)), jstring ifname);
extern void Java_gnu_java_net_VMPlainSocketImpl_setOption (JNIEnv *env, jclass c __attribute__((unused)), jint fd, jint option, jint value);
extern void Java_gnu_java_net_VMPlainSocketImpl_shutdownInput (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern void Java_gnu_java_net_VMPlainSocketImpl_shutdownOutput (JNIEnv *env, jclass c __attribute__((unused)), jint fd);
extern  void Java_gnu_java_net_local_LocalSocketImpl_accept (JNIEnv *env, jobject this, jobject socket);
extern jint Java_gnu_java_net_local_LocalSocketImpl_available (JNIEnv *env, jobject this __attribute__((unused)), jint fd);
extern  void Java_gnu_java_net_local_LocalSocketImpl_close (JNIEnv *env, jobject this);
extern  void Java_gnu_java_net_local_LocalSocketImpl_create (JNIEnv *env, jobject this, jboolean stream);
extern  void Java_gnu_java_net_local_LocalSocketImpl_listen (JNIEnv *env, jobject this, jint backlog);
extern  void Java_gnu_java_net_local_LocalSocketImpl_localBind (JNIEnv *env, jobject this, jobject address);
extern  void Java_gnu_java_net_local_LocalSocketImpl_localConnect (JNIEnv *env, jobject this, jobject address);
extern jint Java_gnu_java_net_local_LocalSocketImpl_read (JNIEnv *env, jobject this __attribute__((unused)), jint fd, jbyteArray buf, jint off, jint len);
extern  void Java_gnu_java_net_local_LocalSocketImpl_sendUrgentData (JNIEnv *env, jobject this __attribute__((unused)), jint data __attribute__((unused)));
extern  void Java_gnu_java_net_local_LocalSocketImpl_shutdownInput (JNIEnv *env, jobject this);
extern  void Java_gnu_java_net_local_LocalSocketImpl_shutdownOutput (JNIEnv *env, jobject this);
extern  void Java_gnu_java_net_local_LocalSocketImpl_unlink (JNIEnv *env, jobject this);
extern  void Java_gnu_java_net_local_LocalSocketImpl_write (JNIEnv *env, jobject this __attribute__((unused)), jint fd, jbyteArray buf, jint off, jint len);
extern jbyteArray Java_java_net_VMInetAddress_aton (JNIEnv *env, jclass class __attribute__ ((__unused__)), jstring host);
extern jstring Java_java_net_VMInetAddress_getHostByAddr (JNIEnv * env, jclass class __attribute__ ((__unused__)), jarray arr);
extern jobjectArray Java_java_net_VMInetAddress_getHostByName (JNIEnv * env, jclass class __attribute__ ((__unused__)), jstring host);
extern jstring Java_java_net_VMInetAddress_getLocalHostname (JNIEnv * env, jclass class __attribute__ ((__unused__)));
extern jarray Java_java_net_VMInetAddress_lookupInaddrAny (JNIEnv * env, jclass class __attribute__ ((__unused__)));
extern jobjectArray Java_java_net_VMNetworkInterface_getVMInterfaces (JNIEnv * env, jclass clazz UNUSED);
extern void Java_java_net_VMNetworkInterface_initIds (JNIEnv *env, jclass clazz);
extern jboolean Java_java_net_VMNetworkInterface_isLoopback (JNIEnv *env, jclass class UNUSED, jstring name);
extern jboolean Java_java_net_VMNetworkInterface_isPointToPoint (JNIEnv *env, jclass class UNUSED, jstring name);
extern jboolean Java_java_net_VMNetworkInterface_isUp (JNIEnv *env, jclass class UNUSED, jstring name);
extern jboolean Java_java_net_VMNetworkInterface_supportsMulticast (JNIEnv *env, jclass class UNUSED, jstring name);
extern jstring Java_java_net_VMURLConnection_guessContentTypeFromBuffer (JNIEnv *env, jclass klass __attribute__ ((__unused__)), jbyteArray bytes, jint valid);
extern  void Java_java_net_VMURLConnection_init (JNIEnv *env __attribute__ ((__unused__)), jclass klass __attribute__ ((__unused__)));

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{ "JNI_OnLoad", &JNI_OnLoad, 2 },

	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1add", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1add, 1 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1create", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1create, 2 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1delete", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1delete, 1 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1modify", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1modify, 1 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1supported", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1supported, 2 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_epoll_1wait", &Java_gnu_java_nio_EpollSelectorImpl_epoll_1wait, 2 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_selected_1fd", &Java_gnu_java_nio_EpollSelectorImpl_selected_1fd, 2 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_selected_1ops", &Java_gnu_java_nio_EpollSelectorImpl_selected_1ops, 2 },
	{ "Java_gnu_java_nio_EpollSelectorImpl_sizeof_1struct", &Java_gnu_java_nio_EpollSelectorImpl_sizeof_1struct, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_check_1eof", &Java_gnu_java_nio_KqueueSelectorImpl_check_1eof, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key", &Java_gnu_java_nio_KqueueSelectorImpl_fetch_1key, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_implClose", &Java_gnu_java_nio_KqueueSelectorImpl_implClose, 1 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_implOpen", &Java_gnu_java_nio_KqueueSelectorImpl_implOpen, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kevent", &Java_gnu_java_nio_KqueueSelectorImpl_kevent, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set", &Java_gnu_java_nio_KqueueSelectorImpl_kevent_1set, 1 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported", &Java_gnu_java_nio_KqueueSelectorImpl_kqueue_1supported, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops", &Java_gnu_java_nio_KqueueSelectorImpl_ready_1ops, 2 },
	{ "Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent", &Java_gnu_java_nio_KqueueSelectorImpl_sizeof_1struct_1kevent, 2 },
	{ "Java_gnu_java_nio_VMChannel_accept", &Java_gnu_java_nio_VMChannel_accept, 2 },
	{ "Java_gnu_java_nio_VMChannel_available", &Java_gnu_java_nio_VMChannel_available, 2 },
	{ "Java_gnu_java_nio_VMChannel_close", &Java_gnu_java_nio_VMChannel_close, 1 },
	{ "Java_gnu_java_nio_VMChannel_connect", &Java_gnu_java_nio_VMChannel_connect, 2 },
	{ "Java_gnu_java_nio_VMChannel_connect6", &Java_gnu_java_nio_VMChannel_connect6, 2 },
	{ "Java_gnu_java_nio_VMChannel_disconnect", &Java_gnu_java_nio_VMChannel_disconnect, 1 },
	{ "Java_gnu_java_nio_VMChannel_flush", &Java_gnu_java_nio_VMChannel_flush, 2 },
	{ "Java_gnu_java_nio_VMChannel_getpeername", &Java_gnu_java_nio_VMChannel_getpeername, 2 },
	{ "Java_gnu_java_nio_VMChannel_getsockname", &Java_gnu_java_nio_VMChannel_getsockname, 2 },
	{ "Java_gnu_java_nio_VMChannel_initIDs", &Java_gnu_java_nio_VMChannel_initIDs, 1 },
	{ "Java_gnu_java_nio_VMChannel_lock", &Java_gnu_java_nio_VMChannel_lock, 2 },
	{ "Java_gnu_java_nio_VMChannel_map", &Java_gnu_java_nio_VMChannel_map, 3 },
	{ "Java_gnu_java_nio_VMChannel_open", &Java_gnu_java_nio_VMChannel_open, 2 },
	{ "Java_gnu_java_nio_VMChannel_position", &Java_gnu_java_nio_VMChannel_position, 2 },
	{ "Java_gnu_java_nio_VMChannel_readScattering", &Java_gnu_java_nio_VMChannel_readScattering, 2 },
	{ "Java_gnu_java_nio_VMChannel_read__I", &Java_gnu_java_nio_VMChannel_read__I, 2 },
	{ "Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2", &Java_gnu_java_nio_VMChannel_read__ILjava_nio_ByteBuffer_2, 2 },
	{ "Java_gnu_java_nio_VMChannel_receive", &Java_gnu_java_nio_VMChannel_receive, 2 },
	{ "Java_gnu_java_nio_VMChannel_seek", &Java_gnu_java_nio_VMChannel_seek, 1 },
	{ "Java_gnu_java_nio_VMChannel_send", &Java_gnu_java_nio_VMChannel_send, 2 },
	{ "Java_gnu_java_nio_VMChannel_send6", &Java_gnu_java_nio_VMChannel_send6, 2 },
	{ "Java_gnu_java_nio_VMChannel_setBlocking", &Java_gnu_java_nio_VMChannel_setBlocking, 1 },
	{ "Java_gnu_java_nio_VMChannel_size", &Java_gnu_java_nio_VMChannel_size, 2 },
	{ "Java_gnu_java_nio_VMChannel_socket", &Java_gnu_java_nio_VMChannel_socket, 2 },
	{ "Java_gnu_java_nio_VMChannel_stderr_1fd", &Java_gnu_java_nio_VMChannel_stderr_1fd, 2 },
	{ "Java_gnu_java_nio_VMChannel_stdin_1fd", &Java_gnu_java_nio_VMChannel_stdin_1fd, 2 },
	{ "Java_gnu_java_nio_VMChannel_stdout_1fd", &Java_gnu_java_nio_VMChannel_stdout_1fd, 2 },
	{ "Java_gnu_java_nio_VMChannel_truncate", &Java_gnu_java_nio_VMChannel_truncate, 1 },
	{ "Java_gnu_java_nio_VMChannel_unlock", &Java_gnu_java_nio_VMChannel_unlock, 1 },
	{ "Java_gnu_java_nio_VMChannel_writeGathering", &Java_gnu_java_nio_VMChannel_writeGathering, 2 },
	{ "Java_gnu_java_nio_VMChannel_write__II", &Java_gnu_java_nio_VMChannel_write__II, 1 },
	{ "Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2", &Java_gnu_java_nio_VMChannel_write__ILjava_nio_ByteBuffer_2, 2 },
	{ "Java_gnu_java_nio_VMPipe_pipe0", &Java_gnu_java_nio_VMPipe_pipe0, 3 },
	{ "Java_gnu_java_nio_VMSelector_select", &Java_gnu_java_nio_VMSelector_select, 2 },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv", &Java_gnu_java_nio_charset_iconv_IconvDecoder_closeIconv, 1 },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_decode", &Java_gnu_java_nio_charset_iconv_IconvDecoder_decode, 2 },
	{ "Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv", &Java_gnu_java_nio_charset_iconv_IconvDecoder_openIconv, 1 },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv", &Java_gnu_java_nio_charset_iconv_IconvEncoder_closeIconv, 1 },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_encode", &Java_gnu_java_nio_charset_iconv_IconvEncoder_encode, 2 },
	{ "Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv", &Java_gnu_java_nio_charset_iconv_IconvEncoder_openIconv, 1 },
	{ "Java_java_nio_MappedByteBufferImpl_forceImpl", &Java_java_nio_MappedByteBufferImpl_forceImpl, 1 },
	{ "Java_java_nio_MappedByteBufferImpl_isLoadedImpl", &Java_java_nio_MappedByteBufferImpl_isLoadedImpl, 2 },
	{ "Java_java_nio_MappedByteBufferImpl_loadImpl", &Java_java_nio_MappedByteBufferImpl_loadImpl, 1 },
	{ "Java_java_nio_MappedByteBufferImpl_unmapImpl", &Java_java_nio_MappedByteBufferImpl_unmapImpl, 1 },
	{ "Java_java_nio_VMDirectByteBuffer_adjustAddress", &Java_java_nio_VMDirectByteBuffer_adjustAddress, 3 },
	{ "Java_java_nio_VMDirectByteBuffer_allocate", &Java_java_nio_VMDirectByteBuffer_allocate, 3 },
	{ "Java_java_nio_VMDirectByteBuffer_free", &Java_java_nio_VMDirectByteBuffer_free, 1 },
	{ "Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I", &Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I, 2 },
	{ "Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII", &Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII, 1 },
	{ "Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB", &Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB, 1 },
	{ "Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII", &Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII, 1 },
	{ "Java_java_nio_VMDirectByteBuffer_shiftDown", &Java_java_nio_VMDirectByteBuffer_shiftDown, 1 },

	{ "Java_java_lang_VMDouble_doubleToRawLongBits", &Java_java_lang_VMDouble_doubleToRawLongBits, 2 },
	{ "Java_java_lang_VMDouble_initIDs", &Java_java_lang_VMDouble_initIDs, 1 },
	{ "Java_java_lang_VMDouble_longBitsToDouble", &Java_java_lang_VMDouble_longBitsToDouble, 2 },
	{ "Java_java_lang_VMDouble_parseDouble", &Java_java_lang_VMDouble_parseDouble, 2 },
	{ "Java_java_lang_VMDouble_toString", &Java_java_lang_VMDouble_toString, 3 },
	{ "Java_java_lang_VMFloat_floatToRawIntBits", &Java_java_lang_VMFloat_floatToRawIntBits, 2 },
	{ "Java_java_lang_VMFloat_intBitsToFloat", &Java_java_lang_VMFloat_intBitsToFloat, 2 },
	{ "Java_java_lang_VMMath_IEEEremainder", &Java_java_lang_VMMath_IEEEremainder, 2 },
	{ "Java_java_lang_VMMath_acos", &Java_java_lang_VMMath_acos, 2 },
	{ "Java_java_lang_VMMath_asin", &Java_java_lang_VMMath_asin, 2 },
	{ "Java_java_lang_VMMath_atan", &Java_java_lang_VMMath_atan, 2 },
	{ "Java_java_lang_VMMath_atan2", &Java_java_lang_VMMath_atan2, 2 },
	{ "Java_java_lang_VMMath_cbrt", &Java_java_lang_VMMath_cbrt, 2 },
	{ "Java_java_lang_VMMath_ceil", &Java_java_lang_VMMath_ceil, 2 },
	{ "Java_java_lang_VMMath_cos", &Java_java_lang_VMMath_cos, 2 },
	{ "Java_java_lang_VMMath_cosh", &Java_java_lang_VMMath_cosh, 2 },
	{ "Java_java_lang_VMMath_exp", &Java_java_lang_VMMath_exp, 2 },
	{ "Java_java_lang_VMMath_expm1", &Java_java_lang_VMMath_expm1, 2 },
	{ "Java_java_lang_VMMath_floor", &Java_java_lang_VMMath_floor, 2 },
	{ "Java_java_lang_VMMath_hypot", &Java_java_lang_VMMath_hypot, 2 },
	{ "Java_java_lang_VMMath_log", &Java_java_lang_VMMath_log, 2 },
	{ "Java_java_lang_VMMath_log10", &Java_java_lang_VMMath_log10, 2 },
	{ "Java_java_lang_VMMath_log1p", &Java_java_lang_VMMath_log1p, 2 },
	{ "Java_java_lang_VMMath_pow", &Java_java_lang_VMMath_pow, 2 },
	{ "Java_java_lang_VMMath_rint", &Java_java_lang_VMMath_rint, 2 },
	{ "Java_java_lang_VMMath_sin", &Java_java_lang_VMMath_sin, 2 },
	{ "Java_java_lang_VMMath_sinh", &Java_java_lang_VMMath_sinh, 2 },
	{ "Java_java_lang_VMMath_sqrt", &Java_java_lang_VMMath_sqrt, 2 },
	{ "Java_java_lang_VMMath_tan", &Java_java_lang_VMMath_tan, 2 },
	{ "Java_java_lang_VMMath_tanh", &Java_java_lang_VMMath_tanh, 2 },
	{ "Java_java_lang_VMProcess_nativeKill", &Java_java_lang_VMProcess_nativeKill, 1 },
	{ "Java_java_lang_VMProcess_nativeReap", &Java_java_lang_VMProcess_nativeReap, 2 },
	{ "Java_java_lang_VMProcess_nativeSpawn", &Java_java_lang_VMProcess_nativeSpawn, 1 },
	{ "Java_java_lang_VMSystem_currentTimeMillis", &Java_java_lang_VMSystem_currentTimeMillis, 2 },
	{ "Java_java_lang_VMSystem_environ", &Java_java_lang_VMSystem_environ, 3 },
	{ "Java_java_lang_VMSystem_getenv", &Java_java_lang_VMSystem_getenv, 3 },
	{ "Java_java_lang_VMSystem_nanoTime", &Java_java_lang_VMSystem_nanoTime, 2 },
	{ "Java_java_lang_VMSystem_setErr", &Java_java_lang_VMSystem_setErr, 1 },
	{ "Java_java_lang_VMSystem_setIn", &Java_java_lang_VMSystem_setIn, 1 },
	{ "Java_java_lang_VMSystem_setOut", &Java_java_lang_VMSystem_setOut, 1 },

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

	{ "Java_gnu_java_net_VMPlainSocketImpl_bind", &Java_gnu_java_net_VMPlainSocketImpl_bind, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_bind6", &Java_gnu_java_net_VMPlainSocketImpl_bind6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_getMulticastInterface", &Java_gnu_java_net_VMPlainSocketImpl_getMulticastInterface, 3 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_getOption", &Java_gnu_java_net_VMPlainSocketImpl_getOption, 2 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_join", &Java_gnu_java_net_VMPlainSocketImpl_join, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_join6", &Java_gnu_java_net_VMPlainSocketImpl_join6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_joinGroup", &Java_gnu_java_net_VMPlainSocketImpl_joinGroup, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_joinGroup6", &Java_gnu_java_net_VMPlainSocketImpl_joinGroup6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_leave", &Java_gnu_java_net_VMPlainSocketImpl_leave, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_leave6", &Java_gnu_java_net_VMPlainSocketImpl_leave6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_leaveGroup", &Java_gnu_java_net_VMPlainSocketImpl_leaveGroup, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_leaveGroup6", &Java_gnu_java_net_VMPlainSocketImpl_leaveGroup6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_listen", &Java_gnu_java_net_VMPlainSocketImpl_listen, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_sendUrgentData", &Java_gnu_java_net_VMPlainSocketImpl_sendUrgentData, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface", &Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface6", &Java_gnu_java_net_VMPlainSocketImpl_setMulticastInterface6, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_setOption", &Java_gnu_java_net_VMPlainSocketImpl_setOption, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_shutdownInput", &Java_gnu_java_net_VMPlainSocketImpl_shutdownInput, 1 },
	{ "Java_gnu_java_net_VMPlainSocketImpl_shutdownOutput", &Java_gnu_java_net_VMPlainSocketImpl_shutdownOutput, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_accept", &Java_gnu_java_net_local_LocalSocketImpl_accept, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_available", &Java_gnu_java_net_local_LocalSocketImpl_available, 2 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_close", &Java_gnu_java_net_local_LocalSocketImpl_close, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_create", &Java_gnu_java_net_local_LocalSocketImpl_create, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_listen", &Java_gnu_java_net_local_LocalSocketImpl_listen, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_localBind", &Java_gnu_java_net_local_LocalSocketImpl_localBind, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_localConnect", &Java_gnu_java_net_local_LocalSocketImpl_localConnect, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_read", &Java_gnu_java_net_local_LocalSocketImpl_read, 2 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_sendUrgentData", &Java_gnu_java_net_local_LocalSocketImpl_sendUrgentData, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_shutdownInput", &Java_gnu_java_net_local_LocalSocketImpl_shutdownInput, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_shutdownOutput", &Java_gnu_java_net_local_LocalSocketImpl_shutdownOutput, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_unlink", &Java_gnu_java_net_local_LocalSocketImpl_unlink, 1 },
	{ "Java_gnu_java_net_local_LocalSocketImpl_write", &Java_gnu_java_net_local_LocalSocketImpl_write, 1 },
	{ "Java_java_net_VMInetAddress_aton", &Java_java_net_VMInetAddress_aton, 3 },
	{ "Java_java_net_VMInetAddress_getHostByAddr", &Java_java_net_VMInetAddress_getHostByAddr, 3 },
	{ "Java_java_net_VMInetAddress_getHostByName", &Java_java_net_VMInetAddress_getHostByName, 3 },
	{ "Java_java_net_VMInetAddress_getLocalHostname", &Java_java_net_VMInetAddress_getLocalHostname, 3 },
	{ "Java_java_net_VMInetAddress_lookupInaddrAny", &Java_java_net_VMInetAddress_lookupInaddrAny, 3 },
	{ "Java_java_net_VMNetworkInterface_getVMInterfaces", &Java_java_net_VMNetworkInterface_getVMInterfaces, 3 },
	{ "Java_java_net_VMNetworkInterface_initIds", &Java_java_net_VMNetworkInterface_initIds, 1 },
	{ "Java_java_net_VMNetworkInterface_isLoopback", &Java_java_net_VMNetworkInterface_isLoopback, 2 },
	{ "Java_java_net_VMNetworkInterface_isPointToPoint", &Java_java_net_VMNetworkInterface_isPointToPoint, 2 },
	{ "Java_java_net_VMNetworkInterface_isUp", &Java_java_net_VMNetworkInterface_isUp, 2 },
	{ "Java_java_net_VMNetworkInterface_supportsMulticast", &Java_java_net_VMNetworkInterface_supportsMulticast, 2 },
	{ "Java_java_net_VMURLConnection_guessContentTypeFromBuffer", &Java_java_net_VMURLConnection_guessContentTypeFromBuffer, 3 },
	{ "Java_java_net_VMURLConnection_init", &Java_java_net_VMURLConnection_init, 1 },
	{NULL, NULL, 0}
};
