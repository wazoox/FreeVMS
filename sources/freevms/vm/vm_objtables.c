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

static BTree _objtable;
static GBTree objtable = 0;

static struct slab_cache bt_cache =
		SLAB_CACHE_INITIALIZER(sizeof(struct sBTPage), &bt_cache);

struct memsection *
vms$objtable_lookup(void *addr)
{
	extern struct memsection_list		internal_memsections;

    struct memsection       			*memsection;

    struct memsection_node  			*first_ms;
    struct memsection_node  			*ms;

    int                     			r;

    r = ObjBTSearch(objtable, (vms$pointer) addr, &memsection);

    if ((r == BT_NOT_FOUND) || (r == BT_INVALID))
    {
        first_ms = internal_memsections.first;

        for(ms = first_ms; ms->next != first_ms; ms = ms->next)
        {
            if ((((vms$pointer) addr) >= ms->data.base) &&
                    (((vms$pointer) addr) <= ms->data.end))
            {
                return(&ms->data);
            }
        }

        return((struct memsection *) NULL);
    }

    return(memsection);
}

static int
insert(struct memsection *memsection)
{
	struct memsection			*ignored;

	if (ObjBTIns(objtable, memsection, &ignored) != BT_FOUND)
	{
		return(-1);
	}

	return(0);
}

static int
objtable_insert(struct memsection *memsection)
{
	int				r;

	r = 0;

	if (vms$pd_initialized)
	{
		r = insert(memsection);
	}

	return(r);
}

int
objtable_setup(struct memsection *ms, vms$pointer size, unsigned int flags)
{
	extern struct fpage_alloc   pm_alloc;
	extern struct fpage_alloc   vm_alloc;

	int							r;

	PANIC(!(flags & VMS$MEM_NORMAL));

	ms->flags = flags;
	ms->base = vms$fpage_alloc_chunk(&vm_alloc, size);

	if (ms->base == INVALID_ADDR)
	{
		return(-1);
	}

	ms->end = ms->base + (size - 1);

	if (!(flags & VMS$MEM_USER))
	{
		ms->phys.list = vms$fpage_alloc_list(&pm_alloc, ms->base, ms->end);

		if (TAILQ_EMPTY(&ms->phys.list))
		{
			vms$fpage_free_chunk(&vm_alloc, ms->base, ms->end);
			return(-1);
		}

		memsection_back(ms);
	}

	if ((r = objtable_insert(ms)) != 0)
	{
		vms$fpage_free_chunk(&vm_alloc, ms->base, ms->end);
		vms$fpage_free_list(&pm_alloc, ms->phys.list);
	}

	return(r);
}

struct sBTPage *
ObjAllocPage(PagePool *pool)
{
	return((struct sBTPage *) vms$slab_cache_alloc(&bt_cache));
}

void
ObjFreePage(PagePool *pool, struct sBTPage *page)
{
	vms$slab_cache_free(&bt_cache, page);
	return;
}
