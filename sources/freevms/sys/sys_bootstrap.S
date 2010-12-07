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
    leaq    stack, %rsp
    callq   __L4_Init
    pushq   $0
    pushq   $0
    callq   freevms_main
    popq    %r15
    popq    %r15
    int $3
_loop:
    jmp _loop

    .bss

    .align  16
    .space  4096 * 64 /* 256k */

stack:
#endif
