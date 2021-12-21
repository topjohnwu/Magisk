@ RUN: %cxx %flags %link_flags %s -o %t.exe
@ RUN: %t.exe
@ UNSUPPORTED: libcxxabi-no-exceptions

@ PURPOSE: Check that 0x00 is a valid value for ttype encoding.  LLVM and
@ GCC 4.6 are generating 0x00 as ttype encoding.  libc++abi should provide
@ legacy support.

@ NOTE:
@
@ This file is generated from the following C++ source code:
@
@ ```
@ int main() {
@   try {
@     throw 5;
@   } catch (int i) {
@     if (i != 5)
@       abort();
@     return 0;
@   }
@ }
@ ```

	.syntax unified

	.text
	.globl	main
	.p2align	2
	.type	main,%function
main:                                   @ @main
.Lfunc_begin0:
	.fnstart
@ BB#0:                                 @ %entry
	.save	{r11, lr}
	push	{r11, lr}
	.setfp	r11, sp
	mov	r11, sp
	mov	r0, #4
	bl	__cxa_allocate_exception
	mov	r1, #5
	str	r1, [r0]
.Ltmp0:
	ldr	r1, .LCPI0_0
	mov	r2, #0
	bl	__cxa_throw
.Ltmp1:

@ BB#2:                                 @ %lpad
.Ltmp2:
	bl	__cxa_begin_catch
	ldr	r0, [r0]
	cmp	r0, #5
	bne	.LBB0_4
@ BB#3:                                 @ %if.end
	bl	__cxa_end_catch
	mov	r0, #0
	pop	{r11, lr}
	bx	lr
.LBB0_4:                                @ %if.then
	bl	abort
	.p2align	2
@ BB#5:
.LCPI0_0:
	.long	_ZTIi
.Lfunc_end0:

	.size	main, .Lfunc_end0-main
	.globl	__gxx_personality_v0
	.personality __gxx_personality_v0
	.handlerdata
	.p2align	2
GCC_except_table0:
.Lexception0:
	.byte	255                     @ @LPStart Encoding = omit
	.byte	0                       @ @TType Encoding = absptr
	.asciz	"\257\200"              @ @TType base offset
	.byte	3                       @ Call site Encoding = udata4
	.byte	39                      @ Call site table length
	.long	.Lfunc_begin0-.Lfunc_begin0 @ >> Call Site 1 <<
	.long	.Ltmp0-.Lfunc_begin0    @   Call between .Lfunc_begin0 and .Ltmp0
	.long	0                       @     has no landing pad
	.byte	0                       @   On action: cleanup
	.long	.Ltmp0-.Lfunc_begin0    @ >> Call Site 2 <<
	.long	.Ltmp1-.Ltmp0           @   Call between .Ltmp0 and .Ltmp1
	.long	.Ltmp2-.Lfunc_begin0    @     jumps to .Ltmp2
	.byte	1                       @   On action: 1
	.long	.Ltmp1-.Lfunc_begin0    @ >> Call Site 3 <<
	.long	.Lfunc_end0-.Ltmp1      @   Call between .Ltmp1 and .Lfunc_end0
	.long	0                       @     has no landing pad
	.byte	0                       @   On action: cleanup
	.byte	1                       @ >> Action Record 1 <<
                                        @   Catch TypeInfo 1
	.byte	0                       @   No further actions
                                        @ >> Catch TypeInfos <<
	.long	_ZTIi(target2)          @ TypeInfo 1
	.p2align	2
	.fnend
