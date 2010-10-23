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
static GBTree objtable;

static struct slab_cache ms_cache =
		SLAB_CACHE_INITIALIZER(sizeof(struct memsection_node), &ms_cache);

static struct memsection_list internal_memsections =
{
	(struct memsection_node *) &internal_memsections,
	(struct memsection_node *) &internal_memsections
};

struct memsection *
vms$objtable_lookup(void *addr)
{
	struct memsection		*memsection;

	struct memsection_node	*first_ms;
	struct memsection_node	*ms;

	int						r;

	vms$debug(__func__);

	r = ObjBTSearch(objtable, (vms$pointer) addr, &memsection);

	if ((r == BT_NOT_FOUND) || (r == BT_INVALID))
	{
		first_ms = internal_memsections.first;

		for(ms = first_ms; ms->next != first_ms; ms = ms->next)
		{
			if (((vms$pointer) addr >= ms->data.base) &&
					((vms$pointer) addr <= ms->data.end))
			{
				return(&(ms->data));
			}
		}

		return((struct memsection *) NULL);
	}

	return(memsection);
}

void
vms$initmem(vms$pointer zone, vms$pointer len)
{
	unsigned char		*ptr;

	ptr = (unsigned char *) zone;
	while(len-- > 0)
	{
		(*ptr) = '\0';
		ptr++;
		len--;
	}

	return;
}

struct memsection *
vms$memsection_create_cache(struct slab_cache *sc)
{
	// Create a memsection for use by the internal slab allocator

	extern struct fpage_alloc 	pm_alloc;
	extern struct fpage_alloc 	vm_alloc;

	struct memsection_node		*node;
	struct memsection_list		*list;
	struct memsection			*ms;

	vms$pointer					page_size;
	vms$pointer					phys;
	vms$pointer					virt;

	vms$debug(__func__);

	page_size = (int) vms$min_pagesize();

notice("V %d\n", page_size);
	virt = vms$fpage_alloc_internal(&vm_alloc, page_size);

	if (virt == INVALID_ADDR)
	{
		return((struct memsection *) NULL);
	}

notice("P\n");
	phys = vms$fpage_alloc_internal(&pm_alloc, page_size);

	if (phys == INVALID_ADDR)
	{
		vms$fpage_free_internal(&vm_alloc, virt, (virt + page_size) - 1);
		return((struct memsection *) NULL);
	}

	vms$sigma0_map(virt, phys, (vms$pointer) page_size);
	vms$initmem(virt, page_size);

	if (sc == &(ms_cache))
	{
		node = (struct memsection_node *) virt;
		ms = &(node->data);
		ms->base = virt;
		virt += sizeof(struct memsection_node);
	}
	else
	{
		node = (struct memsection_node *) vms$slab_cache_alloc(&ms_cache);

		if (node == (struct memsection_node *) NULL)
		{
			L4_Flush(L4_Fpage(virt, page_size));
			vms$fpage_free_chunk(&vm_alloc, virt, (virt + page_size) - 1);
			vms$fpage_free_chunk(&pm_alloc, phys, (phys + page_size) - 1);

			return((struct memsection *) NULL);
		}

		PANIC(node == NULL);

		ms = &(node->data);
		ms->base = virt;
	}

	ms->end = (ms->base + page_size) - 1;
	ms->flags = 0;
	ms->slab_cache = sc;
	ms->phys_active = 1;
	ms->phys.base = phys;

	// Insert into internal memsections until it is safe to go
	// into the regular list and the objtable.

	list = &internal_memsections;
	node->next = (struct memsection_node *) list;
	list->last->next = node;
	node->prev = list->last;
	list->last = node;

	// Append the new pool to the list of pools in the allocator.

	TAILQ_INIT(&ms->slabs);

	for (; (virt + sc->slab_size) - 1 <= ms->end; virt += sc->slab_size)
	{
		TAILQ_INSERT_TAIL(&ms->slabs, (struct slab *)virt, slabs);
	}

	ms->slab_cache = sc;
	TAILQ_INSERT_TAIL(&sc->pools, ms, pools);

	return(ms);
}

void *
vms$slab_cache_alloc(struct slab_cache *sc)
{
	int						length;

	struct memsection		*pool;
	struct slab				*slab;

	unsigned char			*ptr;

	vms$debug(__func__);

	TAILQ_FOREACH(pool, &sc->pools, pools)
	{
		if (!TAILQ_EMPTY(&pool->slabs))
		{
			break;
		}
	}

	if (pool == NULL)
	{
		notice("slab_cache_alloc <1>\n");
		pool = vms$memsection_create_cache(sc);
		notice("slab_cache_alloc <2>\n");
	}

	if (pool == NULL)
	{
		return(NULL);
	}

	slab = TAILQ_FIRST(&pool->slabs);
	TAILQ_REMOVE(&pool->slabs, TAILQ_FIRST(&pool->slabs), slabs);

	length = sc->slab_size;
	ptr = (unsigned char *) slab;

	while(length > 0)
	{
		(*ptr) = '\0';
		length--;
		ptr++;
	}

	return(slab);
}

void
vms$slab_cache_free(struct slab_cache *sc, void *ptr)
{
	struct memsection		*pool;
	struct slab				*slab;

	vms$debug(__func__);

	slab = (struct slab *) ptr;
	pool = vms$objtable_lookup((void *) ((vms$pointer) ptr));

	TAILQ_INSERT_TAIL(&(pool->slabs), slab, slabs);
	return;
}
