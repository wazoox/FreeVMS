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
    volatile unsigned char      *ptr;

    ptr = (volatile unsigned char *) zone;
    while(len > 0)
    {
        (*ptr) = (unsigned char) 0x0;
        ptr++;
        len--;
    }

    return;
}

void
vms$memcopy(vms$pointer dest, vms$pointer src, vms$pointer size)
{
    char            *ptr1;
    char            *ptr2;

    ptr1 = (char *) src;
    ptr2 = (char *) dest;

    while(size--)
    {
        (*ptr2) = (*ptr1);
        ptr1++;
        ptr2++;
    }

    return;
}

static vms$pointer
vms$memsection_lookup_phys(struct memsection *memsection, vms$pointer addr)
{
    struct fpage_list       *node;

    vms$pointer             virt;

    virt = memsection->base;

    TAILQ_FOREACH(node, &memsection->phys.list, flist)
    {
        if ((addr >= virt) && (addr < (virt + L4_Size(node->fpage))))
        {
            break;
        }

        virt += L4_Size(node->fpage);
    }

    return(L4_Address(node->fpage) + (addr - virt));
}

int
vms$memsection_page_map(struct memsection *self, L4_Fpage_t from_page,
        L4_Fpage_t to_page)
{
    struct memsection   *src;

    vms$pointer         from_base;
    vms$pointer         from_end;
    vms$pointer         offset;
    vms$pointer         phys;
    vms$pointer         size;
    vms$pointer         to_base;
    vms$pointer         to_end;

    if ((self->flags & VMS$MEM_USER))
    {
        return(-1);
    }

    if (L4_Size(from_page) != L4_Size(to_page))
    {
        return(-1);
    }

    from_base = L4_Address(from_page);
    from_end = from_base + (L4_Size(from_page) - 1);
    to_base = L4_Address(to_page);
    to_end = to_base + (L4_Size(to_page) - 1);

    src = vms$objtable_lookup((void *) from_base);

    if (!src)
    {
        // can't map from user-paged ms
        return(-1);
    }

    if (src->flags & VMS$MEM_USER)
    {
        // can't map from user-paged ms
        return(-1);
    }

    if (src->flags & VMS$MEM_INTERNAL)
    {
        return(-1);
    }

    if ((to_base < self->base) || (to_end > self->end))
    {
        return(-1);
    }

    size = (from_end - from_base) + 1;

    // we map vms$min_pagesize() fpages even when bigger mappings are possible
    for(offset = 0; offset < size; offset += vms$min_pagesize())
    {
        phys = vms$memsection_lookup_phys(src, from_base + offset);
        vms$sigma0_map(to_base + offset, phys, vms$min_pagesize(),
                L4_FullyAccessible);
    }

    return(0);
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

    struct memsection           *ms;

    vms$pointer                 phys;
    vms$pointer                 virt;

    page_size = (int) vms$min_pagesize();

    virt = vms$fpage_alloc_internal(&vm_alloc, page_size);

    if (virt == INVALID_ADDR)
    {
        return((struct memsection *) NULL);
    }

    PANIC(virt % vms$min_pagesize(), notice("virt not aligned (%lx)!\n", virt));
    phys = vms$fpage_alloc_internal(&pm_alloc, page_size);

    if (phys == INVALID_ADDR)
    {
        vms$fpage_free_internal(&vm_alloc, virt, virt + (page_size - 1));
        return((struct memsection *) NULL);
    }

    PANIC(phys % vms$min_pagesize(), notice("phys not aligned (%lx)!\n", virt));
    vms$sigma0_map(virt, phys, page_size, L4_FullyAccessible);
    vms$initmem(virt, page_size);

    if (sc == (&ms_cache))
    {
        // If this will be used to back memsections, put
        // the new memsection into the pool itself.
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

        ms = &(node->data);
        ms->base = virt;
    }

    ms->end = ms->base + (page_size - 1);
    ms->flags = 0;
    ms->slab_cache = sc;
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

    for(; virt + (sc->slab_size - 1) <= ms->end; virt += sc->slab_size)
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
    struct memsection       *pool;
    struct slab             *slab;

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
    vms$initmem((vms$pointer) slab, sc->slab_size);

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
vms$memsection_new(void)
{
    struct memsection_node  *node;

    if ((node = (struct memsection_node *) vms$slab_cache_alloc(&ms_cache))
            == NULL)
    {
        return(NULL);
    }

    vms$initmem((vms$pointer) node, sizeof(struct memsection_node));
    return(node);
}

static void
vms$delete_memsection_from_allocator(struct memsection_node *node)
{
    vms$slab_cache_free(&ms_cache, node);
    return;
}

int
vms$memsection_back(struct memsection *memsection)
{
    L4_Fpage_t              vpage;

    struct fpage_list       *node;

    vms$pointer             addr;
    vms$pointer             size;
    vms$pointer             flags;

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
        vms$sigma0_map(addr, memsection->phys.base, size, L4_FullyAccessible);
    }
    else
    {
        // Iterate through fpage list
        TAILQ_FOREACH(node, &memsection->phys.list, flist)
        {
            vpage = L4_Fpage(addr, L4_Size(node->fpage));
            vms$sigma0_map_fpage(vpage, node->fpage, L4_FullyAccessible);
            vms$initmem(L4_Address(vpage), L4_Size(vpage));
            addr += L4_Size(vpage);
        }
    }

    return(0);
}

struct memsection *
vms$pd_create_memsection(struct pd *self, vms$pointer size, vms$pointer base,
        unsigned int flags, vms$pointer pagesize)
{
    extern int                  vms$pd_initialized;

    int                         r;

    struct memsection           *memsection;

    struct memsection_list      *list;

    struct memsection_node      *node;

    if ((node = vms$memsection_new()) == NULL)
    {
        return(NULL);
    }

    memsection = &(node->data);

    if (flags & VMS$MEM_NORMAL)
    {
        r = objtable_setup(memsection, size, flags, pagesize);
    }
    else if (flags & VMS$MEM_FIXED)
    {
        r = objtable_setup_fixed(memsection, size, base, flags, pagesize);
    }
    else if (flags & VMS$MEM_UTCB)
    {
        r = objtable_setup_utcb(memsection, size, flags);
    }
    else if (flags & VMS$MEM_INTERNAL)
    {
        r = objtable_setup_internal(memsection, size, base, flags);
    }
    else
    {
        r = -1;
    }

    if (r != 0)
    {
        // Insertion into object table failed. Delete memsection from mem_alloc,
        // need not to delete it from memsection_list.
        vms$delete_memsection_from_allocator(node);
        return(NULL);
    }

    if (vms$pd_initialized)
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
