#ifndef __SANDBOX_SHARED_H__
#define __SANDBOX_SHARED_H__

#include <cheri/cheri_enter.h>
#include <machine/cheric.h>

#define CNULL   ((__capability void*) 0)

#define	CHERI_CAP_PRINT(cap) do {					\
	printf("tag %ju u %ju perms %08jx type %016jx\n",		\
	    (uintptr_t)cheri_gettag(cap),				\
	    (uintptr_t)cheri_getunsealed(cap),				\
	    (uintptr_t)cheri_getperm(cap),				\
	    (uintptr_t)cheri_gettype(cap));				\
	printf("\tbase %016jx length %016jx\n",				\
	    (uintptr_t)cheri_getbase(cap),				\
	    (uintptr_t)cheri_getlen(cap));				\
} while (0)

#define cap_string(str)      (cheri_ptrperm((void*) str, strlen(str) + 1, CHERI_PERM_LOAD))

typedef register_t (*cheri_invoke_prim)(struct cheri_object, u_int, register_t, register_t, register_t, register_t, register_t, register_t, register_t, __capability void *, __capability void *, __capability void *, __capability void *,   __capability void *,__capability void *, __capability void *, __capability void *) __attribute__((cheri_ccall));
typedef __capability void* (*cheri_invoke_cap)(struct cheri_object, u_int, register_t, register_t, register_t, register_t, register_t, register_t, register_t, __capability void *, __capability void *, __capability void *, __capability void *,   __capability void *,__capability void *, __capability void *, __capability void *) __attribute__((cheri_ccall));

#define scanSignature(sig, ARG_PRIM_SINGLE, ARG_PRIM_DOUBLE, ARG_OBJECT, RET_VOID, RET_PRIM_SINGLE, RET_PRIM_DOUBLE, RET_OBJECT) \
{                                         \
	const char *s = sig;                  \
	char c = s[1];                        \
	while (c != ')') {                    \
		if (c == 'D' || c == 'J')         \
			ARG_PRIM_DOUBLE               \
		else {                            \
			if (c == 'L' || c == '[')     \
				ARG_OBJECT                \
			else                          \
				ARG_PRIM_SINGLE           \
			                              \
			while (c == '[')              \
				c = (++s)[1];             \
			if (c == 'L')                 \
				while (c != ';')          \
					c = (++s)[1];         \
		}                                 \
		c = (++s)[1];                     \
	}                                     \
	c = (++s)[1];                         \
	if (c == 'V')                         \
        RET_VOID                          \
	else if (c == 'D' || c == 'J')        \
		RET_PRIM_DOUBLE                   \
	else if (c == 'L' || c == '[')        \
		RET_OBJECT                        \
	else                                  \
		RET_PRIM_SINGLE                   \
}

#define CHERI_FAIL                           (-1)
#define CHERI_SUCCESS                        0

#define CHERIJNI_METHOD_INIT                 0
#define CHERIJNI_METHOD_LOOKUP               1
#define CHERIJNI_METHOD_ONLOAD_ONUNLOAD      2
#define CHERIJNI_METHOD_RUN                  3
// #define CHERIJNI_METHOD_TEST                 4

#define CHERIJNI_JNIEnv_GetVersion			(CHERI_SYSTEM_USER_BASE + 1)
#define CHERIJNI_JNIEnv_DefineClass			(CHERI_SYSTEM_USER_BASE + 2)
#define CHERIJNI_JNIEnv_FindClass			(CHERI_SYSTEM_USER_BASE + 3)
#define CHERIJNI_JNIEnv_FromReflectedMethod			(CHERI_SYSTEM_USER_BASE + 4)
#define CHERIJNI_JNIEnv_FromReflectedField			(CHERI_SYSTEM_USER_BASE + 5)
#define CHERIJNI_JNIEnv_ToReflectedMethod			(CHERI_SYSTEM_USER_BASE + 6)
#define CHERIJNI_JNIEnv_GetSuperclass			(CHERI_SYSTEM_USER_BASE + 7)
#define CHERIJNI_JNIEnv_IsAssignableFrom			(CHERI_SYSTEM_USER_BASE + 8)
#define CHERIJNI_JNIEnv_ToReflectedField			(CHERI_SYSTEM_USER_BASE + 9)
#define CHERIJNI_JNIEnv_Throw			(CHERI_SYSTEM_USER_BASE + 10)
#define CHERIJNI_JNIEnv_ThrowNew			(CHERI_SYSTEM_USER_BASE + 11)
#define CHERIJNI_JNIEnv_ExceptionOccurred			(CHERI_SYSTEM_USER_BASE + 12)
#define CHERIJNI_JNIEnv_ExceptionDescribe			(CHERI_SYSTEM_USER_BASE + 13)
#define CHERIJNI_JNIEnv_ExceptionClear			(CHERI_SYSTEM_USER_BASE + 14)
#define CHERIJNI_JNIEnv_FatalError			(CHERI_SYSTEM_USER_BASE + 15)
#define CHERIJNI_JNIEnv_PushLocalFrame			(CHERI_SYSTEM_USER_BASE + 16)
#define CHERIJNI_JNIEnv_PopLocalFrame			(CHERI_SYSTEM_USER_BASE + 17)
#define CHERIJNI_JNIEnv_NewGlobalRef			(CHERI_SYSTEM_USER_BASE + 18)
#define CHERIJNI_JNIEnv_DeleteGlobalRef			(CHERI_SYSTEM_USER_BASE + 19)
#define CHERIJNI_JNIEnv_DeleteLocalRef			(CHERI_SYSTEM_USER_BASE + 20)
#define CHERIJNI_JNIEnv_IsSameObject			(CHERI_SYSTEM_USER_BASE + 21)
#define CHERIJNI_JNIEnv_NewLocalRef			(CHERI_SYSTEM_USER_BASE + 22)
#define CHERIJNI_JNIEnv_EnsureLocalCapacity			(CHERI_SYSTEM_USER_BASE + 23)
#define CHERIJNI_JNIEnv_AllocObject			(CHERI_SYSTEM_USER_BASE + 24)
#define CHERIJNI_JNIEnv_NewObject			(CHERI_SYSTEM_USER_BASE + 25)
//#define CHERIJNI_JNIEnv_NewObjectV			(CHERI_SYSTEM_USER_BASE + 26)
//#define CHERIJNI_JNIEnv_NewObjectA			(CHERI_SYSTEM_USER_BASE + 27)
#define CHERIJNI_JNIEnv_GetObjectClass			(CHERI_SYSTEM_USER_BASE + 28)
#define CHERIJNI_JNIEnv_IsInstanceOf			(CHERI_SYSTEM_USER_BASE + 29)
#define CHERIJNI_JNIEnv_GetMethodID			(CHERI_SYSTEM_USER_BASE + 30)
#define CHERIJNI_JNIEnv_CallObjectMethod			(CHERI_SYSTEM_USER_BASE + 31)
//#define CHERIJNI_JNIEnv_CallObjectMethodV			(CHERI_SYSTEM_USER_BASE + 32)
//#define CHERIJNI_JNIEnv_CallObjectMethodA			(CHERI_SYSTEM_USER_BASE + 33)
#define CHERIJNI_JNIEnv_CallBooleanMethod			(CHERI_SYSTEM_USER_BASE + 34)
//#define CHERIJNI_JNIEnv_CallBooleanMethodV			(CHERI_SYSTEM_USER_BASE + 35)
//#define CHERIJNI_JNIEnv_CallBooleanMethodA			(CHERI_SYSTEM_USER_BASE + 36)
#define CHERIJNI_JNIEnv_CallByteMethod			(CHERI_SYSTEM_USER_BASE + 37)
//#define CHERIJNI_JNIEnv_CallByteMethodV			(CHERI_SYSTEM_USER_BASE + 38)
//#define CHERIJNI_JNIEnv_CallByteMethodA			(CHERI_SYSTEM_USER_BASE + 39)
#define CHERIJNI_JNIEnv_CallCharMethod			(CHERI_SYSTEM_USER_BASE + 40)
//#define CHERIJNI_JNIEnv_CallCharMethodV			(CHERI_SYSTEM_USER_BASE + 41)
//#define CHERIJNI_JNIEnv_CallCharMethodA			(CHERI_SYSTEM_USER_BASE + 42)
#define CHERIJNI_JNIEnv_CallShortMethod			(CHERI_SYSTEM_USER_BASE + 43)
//#define CHERIJNI_JNIEnv_CallShortMethodV			(CHERI_SYSTEM_USER_BASE + 44)
//#define CHERIJNI_JNIEnv_CallShortMethodA			(CHERI_SYSTEM_USER_BASE + 45)
#define CHERIJNI_JNIEnv_CallIntMethod			(CHERI_SYSTEM_USER_BASE + 46)
//#define CHERIJNI_JNIEnv_CallIntMethodV			(CHERI_SYSTEM_USER_BASE + 47)
//#define CHERIJNI_JNIEnv_CallIntMethodA			(CHERI_SYSTEM_USER_BASE + 48)
#define CHERIJNI_JNIEnv_CallLongMethod			(CHERI_SYSTEM_USER_BASE + 49)
//#define CHERIJNI_JNIEnv_CallLongMethodV			(CHERI_SYSTEM_USER_BASE + 50)
//#define CHERIJNI_JNIEnv_CallLongMethodA			(CHERI_SYSTEM_USER_BASE + 51)
#define CHERIJNI_JNIEnv_CallFloatMethod			(CHERI_SYSTEM_USER_BASE + 52)
//#define CHERIJNI_JNIEnv_CallFloatMethodV			(CHERI_SYSTEM_USER_BASE + 53)
//#define CHERIJNI_JNIEnv_CallFloatMethodA			(CHERI_SYSTEM_USER_BASE + 54)
#define CHERIJNI_JNIEnv_CallDoubleMethod			(CHERI_SYSTEM_USER_BASE + 55)
//#define CHERIJNI_JNIEnv_CallDoubleMethodV			(CHERI_SYSTEM_USER_BASE + 56)
//#define CHERIJNI_JNIEnv_CallDoubleMethodA			(CHERI_SYSTEM_USER_BASE + 57)
#define CHERIJNI_JNIEnv_CallVoidMethod			(CHERI_SYSTEM_USER_BASE + 58)
//#define CHERIJNI_JNIEnv_CallVoidMethodV			(CHERI_SYSTEM_USER_BASE + 59)
//#define CHERIJNI_JNIEnv_CallVoidMethodA			(CHERI_SYSTEM_USER_BASE + 60)
#define CHERIJNI_JNIEnv_CallNonvirtualObjectMethod			(CHERI_SYSTEM_USER_BASE + 61)
//#define CHERIJNI_JNIEnv_CallNonvirtualObjectMethodV			(CHERI_SYSTEM_USER_BASE + 62)
//#define CHERIJNI_JNIEnv_CallNonvirtualObjectMethodA			(CHERI_SYSTEM_USER_BASE + 63)
#define CHERIJNI_JNIEnv_CallNonvirtualBooleanMethod			(CHERI_SYSTEM_USER_BASE + 64)
//#define CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodV			(CHERI_SYSTEM_USER_BASE + 65)
//#define CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodA			(CHERI_SYSTEM_USER_BASE + 66)
#define CHERIJNI_JNIEnv_CallNonvirtualByteMethod			(CHERI_SYSTEM_USER_BASE + 67)
//#define CHERIJNI_JNIEnv_CallNonvirtualByteMethodV			(CHERI_SYSTEM_USER_BASE + 68)
//#define CHERIJNI_JNIEnv_CallNonvirtualByteMethodA			(CHERI_SYSTEM_USER_BASE + 69)
#define CHERIJNI_JNIEnv_CallNonvirtualCharMethod			(CHERI_SYSTEM_USER_BASE + 70)
//#define CHERIJNI_JNIEnv_CallNonvirtualCharMethodV			(CHERI_SYSTEM_USER_BASE + 71)
//#define CHERIJNI_JNIEnv_CallNonvirtualCharMethodA			(CHERI_SYSTEM_USER_BASE + 72)
#define CHERIJNI_JNIEnv_CallNonvirtualShortMethod			(CHERI_SYSTEM_USER_BASE + 73)
//#define CHERIJNI_JNIEnv_CallNonvirtualShortMethodV			(CHERI_SYSTEM_USER_BASE + 74)
//#define CHERIJNI_JNIEnv_CallNonvirtualShortMethodA			(CHERI_SYSTEM_USER_BASE + 75)
#define CHERIJNI_JNIEnv_CallNonvirtualIntMethod			(CHERI_SYSTEM_USER_BASE + 76)
//#define CHERIJNI_JNIEnv_CallNonvirtualIntMethodV			(CHERI_SYSTEM_USER_BASE + 77)
//#define CHERIJNI_JNIEnv_CallNonvirtualIntMethodA			(CHERI_SYSTEM_USER_BASE + 78)
#define CHERIJNI_JNIEnv_CallNonvirtualLongMethod			(CHERI_SYSTEM_USER_BASE + 79)
//#define CHERIJNI_JNIEnv_CallNonvirtualLongMethodV			(CHERI_SYSTEM_USER_BASE + 80)
//#define CHERIJNI_JNIEnv_CallNonvirtualLongMethodA			(CHERI_SYSTEM_USER_BASE + 81)
#define CHERIJNI_JNIEnv_CallNonvirtualFloatMethod			(CHERI_SYSTEM_USER_BASE + 82)
//#define CHERIJNI_JNIEnv_CallNonvirtualFloatMethodV			(CHERI_SYSTEM_USER_BASE + 83)
//#define CHERIJNI_JNIEnv_CallNonvirtualFloatMethodA			(CHERI_SYSTEM_USER_BASE + 84)
#define CHERIJNI_JNIEnv_CallNonvirtualDoubleMethod			(CHERI_SYSTEM_USER_BASE + 85)
//#define CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodV			(CHERI_SYSTEM_USER_BASE + 86)
//#define CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodA			(CHERI_SYSTEM_USER_BASE + 87)
#define CHERIJNI_JNIEnv_CallNonvirtualVoidMethod			(CHERI_SYSTEM_USER_BASE + 88)
//#define CHERIJNI_JNIEnv_CallNonvirtualVoidMethodV			(CHERI_SYSTEM_USER_BASE + 89)
//#define CHERIJNI_JNIEnv_CallNonvirtualVoidMethodA			(CHERI_SYSTEM_USER_BASE + 90)
#define CHERIJNI_JNIEnv_GetFieldID			(CHERI_SYSTEM_USER_BASE + 91)
#define CHERIJNI_JNIEnv_GetObjectField			(CHERI_SYSTEM_USER_BASE + 92)
#define CHERIJNI_JNIEnv_GetBooleanField			(CHERI_SYSTEM_USER_BASE + 93)
#define CHERIJNI_JNIEnv_GetByteField			(CHERI_SYSTEM_USER_BASE + 94)
#define CHERIJNI_JNIEnv_GetCharField			(CHERI_SYSTEM_USER_BASE + 95)
#define CHERIJNI_JNIEnv_GetShortField			(CHERI_SYSTEM_USER_BASE + 96)
#define CHERIJNI_JNIEnv_GetIntField			(CHERI_SYSTEM_USER_BASE + 97)
#define CHERIJNI_JNIEnv_GetLongField			(CHERI_SYSTEM_USER_BASE + 98)
#define CHERIJNI_JNIEnv_GetFloatField			(CHERI_SYSTEM_USER_BASE + 99)
#define CHERIJNI_JNIEnv_GetDoubleField			(CHERI_SYSTEM_USER_BASE + 100)
#define CHERIJNI_JNIEnv_SetObjectField			(CHERI_SYSTEM_USER_BASE + 101)
#define CHERIJNI_JNIEnv_SetBooleanField			(CHERI_SYSTEM_USER_BASE + 102)
#define CHERIJNI_JNIEnv_SetByteField			(CHERI_SYSTEM_USER_BASE + 103)
#define CHERIJNI_JNIEnv_SetCharField			(CHERI_SYSTEM_USER_BASE + 104)
#define CHERIJNI_JNIEnv_SetShortField			(CHERI_SYSTEM_USER_BASE + 105)
#define CHERIJNI_JNIEnv_SetIntField			(CHERI_SYSTEM_USER_BASE + 106)
#define CHERIJNI_JNIEnv_SetLongField			(CHERI_SYSTEM_USER_BASE + 107)
#define CHERIJNI_JNIEnv_SetFloatField			(CHERI_SYSTEM_USER_BASE + 108)
#define CHERIJNI_JNIEnv_SetDoubleField			(CHERI_SYSTEM_USER_BASE + 109)
#define CHERIJNI_JNIEnv_GetStaticMethodID			(CHERI_SYSTEM_USER_BASE + 110)
#define CHERIJNI_JNIEnv_CallStaticObjectMethod			(CHERI_SYSTEM_USER_BASE + 111)
//#define CHERIJNI_JNIEnv_CallStaticObjectMethodV			(CHERI_SYSTEM_USER_BASE + 112)
//#define CHERIJNI_JNIEnv_CallStaticObjectMethodA			(CHERI_SYSTEM_USER_BASE + 113)
#define CHERIJNI_JNIEnv_CallStaticBooleanMethod			(CHERI_SYSTEM_USER_BASE + 114)
//#define CHERIJNI_JNIEnv_CallStaticBooleanMethodV			(CHERI_SYSTEM_USER_BASE + 115)
//#define CHERIJNI_JNIEnv_CallStaticBooleanMethodA			(CHERI_SYSTEM_USER_BASE + 116)
#define CHERIJNI_JNIEnv_CallStaticByteMethod			(CHERI_SYSTEM_USER_BASE + 117)
//#define CHERIJNI_JNIEnv_CallStaticByteMethodV			(CHERI_SYSTEM_USER_BASE + 118)
//#define CHERIJNI_JNIEnv_CallStaticByteMethodA			(CHERI_SYSTEM_USER_BASE + 119)
#define CHERIJNI_JNIEnv_CallStaticCharMethod			(CHERI_SYSTEM_USER_BASE + 120)
//#define CHERIJNI_JNIEnv_CallStaticCharMethodV			(CHERI_SYSTEM_USER_BASE + 121)
//#define CHERIJNI_JNIEnv_CallStaticCharMethodA			(CHERI_SYSTEM_USER_BASE + 122)
#define CHERIJNI_JNIEnv_CallStaticShortMethod			(CHERI_SYSTEM_USER_BASE + 123)
//#define CHERIJNI_JNIEnv_CallStaticShortMethodV			(CHERI_SYSTEM_USER_BASE + 124)
//#define CHERIJNI_JNIEnv_CallStaticShortMethodA			(CHERI_SYSTEM_USER_BASE + 125)
#define CHERIJNI_JNIEnv_CallStaticIntMethod			(CHERI_SYSTEM_USER_BASE + 126)
//#define CHERIJNI_JNIEnv_CallStaticIntMethodV			(CHERI_SYSTEM_USER_BASE + 127)
//#define CHERIJNI_JNIEnv_CallStaticIntMethodA			(CHERI_SYSTEM_USER_BASE + 128)
#define CHERIJNI_JNIEnv_CallStaticLongMethod			(CHERI_SYSTEM_USER_BASE + 129)
//#define CHERIJNI_JNIEnv_CallStaticLongMethodV			(CHERI_SYSTEM_USER_BASE + 130)
//#define CHERIJNI_JNIEnv_CallStaticLongMethodA			(CHERI_SYSTEM_USER_BASE + 131)
#define CHERIJNI_JNIEnv_CallStaticFloatMethod			(CHERI_SYSTEM_USER_BASE + 132)
//#define CHERIJNI_JNIEnv_CallStaticFloatMethodV			(CHERI_SYSTEM_USER_BASE + 133)
//#define CHERIJNI_JNIEnv_CallStaticFloatMethodA			(CHERI_SYSTEM_USER_BASE + 134)
#define CHERIJNI_JNIEnv_CallStaticDoubleMethod			(CHERI_SYSTEM_USER_BASE + 135)
//#define CHERIJNI_JNIEnv_CallStaticDoubleMethodV			(CHERI_SYSTEM_USER_BASE + 136)
//#define CHERIJNI_JNIEnv_CallStaticDoubleMethodA			(CHERI_SYSTEM_USER_BASE + 137)
#define CHERIJNI_JNIEnv_CallStaticVoidMethod			(CHERI_SYSTEM_USER_BASE + 138)
//#define CHERIJNI_JNIEnv_CallStaticVoidMethodV			(CHERI_SYSTEM_USER_BASE + 139)
//#define CHERIJNI_JNIEnv_CallStaticVoidMethodA			(CHERI_SYSTEM_USER_BASE + 140)
#define CHERIJNI_JNIEnv_GetStaticFieldID			(CHERI_SYSTEM_USER_BASE + 141)
#define CHERIJNI_JNIEnv_GetStaticObjectField			(CHERI_SYSTEM_USER_BASE + 142)
#define CHERIJNI_JNIEnv_GetStaticBooleanField			(CHERI_SYSTEM_USER_BASE + 143)
#define CHERIJNI_JNIEnv_GetStaticByteField			(CHERI_SYSTEM_USER_BASE + 144)
#define CHERIJNI_JNIEnv_GetStaticCharField			(CHERI_SYSTEM_USER_BASE + 145)
#define CHERIJNI_JNIEnv_GetStaticShortField			(CHERI_SYSTEM_USER_BASE + 146)
#define CHERIJNI_JNIEnv_GetStaticIntField			(CHERI_SYSTEM_USER_BASE + 147)
#define CHERIJNI_JNIEnv_GetStaticLongField			(CHERI_SYSTEM_USER_BASE + 148)
#define CHERIJNI_JNIEnv_GetStaticFloatField			(CHERI_SYSTEM_USER_BASE + 149)
#define CHERIJNI_JNIEnv_GetStaticDoubleField			(CHERI_SYSTEM_USER_BASE + 150)
#define CHERIJNI_JNIEnv_SetStaticObjectField			(CHERI_SYSTEM_USER_BASE + 151)
#define CHERIJNI_JNIEnv_SetStaticBooleanField			(CHERI_SYSTEM_USER_BASE + 152)
#define CHERIJNI_JNIEnv_SetStaticByteField			(CHERI_SYSTEM_USER_BASE + 153)
#define CHERIJNI_JNIEnv_SetStaticCharField			(CHERI_SYSTEM_USER_BASE + 154)
#define CHERIJNI_JNIEnv_SetStaticShortField			(CHERI_SYSTEM_USER_BASE + 155)
#define CHERIJNI_JNIEnv_SetStaticIntField			(CHERI_SYSTEM_USER_BASE + 156)
#define CHERIJNI_JNIEnv_SetStaticLongField			(CHERI_SYSTEM_USER_BASE + 157)
#define CHERIJNI_JNIEnv_SetStaticFloatField			(CHERI_SYSTEM_USER_BASE + 158)
#define CHERIJNI_JNIEnv_SetStaticDoubleField			(CHERI_SYSTEM_USER_BASE + 159)
#define CHERIJNI_JNIEnv_NewString			(CHERI_SYSTEM_USER_BASE + 160)
#define CHERIJNI_JNIEnv_GetStringLength			(CHERI_SYSTEM_USER_BASE + 161)
#define CHERIJNI_JNIEnv_GetStringChars			(CHERI_SYSTEM_USER_BASE + 162)
// #define CHERIJNI_JNIEnv_ReleaseStringChars			(CHERI_SYSTEM_USER_BASE + 163)
#define CHERIJNI_JNIEnv_NewStringUTF			(CHERI_SYSTEM_USER_BASE + 164)
#define CHERIJNI_JNIEnv_GetStringUTFLength			(CHERI_SYSTEM_USER_BASE + 165)
#define CHERIJNI_JNIEnv_GetStringUTFChars			(CHERI_SYSTEM_USER_BASE + 166)
// #define CHERIJNI_JNIEnv_ReleaseStringUTFChars			(CHERI_SYSTEM_USER_BASE + 167)
#define CHERIJNI_JNIEnv_GetArrayLength			(CHERI_SYSTEM_USER_BASE + 168)
#define CHERIJNI_JNIEnv_NewObjectArray			(CHERI_SYSTEM_USER_BASE + 169)
#define CHERIJNI_JNIEnv_GetObjectArrayElement			(CHERI_SYSTEM_USER_BASE + 170)
#define CHERIJNI_JNIEnv_SetObjectArrayElement			(CHERI_SYSTEM_USER_BASE + 171)
#define CHERIJNI_JNIEnv_NewBooleanArray			(CHERI_SYSTEM_USER_BASE + 172)
#define CHERIJNI_JNIEnv_NewByteArray			(CHERI_SYSTEM_USER_BASE + 173)
#define CHERIJNI_JNIEnv_NewCharArray			(CHERI_SYSTEM_USER_BASE + 174)
#define CHERIJNI_JNIEnv_NewShortArray			(CHERI_SYSTEM_USER_BASE + 175)
#define CHERIJNI_JNIEnv_NewIntArray			(CHERI_SYSTEM_USER_BASE + 176)
#define CHERIJNI_JNIEnv_NewLongArray			(CHERI_SYSTEM_USER_BASE + 177)
#define CHERIJNI_JNIEnv_NewFloatArray			(CHERI_SYSTEM_USER_BASE + 178)
#define CHERIJNI_JNIEnv_NewDoubleArray			(CHERI_SYSTEM_USER_BASE + 179)
#define CHERIJNI_JNIEnv_GetBooleanArrayElements			(CHERI_SYSTEM_USER_BASE + 180)
#define CHERIJNI_JNIEnv_GetByteArrayElements			(CHERI_SYSTEM_USER_BASE + 181)
#define CHERIJNI_JNIEnv_GetCharArrayElements			(CHERI_SYSTEM_USER_BASE + 182)
#define CHERIJNI_JNIEnv_GetShortArrayElements			(CHERI_SYSTEM_USER_BASE + 183)
#define CHERIJNI_JNIEnv_GetIntArrayElements			(CHERI_SYSTEM_USER_BASE + 184)
#define CHERIJNI_JNIEnv_GetLongArrayElements			(CHERI_SYSTEM_USER_BASE + 185)
#define CHERIJNI_JNIEnv_GetFloatArrayElements			(CHERI_SYSTEM_USER_BASE + 186)
#define CHERIJNI_JNIEnv_GetDoubleArrayElements			(CHERI_SYSTEM_USER_BASE + 187)
//#define CHERIJNI_JNIEnv_ReleaseBooleanArrayElements			(CHERI_SYSTEM_USER_BASE + 188)
//#define CHERIJNI_JNIEnv_ReleaseByteArrayElements			(CHERI_SYSTEM_USER_BASE + 189)
//#define CHERIJNI_JNIEnv_ReleaseCharArrayElements			(CHERI_SYSTEM_USER_BASE + 190)
//#define CHERIJNI_JNIEnv_ReleaseShortArrayElements			(CHERI_SYSTEM_USER_BASE + 191)
//#define CHERIJNI_JNIEnv_ReleaseIntArrayElements			(CHERI_SYSTEM_USER_BASE + 192)
//#define CHERIJNI_JNIEnv_ReleaseLongArrayElements			(CHERI_SYSTEM_USER_BASE + 193)
//#define CHERIJNI_JNIEnv_ReleaseFloatArrayElements			(CHERI_SYSTEM_USER_BASE + 194)
//#define CHERIJNI_JNIEnv_ReleaseDoubleArrayElements			(CHERI_SYSTEM_USER_BASE + 195)
#define CHERIJNI_JNIEnv_GetBooleanArrayRegion			(CHERI_SYSTEM_USER_BASE + 196)
#define CHERIJNI_JNIEnv_GetByteArrayRegion			(CHERI_SYSTEM_USER_BASE + 197)
#define CHERIJNI_JNIEnv_GetCharArrayRegion			(CHERI_SYSTEM_USER_BASE + 198)
#define CHERIJNI_JNIEnv_GetShortArrayRegion			(CHERI_SYSTEM_USER_BASE + 199)
#define CHERIJNI_JNIEnv_GetIntArrayRegion			(CHERI_SYSTEM_USER_BASE + 200)
#define CHERIJNI_JNIEnv_GetLongArrayRegion			(CHERI_SYSTEM_USER_BASE + 201)
#define CHERIJNI_JNIEnv_GetFloatArrayRegion			(CHERI_SYSTEM_USER_BASE + 202)
#define CHERIJNI_JNIEnv_GetDoubleArrayRegion			(CHERI_SYSTEM_USER_BASE + 203)
#define CHERIJNI_JNIEnv_SetBooleanArrayRegion			(CHERI_SYSTEM_USER_BASE + 204)
#define CHERIJNI_JNIEnv_SetByteArrayRegion			(CHERI_SYSTEM_USER_BASE + 205)
#define CHERIJNI_JNIEnv_SetCharArrayRegion			(CHERI_SYSTEM_USER_BASE + 206)
#define CHERIJNI_JNIEnv_SetShortArrayRegion			(CHERI_SYSTEM_USER_BASE + 207)
#define CHERIJNI_JNIEnv_SetIntArrayRegion			(CHERI_SYSTEM_USER_BASE + 208)
#define CHERIJNI_JNIEnv_SetLongArrayRegion			(CHERI_SYSTEM_USER_BASE + 209)
#define CHERIJNI_JNIEnv_SetFloatArrayRegion			(CHERI_SYSTEM_USER_BASE + 210)
#define CHERIJNI_JNIEnv_SetDoubleArrayRegion			(CHERI_SYSTEM_USER_BASE + 211)
#define CHERIJNI_JNIEnv_RegisterNatives			(CHERI_SYSTEM_USER_BASE + 212)
#define CHERIJNI_JNIEnv_UnregisterNatives			(CHERI_SYSTEM_USER_BASE + 213)
#define CHERIJNI_JNIEnv_MonitorEnter			(CHERI_SYSTEM_USER_BASE + 214)
#define CHERIJNI_JNIEnv_MonitorExit			(CHERI_SYSTEM_USER_BASE + 215)
#define CHERIJNI_JNIEnv_GetJavaVM			(CHERI_SYSTEM_USER_BASE + 216)
#define CHERIJNI_JNIEnv_GetStringRegion			(CHERI_SYSTEM_USER_BASE + 217)
#define CHERIJNI_JNIEnv_GetStringUTFRegion			(CHERI_SYSTEM_USER_BASE + 218)
#define CHERIJNI_JNIEnv_GetPrimitiveArrayCritical			(CHERI_SYSTEM_USER_BASE + 219)
#define CHERIJNI_JNIEnv_ReleasePrimitiveArrayCritical			(CHERI_SYSTEM_USER_BASE + 220)
#define CHERIJNI_JNIEnv_GetStringCritical			(CHERI_SYSTEM_USER_BASE + 221)
#define CHERIJNI_JNIEnv_ReleaseStringCritical			(CHERI_SYSTEM_USER_BASE + 222)
#define CHERIJNI_JNIEnv_NewWeakGlobalRef			(CHERI_SYSTEM_USER_BASE + 223)
#define CHERIJNI_JNIEnv_DeleteWeakGlobalRef			(CHERI_SYSTEM_USER_BASE + 224)
#define CHERIJNI_JNIEnv_ExceptionCheck			(CHERI_SYSTEM_USER_BASE + 225)
#define CHERIJNI_JNIEnv_NewDirectByteBuffer			(CHERI_SYSTEM_USER_BASE + 226)
#define CHERIJNI_JNIEnv_GetDirectBufferAddress			(CHERI_SYSTEM_USER_BASE + 227)
#define CHERIJNI_JNIEnv_GetDirectBufferCapacity			(CHERI_SYSTEM_USER_BASE + 228)
#define CHERIJNI_JNIEnv_GetObjectRefType			(CHERI_SYSTEM_USER_BASE + 229)

#define CHERIJNI_LIBC_GetStdinFD					(CHERI_SYSTEM_USER_BASE + 501)
#define CHERIJNI_LIBC_GetStdoutFD					(CHERI_SYSTEM_USER_BASE + 502)
#define CHERIJNI_LIBC_GetStderrFD					(CHERI_SYSTEM_USER_BASE + 503)
#define CHERIJNI_LIBC_GetStream						(CHERI_SYSTEM_USER_BASE + 504)
#define CHERIJNI_LIBC_write							(CHERI_SYSTEM_USER_BASE + 505)

#endif //__SANDBOX_SHARED_H__
