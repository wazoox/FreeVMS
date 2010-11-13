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

static void
sys$pagefault(L4_ThreadId_t caller, vms$pointer addr, vms$pointer ip,
		vms$pointer priv)
{
	// fpage [...........0RWX]

	struct memsection		*memsection;

	struct thread			*thread;

	vms$pointer				ref;
	vms$pointer				size;

	// Read privileges
	priv = (priv & 0xf0000) >> 16;

	// Find memory section it belongs too
	if ((memsection = vms$objtable_lookup((void *) addr)) == NULL)
	{
		notice(MEM_F_MEMSEC "no memory section\n");
		goto fail;
	}

	ref = (vms$pointer) memsection;
	notice("%lx\n", ref);

	if (sec$check(caller, ref) == 0)
	{
		// The memory is now backed in our address space.
		size = vms$pagefault(addr);

		if (size == 0)
		{
			notice(MEM_F_OUTMEM "out of memory at IP=$%016lX, TID=$%lX\n",
					ip, jobctl$threadno(L4_ThreadNo(caller)));
			goto fail;
		}

		/*
		vms$fpage_set_base(fp, vms$page_round_down(addr, size));
		vms$fpage_set_mode(fp, VMS$FPAGE_MODE_MAP);
		vms$fpage_set_page(fp, L4_Fpage(vms$page_round_down(addr, size), size));
		vms$fpage_set_permissions(fp, priv);
		*/
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

void
sys$loop()
{
	int						running;

	L4_ThreadId_t			partner;
	L4_MsgTag_t				tag;
	L4_Msg_t				msg;

	running = 1;
	tag = L4_Wait(&partner);

	while(running)
	{
		L4_Clear(&msg);
		L4_Store(tag, &msg);

		if ((tag.raw & L4_REQUEST_MASK) == L4_PAGEFAULT)
		{
			sys$pagefault(partner, L4_Get(&msg, 0), L4_Get(&msg, 1),
					tag.raw);
		}

		L4_ReplyWait(partner, &partner);
	}

    return;
}
