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

inline vms$pointer
dbg$get_registers_amd64(void)
{
    vms$pointer         sp;

    asm("movq %%rsp, %%rax; movq %%rax, %0\n" :"=r"(sp)::"%rax");

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

