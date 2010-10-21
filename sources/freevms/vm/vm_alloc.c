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
vms$fpage_clear_internal(struct fpage_alloc *alloc)
{
	struct fpage_list		*fpage;

	if (alloc->internal.active)
	{
		while(alloc->internal.base < alloc->internal.end)
		{
			/*
			if ((fpage = slab_cache_alloc(&fp_cache)) == NULL)
			{
				return;
			}

			fpage->fpage = l4e_biggest_fpage(alloc->internal.base,
					alloc->internal.base, alloc->internal.end);
			TAILQ_INSERT_TAIL(&alloc->flist[fp_order(fpage->fpage)],
					fpage, flist);
			alloc->internal.base += L4_Size(fpage->fpage);
			*/
		}

		alloc->internal.active = 0;
	}

	return;
}

void
vms$fpage_free_internal(struct fpage_alloc *alloc, vms$pointer base,
		vms$pointer end)
{
	if (alloc->internal.active)
	{
		vms$fpage_clear_internal(alloc);
	}

	alloc->internal.base = base;
	alloc->internal.end = end;
	alloc->internal.active = 1;

	return;
}
