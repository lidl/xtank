#****************************************************************************/
#* mpvaxsj.s -- VAX ASSEMBLY FOR C setjmp & longjmp FUNCTIONS 	  	    */
#* Created:  11/29/87		Release:  0.7		Version:  12/3/87   */
#****************************************************************************
#(c) Copyright 1987 by Michael Benjamin Parker           (USA SS# 557-49-4130)
#
#All Rights Reserved unless specified in the following include files: */
##include "mptsk.cpy" /*
#
#DO NOT REMOVE OR ALTER THIS NOTICE AND ITS PROVISIONS.
#****************************************************************************/
#****************************************************************************/
#* OVERVIEW: see mpvaxsj.h
# This is a reimplementation of the standard "C" setjmp and longjmp functions.
#****************************************************************************/
#****************************************************************************/
#

# Format for MPVAXSJ:
#	0:	sp (r14)
#	1:	SPA | 1 | 0 | REGISTER MASK <11:0> | PSW <15:5> | 0
#	2:	ap (r12)
#	3:	fp (r13)
#	4:	pc (r15)
#	5:	r1
#	....
#	15:	r11
#	16:	No of Arguments (assumes "calls" call)
#
# The "setjmp" function pushes all the remaining registers (r1-r11) onto the
# stack frame (see 1111 1111 1110b = FFEh register mask) on entry, then
# copies the entire stack frame (64d bytes or 16 int's) into the MPVAXSJ
# buffer and returns 0.
# Note: BSD 4.2 Vax cc only assumes registers r6-r11 saved across function
#	calls (mask FC0h), but I save them all just in case.
 # ENTRY(mpvaxsj_setjmp, 0xffe)
	.text
	.align 1
	.globl	_mpvaxsj_setjmp
_mpvaxsj_setjmp:

	.word 0xffe		# Save all registers r1-r11
	movl	4(ap),	r0	# Get the address of the MPVAXSJ buffer
	movl	fp,	(r0)	# Store the Stack Pointer
	movc3	$64,4(fp),4(r0)	# Store the Stack Frame (except Cond. Codes)
	clrl	r0		# Return 0
	ret

# The "longjmp" function returns the argument passed to it using the MPVAXSJ
# buffer (previously set) as the stack frame.
 # ENTRY(mpvaxsj_longjmp, 0)
	.text
	.align 1
	.globl _mpvaxsj_longjmp
_mpvaxsj_longjmp:

	.word 0			# Don't save anything (we'll never return)
	movl	8(ap),	r6	# Save Return Value
	movl	4(ap),	r0	# Get the address of the MPVAXSJ buffer
	movl	(r0),	fp	# Restore the old stack pointer
	movc3	$64,4(r0),4(fp)	# Restore the Stack Frame (except Cond. Codes)
	clrl	(fp)		# Clear the Condition Codes
	movl	r6,	r0	# Return the value passed
	ret




