/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
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

#include  "config.h"

#ifndef USE_FFI
.text
.align 8
.set reorder
.globl callJNIMethod
    .ent callJNIMethod
    .type callJNIMethod, @function

/* 
    On function entry parameters:
        $a0 = JNIEnv
        $a1 = Class if static or NULL
        $a2 = sig     
        $a3 = extra args
        $a4 = operand stack, pointing to first argument
        $a5 = func pntr
        $a6 = args count

    Stack:
         0 = $fp
         8 = $ra
        16 = ???
        24 = ???
*/

callJNIMethod:
    .frame $fp,16,$0
    .mask 0x00000000,0
    .fmask 0x00000000,0

    // push frame
    daddiu $sp, $sp, -32

    // save registers
    sd $fp, 0($sp)
    sd $ra, 8($sp)
    move $fp, $sp

    // don't change the original opstack pointer, work on $t8
    move $t8, $a4
    
/* INSTANCE POINTER */

    // if class is NULL, get the first argument from opstack
    // and store it to $a1
    bne $a1, 0, prepare_args
    ld $a1, 0($t8)
    daddiu $t8, $t8, 8

/* ARGUMENT PREPARATION */

prepare_args:

    // pop args off the op stack and push on the native stack

    // create space on the native stack 
    dsubu $sp, $sp, $a3
    move $t3, $sp           // store the top of the stack

    // later advance $sp how many times? (max 6x 64b)
    daddu $t2, $zero, $zero

scan_signature:
    lbu $t0, 1($a2)
    daddiu $a2, $a2, 1

    // are we done?
    beq $t0, 41, ready    /* ')' */

    // load value from top of opstack
    ld $t1, 0($t8)
    daddiu $t8, $t8, 8

    // later advance increment (if reached max, skip)
    beq $t2, 48, later_adv_skip
    daddiu $t2, $t2, 8
later_adv_skip:

    // wide primitives => advance opstack once more
    beq $t0, 68, arg_double  /* 'D' */
    beq $t0, 74, arg_double  /* 'J' */

    // arrays => need to skip the brackets
skip_brackets:
    bne $t0, 91, check_ref  /* '[' */
    lbu $t0, 1($a2)
    daddiu $a2, $a2, 1
    b skip_brackets

    // objects => need to skip until semicolon
check_ref:
    bne $t0, 76, arg_single  /* 'L' */

skip_ref:
    lbu $t0, 1($a2)
    daddiu $a2, $a2, 1
    bne $t0, 59, skip_ref  /* ';' */
    b arg_single

arg_double:
    // just advance opstack once more
    daddiu $t8, $t8, 8
    
arg_single:
    // store value on the native stack
    sd $t1, 0($sp)
    daddiu $sp, $sp, 8
    b scan_signature

/* JUMP TO NATIVE FUNCTION
     - preserves: $a4 (op stack), $a2 (signature)
     - must jump with the address in $a5 (used in the function)
 */
ready:

    // store arguments which will be needed later
    sd $a4, 16($fp)
    sd $a2, 24($fp)

    // prepare jump address
    move $t9, $a5

    // load first six arguments to registers $a2-$a7
    // (doesn't matter if it's crap)
    move $sp, $t3
    ld $a2,  0($sp)
    ld $a3,  8($sp)
    ld $a4, 16($sp)
    ld $a5, 24($sp)
    ld $a6, 32($sp)
    ld $a7, 40($sp)

    // advance stack (num of args, max 6)
    daddu $sp, $sp, $t2

    // run the native method
    jalr $t9

    // restore preserved values
    ld $a4, 16($fp)
    ld $a2, 24($fp)

/* RETURN VALUE RETRIEVAL */

    // get first character of return type
    lbu $t0, 1($a2)

    // skip if no return value
    beq $t0, 86, return    // V

    // push return value on the stack
    sd $v0, 0($a4)

    // advance opstack pointer
    // if a wide primitive, need to advance two steps
    beq $t0, 74, ret_wide   /* 'J' */
    beq $t0, 68, ret_wide   /* 'D' */

    daddiu $a4, $a4, 8
    b return

ret_wide:
    daddiu $a4, $a4, 16

/* RETURN */
return:
    // restore saved registers
    move $sp, $fp
    ld $fp, 0($sp)
    ld $ra, 8($sp)

    // pop frame
    daddiu $sp, 32

    // return value is operand stack pointer
    move $v0, $a4

    // return
    jr $ra

.end callJNIMethod
#endif
