;
; Xtank
;
; $Id$

;
; This was sent out over Netnews this past summer, so I am assuming
; it is ok to freely distribute it with Xtank.
;
; Routine to flush and synchronize data and instruction caches
; for dynamic loading
;
; Copyright Hewlett-Packard Co. 1985
;

	.code

; flush_cache(addr, len) - executes FDC and FIC instructions for every cache
; line in the text region given by the starting address in arg0 and
; the length in arg1.  When done, it executes a SYNC instruction and
; the seven NOPs required to assure that the cache has been flushed.
;
; Assumption:  the cache line size must be at least 16 bytes.

	.proc
	.callinfo
	.export	flush_cache,entry
flush_cache
	.enter
	ldsid	(0,%arg0),%r1
	mtsp	%r1,%sr0
	ldo	-1(%arg1),%arg1
	copy	%arg0,%arg2
	copy	%arg1,%arg3

	fdc	%arg1(0,%arg0)
loop1	addib,>,n	-16,%arg1,loop1	; decrement by cache line size
	fdc	%arg1(0,%arg0)
	; flush first word at addr, to handle arbitrary cache line boundary
	fdc	0(0,%arg0)
	sync

	fic	%arg3(%sr0,%arg2)
loop2	addib,>,n	-16,%arg3,loop2	; decrement by cache line size
	fic	%arg3(%sr0,%arg2)
	; flush first word at addr, to handle arbitrary cache line boundary
	fic	0(%sr0,%arg2)

	sync
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	.leave
	.procend

	.end
