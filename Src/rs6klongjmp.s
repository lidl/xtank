;
; XTank
;
; $Id$
;

;-----mylongjmp.s------------------------------------------------------------
	    .file  "mylongjmp.s"
	    .extern .longjmperror
	    .extern .abort
	    .extern .sigcleanup
	    .extern .jmprestfpr
	    .extern c56


	    .globl .mylongjmp	
.mylongjmp:    l   13,0xc(3)
	     cmp   0,1,13
.mylongjmp2:  mr   13,3
	      mr   14,4
	     stu   1,-56(1)
	      bl   .sigcleanup
	       l   2,0x14(1)
	     cal   1,0x38(1)
	      mr   3,13
	      mr   4,14
	       l   5,0x8(3)
	       l   1,0xc(3)
	       l   2,0x10(3)
	      bl   .jmprestfpr
	    cmpi   0,4,0x0
	    mtlr   5
	      lm   13,0x14(3)
	       l   5,0x60(3)
	   mtcrf   56,5
	      mr   3,4
	     bne   .mylongjmp3
	     lil   3,0x1
.mylongjmp3:  br
