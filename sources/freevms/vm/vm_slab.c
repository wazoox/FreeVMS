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

static struct slab_cache ms_cache =
        SLAB_CACHE_INITIALIZER(sizeof(struct memsection_node), &ms_cache);

struct memsection_list internal_memsections =
{
    (struct memsection_node *) &internal_memsections,
    (struct memsection_node *) &internal_memsections
};

void
vms$initmem(vms$pointer zone, vms$pointer len)
{
    unsigned char       *ptr;

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

    extern struct fpage_alloc   pm_alloc;
    extern struct fpage_alloc   vm_alloc;

    int                         page_size;

    struct memsection_node      *node;
    struct memsection_list      *list;

	// ms has to be declared as volatile to avoid a strange gcc optimization bug
    volatile struct memsection  *ms;

    vms$pointer                 phys;
    vms$pointer                 virt;

    page_size = (int) vms$min_pagesize();

    virt = vms$fpage_alloc_internal(&vm_alloc, page_size);

    if (virt == INVALID_ADDR)
    {
        return((struct memsection *) NULL);
    }

    phys = vms$fpage_alloc_internal(&pm_alloc, page_size);

    if (phys == INVALID_ADDR)
    {
        vms$fpage_free_internal(&vm_alloc, virt, virt + (page_size - 1));
        return((struct memsection *) NULL);
    }

    vms$sigma0_map(virt, phys, page_size);
    vms$initmem(virt, page_size);

    if (sc == (&ms_cache))
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
            vms$fpage_free_chunk(&vm_alloc, virt, virt + (page_size - 1));
            vms$fpage_free_chunk(&pm_alloc, phys, phys + (page_size - 1));

            return((struct memsection *) NULL);
        }

        PANIC(node == NULL);

        ms = &(node->data);
        ms->base = virt;
    }

    ms->end = ms->base + (page_size - 1);
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

    for(; (virt + sc->slab_size) - 1 <= ms->end; virt += sc->slab_size)
    {
        TAILQ_INSERT_TAIL(&ms->slabs, (struct slab *) virt, slabs);
    }

    ms->slab_cache = sc;
    TAILQ_INSERT_TAIL(&sc->pools, (struct memsection *) ms, pools);

    return((struct memsection *) ms);
}

void *
vms$slab_cache_alloc(struct slab_cache *sc)
{
    int                     length;

    struct memsection       *pool;
    struct slab             *slab;

    unsigned char           *ptr;

    TAILQ_FOREACH(pool, &sc->pools, pools)
    {
        if (!TAILQ_EMPTY(&pool->slabs))
        {
            break;
        }
    }

    if (pool == NULL)
    {
        pool = vms$memsection_create_cache(sc);
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
    struct memsection       *pool;
    struct slab             *slab;

    slab = (struct slab *) ptr;
    pool = vms$objtable_lookup((void *) ((vms$pointer) ptr));

    TAILQ_INSERT_TAIL(&pool->slabs, slab, slabs);
    return;
}

static struct memsection_node *
memsection_new(void)
{
	int						length;

	struct memsection_node	*node;

	unsigned char			*ptr;

	if ((node = (struct memsection_node *) vms$slab_cache_alloc(&ms_cache))
			== NULL)
	{
		return(NULL);
	}

    length = sizeof(struct memsection_node);
    ptr = (unsigned char *) node;

    while(length > 0)
    {
        (*ptr) = '\0';
        length--;
        ptr++;
    }

	return(node);
}

static void
delete_memsection_from_allocator(struct memsection_node *node)
{
	vms$slab_cache_free(&ms_cache, node);
	return;
}

int
memsection_back(struct memsection *memsection)
{
	L4_Fpage_t				vpage;

	struct fpage_list		*node;

	vms$pointer				addr;
	vms$pointer				size;
	vms$pointer				flags;

	addr = memsection->base;
	size = (memsection->end - memsection->base) + 1;
	flags = memsection->flags;

	if (flags & (VMS$MEM_USER | VMS$MEM_UTCB))
	{
		return(-1);
	}
	else if (flags & VMS$MEM_INTERNAL)
	{
		// Map it 1:1
		vms$sigma0_map(addr, memsection->phys.base, size);
		return(0);
	}
	else
	{
		// Iterate through fpage list
		TAILQ_FOREACH(node, &memsection->phys.list, flist)
		{
			vpage = L4_Fpage(addr, L4_Size(node->fpage));
			vms$sigma0_map_fpage(vpage, node->fpage);
			vms$initmem(addr, L4_Size(vpage));
			addr += L4_Size(vpage);
		}
	}

	return(0);
}

struct memsection *
vms$pd_create_memsection(struct pd *self, vms$pointer size, vms$pointer base,
		unsigned int flags)
{
	int							r;

	struct memsection			*memsection;

	struct memsection_list		*list;

	struct memsection_node		*node;

	if ((node = memsection_new()) == NULL)
	{
		return(NULL);
	}

	memsection = &(node->data);

	if (flags & VMS$MEM_NORMAL)
	{
		//r = objtable_setup(memsection, size, flags);
		r = 0;
	}
	else if (flags & VMS$MEM_FIXED)
	{
		//r = objtable_setup_fixed(memsection, size, base, flags);
	}
	else if (flags & VMS$MEM_UTCB)
	{
		//r = objtable_setup_utcb(memsection, size, flags);
	}
	else if (flags & VMS$MEM_INTERNAL)
	{
		//r = objtable_setup_internal(memsection, size, base, flags);
	}
	else
	{
		r = -1;
	}

	if (r != 0)
	{
		// Insertion into object table failed. Delete memsection from mem_alloc,
		// need not to delete it from memsection_list.
		delete_memsection_from_allocator(node);
		return(NULL);
	}

	if (self != NULL)
	{
		list = &self->memsections;
	}
	else
	{
		list = &internal_memsections;
	}

	node->next = (struct memsection_node *) list;
	list->last->next = node;
	node->prev = list->last;
	list->last = node;

	return(memsection);
}
