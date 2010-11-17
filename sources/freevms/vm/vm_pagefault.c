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

void
vms$pagefault(L4_ThreadId_t caller, vms$pointer addr, vms$pointer ip,
		vms$pointer tag)
{
	// fpage [...........0RWX]

	L4_Fpage_t				fpage;

	L4_Msg_t				msg;

	struct memsection		*memsection;

	struct thread			*thread;

	vms$pointer				priv;
	vms$pointer				ref;
	vms$pointer				size;

	// Read privileges
	priv = (tag & 0xf0000) >> 16;

	// Find memory section it belongs too
	if ((memsection = vms$objtable_lookup((void *) addr)) == NULL)
	{
		notice(MEM_F_MEMSEC "no memory section\n");
		goto fail;
	}

	ref = (vms$pointer) memsection;

dbg$sigma0(10000000);
	if (sec$check(caller, ref) == 0)
	{
		extern fpage_alloc			pm_alloc;
		vms$pointer phys;
		vms$pointer virt;

		// The memory is now backed in our address space.
		if (memsection->flags == VMS$MEM_USER)
		{
			// For user backed memsections, we have no way of telling how
			// big of an area is mapped, so we map as little as possible.

			size = vms$min_pagesize();
			virt = vms$page_round_down(addr, vms$min_pagesize());
			fpage = L4_Fpage(virt, size);
		}
		else
		{
			fpage = vms$biggest_fpage(addr, memsection->base,
					memsection->end);
			virt = L4_Address(fpage);
			size = L4_Size(fpage);
		}

notice("vms$pagefault(addr:%lx, s:%lx) [priv=%lx]\n", addr, size, priv);
notice("memsection %lx %lx\n", memsection->base, memsection->end);

		if (size == 0)
		{
			notice(MEM_F_OUTMEM "out of memory at IP=$%016lX, TID=$%lX\n",
					ip, jobctl$threadno(L4_ThreadNo(caller)));
			goto fail;
		}

notice("%lx %lx\n", memsection->flags, memsection->phys_active);

		// If VMS$MEM_INTERNAL is set, memory is mapped 1:1
		if (memsection->flags != VMS$MEM_INTERNAL)
		{
			// Find a free physical memory to map requested page.
			phys = vms$fpage_alloc_chunk(&pm_alloc, size);

			if (phys == INVALID_ADDR)
			{
				notice(MEM_F_OUTMEM "out of memory at IP=$%016lX, TID=$%lX\n",
						ip, jobctl$threadno(L4_ThreadNo(caller)));
				goto fail;
			}

notice("V=%lx P=%lx\n", virt, phys);
			vms$sigma0_map(virt, phys, size, priv);
		}
notice("After vms$sigma0_map\n");

		L4_Clear(&msg);
		L4_Append(&msg, L4_MapItem(fpage, addr));
		L4_Load(&msg);
	}
	else
	{
		notice(MEM_F_SECFLD "security check failed\n");
		goto fail;
	}

	return;

fail:
	L4_Stop(caller);
	thread = jobctl$thread_lookup(caller);
	jobctl$thread_delete(thread);
	return;
}
