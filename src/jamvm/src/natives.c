/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009
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

#ifdef NO_JNI
#error to use classpath, Jam must be compiled with JNI!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "jam.h"
#include "alloc.h"
#include "thread.h"
#include "lock.h"
#include "natives.h"
#include "symbol.h"
#include "excep.h"
#include "reflect.h"

static int pd_offset;

void initialiseNatives() {
    pFieldBlock pd = findField(java_lang_Class, SYMBOL(pd),
                               SYMBOL(sig_java_security_ProtectionDomain));

    if(pd == NULL) {
        jam_fprintf(stderr, "Error initialising VM (initialiseNatives)\n");
        exitVM(1);
    }
    pd_offset = pd->u.offset;
}

/* java.lang.VMObject */

uintptr_t *getClass(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject ob = (pObject)*ostack;
    *ostack++ = (uintptr_t)ob->class;
    return ostack;
}

uintptr_t *jamClone(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject ob = (pObject)*ostack;
    *ostack++ = (uintptr_t)cloneObject(ob);
    return ostack;
}

/* static method wait(Ljava/lang/Object;JI)V */
uintptr_t *jamWait(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject obj = (pObject )ostack[0];
    long long ms = *((long long *)&ostack[1]);
    int ns = ostack[3];

    objectWait(obj, ms, ns);
    return ostack;
}

/* static method notify(Ljava/lang/Object;)V */
uintptr_t *notify(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject obj = (pObject )*ostack;
    objectNotify(obj);
    return ostack;
}

/* static method notifyAll(Ljava/lang/Object;)V */
uintptr_t *notifyAll(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject obj = (pObject )*ostack;
    objectNotifyAll(obj);
    return ostack;
}

/* java.lang.VMSystem */

/* arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V */
uintptr_t *arraycopy(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject src = (pObject )ostack[0];
    int start1 = ostack[1];
    pObject dest = (pObject )ostack[2];
    int start2 = ostack[3];
    int length = ostack[4];

    if((src == NULL) || (dest == NULL))
        signalException(java_lang_NullPointerException, NULL);
    else {
        ClassBlock *scb = CLASS_CB(src->class);
        ClassBlock *dcb = CLASS_CB(dest->class);
        char *sdata = ARRAY_DATA(src, char);            
        char *ddata = ARRAY_DATA(dest, char);            

        if((scb->name[0] != '[') || (dcb->name[0] != '['))
            goto storeExcep; 

        if((start1 < 0) || (start2 < 0) || (length < 0)
                        || ((start1 + length) > ARRAY_LEN(src))
                        || ((start2 + length) > ARRAY_LEN(dest))) {
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
            return ostack;
        }

        if(isInstanceOf(dest->class, src->class)) {
            int size = sigElement2Size(scb->name[1]);
            memmove(ddata + start2*size, sdata + start1*size, length*size);
        } else {
            pObject *sob, *dob;
            int i;

            if(!(((scb->name[1] == 'L') || (scb->name[1] == '[')) &&
                          ((dcb->name[1] == 'L') || (dcb->name[1] == '['))))
                goto storeExcep; 

            /* Not compatible array types, but elements may be compatible...
               e.g. src = [Ljava/lang/Object, dest = [Ljava/lang/String, but
               all src = Strings - check one by one...
             */
            
            if(scb->dim > dcb->dim)
                goto storeExcep;

            sob = &((pObject*)sdata)[start1];
            dob = &((pObject*)ddata)[start2];

            for(i = 0; i < length; i++) {
                if((*sob != NULL) && !arrayStoreCheck(dest->class,
                                                      (*sob)->class))
                    goto storeExcep;
                *dob++ = *sob++;
            }
        }
    }
    return ostack;

storeExcep:
    signalException(java_lang_ArrayStoreException, NULL);
    return ostack;
}

uintptr_t *identityHashCode(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject ob = (pObject)*ostack;
    uintptr_t addr = ob == NULL ? 0 : getObjectHashcode(ob);

    *ostack++ = addr & 0xffffffff;
    return ostack;
}

/* java.lang.VMRuntime */

uintptr_t *availableProcessors(pClass class, pMethodBlock mb,
                               uintptr_t *ostack) {

    *ostack++ = nativeAvailableProcessors();
    return ostack;
}

uintptr_t *freeMemory(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *(u8*)ostack = freeHeapMem();
    return ostack + 2;
}

uintptr_t *totalMemory(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *(u8*)ostack = totalHeapMem();
    return ostack + 2;
}

uintptr_t *maxMemory(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *(u8*)ostack = maxHeapMem();
    return ostack + 2;
}

uintptr_t *gc(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    gc1();
    return ostack;
}

uintptr_t *runFinalization(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    runFinalizers();
    return ostack;
}

uintptr_t *exitInternal(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    int status = ostack[0];
    shutdownVM(status);
    /* keep compiler happy */
    return ostack;
}

uintptr_t *nativeLoad(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    char *name = String2Cstr((pObject)ostack[0]);
    pObject class_loader = (pObject )ostack[1];

    ostack[0] = resolveDll(name, class_loader);
    sysFree(name);

    return ostack + 1;
}

uintptr_t *mapLibraryName(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    char *name = String2Cstr((pObject)ostack[0]);
    char *lib = getDllName(name);
    sysFree(name);

    *ostack++ = (uintptr_t)Cstr2String(lib);
    sysFree(lib);

    return ostack;
}

uintptr_t *propertiesPreInit(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject properties = (pObject )*ostack;
    addDefaultProperties(properties);
    return ostack;
}

uintptr_t *propertiesPostInit(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject properties = (pObject )*ostack;
    addCommandLineProperties(properties);
    return ostack;
}

/* java.lang.VMClass */

uintptr_t *isInstance(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    pObject ob = (pObject)ostack[1];

    *ostack++ = ob == NULL ? FALSE : (uintptr_t)isInstanceOf(clazz, ob->class);
    return ostack;
}

uintptr_t *isAssignableFrom(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    pClass clazz2 = (pClass)ostack[1];

    if(clazz2 == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        *ostack++ = (uintptr_t)isInstanceOf(clazz, clazz2);

    return ostack;
}

uintptr_t *isInterface(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_INTERFACE(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isPrimitive(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_PRIMITIVE(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isArray(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_ARRAY(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isMember(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_MEMBER(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isLocal(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_LOCAL(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isAnonymous(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = IS_ANONYMOUS(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *getEnclosingClass0(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingClass(clazz);
    return ostack;
}

uintptr_t *getEnclosingMethod0(pClass class, pMethodBlock mb,
                               uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingMethodObject(clazz);
    return ostack;
}

uintptr_t *getEnclosingConstructor(pClass class, pMethodBlock mb,
                                   uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingConstructorObject(clazz);
    return ostack;
}

uintptr_t *getClassSignature(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    pObject string = cb->signature == NULL ? NULL : createString(cb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *getSuperclass(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((pClass)ostack[0]);
    *ostack++ = (uintptr_t) (IS_PRIMITIVE(cb) ||
                             IS_INTERFACE(cb) ? NULL : cb->super);
    return ostack;
}

uintptr_t *getComponentType(pClass clazz, pMethodBlock mb, uintptr_t *ostack) {
    pClass class = (pClass)ostack[0];
    ClassBlock *cb = CLASS_CB(class);
    pClass componentType = NULL;

    if(IS_ARRAY(cb))
        switch(cb->name[1]) {
            case '[':
                componentType = findArrayClassFromClass(&cb->name[1], class);
                break;

            default:
                componentType = cb->element_class;
                break;
        }
 
    *ostack++ = (uintptr_t) componentType;
    return ostack;
}

uintptr_t *getName(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    char *dot_name = slash2dots(CLASS_CB((pClass)ostack[0])->name);
    pObject string = createString(dot_name);
    *ostack++ = (uintptr_t)string;
    sysFree(dot_name);
    return ostack;
}

uintptr_t *getDeclaredClasses(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassClasses(clazz, public);
    return ostack;
}

uintptr_t *getDeclaringClass0(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getDeclaringClass(clazz);
    return ostack;
}

uintptr_t *getDeclaredConstructors(pClass class, pMethodBlock mb,
                                   uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassConstructors(clazz, public);
    return ostack;
}

uintptr_t *getDeclaredMethods(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassMethods(clazz, public);
    return ostack;
}

uintptr_t *getDeclaredFields(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassFields(clazz, public);
    return ostack;
}

uintptr_t *getClassDeclaredAnnotations(pClass class, pMethodBlock mb,
                                       uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getClassAnnotations(clazz);
    return ostack;
}

uintptr_t *getInterfaces(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t) getClassInterfaces(clazz);
    return ostack;
}

uintptr_t *getClassLoader(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    *ostack++ = (uintptr_t)CLASS_CB(clazz)->class_loader;
    return ostack;
}

uintptr_t *getClassModifiers(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass)ostack[0];
    int ignore_inner_attrs = ostack[1];
    ClassBlock *cb = CLASS_CB(clazz);

    if(!ignore_inner_attrs && cb->declaring_class)
        *ostack++ = (uintptr_t)cb->inner_access_flags;
    else
        *ostack++ = (uintptr_t)cb->access_flags;

    return ostack;
}

uintptr_t *forName0(uintptr_t *ostack, int resolve, pObject loader) {
    pObject string = (pObject )ostack[0];
    pClass class = NULL;
    int len, i = 0;
    char *cstr;
    
    if(string == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return ostack;
    }

    cstr = String2Utf8(string);
    len = strlen(cstr);

    /* Check the classname to see if it's valid.  It can be
       a 'normal' class or an array class, starting with a [ */

    if(cstr[0] == '[') {
        for(; cstr[i] == '['; i++);
        switch(cstr[i]) {
            case 'Z':
            case 'B':
            case 'C':
            case 'S':
            case 'I':
            case 'F':
            case 'J':
            case 'D':
                if(len-i != 1)
                    goto out;
                break;
            case 'L':
                if(cstr[i+1] == '[' || cstr[len-1] != ';')
                    goto out;
                break;
            default:
                goto out;
                break;
        }
    }

    /* Scan the classname and convert it to internal form
       by converting dots to slashes.  Reject classnames
       containing slashes, as this is an invalid character */

    for(; i < len; i++) {
        if(cstr[i] == '/')
            goto out;
        if(cstr[i] == '.')
            cstr[i] = '/';
    }

    class = findClassFromClassLoader(cstr, loader);

out:
    if(class == NULL) {
        pObject excep = exceptionOccurred();

        clearException();
        signalChainedException(java_lang_ClassNotFoundException, cstr, excep);
    } else
        if(resolve)
            initClass(class);

    sysFree(cstr);
    *ostack++ = (uintptr_t)class;
    return ostack;
}

uintptr_t *forName(pClass clazz, pMethodBlock mb, uintptr_t *ostack) {
    int init = ostack[1];
    pObject loader = (pObject)ostack[2];
    return forName0(ostack, init, loader);
}

uintptr_t *throwException(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject excep = (pObject )ostack[0];
    setException(excep);
    return ostack;
}

uintptr_t *hasClassInitializer(pClass class, pMethodBlock mb,
                               uintptr_t *ostack) {

    pClass clazz = (pClass)ostack[0];
    *ostack++ = findMethod(clazz, SYMBOL(class_init),
                                  SYMBOL(___V)) == NULL ? FALSE : TRUE;
    return ostack;
}

/* java.lang.VMThrowable */

uintptr_t *fillInStackTrace(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t) setStackTrace();
    return ostack;
}

uintptr_t *getStackTrace(pClass class, pMethodBlock m, uintptr_t *ostack) {
    pObject this = (pObject )*ostack;
    *ostack++ = (uintptr_t) convertStackTrace(this);
    return ostack;
}

/* gnu.classpath.VMStackWalker */

uintptr_t *getCallingClass(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t) getCallerCallerClass();
    return ostack;
}

uintptr_t *getCallingClassLoader(pClass clazz, pMethodBlock mb,
                                 uintptr_t *ostack) {

    pClass class = getCallerCallerClass();

    *ostack++ = (uintptr_t) (class ? CLASS_CB(class)->class_loader : NULL);
    return ostack;
}

uintptr_t *getClassContext(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass class_class = findArrayClass("[Ljava/lang/Class;");
    pObject array;
    Frame *last;

    if(class_class == NULL)
        return ostack;

    if((last = getCallerFrame(getExecEnv()->last_frame)) == NULL)
        array = allocArray(class_class, 0, sizeof(pClass)); 
    else {
        Frame *bottom = last;
        int depth = 0;

        do {
            for(; last->mb != NULL; last = last->prev, depth++);
        } while((last = last->prev)->prev != NULL);
    
        array = allocArray(class_class, depth, sizeof(pClass));

        if(array != NULL) {
            pClass *data = ARRAY_DATA(array, pClass);

            do {
                for(; bottom->mb != NULL; bottom = bottom->prev)
                    *data++ = bottom->mb->class;
            } while((bottom = bottom->prev)->prev != NULL);
        }
    }

    *ostack++ = (uintptr_t)array;
    return ostack;
}

uintptr_t *firstNonNullClassLoader(pClass class, pMethodBlock mb,
                                   uintptr_t *ostack) {

    Frame *last = getExecEnv()->last_frame;
    pObject loader = NULL;

    do {
        for(; last->mb != NULL; last = last->prev)
            if((loader = CLASS_CB(last->mb->class)->class_loader) != NULL)
                goto out;
    } while((last = last->prev)->prev != NULL);

out:
    *ostack++ = (uintptr_t)loader;
    return ostack;
}

/* java.lang.VMClassLoader */

/* loadClass(Ljava/lang/String;I)Ljava/lang/Class; */
uintptr_t *loadClass(pClass clazz, pMethodBlock mb, uintptr_t *ostack) {
    int resolve = ostack[1];
    return forName0(ostack, resolve, NULL);
}

/* getPrimitiveClass(C)Ljava/lang/Class; */
uintptr_t *getPrimitiveClass(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    char prim_type = *ostack;
    *ostack++ = (uintptr_t)findPrimitiveClass(prim_type);
    return ostack;
}

uintptr_t *defineClass0(pClass clazz, pMethodBlock mb, uintptr_t *ostack) {
    pObject class_loader = (pObject )ostack[0];
    pObject string = (pObject )ostack[1];
    pObject array = (pObject )ostack[2];
    int offset = ostack[3];
    int data_len = ostack[4];
    uintptr_t pd = ostack[5];
    pClass class = NULL;

    if(array == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        if((offset < 0) || (data_len < 0) ||
                           ((offset + data_len) > ARRAY_LEN(array)))
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        else {
            char *data = ARRAY_DATA(array, char);
            char *cstr = string ? String2Utf8(string) : NULL;
            int len = string ? strlen(cstr) : 0;
            int i;

            for(i = 0; i < len; i++)
                if(cstr[i]=='.') cstr[i]='/';

            class = defineClass(cstr, data, offset, data_len, class_loader);

            if(class != NULL) {
                INST_DATA(class, uintptr_t, pd_offset) = pd;
                linkClass(class);
            }

            sysFree(cstr);
        }

    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *findLoadedClass(pClass clazz, pMethodBlock mb, uintptr_t *ostack) {
    pObject class_loader = (pObject )ostack[0];
    pObject string = (pObject )ostack[1];
    pClass class;
    char *cstr;
    int len, i;

    if(string == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return ostack;
    }

    cstr = String2Utf8(string);
    len = strlen(cstr);

    for(i = 0; i < len; i++)
        if(cstr[i]=='.') cstr[i]='/';

    class = findHashedClass(cstr, class_loader);

    sysFree(cstr);
    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *resolveClass0(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass clazz = (pClass )*ostack;

    if(clazz == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        initClass(clazz);

    return ostack;
}

uintptr_t *getBootClassPathSize(pClass class, pMethodBlock mb,
                                uintptr_t *ostack) {

    *ostack++ = bootClassPathSize();
    return ostack;
}

uintptr_t *getBootClassPathResource(pClass class, pMethodBlock mb,
                                    uintptr_t *ostack) {

    pObject string = (pObject ) ostack[0];
    char *filename = String2Cstr(string);
    int index = ostack[1];

    *ostack++ = (uintptr_t) bootClassPathResource(filename, index);

    sysFree(filename);
    return ostack;
}

uintptr_t *getBootClassPackage(pClass class, pMethodBlock mb,
                               uintptr_t *ostack) {

    pObject string = (pObject ) ostack[0];
    char *package_name = String2Cstr(string);

    *ostack++ = (uintptr_t) bootPackage(package_name);

    sysFree(package_name);
    return ostack;
}

uintptr_t *getBootClassPackages(pClass class, pMethodBlock mb,
                                uintptr_t *ostack) {

    *ostack++ = (uintptr_t) bootPackages();
    return ostack;
}

/* Helper function for constructorConstruct and methodInvoke */

int checkInvokeAccess(pMethodBlock mb) {
    pClass caller = getCallerCallerClass();

    if(!checkClassAccess(mb->class, caller) ||
                            !checkMethodAccess(mb, caller)) {

        signalException(java_lang_IllegalAccessException,
                        "method is not accessible");
            return FALSE;
    }

    return TRUE;
}

/* java.lang.reflect.Constructor */

uintptr_t *constructorConstruct(pClass class, pMethodBlock mb2,
                                uintptr_t *ostack) {

    pObject this       = (pObject)ostack[0];
    pObject args_array = (pObject)ostack[1];

    pObject param_types = getConsParamTypes(this);
    int no_access_check = getConsAccessFlag(this);
    pMethodBlock mb     = getConsMethodBlock(this);

    ClassBlock *cb = CLASS_CB(mb->class); 
    pObject ob;

    /* First check that the constructor can be accessed (this
       error takes priority in the reference implementation,
       and Mauve expects this to be thrown before instantiation
       error) */

    if(!no_access_check && !checkInvokeAccess(mb))
        return ostack;

    if(cb->access_flags & ACC_ABSTRACT) {
        signalException(java_lang_InstantiationException, cb->name);
        return ostack;
    }

    /* Creating an instance of the class is an
       active use; make sure it is initialised */
    if(initClass(mb->class) == NULL)
        return ostack;

    if((ob = allocObject(mb->class)) != NULL) {
        invoke(ob, mb, args_array, param_types);
        *ostack++ = (uintptr_t) ob;
    }

    return ostack;
}

uintptr_t *constructorModifiers(pClass class, pMethodBlock mb2,
                                uintptr_t *ostack) {

    pMethodBlock mb = getConsMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) mb->access_flags;
    return ostack;
}

uintptr_t *constructorExceptionTypes(pClass class, pMethodBlock mb2,
                                uintptr_t *ostack) {

    pMethodBlock mb = getConsMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) getExceptionTypes(mb);
    return ostack;
}

uintptr_t *methodExceptionTypes(pClass class, pMethodBlock mb2,
                                uintptr_t *ostack) {

    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) getExceptionTypes(mb);
    return ostack;
}

uintptr_t *methodModifiers(pClass class, pMethodBlock mb2, uintptr_t *ostack) {
    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) mb->access_flags;
    return ostack;
}

uintptr_t *methodName(pClass class, pMethodBlock mb2, uintptr_t *ostack) {
    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) createString(mb->name);
    return ostack;
}

uintptr_t *methodSignature(pClass class, pMethodBlock mb2, uintptr_t *ostack) {
    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    pObject string = mb->signature == NULL ? NULL : createString(mb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *constructorSignature(pClass class, pMethodBlock mb2,
                                uintptr_t *ostack) {

    pMethodBlock mb = getConsMethodBlock((pObject)ostack[0]);
    pObject string = mb->signature == NULL ? NULL : createString(mb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *methodDefaultValue(pClass class, pMethodBlock mb2,
                              uintptr_t *ostack) {

    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getMethodDefaultValue(mb);
    return ostack;
}

uintptr_t *methodDeclaredAnnotations(pClass class, pMethodBlock mb2,
                                     uintptr_t *ostack) {

    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getMethodAnnotations(mb);
    return ostack;
}

uintptr_t *constructorDeclaredAnnotations(pClass class, pMethodBlock mb2,
                                          uintptr_t *ostack) {

    pMethodBlock mb = getConsMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getMethodAnnotations(mb);
    return ostack;
}

uintptr_t *methodParameterAnnotations(pClass class, pMethodBlock mb2,
                                      uintptr_t *ostack) {

    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getMethodParameterAnnotations(mb);
    return ostack;
}

uintptr_t *constructorParameterAnnotations(pClass class, pMethodBlock mb2,
                                           uintptr_t *ostack) {

    pMethodBlock mb = getMethodMethodBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getMethodParameterAnnotations(mb);
    return ostack;
}

uintptr_t *fieldModifiers(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pFieldBlock fb = getFieldFieldBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) fb->access_flags;
    return ostack;
}

uintptr_t *fieldName(pClass class, pMethodBlock mb2, uintptr_t *ostack) {
    pFieldBlock fb = getFieldFieldBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t) createString(fb->name);
    return ostack;
}

uintptr_t *fieldSignature(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pFieldBlock fb = getFieldFieldBlock((pObject)ostack[0]);
    pObject string = fb->signature == NULL ? NULL : createString(fb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *fieldDeclaredAnnotations(pClass class, pMethodBlock mb,
                                    uintptr_t *ostack) {

    pFieldBlock fb = getFieldFieldBlock((pObject)ostack[0]);
    *ostack++ = (uintptr_t)getFieldAnnotations(fb);
    return ostack;
}

pObject getAndCheckObject(uintptr_t *ostack, pClass type) {
    pObject ob = (pObject)ostack[1];

    if(ob == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return NULL;
    }

    if(!isInstanceOf(type, ob->class)) {
        signalException(java_lang_IllegalArgumentException,
                        "object is not an instance of declaring class");
        return NULL;
    }

    return ob;
}

void *getPntr2Field(uintptr_t *ostack) {
    pObject this        = (pObject)ostack[0];
    pFieldBlock fb      = getFieldFieldBlock(this);
    int no_access_check = getFieldAccessFlag(this);
    pObject ob;

    if(!no_access_check) {
        pClass caller = getCallerCallerClass();

        if(!checkClassAccess(fb->class, caller) ||
           !checkFieldAccess(fb, caller)) {

            signalException(java_lang_IllegalAccessException,
                            "field is not accessible");
            return NULL;
        }
    }

    if(fb->access_flags & ACC_STATIC) {

        /* Setting/getting a static field of a class is an
           active use.  Make sure it is initialised */
        if(initClass(fb->class) == NULL)
            return NULL;

        return fb->u.static_value.data;
    }

    if((ob = getAndCheckObject(ostack, fb->class)) == NULL)
        return NULL;

    return &INST_DATA(ob, int, fb->u.offset);
}

uintptr_t *fieldGet(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass field_type = getFieldType((pObject)ostack[0]);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL)
        *ostack++ = (uintptr_t) getReflectReturnObject(field_type, field,
                                                       REF_SRC_FIELD);

    return ostack;
}

uintptr_t *fieldGetPrimitive(int type_no, uintptr_t *ostack) {

    pClass field_type = getFieldType((pObject)ostack[0]);
    ClassBlock *type_cb = CLASS_CB(field_type);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        if(IS_PRIMITIVE(type_cb)) {
            int size = widenPrimitiveValue(getPrimTypeIndex(type_cb),
                                           type_no, field, ostack,
                                           REF_SRC_FIELD | REF_DST_OSTACK);

            if(size > 0)
                return ostack + size;
        }

        /* Field is not primitive, or the source type cannot
           be widened to the destination type */
        signalException(java_lang_IllegalArgumentException,
                        "field type mismatch");
    }

    return ostack;
}

#define FIELD_GET_PRIMITIVE(name, type)                                       \
uintptr_t *fieldGet##name(pClass class, pMethodBlock mb, uintptr_t *ostack) { \
    return fieldGetPrimitive(PRIM_IDX_##type, ostack);                        \
}

FIELD_GET_PRIMITIVE(Boolean, BOOLEAN)
FIELD_GET_PRIMITIVE(Byte, BYTE)
FIELD_GET_PRIMITIVE(Char, CHAR)
FIELD_GET_PRIMITIVE(Short, SHORT)
FIELD_GET_PRIMITIVE(Int, INT)
FIELD_GET_PRIMITIVE(Float, FLOAT)
FIELD_GET_PRIMITIVE(Long, LONG)
FIELD_GET_PRIMITIVE(Double, DOUBLE)

uintptr_t *fieldSet(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass field_type = getFieldType((pObject)ostack[0]);
    pObject value = (pObject)ostack[2];

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        int size = unwrapAndWidenObject(field_type, value, field,
                                        REF_DST_FIELD);

        if(size == 0)
            signalException(java_lang_IllegalArgumentException,
                            "field type mismatch");
    }

    return ostack;
}

uintptr_t *fieldSetPrimitive(int type_no, uintptr_t *ostack) {

    pClass field_type = getFieldType((pObject)ostack[0]);
    ClassBlock *type_cb = CLASS_CB(field_type);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        if(IS_PRIMITIVE(type_cb)) {

            int size = widenPrimitiveValue(type_no, getPrimTypeIndex(type_cb),
                                           &ostack[2], field,
                                           REF_SRC_OSTACK | REF_DST_FIELD);

            if(size > 0)
                return ostack;
        }

        /* Field is not primitive, or the source type cannot
           be widened to the destination type */
        signalException(java_lang_IllegalArgumentException,
                        "field type mismatch");
    }

    return ostack;
}

#define FIELD_SET_PRIMITIVE(name, type)                                       \
uintptr_t *fieldSet##name(pClass class, pMethodBlock mb, uintptr_t *ostack) { \
    return fieldSetPrimitive(PRIM_IDX_##type, ostack);                        \
}

FIELD_SET_PRIMITIVE(Boolean, BOOLEAN)
FIELD_SET_PRIMITIVE(Byte, BYTE)
FIELD_SET_PRIMITIVE(Char, CHAR)
FIELD_SET_PRIMITIVE(Short, SHORT)
FIELD_SET_PRIMITIVE(Int, INT)
FIELD_SET_PRIMITIVE(Float, FLOAT)
FIELD_SET_PRIMITIVE(Long, LONG)
FIELD_SET_PRIMITIVE(Double, DOUBLE)

/* java.lang.reflect.Method */

uintptr_t *methodInvoke(pClass class, pMethodBlock mb2, uintptr_t *ostack) {
    pObject this       = (pObject)ostack[0];
    pObject args_array = (pObject)ostack[2];

    pClass ret_type     = getMethodReturnType(this);
    pObject param_types = getMethodParamTypes(this);
    int no_access_check = getMethodAccessFlag(this);
    pMethodBlock mb     = getMethodMethodBlock(this);

    pObject ob = NULL;
    uintptr_t *ret;

    /* First check that the method can be accessed (this
       error takes priority in the reference implementation) */

    if(!no_access_check && !checkInvokeAccess(mb))
        return ostack;

    /* If it's a static method, class may not be initialised;
       interfaces are also not normally initialised. */

    if((mb->access_flags & ACC_STATIC) || IS_INTERFACE(CLASS_CB(mb->class)))
        if(initClass(mb->class) == NULL)
            return ostack;

    if(!(mb->access_flags & ACC_STATIC))
        if(((ob = getAndCheckObject(ostack, mb->class)) == NULL) ||
                               ((mb = lookupVirtualMethod(ob, mb)) == NULL))
            return ostack;
 
    if((ret = (uintptr_t*) invoke(ob, mb, args_array, param_types)) != NULL)
        *ostack++ = (uintptr_t) getReflectReturnObject(ret_type, ret,
                                                       REF_SRC_OSTACK);

    return ostack;
}

/* java.lang.VMString */

/* static method - intern(Ljava/lang/String;)Ljava/lang/String; */
uintptr_t *intern(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject string = (pObject)ostack[0];
    ostack[0] = (uintptr_t)findInternedString(string);
    return ostack + 1;
}

/* java.lang.VMThread */

/* static method currentThread()Ljava/lang/Thread; */
uintptr_t *currentThread(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)getExecEnv()->thread;
    return ostack;
}

/* static method create(Ljava/lang/Thread;J)V */
uintptr_t *create(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject this = (pObject )ostack[0];
    long long stack_size = *((long long*)&ostack[1]);
    createJavaThread(this, stack_size);
    return ostack;
}

/* static method sleep(JI)V */
uintptr_t *jamSleep(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long ms = *((long long *)&ostack[0]);
    int ns = ostack[2];
    Thread *thread = threadSelf();

    threadSleep(thread, ms, ns);
    return ostack;
}

/* instance method interrupt()V */
uintptr_t *interrupt(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject vmThread = (pObject )*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    if(thread)
        threadInterrupt(thread);
    return ostack;
}

/* instance method isAlive()Z */
uintptr_t *isAlive(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject vmThread = (pObject )*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    *ostack++ = thread ? threadIsAlive(thread) : FALSE;
    return ostack;
}

/* static method yield()V */
uintptr_t *yield(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    Thread *thread = threadSelf();
    threadYield(thread);
    return ostack;
}

/* instance method isInterrupted()Z */
uintptr_t *isInterrupted(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject vmThread = (pObject )*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    *ostack++ = thread ? threadIsInterrupted(thread) : FALSE;
    return ostack;
}

/* static method interrupted()Z */
uintptr_t *interrupted(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    Thread *thread = threadSelf();
    *ostack++ = threadInterrupted(thread);
    return ostack;
}

/* instance method nativeSetPriority(I)V */
uintptr_t *nativeSetPriority(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    return ostack + 1;
}

/* instance method holdsLock(Ljava/lang/Object;)Z */
uintptr_t *holdsLock(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject ob = (pObject )ostack[0];
    if(ob == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        *ostack++ = objectLockedByCurrent(ob);
    return ostack;
}

/* instance method getState()Ljava/lang/String; */
uintptr_t *getState(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject vmThread = (pObject )*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    char *state = thread ? getThreadStateString(thread) : "TERMINATED";

    *ostack++ = (uintptr_t)Cstr2String(state);
    return ostack;
}

/* java.security.VMAccessController */

/* instance method getStack()[[Ljava/lang/Object; */
uintptr_t *getStack(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass object_class = findArrayClass("[[Ljava/lang/Object;");
    pClass class_class = findArrayClass("[Ljava/lang/Class;");
    pClass string_class = findArrayClass("[Ljava/lang/String;");
    pObject stack, names, classes;
    Frame *frame;
    int depth;

    if(object_class == NULL || class_class == NULL || string_class == NULL)
      return ostack;

    frame = getExecEnv()->last_frame;
    depth = 0;

    do {
        for(; frame->mb != NULL; frame = frame->prev, depth++);
    } while((frame = frame->prev)->prev != NULL);

    stack = allocArray(object_class, 2, sizeof(pObject));
    classes = allocArray(class_class, depth, sizeof(pObject));
    names = allocArray(string_class, depth, sizeof(pObject));

    if(stack != NULL && names != NULL && classes != NULL) {
        pClass *dcl = ARRAY_DATA(classes, pClass);
        pObject *dnm = ARRAY_DATA(names, pObject);
        pObject *stk = ARRAY_DATA(stack, pObject);

        frame = getExecEnv()->last_frame;

        do {
            for(; frame->mb != NULL; frame = frame->prev) {
                *dcl++ = frame->mb->class;
                *dnm++ = createString(frame->mb->name);
            }
        } while((frame = frame->prev)->prev != NULL);

        stk[0] = classes;
        stk[1] = names;
    }

    *ostack++ = (uintptr_t) stack;
    return ostack;
}

uintptr_t *getThreadCount(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = getThreadsCount();
    return ostack;
}

uintptr_t *getPeakThreadCount(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    *ostack++ = getPeakThreadsCount();
    return ostack;
}

uintptr_t *getTotalStartedThreadCount(pClass class, pMethodBlock mb,
                                      uintptr_t *ostack) {

    *(u8*)ostack = getTotalStartedThreadsCount();
    return ostack + 2;
}

uintptr_t *resetPeakThreadCount(pClass class, pMethodBlock mb,
                                uintptr_t *ostack) {
    resetPeakThreadsCount();
    return ostack;
}

uintptr_t *findMonitorDeadlockedThreads(pClass class, pMethodBlock mb,
                                        uintptr_t *ostack) {
    *ostack++ = (uintptr_t)NULL;
    return ostack;
}

uintptr_t *getThreadInfoForId(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    long long id = *((long long *)&ostack[0]);
    int max_depth = ostack[2];

    Thread *thread = findThreadById(id);
    pObject info = NULL;

    if(thread != NULL) {
        pClass helper_class = findSystemClass("jamvm/ThreadInfoHelper");
        pClass info_class = findSystemClass("java/lang/management/ThreadInfo");

        if(info_class != NULL && helper_class != NULL) {
            pMethodBlock helper = findMethod(helper_class,
                                             newUtf8("createThreadInfo"),
                                             newUtf8("(Ljava/lang/Thread;"
                                                     "Ljava/lang/Object;"
                                                     "Ljava/lang/Thread;)"
                                                     "[Ljava/lang/Object;"));
            pMethodBlock init = findMethod(info_class, SYMBOL(object_init),
                                 newUtf8("(JLjava/lang/String;"
                                         "Ljava/lang/Thread$State;"
                                         "JJLjava/lang/String;"
                                         "JLjava/lang/String;JJZZ"
                                         "[Ljava/lang/StackTraceElement;"
                                         "[Ljava/lang/management/MonitorInfo;"
                                         "[Ljava/lang/management/LockInfo;)V"));


            if(init != NULL && helper != NULL) {
                Frame *last;
                int in_native;
                pObject vmthrowable;
                int self = thread == threadSelf();

                if(!self)
                    suspendThread(thread);

                vmthrowable = setStackTrace0(thread->ee, max_depth);

                last = thread->ee->last_frame;
                in_native = last->prev == NULL ||
                                      last->mb->access_flags & ACC_NATIVE;

                if(!self)
                    resumeThread(thread);

                if(vmthrowable != NULL) {
                    pObject *helper_info, trace;

                    if((info = allocObject(info_class)) != NULL &&
                             (trace = convertStackTrace(vmthrowable)) != NULL) {

                        Monitor *mon = thread->blocked_mon;
                        pObject lock = mon != NULL ? mon->obj : NULL;
                        Thread *owner = lock != NULL ?
                                             objectLockedBy(lock) : NULL;
                        pObject lock_owner = owner != NULL ?
                                               owner->ee->thread : NULL;
                        long long owner_id = owner != NULL ?
                                               javaThreadId(owner) : -1;

                        helper_info = executeStaticMethod(helper_class,
                                              helper, thread->ee->thread, lock,
                                              lock_owner);

                        if(!exceptionOccurred()) {
                            helper_info = ARRAY_DATA(*helper_info, pObject);

                            executeMethod(info, init, id,
                                          helper_info[0], helper_info[1],
                                          thread->blocked_count, 0LL,
                                          helper_info[2], owner_id,
                                          helper_info[3], thread->waited_count,
                                          0LL, in_native, FALSE, trace,
                                          NULL, NULL);
                        }
                    }
                }
            }
        }
    }

    *ostack++ = (uintptr_t)info;
    return ostack;
}

uintptr_t *getMemoryPoolNames(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {
    pClass array_class = findArrayClass(SYMBOL(array_java_lang_String));

    if(array_class != NULL)
        *ostack++ = (uintptr_t)allocArray(array_class, 0, sizeof(pObject));
        
    return ostack;
}

/* sun.misc.Unsafe */

static volatile uintptr_t spinlock = 0;

void lockSpinLock() {
    while(!LOCKWORD_COMPARE_AND_SWAP(&spinlock, 0, 1));
}

void unlockSpinLock() {
    LOCKWORD_WRITE(&spinlock, 0);
}

uintptr_t *objectFieldOffset(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pFieldBlock fb = fbFromReflectObject((pObject)ostack[1]);

    *(long long*)ostack = (long long)(uintptr_t)
                          &(INST_DATA((pObject)NULL, int, fb->u.offset));
    return ostack + 2;
}

uintptr_t *compareAndSwapInt(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);
    unsigned int expect = ostack[4];
    unsigned int update = ostack[5];
    int result;

#ifdef COMPARE_AND_SWAP_32
    result = COMPARE_AND_SWAP_32(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *compareAndSwapLong(pClass class, pMethodBlock mb,
                              uintptr_t *ostack) {

    long long offset = *((long long *)&ostack[2]);
    long long *addr = (long long*)((char*)ostack[1] + offset);
    long long expect = *((long long *)&ostack[4]);
    long long update = *((long long *)&ostack[6]);
    int result;

#ifdef COMPARE_AND_SWAP_64
    result = COMPARE_AND_SWAP_64(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *putOrderedInt(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *putOrderedLong(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long value = *((long long *)&ostack[4]);
    volatile long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *addr = value;
    else {
        lockSpinLock();
        *addr = value;
        unlockSpinLock();
    }

    return ostack;
}

uintptr_t *putIntVolatile(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int *)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    MBARRIER();
    *addr = value;

    return ostack;
}

uintptr_t *getIntVolatile(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);

    *ostack++ = *addr;
    MBARRIER();

    return ostack;
}

uintptr_t *putLong(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long value = *((long long *)&ostack[4]);
    long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *addr = value;
    else {
        lockSpinLock();
        *addr = value;
        unlockSpinLock();
    }

    return ostack;
}

uintptr_t *getLongVolatile(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *(long long*)ostack = *addr;
    else {
        lockSpinLock();
        *(long long*)ostack = *addr;
        unlockSpinLock();
    }

    return ostack + 2;
}

uintptr_t *getLong(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *(long long*)ostack = *addr;
    else {
        lockSpinLock();
        *(long long*)ostack = *addr;
        unlockSpinLock();
    }

    return ostack + 2;
}

uintptr_t *compareAndSwapObject(pClass class, pMethodBlock mb,
                                uintptr_t *ostack) {

    long long offset = *((long long *)&ostack[2]);
    uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t expect = ostack[4];
    uintptr_t update = ostack[5];
    int result;

#ifdef COMPARE_AND_SWAP
    result = COMPARE_AND_SWAP(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *putOrderedObject(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *putObjectVolatile(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    MBARRIER();
    *addr = value;

    return ostack;
}

uintptr_t *getObjectVolatile(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);

    *ostack++ = *addr;
    MBARRIER();

    return ostack;
}

uintptr_t *putObject(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *arrayBaseOffset(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)ARRAY_DATA((pObject)NULL, void);
    return ostack;
}

uintptr_t *arrayIndexScale(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pClass array_class = (pClass)ostack[1];
    ClassBlock *cb = CLASS_CB(array_class);
    int scale = 0;

    /* Sub-int fields within objects are widened to int, whereas
       sub-int arrays are packed.  This means sub-int arrays can
       not be accessed, and must return zero. */

    if(cb->name[0] == '[')
        switch(cb->name[1]) {
            case 'I':
            case 'F':
                scale = 4;
                break;

            case 'J':
            case 'D':
                scale = 8;
                break;

            case '[':
            case 'L':
                scale = sizeof(pObject);
                break;
        }

    *ostack++ = scale;
    return ostack;
}

uintptr_t *unpark(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    pObject jThread = (pObject )ostack[1];

    if(jThread != NULL) {
        Thread *thread = jThread2Thread(jThread);

        if(thread != NULL)
            threadUnpark(thread);
    }
    return ostack;
}

uintptr_t *park(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    int absolute = ostack[1];
    long long time = *((long long *)&ostack[2]);
    Thread *thread = threadSelf();

    threadPark(thread, absolute, time);
    return ostack;
}

uintptr_t *vmSupportsCS8(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    *ostack++ = FALSE;
    return ostack;
}

/* jamvm.java.lang.VMClassLoaderData */

uintptr_t *nativeUnloadDll(pClass class, pMethodBlock mb, uintptr_t *ostack) {
    unloaderUnloadDll((uintptr_t)*(long long*)&ostack[1]);
    return ostack;
}

VMMethod vm_object[] = {
    {"getClass",                    getClass},
    {"clone",                       jamClone},
    {"wait",                        jamWait},
    {"notify",                      notify},
    {"notifyAll",                   notifyAll},
    {NULL,                          NULL}
};

VMMethod vm_system[] = {
    {"arraycopy",                   arraycopy},
    {"identityHashCode",            identityHashCode},
    {NULL,                          NULL}
};

VMMethod vm_runtime[] = {
    {"availableProcessors",         availableProcessors},
    {"freeMemory",                  freeMemory},
    {"totalMemory",                 totalMemory},
    {"maxMemory",                   maxMemory},
    {"gc",                          gc},
    {"runFinalization",             runFinalization},
    {"exit",                        exitInternal},
    {"nativeLoad",                  nativeLoad},
    {"mapLibraryName",              mapLibraryName},
    {NULL,                          NULL}
};

VMMethod vm_class[] = {
    {"isInstance",                  isInstance},
    {"isAssignableFrom",            isAssignableFrom},
    {"isInterface",                 isInterface},
    {"isPrimitive",                 isPrimitive},
    {"isArray",                     isArray},
    {"isMemberClass",               isMember},
    {"isLocalClass",                isLocal},
    {"isAnonymousClass",            isAnonymous},
    {"getEnclosingClass",           getEnclosingClass0},
    {"getEnclosingMethod",          getEnclosingMethod0},
    {"getEnclosingConstructor",     getEnclosingConstructor},
    {"getClassSignature",           getClassSignature},
    {"getSuperclass",               getSuperclass},
    {"getComponentType",            getComponentType},
    {"getName",                     getName},
    {"getDeclaredClasses",          getDeclaredClasses},
    {"getDeclaringClass",           getDeclaringClass0},
    {"getDeclaredConstructors",     getDeclaredConstructors},
    {"getDeclaredMethods",          getDeclaredMethods},
    {"getDeclaredFields",           getDeclaredFields},
    {"getInterfaces",               getInterfaces},
    {"getClassLoader",              getClassLoader},
    {"getModifiers",                getClassModifiers},
    {"forName",                     forName},
    {"throwException",              throwException},
    {"hasClassInitializer",         hasClassInitializer},
    {"getDeclaredAnnotations",      getClassDeclaredAnnotations},
    {NULL,                          NULL}
};

VMMethod vm_string[] = {
    {"intern",                      intern},
    {NULL,                          NULL}
};

VMMethod vm_thread[] = {
    {"currentThread",               currentThread},
    {"create",                      create},
    {"sleep",                       jamSleep},
    {"interrupt",                   interrupt},
    {"isAlive",                     isAlive},
    {"yield",                       yield},
    {"isInterrupted",               isInterrupted},
    {"interrupted",                 interrupted},
    {"nativeSetPriority",           nativeSetPriority},
    {"holdsLock",                   holdsLock},
    {"getState",                    getState},
    {NULL,                          NULL}
};

VMMethod vm_throwable[] = {
    {"fillInStackTrace",            fillInStackTrace},
    {"getStackTrace",               getStackTrace},
    {NULL,                          NULL}
};

VMMethod vm_classloader[] = {
    {"loadClass",                   loadClass},
    {"getPrimitiveClass",           getPrimitiveClass},
    {"defineClass",                 defineClass0},
    {"findLoadedClass",             findLoadedClass},
    {"resolveClass",                resolveClass0},
    {"getBootClassPathSize",        getBootClassPathSize},
    {"getBootClassPathResource",    getBootClassPathResource},
    {"getBootPackages",             getBootClassPackages},
    {"getBootPackage",              getBootClassPackage},
    {NULL,                          NULL}
};

VMMethod vm_reflect_constructor[] = {
    {"construct",                   constructorConstruct},
    {"getSignature",                constructorSignature},
    {"getModifiersInternal",        constructorModifiers},
    {"getExceptionTypes",           constructorExceptionTypes},
    {"getDeclaredAnnotations",      constructorDeclaredAnnotations},
    {"getParameterAnnotations",     constructorParameterAnnotations},
    {NULL,                          NULL}
};

VMMethod vm_reflect_method[] = {
    {"getName",                     methodName},
    {"invoke",                      methodInvoke},
    {"getSignature",                methodSignature},
    {"getModifiersInternal",        methodModifiers},
    {"getDefaultValue",             methodDefaultValue},
    {"getExceptionTypes",           methodExceptionTypes},
    {"getDeclaredAnnotations",      methodDeclaredAnnotations},
    {"getParameterAnnotations",     methodParameterAnnotations},
    {NULL,                          NULL}
};

VMMethod vm_reflect_field[] = {
    {"getName",                     fieldName},
    {"getModifiersInternal",        fieldModifiers},
    {"getSignature",                fieldSignature},
    {"getDeclaredAnnotations",      fieldDeclaredAnnotations},
    {"set",                         fieldSet},
    {"setBoolean",                  fieldSetBoolean},
    {"setByte",                     fieldSetByte},
    {"setChar",                     fieldSetChar},
    {"setShort",                    fieldSetShort},
    {"setInt",                      fieldSetInt},
    {"setLong",                     fieldSetLong},
    {"setFloat",                    fieldSetFloat},
    {"setDouble",                   fieldSetDouble},
    {"get",                         fieldGet},
    {"getBoolean",                  fieldGetBoolean},
    {"getByte",                     fieldGetByte},
    {"getChar",                     fieldGetChar},
    {"getShort",                    fieldGetShort},
    {"getInt",                      fieldGetInt},
    {"getLong",                     fieldGetLong},
    {"getFloat",                    fieldGetFloat},
    {"getDouble",                   fieldGetDouble},
    {NULL,                          NULL}
};

VMMethod vm_system_properties[] = {
    {"preInit",                     propertiesPreInit},
    {"postInit",                    propertiesPostInit},
    {NULL,                          NULL}
};

VMMethod vm_stack_walker[] = {
    {"getClassContext",             getClassContext},
    {"getCallingClass",             getCallingClass},
    {"getCallingClassLoader",       getCallingClassLoader},
    {"firstNonNullClassLoader",     firstNonNullClassLoader},
    {NULL,                          NULL}
};

VMMethod sun_misc_unsafe[] = {
    {"objectFieldOffset",           objectFieldOffset},
    {"compareAndSwapInt",           compareAndSwapInt},
    {"compareAndSwapLong",          compareAndSwapLong},
    {"compareAndSwapObject",        compareAndSwapObject},
    {"putOrderedInt",               putOrderedInt},
    {"putOrderedLong",              putOrderedLong},
    {"putOrderedObject",            putOrderedObject},
    {"putIntVolatile",              putIntVolatile},
    {"getIntVolatile",              getIntVolatile},
    {"putLongVolatile",             putOrderedLong},
    {"putLong",                     putLong},
    {"getLongVolatile",             getLongVolatile},
    {"getLong",                     getLong},
    {"putObjectVolatile",           putObjectVolatile},
    {"putObject",                   putObject},
    {"getObjectVolatile",           getObjectVolatile},
    {"arrayBaseOffset",             arrayBaseOffset},
    {"arrayIndexScale",             arrayIndexScale},
    {"unpark",                      unpark},
    {"park",                        park},
    {NULL,                          NULL}
};

VMMethod vm_access_controller[] = {
    {"getStack",                    getStack},
    {NULL,                          NULL}
};

VMMethod vm_management_factory[] = {
    {"getMemoryPoolNames",          getMemoryPoolNames},
    {"getMemoryManagerNames",       getMemoryPoolNames},
    {"getGarbageCollectorNames",    getMemoryPoolNames},
    {NULL,                          NULL}
};

VMMethod vm_threadmx_bean_impl[] = {
    {"getThreadCount",              getThreadCount},
    {"getPeakThreadCount",          getPeakThreadCount},
    {"getTotalStartedThreadCount",  getTotalStartedThreadCount},
    {"resetPeakThreadCount",        resetPeakThreadCount},
    {"getThreadInfoForId",          getThreadInfoForId},
    {"findMonitorDeadlockedThreads",findMonitorDeadlockedThreads},
    {NULL,                          NULL}
};

VMMethod concurrent_atomic_long[] = {
    {"VMSupportsCS8",               vmSupportsCS8},
    {NULL,                          NULL}
};

VMMethod vm_class_loader_data[] = {
    {"nativeUnloadDll",             nativeUnloadDll},
    {NULL,                          NULL}
};

VMClass native_methods[] = {
    {"java/lang/VMClass",                           vm_class},
    {"java/lang/VMObject",                          vm_object},
    {"java/lang/VMThread",                          vm_thread},
    {"java/lang/VMSystem",                          vm_system},
    {"java/lang/VMString",                          vm_string},
    {"java/lang/VMRuntime",                         vm_runtime},
    {"java/lang/VMThrowable",                       vm_throwable},
    {"java/lang/VMClassLoader",                     vm_classloader},
    {"java/lang/reflect/VMField",                   vm_reflect_field},
    {"java/lang/reflect/VMMethod",                  vm_reflect_method},
    {"java/lang/reflect/VMConstructor",             vm_reflect_constructor},
    {"java/security/VMAccessController",            vm_access_controller},
    {"gnu/classpath/VMSystemProperties",            vm_system_properties},
    {"gnu/classpath/VMStackWalker",                 vm_stack_walker},
    {"java/lang/management/VMManagementFactory",    vm_management_factory},
    {"gnu/java/lang/management/VMThreadMXBeanImpl", vm_threadmx_bean_impl},
    {"sun/misc/Unsafe",                             sun_misc_unsafe},
    {"jamvm/java/lang/VMClassLoaderData$Unloader",  vm_class_loader_data},
    {"java/util/concurrent/atomic/AtomicLong",      concurrent_atomic_long},
    {NULL,                                          NULL}
};
