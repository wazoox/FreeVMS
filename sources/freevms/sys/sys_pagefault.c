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

// This pager is idempotent and only returns physical page. Virtual
// mapping is done in vms$pagefault.

void
sys$pagefault(L4_ThreadId_t caller, vms$pointer addr, vms$pointer ip,
        vms$pointer tag)
{
    // fpage [...........0RWX]

    L4_Fpage_t              fpage;

    L4_Msg_t                msg;

    struct memsection       *memsection;

    struct thread           *thread;

    vms$pointer             priv;
    vms$pointer             ref;

    // Read privileges
    priv = (tag & 0xf0000) >> 16;

    // Find memory section it belongs too
notice("sys$pagefault(addr=%lx)\n", addr);
    if ((memsection = sys$objtable_lookup((void *) addr)) == NULL)
    {
        notice(MEM_F_MEMSEC "no memory section\n");
        goto fail;
    }

    ref = (vms$pointer) memsection + priv;

notice("sys$pagefault(ref=%lx)\n", ref);
    if (sec$check(caller, ref) == 0)
    {
		fpage = L4_Fpage(sys$page_round_down(addr, vms$min_pagesize()),
				vms$min_pagesize());
        L4_Set_Rights(&fpage, L4_FullyAccessible);
        L4_Clear(&msg);
        L4_Append(&msg, L4_MapItem(fpage, L4_Address(fpage)));
        L4_Load(&msg);
    }
    else
    {
        notice(MEM_F_SECFLD "security check failed\n");
        goto fail;
    }

    return;

fail:
    L4_Clear(&msg);
    L4_Append(&msg, L4_MapItem(L4_Nilpage, L4_Address(L4_Nilpage)));
    L4_Load(&msg);

    L4_Stop(caller);
    thread = sys$thread_lookup(caller);
    sys$thread_delete(thread);
    return;
}
