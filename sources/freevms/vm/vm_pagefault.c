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

vms$pointer
vms$pagefault(vms$pointer addr)
{
	struct memsection			*ms;

	ms = vms$objtable_lookup((void *) addr);

	// For user backed memsections, we have no way of telling how
	// big of an area is mapped, so we map as little as possible.

	if (ms->flags & VMS$MEM_USER)
	{
		return(vms$min_pagesize());
	}
	else
	{
		return(L4_Size(vms$biggest_fpage(addr, ms->base, ms->end)));
	}

	return(0);
}
