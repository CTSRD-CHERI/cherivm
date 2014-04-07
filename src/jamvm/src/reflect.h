/*
 * Copyright (C) 2009 Robert Lougher <rob@jamvm.org.uk>.
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

#define REF_SRC_FIELD  0
#define REF_DST_FIELD  0
#define REF_SRC_OSTACK 1
#define REF_DST_OSTACK 2

#define getPrimTypeIndex(cb) (cb->state - CLASS_PRIM)

#define getConsParamTypes(vm_cons_obj) \
    INST_DATA(vm_cons_obj, pObject, vm_cons_param_offset)

#define getMethodParamTypes(vm_method_obj) \
    INST_DATA(vm_method_obj, pObject, vm_mthd_param_offset)

#define getMethodReturnType(vm_method_obj) \
    INST_DATA(vm_method_obj, pClass, vm_mthd_ret_offset)

#define getFieldType(vm_field_obj) \
    INST_DATA(vm_field_obj, pClass, vm_fld_type_offset)

extern pMethodBlock getConsMethodBlock(pObject cons_ref_obj);
extern int getConsAccessFlag(pObject cons_ref_obj);
extern pMethodBlock getMethodMethodBlock(pObject mthd_ref_obj);
extern int getMethodAccessFlag(pObject mthd_ref_obj);
extern pFieldBlock getFieldFieldBlock(pObject fld_ref_obj);
extern int getFieldAccessFlag(pObject fld_ref_obj);

extern int vm_cons_param_offset, vm_mthd_param_offset, vm_mthd_ret_offset;
extern int vm_fld_type_offset;

extern pObject getClassConstructors(pClass class, int public);
extern pObject getClassMethods(pClass class, int public);
extern pObject getClassFields(pClass class, int public);
extern pObject getClassInterfaces(pClass class);
extern pObject getClassClasses(pClass class, int public);
extern pClass getDeclaringClass(pClass class);
extern pClass getEnclosingClass(pClass class);
extern pObject getEnclosingMethodObject(pClass class);
extern pObject getEnclosingConstructorObject(pClass class);
extern pObject getClassAnnotations(pClass class);
extern pObject getFieldAnnotations(pFieldBlock fb);
extern pObject getMethodAnnotations(pMethodBlock mb);
extern pObject getMethodParameterAnnotations(pMethodBlock mb);
extern pObject getMethodDefaultValue(pMethodBlock mb);
extern pObject getExceptionTypes(pMethodBlock mb);

extern pObject getReflectReturnObject(pClass type, void *pntr, int flags);
extern int widenPrimitiveValue(int src_idx, int dest_idx, void *src,
                               void *dest, int flags);
extern int unwrapAndWidenObject(pClass type, pObject arg, void *pntr,
                                int flags);
extern pObject invoke(pObject ob, pMethodBlock mb, pObject arg_array,
                      pObject param_types);

extern pMethodBlock mbFromReflectObject(pObject reflect_ob);
extern pFieldBlock fbFromReflectObject(pObject reflect_ob);

extern pObject createReflectConstructorObject(pMethodBlock mb);
extern pObject createReflectMethodObject(pMethodBlock mb);
extern pObject createReflectFieldObject(pFieldBlock fb);
extern pClass getReflectMethodClass();
