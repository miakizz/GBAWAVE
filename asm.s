@ fixed fastfmul(fixed x, fixed y)
@ Multiply two 16.16 fixed-point numbers.

@ int dv(int num, int den)
@ Divide two signed integers.

.THUMB
.THUMB_FUNC
.ALIGN
.GLOBL  dv

dv:
  cmp r1, #0
  beq 0f
  swi 6
  bx lr
0:
  ldr r0, =0x7fffffff
  bx lr


@ int fracumul(unsigned int x, unsigned int frac)
@ Multiply by a 0.32 fractional number between 0 and 1.
@ Used for fast division by a constant.

.ARM
.ALIGN
.GLOBL  fracumul

fracumul:
  umull r1,r2,r0,r1
  mov r0, r2
  bx lr

