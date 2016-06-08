/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __JNI_H__
#define __JNI_H__
#include <stdio.h>
#include <stdarg.h>

#define JNI_VERSION_1_1 0x00010001
#define JNI_VERSION_1_2 0x00010002
#define JNI_VERSION_1_4 0x00010004
#define JNI_VERSION_1_6 0x00010006

#define JNI_FALSE 0
#define JNI_TRUE 1

#define JNI_OK 0
#define JNI_ERR (-1)
#define JNI_EDETACHED (-2)
#define JNI_EVERSION (-3)

#define JNI_COMMIT 1
#define JNI_ABORT 2

#define JNIEXPORT
#ifdef __CHERI_PURE_CAPABILITY__
#define JNICALL \
    __attribute__((cheri_ccallee)) \
    __attribute__((cheri_method_class(JNI_SANDBOX_CLASS)))
#else
#define JNICALL
#endif

typedef int             jint;
typedef long long       jlong;
typedef signed char     jbyte;   
typedef unsigned char   jboolean;
typedef unsigned short  jchar;
typedef short           jshort;
typedef float           jfloat;
typedef double          jdouble;
typedef jint            jsize;

typedef void *jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jarray jobjectArray;

typedef __capability void *jobject_c;
typedef jobject_c jclass_c;
typedef jobject_c jthrowable_c;
typedef jobject_c jstring_c;
typedef jobject_c jarray_c;
typedef jarray_c jbooleanArray_c;
typedef jarray_c jbyteArray_c;
typedef jarray_c jcharArray_c;
typedef jarray_c jshortArray_c;
typedef jarray_c jintArray_c;
typedef jarray_c jlongArray_c;
typedef jarray_c jfloatArray_c;
typedef jarray_c jdoubleArray_c;
typedef jarray_c jobjectArray_c;

typedef jobject jweak;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

typedef union jvalue_c {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject_c  l;
} jvalue_c;

typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod;

typedef void *jfieldID;
typedef void *jmethodID;
typedef __capability void *jfieldID_c;
typedef __capability void *jmethodID_c;

struct _JNINativeInterface;
struct _JNISandboxedNativeInterface;
#ifdef __CHERI_PURE_CAPABILITY__
typedef const struct _JNISandboxedNativeInterface *JNIEnv;
#else
typedef const struct _JNINativeInterface *JNIEnv;
#endif

struct _JNIInvokeInterface;
typedef const struct _JNIInvokeInterface *JavaVM;

enum _jobjectRefType
{
  JNIInvalidRefType    = 0,
  JNILocalRefType      = 1,
  JNIGlobalRefType     = 2,
  JNIWeakGlobalRefType = 3 
};

typedef enum _jobjectRefType jobjectRefType;
#ifdef __CHERI__
#ifndef CHERI_CALLBACK
#define CHERI_CALLBACK __attribute__((cheri_ccallback))
#endif
#endif

#define VIRTUAL_METHOD(type, native_type)                                                   \
CALLBACK_CC native_type (*Call##type##Method)(JNIEnvType *env, jobject obj, jmethodID mID, ...);            \
CALLBACK_CC native_type (*Call##type##MethodV)(JNIEnvType *env, jobject obj, jmethodID mID, va_list jargs); \
CALLBACK_CC native_type (*Call##type##MethodA)(JNIEnvType *env, jobject obj, jmethodID mID, jvalue *jargs);

#define NONVIRTUAL_METHOD(type, native_type)                                                \
CALLBACK_CC native_type (*CallNonvirtual##type##Method)(JNIEnvType *env, jobject obj, jclass clazz,         \
                jmethodID methodID, ...);                                                   \
CALLBACK_CC native_type (*CallNonvirtual##type##MethodV)(JNIEnvType *env, jobject obj, jclass clazz,        \
                jmethodID methodID, va_list jargs);                                         \
CALLBACK_CC native_type (*CallNonvirtual##type##MethodA)(JNIEnvType *env, jobject obj, jclass clazz,        \
                jmethodID methodID, jvalue *jargs);

#define STATIC_METHOD(type, native_type)                                                    \
CALLBACK_CC native_type (*CallStatic##type##Method)(JNIEnvType *env, jclass clazz, jmethodID methodID, ...);\
CALLBACK_CC native_type (*CallStatic##type##MethodV)(JNIEnvType *env, jclass clazz, jmethodID methodID,     \
                va_list jargs);                                                             \
CALLBACK_CC native_type (*CallStatic##type##MethodA)(JNIEnvType *env, jclass clazz, jmethodID methodID,     \
                jvalue *jargs);

#define VOID_VIRTUAL_METHOD                                                                 \
CALLBACK_CC void (*CallVoidMethod)(JNIEnvType *env, jobject obj, jmethodID methodID, ...);                  \
CALLBACK_CC void (*CallVoidMethodV)(JNIEnvType *env, jobject obj, jmethodID methodID, va_list jargs);       \
CALLBACK_CC void (*CallVoidMethodA)(JNIEnvType *env, jobject obj, jmethodID methodID, jvalue *jargs);       \

#define VOID_NONVIRTUAL_METHOD                                                              \
CALLBACK_CC void (*CallNonvirtualVoidMethod)(JNIEnvType *env, jobject obj, jclass clazz,                    \
                jmethodID methodID, ...);                                                   \
CALLBACK_CC void (*CallNonvirtualVoidMethodV)(JNIEnvType *env, jobject obj, jclass clazz,                   \
                jmethodID methodID, va_list jargs);                                         \
CALLBACK_CC void (*CallNonvirtualVoidMethodA)(JNIEnvType *env, jobject obj, jclass clazz,                   \
                jmethodID methodID, jvalue *jargs);

#define VOID_STATIC_METHOD                                                                  \
CALLBACK_CC void (*CallStaticVoidMethod)(JNIEnvType *env, jclass clazz, jmethodID methodID, ...);           \
CALLBACK_CC void (*CallStaticVoidMethodV)(JNIEnvType *env, jclass clazz, jmethodID methodID, va_list jargs);\
CALLBACK_CC void (*CallStaticVoidMethodA)(JNIEnvType *env, jclass clazz, jmethodID methodID, jvalue *jargs);

#define CALL_METHOD(access)         \
access##_METHOD(Object, jobject);   \
access##_METHOD(Boolean, jboolean); \
access##_METHOD(Byte, jbyte);       \
access##_METHOD(Char, jchar);       \
access##_METHOD(Short, jshort);     \
access##_METHOD(Int, jint);         \
access##_METHOD(Long, jlong);       \
access##_METHOD(Float, jfloat);     \
access##_METHOD(Double, jdouble);   \
VOID_##access##_METHOD;


#define NEW_PRIM_ARRAY(type, native_type, array_type) \
CALLBACK_CC native_type##Array (*New##type##Array)(JNIEnvType *env, jsize length);

#define GET_ELEMENTS_PRIM_ARRAY(type, native_type, array_type) \
CALLBACK_CC native_type *(*Get##type##ArrayElements)(JNIEnvType *env, native_type##Array array, jboolean *isCopy);

#define RELEASE_ELEMENTS_PRIM_ARRAY(type, native_type, array_type) \
CALLBACK_CC void (*Release##type##ArrayElements)(JNIEnvType *env, native_type##Array array, native_type *elems, jint mode);

#define GET_REGION_PRIM_ARRAY(type, native_type, array_type) \
CALLBACK_CC void (*Get##type##ArrayRegion)(JNIEnvType *env, native_type##Array array, jsize start, jsize len, native_type *buf);

#define SET_REGION_PRIM_ARRAY(type, native_type, array_type) \
CALLBACK_CC void (*Set##type##ArrayRegion)(JNIEnvType *env, native_type##Array array, jsize start, jsize len, native_type *buf);

#define PRIM_ARRAY_OP(op)                      \
op##_PRIM_ARRAY(Boolean, jboolean, T_BOOLEAN); \
op##_PRIM_ARRAY(Byte, jbyte, T_BYTE);          \
op##_PRIM_ARRAY(Char, jchar, T_CHAR);          \
op##_PRIM_ARRAY(Short, jshort, T_SHORT);       \
op##_PRIM_ARRAY(Int, jint, T_INT);             \
op##_PRIM_ARRAY(Long, jlong, T_LONG);          \
op##_PRIM_ARRAY(Float, jfloat, T_FLOAT);       \
op##_PRIM_ARRAY(Double, jdouble, T_DOUBLE);


#define GET_FIELD(type, native_type) \
CALLBACK_CC native_type (*Get##type##Field)(JNIEnvType *env, jobject obj, jfieldID fieldID);

#define SET_FIELD(type, native_type) \
CALLBACK_CC void (*Set##type##Field)(JNIEnvType *env, jobject obj, jfieldID fieldID, native_type value);

#define GET_STATIC_FIELD(type, native_type) \
CALLBACK_CC native_type (*GetStatic##type##Field)(JNIEnvType *env, jclass clazz, jfieldID fieldID);

#define SET_STATIC_FIELD(type, native_type) \
CALLBACK_CC void (*SetStatic##type##Field)(JNIEnvType *env, jclass clazz, jfieldID fieldID, native_type value);

#define FIELD_OP(op)           \
CALLBACK_CC op##_FIELD(Object, jobject);   \
CALLBACK_CC op##_FIELD(Boolean, jboolean); \
CALLBACK_CC op##_FIELD(Byte, jbyte);       \
CALLBACK_CC op##_FIELD(Char, jchar);       \
CALLBACK_CC op##_FIELD(Short, jshort);     \
CALLBACK_CC op##_FIELD(Int, jint);         \
CALLBACK_CC op##_FIELD(Long, jlong);       \
CALLBACK_CC op##_FIELD(Float, jfloat);     \
CALLBACK_CC op##_FIELD(Double, jdouble);


struct _JNINativeInterface {
#define JNIEnvType JNIEnv
#define CALLBACK_CC
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    jint (*GetVersion)(JNIEnv *env);

    jclass (*DefineClass)(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len);
    jclass (*FindClass)(JNIEnv *env, const char *name);

    jmethodID (*FromReflectedMethod)(JNIEnv *env, jobject method);
    jfieldID (*FromReflectedField)(JNIEnv *env, jobject field);
    jobject (*ToReflectedMethod)(JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic);

    jclass (*GetSuperclass)(JNIEnv *env, jclass sub);
    jboolean (*IsAssignableFrom)(JNIEnv *env, jclass sub, jclass sup);

    jobject (*ToReflectedField)(JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic);

    jint (*Throw)(JNIEnv *env, jthrowable obj);
    jint (*ThrowNew)(JNIEnv *env, jclass clazz, const char *msg);

    jthrowable (*ExceptionOccurred)(JNIEnv *env);
    void (*ExceptionDescribe)(JNIEnv *env);
    void (*ExceptionClear)(JNIEnv *env);
    void (*FatalError)(JNIEnv *env, const char *msg);

    jint (*PushLocalFrame)(JNIEnv *env, jint capacity);
    jobject (*PopLocalFrame)(JNIEnv *env, jobject result);

    jobject (*NewGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteLocalRef)(JNIEnv *env, jobject obj);
    jboolean (*IsSameObject)(JNIEnv *env, jobject obj1, jobject obj2);

    jobject (*NewLocalRef)(JNIEnv *env, jobject obj);
    jint (*EnsureLocalCapacity)(JNIEnv *env, jint capacity);

    jobject (*AllocObject)(JNIEnv *env, jclass clazz);
    jobject (*NewObject)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject (*NewObjectV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject (*NewObjectA)(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jclass (*GetObjectClass)(JNIEnv *env, jobject obj);
    jboolean (*IsInstanceOf)(JNIEnv *env, jobject obj, jclass clazz);

    jmethodID (*GetMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(VIRTUAL);
    CALL_METHOD(NONVIRTUAL);

    jfieldID (*GetFieldID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET);
    FIELD_OP(SET);

    jmethodID (*GetStaticMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(STATIC);

    jfieldID (*GetStaticFieldID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET_STATIC);
    FIELD_OP(SET_STATIC);

    jstring (*NewString)(JNIEnv *env, const jchar *unicode, jsize len);
    jsize (*GetStringLength)(JNIEnv *env, jstring str);
    const jchar *(*GetStringChars)(JNIEnv *env, jstring str, jboolean *isCopy);
    void (*ReleaseStringChars)(JNIEnv *env, jstring str, const jchar *chars);
  
    jstring (*NewStringUTF)(JNIEnv *env, const char *utf);
    jsize (*GetStringUTFLength)(JNIEnv *env, jstring str);
    const char* (*GetStringUTFChars)(JNIEnv *env, jstring str, jboolean *isCopy);
    void (*ReleaseStringUTFChars)(JNIEnv *env, jstring str, const char* chars);
  
    jsize (*GetArrayLength)(JNIEnv *env, jarray array);

    jobjectArray (*NewObjectArray)(JNIEnv *env, jsize len, jclass clazz, jobject init);
    jobject (*GetObjectArrayElement)(JNIEnv *env, jobjectArray array, jsize index);
    void (*SetObjectArrayElement)(JNIEnv *env, jobjectArray array, jsize index, jobject val);

    PRIM_ARRAY_OP(NEW);
    PRIM_ARRAY_OP(GET_ELEMENTS);
    PRIM_ARRAY_OP(RELEASE_ELEMENTS);
    PRIM_ARRAY_OP(GET_REGION);
    PRIM_ARRAY_OP(SET_REGION);

    jint (*RegisterNatives)(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);
    jint (*UnregisterNatives)(JNIEnv *env, jclass clazz);

    jint (*MonitorEnter)(JNIEnv *env, jobject obj);
    jint (*MonitorExit)(JNIEnv *env, jobject obj);
 
    jint (*GetJavaVM)(JNIEnv *env, JavaVM **vm);

    void (*GetStringRegion)(JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf);
    void (*GetStringUTFRegion)(JNIEnv *env, jstring str, jsize start, jsize len, char *buf);

    void *(*GetPrimitiveArrayCritical)(JNIEnv *env, jarray array, jboolean *isCopy);
    void (*ReleasePrimitiveArrayCritical)(JNIEnv *env, jarray array, void *carray, jint mode);

    const jchar *(*GetStringCritical)(JNIEnv *env, jstring string, jboolean *isCopy);
    void (*ReleaseStringCritical)(JNIEnv *env, jstring string, const jchar *cstring);

    jweak (*NewWeakGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteWeakGlobalRef)(JNIEnv *env, jweak obj);

    jboolean (*ExceptionCheck)(JNIEnv *env);
    jobject (*NewDirectByteBuffer)(JNIEnv *env, void *addr, jlong capacity);
    void* (*GetDirectBufferAddress)(JNIEnv *env, jobject buffer);
    jlong (*GetDirectBufferCapacity)(JNIEnv *env, jobject buffer);
    jobjectRefType (*GetObjectRefType)(JNIEnv *env, jobject obj);
#undef JNIEnvType
#undef CALLBACK_CC
};


struct _JNISandboxedNativeInterface {
#pragma pointer_interpretation push
#pragma pointer_interpretation capability
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

#define JNIEnvType const __capability struct _JNISandboxedNativeInterface*
#define CALLBACK_CC CHERI_CALLBACK
#define jobject       jobject_c
#define jclass        jclass_c
#define jthrowable    jthrowable_c
#define jstring       jstring_c
#define jarray        jarray_c
#define jbooleanArray jbooleanArray_c
#define jbyteArray    jbyteArray_c
#define jcharArray    jcharArray_c
#define jshortArray   jshortArray_c
#define jintArray     jintArray_c
#define jlongArray    jlongArray_c
#define jfloatArray   jfloatArray_c
#define jdoubleArray  jdoubleArray_c
#define jobjectArray  jobjectArray_c
#define jfieldID      jfieldID_c
#define jmethodID     jmethodID_c
#ifndef __CHERI_PURE_CAPABILITY__
#define jvalue        jvalue_c
#endif
    CHERI_CALLBACK jint (*GetVersion)(JNIEnvType *env);

    CHERI_CALLBACK jclass (*DefineClass)(JNIEnvType *env, const char *name, jobject loader, const jbyte *buf, jsize len);
    CHERI_CALLBACK jclass (*FindClass)(JNIEnvType *env, const char *name);

    CHERI_CALLBACK jmethodID (*FromReflectedMethod)(JNIEnvType *env, jobject method);
    CHERI_CALLBACK jfieldID (*FromReflectedField)(JNIEnvType *env, jobject field);
    CHERI_CALLBACK jobject (*ToReflectedMethod)(JNIEnvType *env, jclass cls, jmethodID methodID, jboolean isStatic);

    CHERI_CALLBACK jclass (*GetSuperclass)(JNIEnvType *env, jclass sub);
    CHERI_CALLBACK jboolean (*IsAssignableFrom)(JNIEnvType *env, jclass sub, jclass sup);

    CHERI_CALLBACK jobject (*ToReflectedField)(JNIEnvType *env, jclass cls, jfieldID fieldID, jboolean isStatic);

    CHERI_CALLBACK jint (*Throw)(JNIEnvType *env, jthrowable obj);
    CHERI_CALLBACK jint (*ThrowNew)(JNIEnvType *env, jclass clazz, const char *msg);

    CHERI_CALLBACK jthrowable (*ExceptionOccurred)(JNIEnvType *env);
    CHERI_CALLBACK void (*ExceptionDescribe)(JNIEnvType *env);
    CHERI_CALLBACK void (*ExceptionClear)(JNIEnvType *env);
    CHERI_CALLBACK void (*FatalError)(JNIEnvType *env, const char *msg);

    CHERI_CALLBACK jint (*PushLocalFrame)(JNIEnvType *env, jint capacity);
    CHERI_CALLBACK jobject (*PopLocalFrame)(JNIEnvType *env, jobject result);

    CHERI_CALLBACK jobject (*NewGlobalRef)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK void (*DeleteGlobalRef)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK void (*DeleteLocalRef)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK jboolean (*IsSameObject)(JNIEnvType *env, jobject obj1, jobject obj2);

    CHERI_CALLBACK jobject (*NewLocalRef)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK jint (*EnsureLocalCapacity)(JNIEnvType *env, jint capacity);

    CHERI_CALLBACK jobject (*AllocObject)(JNIEnvType *env, jclass clazz);
    CHERI_CALLBACK jobject (*NewObject)(JNIEnvType *env, jclass clazz, jmethodID methodID, ...);
    CHERI_CALLBACK jobject (*NewObjectV)(JNIEnvType *env, jclass clazz, jmethodID methodID, va_list args);
    CHERI_CALLBACK jobject (*NewObjectA)(JNIEnvType *env, jclass clazz, jmethodID methodID, jvalue *args);

    CHERI_CALLBACK jclass (*GetObjectClass)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK jboolean (*IsInstanceOf)(JNIEnvType *env, jobject obj, jclass clazz);

    CHERI_CALLBACK jmethodID (*GetMethodID)(JNIEnvType *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(VIRTUAL);
    CALL_METHOD(NONVIRTUAL);

    CHERI_CALLBACK jfieldID (*GetFieldID)(JNIEnvType *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET);
    FIELD_OP(SET);

    CHERI_CALLBACK jmethodID (*GetStaticMethodID)(JNIEnvType *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(STATIC);

    CHERI_CALLBACK jfieldID (*GetStaticFieldID)(JNIEnvType *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET_STATIC);
    FIELD_OP(SET_STATIC);

    CHERI_CALLBACK jstring (*NewString)(JNIEnvType *env, const jchar *unicode, jsize len);
    CHERI_CALLBACK jsize (*GetStringLength)(JNIEnvType *env, jstring str);
    CHERI_CALLBACK const jchar *(*GetStringChars)(JNIEnvType *env, jstring str, jboolean *isCopy);
    CHERI_CALLBACK void (*ReleaseStringChars)(JNIEnvType *env, jstring str, const jchar *chars);
  
    CHERI_CALLBACK jstring (*NewStringUTF)(JNIEnvType *env, const char *utf);
    CHERI_CALLBACK jsize (*GetStringUTFLength)(JNIEnvType *env, jstring str);
    CHERI_CALLBACK const char* (*GetStringUTFChars)(JNIEnvType *env, jstring str, jboolean *isCopy);
    CHERI_CALLBACK void (*ReleaseStringUTFChars)(JNIEnvType *env, jstring str, const char* chars);
  
    CHERI_CALLBACK jsize (*GetArrayLength)(JNIEnvType *env, jarray array);

    CHERI_CALLBACK jobjectArray (*NewObjectArray)(JNIEnvType *env, jsize len, jclass clazz, jobject init);
    CHERI_CALLBACK jobject (*GetObjectArrayElement)(JNIEnvType *env, jobjectArray array, jsize index);
    CHERI_CALLBACK void (*SetObjectArrayElement)(JNIEnvType *env, jobjectArray array, jsize index, jobject val);

    PRIM_ARRAY_OP(NEW);
    PRIM_ARRAY_OP(GET_ELEMENTS);
    PRIM_ARRAY_OP(RELEASE_ELEMENTS);
    PRIM_ARRAY_OP(GET_REGION);
    PRIM_ARRAY_OP(SET_REGION);

    CHERI_CALLBACK jint (*RegisterNatives)(JNIEnvType *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);
    CHERI_CALLBACK jint (*UnregisterNatives)(JNIEnvType *env, jclass clazz);

    CHERI_CALLBACK jint (*MonitorEnter)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK jint (*MonitorExit)(JNIEnvType *env, jobject obj);
 
    CHERI_CALLBACK jint (*GetJavaVM)(JNIEnvType *env, JavaVM **vm);

    CHERI_CALLBACK void (*GetStringRegion)(JNIEnvType *env, jstring str, jsize start, jsize len, jchar *buf);
    CHERI_CALLBACK void (*GetStringUTFRegion)(JNIEnvType *env, jstring str, jsize start, jsize len, char *buf);

    CHERI_CALLBACK void *(*GetPrimitiveArrayCritical)(JNIEnvType *env, jarray array, jboolean *isCopy);
    CHERI_CALLBACK void (*ReleasePrimitiveArrayCritical)(JNIEnvType *env, jarray array, void *carray, jint mode);

    CHERI_CALLBACK const jchar *(*GetStringCritical)(JNIEnvType *env, jstring string, jboolean *isCopy);
    CHERI_CALLBACK void (*ReleaseStringCritical)(JNIEnvType *env, jstring string, const jchar *cstring);

    CHERI_CALLBACK jweak (*NewWeakGlobalRef)(JNIEnvType *env, jobject obj);
    CHERI_CALLBACK void (*DeleteWeakGlobalRef)(JNIEnvType *env, jweak obj);

    CHERI_CALLBACK jboolean (*ExceptionCheck)(JNIEnvType *env);
    CHERI_CALLBACK jobject (*NewDirectByteBuffer)(JNIEnvType *env, void *addr, jlong capacity);
    CHERI_CALLBACK void* (*GetDirectBufferAddress)(JNIEnvType *env, jobject buffer);
    CHERI_CALLBACK jlong (*GetDirectBufferCapacity)(JNIEnvType *env, jobject buffer);
    CHERI_CALLBACK jobjectRefType (*GetObjectRefType)(JNIEnvType *env, jobject obj);
#undef jobject
#undef jclass
#undef jthrowable
#undef jstring
#undef jarray
#undef jbooleanArray
#undef jbyteArray
#undef jcharArray
#undef jshortArray
#undef jintArray
#undef jlongArray
#undef jfloatArray
#undef jdoubleArray
#undef jobjectArray
#undef jfieldID
#undef jmethodID
#ifndef __CHERI_PURE_CAPABILITY__
#undef jvalue
#endif
#undef JNIEnvType
#undef CALLBACK_CC
#pragma pointer_interpretation pop
};

struct _JNIInvokeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    jint (*DestroyJavaVM)(JavaVM *vm);
    jint (*AttachCurrentThread)(JavaVM *vm, void **penv, void *args);
    jint (*DetachCurrentThread)(JavaVM *vm);
    jint (*GetEnv)(JavaVM *vm, void **penv, jint version);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args);
};

typedef struct JavaVMOption {
    char *optionString;
    void *extraInfo;
} JavaVMOption;

typedef struct JavaVMInitArgs {
    jint version;
    jint nOptions;
    JavaVMOption *options;
    jboolean ignoreUnrecognized;
} JavaVMInitArgs;

typedef struct JavaVMAttachArgs {
    jint version;
    char *name;
    jobject group;
} JavaVMAttachArgs;

#undef VIRTUAL_METHOD
#undef NONVIRTUAL_METHOD
#undef STATIC_METHOD
#undef VOID_VIRTUAL_METHOD
#undef VOID_NONVIRTUAL_METHOD
#undef VOID_STATIC_METHOD
#undef CALL_METHOD
#undef NEW_PRIM_ARRAY
#undef GET_ELEMENTS_PRIM_ARRAY
#undef RELEASE_ELEMENTS_PRIM_ARRAY
#undef GET_REGION_PRIM_ARRAY
#undef SET_REGION_PRIM_ARRAY
#undef PRIM_ARRAY_OP
#undef GET_FIELD
#undef SET_FIELD
#undef GET_STATIC_FIELD
#undef SET_STATIC_FIELD
#undef FIELD_OP
#endif
