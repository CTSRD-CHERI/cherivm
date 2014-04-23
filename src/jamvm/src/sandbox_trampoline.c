#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#include "jam.h"

#ifdef JNI_CHERI

#include "sandbox.h"
#include "sandbox_internal.h"
#include "sandbox_shared.h"

extern const JNIEnv globalJNIEnv;

#define unseal_obj(ptr)    cherijni_unsealObject(capAt(convPtr(ptr, cap_default)))
#define return_obj(obj)    { *mem_output = cherijni_sealObject(obj); }

static inline void *convPtr(register_t guest_ptr, __capability void *cap_default) {
	// NULL pointer will be zero
	if (guest_ptr == 0)
		return NULL;

	// check it is within bounds
	if (guest_ptr < 0 || guest_ptr >= cheri_getlen(cap_default)) {
		jam_printf("Warning: sandbox gave a pointer outside of its bounds\n");
		return NULL;
	}

	// translate
	return (void*) cheri_incbase(cap_default, guest_ptr);
}

static inline __capability void *capAt(void *ptr) {
	if (((uintptr_t) ptr) & (sizeof(__capability void*) - 1)) {
		jam_printf("Warning: sandbox gave a misaligned capability pointer\n");
		return cheri_zerocap();
	}

	return *((__capability void**) ptr);
}

register_t cherijni_trampoline(register_t methodnum,
                               register_t a1, register_t a2, register_t a3, register_t a4,
                               register_t a5, register_t a6, register_t a7,
                               struct cheri_object system_object,
                               __capability void *cap_default,
                               __capability void *cap_context,
                               __capability void *cap_output,
                               __capability void *c1, __capability void *c2)
                               __attribute__((cheri_ccall)) {

	JNIEnv *env = &globalJNIEnv;

	/*
	 * Convert the output capability to a pointer.
	 * TODO: test it is not sealed, has the right size, etc...
	 */
	__capability void **mem_output = (__capability void **) cap_output;

	/*
	 * The call must provide a context (sealed pClass).
	 * unsealContext will check it is correct (and of correct type),
	 * hence if it returns NULL, a correct context has not been provided
	 */
	const pClass context = cherijni_unsealContext(cap_context);
	if (!context)
		return CHERI_FAIL;

	switch(methodnum) {
	case CHERIJNI_JNIEnv_GetVersion:
		return (*env)->GetVersion(env);
	case CHERIJNI_JNIEnv_DefineClass:
		break;
	case CHERIJNI_JNIEnv_FindClass:	{
			const char *className = (const char*) convPtr(a1, cap_default);
			jclass clazz = (*env)->FindClass(env, className);
			return_obj(clazz);
			return 0;
		} break;
	case CHERIJNI_JNIEnv_FromReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_FromReflectedField:
		break;
	case CHERIJNI_JNIEnv_ToReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_GetSuperclass:
		break;
	case CHERIJNI_JNIEnv_IsAssignableFrom:
		break;
	case CHERIJNI_JNIEnv_ToReflectedField:
		break;
	case CHERIJNI_JNIEnv_Throw:
		break;
	case CHERIJNI_JNIEnv_ThrowNew:
		break;
	case CHERIJNI_JNIEnv_ExceptionOccurred:
		break;
	case CHERIJNI_JNIEnv_ExceptionDescribe:
		break;
	case CHERIJNI_JNIEnv_ExceptionClear:
		break;
	case CHERIJNI_JNIEnv_FatalError:
		break;
	case CHERIJNI_JNIEnv_PushLocalFrame:
		break;
	case CHERIJNI_JNIEnv_PopLocalFrame:
		break;
	case CHERIJNI_JNIEnv_NewGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteLocalRef:
		break;
	case CHERIJNI_JNIEnv_IsSameObject:
		break;
	case CHERIJNI_JNIEnv_NewLocalRef:
		break;
	case CHERIJNI_JNIEnv_EnsureLocalCapacity:
		break;
	case CHERIJNI_JNIEnv_AllocObject:
		break;
	case CHERIJNI_JNIEnv_NewObject:
		break;
	case CHERIJNI_JNIEnv_NewObjectV:
		break;
	case CHERIJNI_JNIEnv_NewObjectA:
		break;
	case CHERIJNI_JNIEnv_GetObjectClass:
		break;
	case CHERIJNI_JNIEnv_IsInstanceOf: {
		jobject obj = unseal_obj(a1);
		jclass clazz = unseal_obj(a2);
		jboolean result = (*env)->IsInstanceOf(env, obj, clazz);
		return (register_t) result;
		} break;
	case CHERIJNI_JNIEnv_GetMethodID:
		break;
	case CHERIJNI_JNIEnv_CallObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_GetFieldID:
		break;
	case CHERIJNI_JNIEnv_GetObjectField:
		break;
	case CHERIJNI_JNIEnv_GetBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetByteField:
		break;
	case CHERIJNI_JNIEnv_GetCharField:
		break;
	case CHERIJNI_JNIEnv_GetShortField:
		break;
	case CHERIJNI_JNIEnv_GetIntField:
		break;
	case CHERIJNI_JNIEnv_GetLongField:
		break;
	case CHERIJNI_JNIEnv_GetFloatField:
		break;
	case CHERIJNI_JNIEnv_GetDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetObjectField:
		break;
	case CHERIJNI_JNIEnv_SetBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetByteField:
		break;
	case CHERIJNI_JNIEnv_SetCharField:
		break;
	case CHERIJNI_JNIEnv_SetShortField:
		break;
	case CHERIJNI_JNIEnv_SetIntField:
		break;
	case CHERIJNI_JNIEnv_SetLongField:
		break;
	case CHERIJNI_JNIEnv_SetFloatField:
		break;
	case CHERIJNI_JNIEnv_SetDoubleField:
		break;
	case CHERIJNI_JNIEnv_GetStaticMethodID:
		break;
	case CHERIJNI_JNIEnv_CallStaticObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_GetStaticFieldID:
		break;
	case CHERIJNI_JNIEnv_GetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_GetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_GetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_GetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_GetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_GetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_GetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_GetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_SetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_SetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_SetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_SetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_SetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_SetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_SetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_NewString:
		break;
	case CHERIJNI_JNIEnv_GetStringLength:
		break;
	case CHERIJNI_JNIEnv_GetStringChars:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringChars:
		break;
	case CHERIJNI_JNIEnv_NewStringUTF:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFLength:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFChars:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringUTFChars:
		break;
	case CHERIJNI_JNIEnv_GetArrayLength:
		break;
	case CHERIJNI_JNIEnv_NewObjectArray:
		break;
	case CHERIJNI_JNIEnv_GetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_SetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_NewBooleanArray:
		break;
	case CHERIJNI_JNIEnv_NewByteArray:
		break;
	case CHERIJNI_JNIEnv_NewCharArray:
		break;
	case CHERIJNI_JNIEnv_NewShortArray:
		break;
	case CHERIJNI_JNIEnv_NewIntArray:
		break;
	case CHERIJNI_JNIEnv_NewLongArray:
		break;
	case CHERIJNI_JNIEnv_NewFloatArray:
		break;
	case CHERIJNI_JNIEnv_NewDoubleArray:
		break;
	case CHERIJNI_JNIEnv_GetBooleanArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetByteArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetCharArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetShortArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetIntArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetLongArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetFloatArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetDoubleArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseBooleanArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseByteArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseCharArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseShortArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseIntArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseLongArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseFloatArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseDoubleArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetBooleanArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetByteArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetCharArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetShortArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetIntArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetLongArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetFloatArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetDoubleArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetBooleanArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetByteArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetCharArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetShortArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetIntArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetLongArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetFloatArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetDoubleArrayRegion:
		break;
	case CHERIJNI_JNIEnv_RegisterNatives:
		break;
	case CHERIJNI_JNIEnv_UnregisterNatives:
		break;
	case CHERIJNI_JNIEnv_MonitorEnter:
		break;
	case CHERIJNI_JNIEnv_MonitorExit:
		break;
	case CHERIJNI_JNIEnv_GetJavaVM:
		break;
	case CHERIJNI_JNIEnv_GetStringRegion:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFRegion:
		break;
	case CHERIJNI_JNIEnv_GetPrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_ReleasePrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_GetStringCritical:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringCritical:
		break;
	case CHERIJNI_JNIEnv_NewWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_ExceptionCheck:
		break;
	case CHERIJNI_JNIEnv_NewDirectByteBuffer:
		break;
	case CHERIJNI_JNIEnv_GetDirectBufferAddress:
		break;
	case CHERIJNI_JNIEnv_GetDirectBufferCapacity:
		break;
	case CHERIJNI_JNIEnv_GetObjectRefType:
		break;
	}

	return CHERI_FAIL;
}

#endif
