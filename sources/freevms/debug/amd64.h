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

vms$pointer
dbg$get_registers_amd64(void)
{
    vms$pointer         reg;
    vms$pointer         sp;

    asm("movq %%rax, %0\n" :"=r"(reg));
    notice("  R0 [RAX] = $%016lX\n", reg);

    asm("movq %%rbx, %0\n" :"=r"(reg));
    notice("  R1 [RBX] = $%016lX\n", reg);

    asm("movq %%rcx, %0\n" :"=r"(reg));
    notice("  R2 [RCX] = $%016lX\n", reg);

    asm("movq %%rdx, %0\n" :"=r"(reg));
    notice("  R3 [RDX] = $%016lX\n", reg);

    asm("movq %%rsi, %0\n" :"=r"(reg));
    notice("  R4 [RSI] = $%016lX\n", reg);

    asm("movq %%rdi, %0\n" :"=r"(reg));
    notice("  R5 [RDI] = $%016lX\n", reg);

    asm("movq %%rbp, %0\n" :"=r"(reg));
    notice("  R6 [RBP] = $%016lX\n", reg);

    asm("movq %%rsp, %0\n" :"=r"(sp));
    notice("  R7 [RSP] = $%016lX\n", sp);

    asm("movq %%r8, %0\n" :"=r"(reg));
    notice("  R8       = $%016lX\n", reg);

    asm("movq %%r9, %0\n" :"=r"(reg));
    notice("  R9       = $%016lX\n", reg);

    asm("movq %%r10, %0\n" :"=r"(reg));
    notice("  R10      = $%016lX\n", reg);

    asm("movq %%r11, %0\n" :"=r"(reg));
    notice("  R11      = $%016lX\n", reg);

    asm("movq %%r12, %0\n" :"=r"(reg));
    notice("  R12      = $%016lX\n", reg);

    asm("movq %%r13, %0\n" :"=r"(reg));
    notice("  R13      = $%016lX\n", reg);

    asm("movq %%r14, %0\n" :"=r"(reg));
    notice("  R14      = $%016lX\n", reg);

    asm("movq %%r15, %0\n" :"=r"(reg));
    notice("  R15      = $%016lX\n", reg);

    return(sp);
}

void
dbg$backtrace_amd64(vms$pointer sp)
{
    unsigned int            i;

    vms$pointer             fp;

    i = 0;
    // First frame pointer
    fp = sp + sizeof(vms$pointer);

    do
    {
        sp = fp + sizeof(vms$pointer);
        notice("  <%02u> [$%016lX] -> $%016lX (%s)\n", i, sp, dbg$direct(sp),
                dbg$symbol(dbg$direct(sp)));

        // Previous frame pointer
        fp = dbg$direct(fp);
        i++;

		// Stack is ended by two 0x0 (16 bytes aligned stack)
    } while((dbg$direct(sp + sizeof(vms$pointer)) != 0) ||
			(dbg$direct(sp + (2 * sizeof(vms$pointer))) != 0));

    return;
}

