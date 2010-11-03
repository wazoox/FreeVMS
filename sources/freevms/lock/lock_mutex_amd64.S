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

/* int try_lock(mutex_t m, vms$pointer me) 
 *  - m: the mutex we are trying to grab
 *  - me: the thread id of the thread trying to aquire the lock
 *  => true if the lock was aquired, false otherwise
 */

#include "freevms/arch.h"

#ifdef AMD64
    .text
    .global     try_lock_amd64
    .type       try_lock_amd64, @function
    .align		16

try_lock_amd64:
    // Setup the local variables
    pushq       %rbp
    movq        %rsp, %rbp
    movq        24(%rbp), %rcx      /* %rcx = &m->holder     	*/
    movq		16(%rbp), %rdx		/* %rdx = me				*/

    // Test the lock :
	// if (%rcx) == $0, write %rdx into (%rcx)
	// if not, write (%rcx) into %rax.
	movq		$0, %rax
	lock
	cmpxchgq	%rdx, (%rcx)

    // What is the result
	movq		(%rcx), %rcx		/* %rcx = m->holder         */
	cmpq		16(%rbp), %rcx		/* m->holder == me          */
    sete        %al
    andq        $0xff, %rax

    popq        %rbp
    ret
#endif
