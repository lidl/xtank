#include "/usr/include/mips/asm.h"
#include "/usr/include/mips/regdef.h"
LEAF(setjmp)
addiu sp,sp,-32
sw    ra,28(sp)
sw    a0,24(sp)
jal   sigblock
move  a0,zero
lw    v1,24(sp)
move  a0,zero
move  a1,v1
addiu a1,a1,0
jal   sigstack
sw    v0,4(v1)
lw    a0,24(sp)
lw    ra,28(sp)
addiu sp,sp,32
cfc1  v0,$31
cfc1  v0,$31
sw    ra,8(a0)
sw    gp,124(a0)
sw    v0,280(a0)
lui   v0,0xaced
ori   v0,v0,0xbade
sw    v0,12(a0)
move  v0,zero
sw    sp,128(a0)
sw    s0,76(a0)
sw    s1,80(a0)
sw    s2,84(a0)
sw    s3,88(a0)
sw    s4,92(a0)
sw    s5,96(a0)
sw    s6,100(a0)
sw    s7,104(a0)
sw    s8,132(a0)
swc1  $f20,232(a0)
swc1  $f21,236(a0)
swc1  $f22,240(a0)
swc1  $f23,244(a0)
swc1  $f24,248(a0)
swc1  $f25,252(a0)
swc1  $f26,256(a0)
swc1  $f27,260(a0)
swc1  $f28,264(a0)
swc1  $f29,268(a0)
swc1  $f30,272(a0)
j     ra
swc1  $f31,276(a0)
.end  setjmp
LEAF(longjmp)
lw    s0,128(a0)
nop
sltu  $at,s0,sp
nop
lw    v0,12(a0)
lui   $at,0xaced
ori   $at,$at,0xbade
bne   v0,$at,botch
nop
sw    a1,20(a0)
li    v0,103
syscall
XLEAF(botch)
jal   longjmperror
nop
jal   setjmp
nop
.end  longjmp
