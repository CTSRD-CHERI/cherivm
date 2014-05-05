#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

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
