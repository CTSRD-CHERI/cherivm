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

#ifdef __mips64__


#ifdef __CHERI__

#ifndef _MIPS_SZCAP
#error Can't determine capability size
#endif
#ifndef _MIPS_SZLONG
#error Can't determine integer register size
#endif
#define CAP_SIZE (_MIPS_SZCAP / 8)
#define LONG_SIZE (_MIPS_SZLONG / 8)

/* On function entry parameters:
 * $a0 cap_args
 * $a1 cap_arg_count
 * $a2 int_args
 * $a3 int_arg_count
 * $a4 operand stack
 * $a5 method number
 * $a6 return value signature
 *
 * $c3 env
 * $c4 sandbox code
 * $c5 sandbox data
 */
.text
.align 8
.set noreorder
.globl callSandboxedJNIMethod
    .ent callSandboxedJNIMethod
    .type callSandboxedJNIMethod, @function
callSandboxedJNIMethod:
    // Spill things that we'll need later.  Stack alignment must be 32 bytes
    // (16 CHERI128, but it doesn't cost us much to waste 16 bytes).
    daddiu   $sp, $sp, -64
    sd       $ra, 56($sp)
    sd       $fp, 48($sp)
    sd       $gp, 40($sp)
    sd       $a4, 32($sp)
    sd       $a6, 24($sp)
    move     $fp, $sp

    // Set up $gp
    lui      $at, %hi(%neg(%gp_rel(callSandboxedJNIMethod)))
    daddu    $at, $at, $25
    daddiu   $gp, $at, %lo(%neg(%gp_rel(callSandboxedJNIMethod)))

    // Move the object / class and method number to the correct argument
    // registers
    cmove    $c1, $c3
    cmove    $c2, $c4
    move     $v0, $a5

    // FIXME: on-stack arguments
    // Load capability arguments.  Some of these will be null.
    clc      $c3, $a0, (CAP_SIZE * 0)($c0)
    clc      $c4, $a0, (CAP_SIZE * 1)($c0)
    clc      $c5, $a0, (CAP_SIZE * 2)($c0)
    clc      $c6, $a0, (CAP_SIZE * 3)($c0)
    clc      $c7, $a0, (CAP_SIZE * 4)($c0)
    clc      $c8, $a0, (CAP_SIZE * 5)($c0)
    clc      $c9, $a0, (CAP_SIZE * 6)($c0)
    clc      $c10, $a0, (CAP_SIZE * 7)($c0)
    // Load integer arguments.  Some will be zero.
    ld       $a0, (LONG_SIZE * 0)($a2)
    ld       $a1, (LONG_SIZE * 1)($a2)
    ld       $a3, (LONG_SIZE * 3)($a2)
    ld       $a4, (LONG_SIZE * 4)($a2)
    ld       $a5, (LONG_SIZE * 5)($a2)
    ld       $a6, (LONG_SIZE * 6)($a2)
    ld       $a7, (LONG_SIZE * 7)($a2)

    ld       $t9, %call16(cheri_invoke)($gp)
    jalr     $t9
    ld       $a2, (LONG_SIZE * 2)($a2) // Delay slot

    // FIXME: non-doubleword returns
    // FIXME: If the return value is a capability then we need to transform it
    // (probably unseal it?)

    // Reload the top of the object stack and store the return value there.
    ld       $a4, 32($sp)
    ld       $a6, 24($sp)
    sd       $v0, 0($a4)
    // Move the top of the object stack
    daddiu   $v0, $a4, 8

    // Epilog
    move     $sp, $fp
    ld       $gp, 40($sp)
    ld       $fp, 48($sp)
    ld       $ra, 56($sp)
    jr       $ra
    daddiu   $sp, $sp, 64
.end callJNIMethod

#endif // __CHERI__

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

#else // __mips64__

        .text
        .align 2
        .globl callJNIMethod
	.ent callJNIMethod
	.type callJNIMethod, @function

/* On function entry parameters:
$4 = JNIEnv
$5 = Class if static or NULL
$6 = sig
$7 = extra args
16(sp) = operand stack
20(sp) = func pntr
*/

callJNIMethod:
	.frame $fp,8,$0
	.mask 0x00000000,0
	.fmask 0x00000000,0

	/* Fixed part of frame */
	subu $sp,8

	/* save registers */
	sw $31,4($sp)
	sw $fp,0($sp)

	/* set frame pointer to bottom of fixed frame */
	move $fp,$sp

	/* create variable frame area */
	subu $sp,$7

	/* setup operand and args stack pntrs */
	lw $8,24($fp)
	addu $9,$sp,8

        /* if instance method set second argument
           to object pntr  */

	bne $5,0,scan_sig
	lw $5,0($8)
	addu $8,4

scan_sig:
	lbu $7,1($6)
	addu $6,1

	beq $7,41,done    /* ')' */

	lw $10,0($8)
	addu $8,4

	beq $7,68,double  /* 'D' */
	beq $7,74,double  /* 'J' */

skip_brackets:
	bne $7,91,check_ref /* '[' */
	lbu $7,1($6)
	addu $6,1
	j skip_brackets

check_ref:
	bne $7,76,single /* 'L' */

skip_ref:
	lbu $7,1($6)
	addu $6,1
	bne $7,59,skip_ref /* ';' */
	
single:
	sw $10,0($9)
	addu $9,4
	j scan_sig

double:
	/* Ensure address is 8-byte aligned */
	addu $9,7
	and $9,~7

	sw $10,0($9)
	lw $7,0($8)
	addu $8,4
	sw $7,4($9)
	addu $9,8
	j scan_sig

done:
	/* store pntr to sig return value */
	sw $6,16($fp)

	/* arguments setup, load the argument
	   registers and call the function */

	lw $6, 8($sp)
	lw $7,12($sp)
	
	/* load function pntr */
	lw $25,28($fp)

	/* call the function */
	jalr $25

	/* restore return sig pntr */
	lw $6,16($fp)

	/* Reload operand stack pntr to store
	   return value */
	lw $8,24($fp)

	/* Get return type */
	lbu $9,1($6)

	beq $9,86,return     /* 'V' */
	beq $9,74,ret_long   /* 'J' */
	beq $9,68,ret_double /* 'D' */

#ifdef __mips_hard_float
	beq $9,70,ret_float  /* 'F' */
#endif

	/* int */
	sw $2,0($8)
	addu $8,4
	j return

#ifdef __mips_hard_float
ret_float:
	swc1 $f0,0($8)
	addu $8,4
	j return
#endif

ret_double:
#ifdef __mips_hard_float
	swc1 $f0,0($8)
	swc1 $f1,4($8)
	addu $8,8
	j return
#endif

ret_long:
	sw $2,0($8)
	sw $3,4($8)
	addu $8,8

return:
	/* restore saved registers */

	move $sp,$fp
	lw $31,4($sp)
	lw $fp,0($sp)

	/* pop frame */
	addu $sp,$sp,8

	/* return value is operand stack pntr */
	move $2,$8

	j $31
.end callJNIMethod

#endif // __mips64__

#endif
