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

#include "freevms/freevms.h"

#define arch_specific(func)         CONCAT(func, ARCH)
#define CONCAT(a, b)                XCAT(a, _, b)
#define XCAT(a, c, b)               a##c##b

static inline vms$pointer
dbg$direct(vms$pointer reg)
{
    vms$pointer         *ireg;

    ireg = (vms$pointer *) reg;
    return((*ireg));
}

#ifdef AMD64
#   include "./amd64.h"
#endif

void
dbg$backtrace(void)
{
    vms$pointer         sp;

    sp = arch_specific(dbg$get_registers)();

    notice("\nBacktrace:\n\n");
    arch_specific(dbg$backtrace)(sp);
    notice("\n");

    return;
}

