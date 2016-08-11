/*
 * ARMv7 bare metal binary instrumentation trampolines
 *
 * Copyright (c) 2016 Aalto University
 */
#include "lib/cfa_common.h"

.arch_extension sec

.section .hook

/*---------------------------------------------------------------------------*/
/* hooking trampoline for f(void) { return 0; } hook for debugging purposes  */
/*---------------------------------------------------------------------------*/

__hook_ret0:
	mov	r0, #0
	bx	lr

/*---------------------------------------------------------------------------*/
/* hooking trampoline for branch instruction                                 */
/*---------------------------------------------------------------------------*/

__hook_b:
	push	{r0-r4}       /* store scratch registers */

	ldr	r0, =__btbl_start
	ldr	r1, =__btbl_end
	mov	r4, lr
	bl	_btbl_bsearch

_hook_b_ret:                  /* cleanup and return (branch)             */
	mov	lr, r4
	add	r0, #4        /* advance r0 to entry dest addr           */
	ldr	r12, [r0]     /* retrieve branch target addr             */

	mov	r0, #CFA_EVENT_B

	sub	r1, lr, #4
	mov	r2, r12

	ldr	r3, __shdw_lr
	ldr	lr, [r3]      /* restore stored lr value                 */

	mov	r3, lr

	smc	#0

	pop	{r0-r4}       /* restore scratch registers               */
	bx	r12           /* return                                  */

/*---------------------------------------------------------------------------*/
/* hooking trampoline for branch w/ link instruction                         */
/*---------------------------------------------------------------------------*/

__hook_bl:
	push	{r0-r4}       /* store scratch registers */

	ldr	r0, =__btbl_start
	ldr	r1, =__btbl_end
	mov	r4, lr
	bl	_btbl_bsearch

_hook_bl_ret:                 /* cleanup and return (branch w/ link)     */
	mov	lr, r4
	add	r0, #4        /* advance r0 to entry dest addr           */
	ldr	r12, [r0]     /* retrieve branch target addr             */

	mov	r0, #CFA_EVENT_BL
	sub	r1, lr, #4
	mov	r2, r12
	mov	r3, lr

	smc	#0

	ldr	r0, __shdw_lr
	str	lr, [r0]      /* store lr value                          */

	pop	{r0-r4}       /* restore scratch registers               */
	bx	r12           /* return                                  */

/*---------------------------------------------------------------------------*/
/* branch table binary search routine */
/*---------------------------------------------------------------------------*/

_btbl_bsearch:
	cmp	r0, r1        /* has end of auxiliary data been reached? */
	bgt	_btbl_bsearch_error

	add	r2, r0, r1
	mov	r2, r2, lsr #1
	bic 	r2, r2, #7    /* entry dest addr at 8-byte */

	ldr	r3, [r2]
	add	r3, #4        /* adjust source addr by 4 to match lr     */
	cmp	r3, r4        /* has matching source address been found? */

	bgt	_btbl_bsearch_right  /* no match, search lower addrs     */
	blt	_btbl_bsearch_left   /* no match, search higher addrs    */

	mov	r0, r2        /* match! branch to handler                */
	bx	lr
_btbl_bsearch_right:
	mov	r1, r2
	b	_btbl_bsearch
_btbl_bsearch_left:
	mov	r0, r2
	b	_btbl_bsearch
_btbl_bsearch_error:
	mov	r0, #CFA_EVENT_ERROR
	sub	r1, r4, #4
	smc	#0

/*---------------------------------------------------------------------------*/
/* hooking trampoline for branch and exchange where operand is link register */
/*---------------------------------------------------------------------------*/

__hook_bx_lr:
	push	{r0-r3}           /* store scratch registers */

	mov	r0, #CFA_EVENT_BX_LR
	mov	r1, lr            /* keep hook trigger addr in r1 */
	sub	r1, #4

	ldr	r3, __shdw_lr
	ldr	r2, [r3]          /* restore stored lr value */
	mov	lr, r2
	mov	r3, lr

	smc	#0

	pop	{r0-r3}           /* restore scratch registers */
	bx	lr

/*---------------------------------------------------------------------------*/
/* hooking trampoline for pop of frame pointer and return address off stack  */
/*---------------------------------------------------------------------------*/

__hook_pop_r3_r4_fp_pc:
	pop	{r3}
__hook_pop_r4_fp_pc:
	pop	{r4}
__hook_pop_fp_pc:
	push	{r0-r3}           /* store scratch registers */

	mov	r0, #CFA_EVENT_POP_FP_PC

	mov	r1, lr            /* keep hook trigger addr in r1 */
	sub	r1, #4

	add	r2, sp, #20       /* addr of stored lr in stack */
	ldr	r2, [r2]          /* load stored lr into r2     */

	ldr	r3, __shdw_lr
	ldr	lr, [r3]          /* restore stored lr value */
	mov	r3, lr

	smc	#0

	pop	{r0-r3}           /* restore scratch registers */
	pop	{fp, pc}

/*---------------------------------------------------------------------------*/
/* hooking trampoline for pop of frame pointer and link register off stack  */
/*---------------------------------------------------------------------------*/

__hook_pop_fp_lr:
	pop	{fp, r12}
	push	{r0-r3}           /* store scratch registers */

	mov	r0, r12
	mov	r1, lr
	mov	lr, r0
	mov	r12, r1           /* keep hook trigger addr in r0 */

	ldr	r0, __shdw_lr
	str	lr, [r0]          /* store lr value */

#ifdef INSTR_AUX_HOOKS_TRAP_INTO_SW
	mov	r0, #CFA_EVENT_POP_FP_LR
	sub	r1, r12, #4
	mov	r2, r12
	mov	r3, lr

	smc	#0
#endif

	pop	{r0-r3}           /* restore scratch registers */
	bx	r12

/*---------------------------------------------------------------------------*/
/* hooking trampoline for branch w/ link and exchange from r3                */
/*---------------------------------------------------------------------------*/
__hook_blx_r3:
	mov	r12, r3           /* save destination addr in r12 */
	push	{r0-r3}           /* store scratch registers */

	mov	r0, #CFA_EVENT_BLX_R3

	mov	r1, lr            /* keep hook trigger addr in r1 */
	sub	r1, #4

	mov	r2, r12           /* destination addr to r2*/

	ldr	r3, __shdw_lr
	str	lr, [r3]          /* store lr value */
	mov	r3, lr

	smc	#0

	pop	{r0-r3}           /* restore scratch registers */
	bx	r12

/*---------------------------------------------------------------------------*/
/* hook data                                                                 */
/*---------------------------------------------------------------------------*/
__shdw_lr:
	.word 0

