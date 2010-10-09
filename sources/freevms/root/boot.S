	.text
	.global _start
	.global _stext

_stext:
_start:
0:	leaq	stack, %rsp
	pushq	%rbx
	pushq	%rax
	callq	__L4_Init
	pushq	$_return_from_main
	jmp		main

_return_from_main:
	int $3
	jmp 1f
	.ascii "System stopped."
1:	jmp _return_from_main

	.bss

	.align	16
	.space	4096 * 64 /* 64k */

stack:
