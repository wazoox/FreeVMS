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

/* int try_lock(mutex_t m, uintptr_t me) 
 *  - m: the mutex we are trying to grab
 *  - me: the thread id of the thread trying to aquire the lock
 *  => true if the lock was aquired, false otherwise
 */

#include "freevms/arch.h"

#ifdef AMD64
	.text
	.global		try_lock_amd64
	.type		try_lock_amd64, @function
	.align

try_lock_amd64:
	// Setup the local variables
	pushq		%rbp
	movq		%rsp, %rbp
	movq		24(%rbp), %rcx		/* %rcx = &m->holder	*/
	movq		40(%rbp), %rdx		/* %rdx = me			*/

	// Test the lock (only in 32 bits mode)
	movq		$0, %rax
	lock
	cmpxchg		%edx, (%rcx)

	// What is the result
	movq		24(%rbp), %rax
	movq		(%rax), %rax		/* %rax = m->holder		*/
	cmpq		40(%rbp), %rax		/* m->holder == me		*/
	sete		%al
	andq		$255, %rax

	popq		%rbp
	ret
#endif
