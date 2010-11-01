/*
================================================================================
  FreeVMS (R)
  Copyright (C) 2010 Dr. BERTRAND Joël and all.

  This file is part of FreeVMS

  FreeVMS is free software; you can redistribute it and/or modify it
  under the terms of the CeCILL V2 License as published by the french
  CEA, CNRS and INRIA.
 
  FreeVMS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the CeCILL V2 License
  for more details.
 
  You should have received a copy of the CeCILL License
  along with FreeVMS. If not, write to info@cecill.info.
================================================================================
*/

#ifdef AMD64
    .text
    .global _start
    .global _stext

_stext:
_start:
0:  leaq    stack, %rsp
    pushq   %rbx
    pushq   %rax
    callq   __L4_Init
    pushq   $0
    pushq   $0
    pushq   $_return_from_main
    jmp     main
    popq    %r15
    popq    %r15

_return_from_main:
    int $3
    jmp 1f
    .ascii "System stopped."
1:  jmp _return_from_main

    .bss

    .align  16
    .space  4096 * 64 /* 256k */

stack:
#endif
