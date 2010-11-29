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

static struct slab_cache fp_cache =
        SLAB_CACHE_INITIALIZER(sizeof(struct fpage_list), &fp_cache);

L4_Word_t
vms$min_pagebits(void)
{
    L4_Word_t           x;

    static int          min_pagebits = 0;

    unsigned int        bits;

    if (min_pagebits == 0)
    {
        x = L4_PageSizeMask(L4_GetKernelInterface());

        for(bits = 0; bits < L4_SIZEOFWORD; min_pagebits++)
        {
            if ((x & 0x1) != 0)
            {
                break;
            }

            x >>= 1;
        }
    }

    return(min_pagebits);
}

static inline L4_Fpage_t
buddy(L4_Fpage_t fpage)
{
    return(L4_Fpage(L4_Address(fpage) ^ L4_Size(fpage), L4_Size(fpage)));
}

L4_Word_t
vms$min_pagesize(void)
{
    return(1 << vms$min_pagebits());
}

vms$pointer
sys$page_round_down(vms$pointer address, vms$pointer page_size)
{
    return(address & (~(((vms$pointer) page_size) - 1)));
}

vms$pointer
sys$page_round_up(vms$pointer address, vms$pointer page_size)
{
    return((address + (((vms$pointer) page_size) - 1)) &
            (~(((vms$pointer) page_size) - 1)));
}

static inline int
sys$fp_order(L4_Fpage_t fpage)
{
    return(L4_SizeLog2(fpage) - vms$min_pagebits());
}

L4_Fpage_t
sys$biggest_fpage(vms$pointer addr, vms$pointer base, vms$pointer end)
{
    unsigned int            bits;

    vms$pointer             next_addr;
    vms$pointer             next_size;

    if ((end < base) || (((end - base) + 1) < vms$min_pagesize()))
    {
        return(L4_Nilpage);
    }

    for(bits = vms$min_pagebits(); bits < L4_SIZEOFWORD; bits++)
    {
        next_addr = (addr >> (bits + 1)) << (bits + 1);
        next_size = ((vms$pointer) 1) << (bits + 1);

        if ((next_addr < base) || (((next_addr + next_size) - 1) > end))
        {
            break;
        }
    }

    return(L4_FpageLog2((addr >> bits) << bits, bits));
}

void
sys$fpage_clear_internal(struct fpage_alloc *alloc)
{
    struct fpage_list       *fpage;

    if (alloc->internal.active)
    {
        while(alloc->internal.base < alloc->internal.end)
        {
            if ((fpage = (struct fpage_list *) sys$slab_cache_alloc(&fp_cache))
                    == (struct fpage_list *) NULL)
            {
                return;
            }

            fpage->fpage = sys$biggest_fpage(alloc->internal.base,
                    alloc->internal.base, alloc->internal.end);
            TAILQ_INSERT_TAIL(&alloc->flist[sys$fp_order(fpage->fpage)],
                    fpage, flist);
            alloc->internal.base += L4_Size(fpage->fpage);
        }

        alloc->internal.active = 0;
    }

    return;
}

void
sys$fpage_free_internal(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end)
{
    if (alloc->internal.active)
    {
        sys$fpage_clear_internal(alloc);
    }

    alloc->internal.base = base;
    alloc->internal.end = end;
    alloc->internal.active = 1;

    return;
}

static inline vms$pointer
sys$fp_end(L4_Fpage_t fpage)
{
    return(L4_Address(fpage) + (L4_Size(fpage) - 1));
}

vms$pointer
sys$fpage_alloc_internal(struct fpage_alloc *alloc, vms$pointer size)
{
    unsigned int            i;

    struct fpage_list       *node;

    vms$pointer             mem;

    if (alloc->internal.active == 0)
    {
        for(i = 0; TAILQ_EMPTY(&alloc->flist[i]) && (i < MAX_FPAGE_ORDER); i++);
        PANIC(i == MAX_FPAGE_ORDER, notice(MEM_F_OUTMEM
                "catching out of memory error\n"));

        node = TAILQ_FIRST(&alloc->flist[i]);
        alloc->internal.base = L4_Address(node->fpage);
        alloc->internal.end = sys$fp_end(node->fpage);
        alloc->internal.active = 1;
        TAILQ_REMOVE(&alloc->flist[i], node, flist);
        sys$slab_cache_free(&fp_cache, node);
    }

    if (((alloc->internal.end - alloc->internal.base) + 1) < (vms$pointer) size)
    {
        return INVALID_ADDR;
    }

    mem = alloc->internal.base;
    alloc->internal.base += size;

    if (alloc->internal.base >= alloc->internal.end)
    {
        alloc->internal.active = 0;
    }

    return(mem);
}

void
sys$remove_virtmem(struct vms$meminfo *mem_info,
        vms$pointer base, vms$pointer end, vms$pointer page_size)
{
    mem_info->num_vm_regions = sys$remove_chunk(mem_info->vm_regions,
            mem_info->num_vm_regions, mem_info->max_vm_regions,
            sys$page_round_down(base, page_size),
            sys$page_round_up(end, page_size) - 1);
}

int
sys$remove_chunk(struct memdesc *mem_desc, int pos, int max,
        vms$pointer low, vms$pointer high)
{
    int             j;
    int             k;

    for(j = 0; j < pos; j++)
    {
        if ((low <= mem_desc[j].base) && (high >= mem_desc[j].end))
        {
            // Remove whole chunk
            for(k = j; k < pos; k++)
            {
                mem_desc[k] = mem_desc[k + 1];
            }

            pos--;
            j--;
        }
        else if ((low >= mem_desc[j].base) && (low < mem_desc[j].end) &&
                (high >= mem_desc[j].end))
        {
            // Chunk overlaps top
            mem_desc[j].end = low - 1;
        }
        else if ((low <= mem_desc[j].base) && (high <= mem_desc[j].end) &&
                (high > mem_desc[j].base))
        {
            // Chuck overlaps bottom
            mem_desc[j].base = high + 1;
        }
        else if ((low > mem_desc[j].base) && (low < mem_desc[j].end) &&
                (high < mem_desc[j].end) && (high > mem_desc[j].base))
        {
            // Chunk slips region
            PANIC(pos >= (max - 1));

            // The following loop creates a free slot for the split.
            for(k = pos; k > j; k--)
            {
                mem_desc[k] = mem_desc[k - 1];
            }

            mem_desc[j + 1].end = mem_desc[j].end;
            mem_desc[j + 1].base = high + 1;
            mem_desc[j].end = low - 1;

            pos++;
        }
    }

    return(pos);
}

void
sys$fpage_free_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end)
{
    if (alloc->internal.active)
    {
        sys$fpage_clear_internal(alloc);
    }

    sys$fpage_free_internal(alloc, base, end);
    sys$fpage_clear_internal(alloc);

    return;
}

static int
sz_order(vms$pointer size)
{
    int                 order;

    order = 0;

    while ((((vms$pointer) 1) << (order + vms$min_pagebits())) < size)
    {
        order++;
    }

    return(order);
}

// Allocate an fpage of exactly the requested size if possible,
// otherwise allocate the biggest available fpage.
static L4_Fpage_t
sys$fpage_alloc(struct fpage_alloc *alloc, vms$pointer size)
{
    int                 i;
    int                 order;

    L4_Fpage_t          fpage;

    struct fpage_list   *node;

    order = sz_order(size);

    // Look for something greater or equal than request size
    for(i = order; i < (int) MAX_FPAGE_ORDER; i++)
    {
        if (!TAILQ_EMPTY(&alloc->flist[i]))
        {
            break;
        }
    }

    // Otherwise try to find something
    while((i >= 0) && TAILQ_EMPTY(&alloc->flist[i]))
    {
        i--;
    }

    // Make sure we have something left
    if (i < 0)
    {
        return(L4_Nilpage);
    }

    node = TAILQ_FIRST(&alloc->flist[i]);
    fpage = node->fpage;
    TAILQ_REMOVE(&alloc->flist[i], node, flist);
    sys$slab_cache_free(&fp_cache, node);

    // Free up any excess
    while(sys$fp_order(fpage) > order)
    {
        node = (struct fpage_list *) sys$slab_cache_alloc(&fp_cache);

        if (node == NULL)
        {
            // Return greater than request size
            return(L4_FpageLog2(L4_Address(fpage), L4_SizeLog2(fpage) + 1));
        }

        fpage = L4_FpageLog2(L4_Address(fpage), L4_SizeLog2(fpage) - 1);
        node->fpage = buddy(fpage);
        TAILQ_INSERT_TAIL(&alloc->flist[sys$fp_order(fpage)], node, flist);
    }

    return(fpage);
}

vms$pointer
sys$fpage_alloc_chunk(struct fpage_alloc *alloc, vms$pointer size)
{
    L4_Fpage_t          fpage;

    fpage = sys$fpage_alloc(alloc, size);

    if (L4_IsNilFpage(fpage))
    {
        return(INVALID_ADDR);
    }

    if (L4_Size(fpage) < size)
    {
        sys$fpage_free_chunk(alloc, L4_Address(fpage), sys$fp_end(fpage));
        return(INVALID_ADDR);
    }

    if (L4_Size(fpage) > size)
    {
        sys$fpage_free_chunk(alloc, L4_Address(fpage) + size,
                sys$fp_end(fpage));
    }

    return(L4_Address(fpage));
}

static void
sys$fpage_free_extra(struct fpage_alloc *alloc, L4_Fpage_t fpage,
        vms$pointer base, vms$pointer end)
{
    if (L4_Address(fpage) < base)
    {
        sys$fpage_free_chunk(alloc, L4_Address(fpage), base - 1);
    }

    if (L4_Address(fpage) + (L4_Size(fpage) - 1) > end)
    {
        sys$fpage_free_chunk(alloc, end + 1, sys$fp_end(fpage));
    }

    return;
}

void
sys$fpage_remove_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end)
{
    L4_Fpage_t                  fpage;

    struct fpage_list           *node;
    struct fpage_list           *tmp;

    unsigned int                i;

    vms$pointer                 fbase;
    vms$pointer                 fend;

    for(i = 0; i <= MAX_FPAGE_ORDER; i++)
    {
        node = TAILQ_FIRST(&alloc->flist[i]);

        for (; node != NULL; node = tmp)
        {
            tmp = TAILQ_NEXT(node, flist);
            fbase = L4_Address(node->fpage);
            fend = fbase + (L4_Size(node->fpage) - 1);

            if (max(base, fbase) < min(end, fend))
            {
                // remove from list, then trim and free
                // any excess memory in the fpage.
                fpage = node->fpage;
                TAILQ_REMOVE(&alloc->flist[i], node, flist);
                sys$slab_cache_free(&fp_cache, node);
                sys$fpage_free_extra(alloc, fpage, base, end);
            }
        }
    }

    return;
}

// Following function is used to back memsections. It shall try to allocate
// the largest fpages it can to back [base, end].
struct flist_head
sys$fpage_alloc_list(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end, vms$pointer pagesize)
{
    struct flist_head           list = TAILQ_HEAD_INITIALIZER(list);

    struct fpage_list           *node;

    L4_Fpage_t                  fpage;

    while(base < end)
    {
        // If region is smaller than page size, we cannot allocate any fpage
        // but we have found any out of memory error.
        if (((end - base) + 1) < pagesize)
        {
            return(list);
        }

        fpage = sys$fpage_alloc(alloc, L4_Size(sys$biggest_fpage(base,
                base, end)));

        if (L4_IsNilFpage(fpage))
        {
            goto out_of_memory;
        }

        if ((node = (struct fpage_list *) sys$slab_cache_alloc(&fp_cache))
                == NULL)
        {
            goto out_of_memory;
        }

        node->fpage = fpage;
        TAILQ_INSERT_TAIL(&list, node, flist);
        base += L4_Size(fpage);
    }

    return(list);

out_of_memory:
    notice(MEM_F_OUTMEM "catching out of memory error at $%016lX "
            "($%lX bytes)\n", base, (end - base) + 1);

    while(!TAILQ_EMPTY(&list))
    {
        node = TAILQ_FIRST(&list);
        TAILQ_REMOVE(&list, node, flist);
        sys$fpage_free_chunk(alloc, L4_Address(node->fpage),
                sys$fp_end(node->fpage));
        sys$slab_cache_free(&fp_cache, node);
    }

    return(list);
}

void
sys$fpage_free_list(struct fpage_alloc *alloc, struct flist_head list)
{
    struct fpage_list           *node;
    struct fpage_list           *tmp;

    for(node = TAILQ_FIRST(&list); node != NULL ; node = tmp)
    {
        tmp = TAILQ_NEXT(node, flist);
        sys$fpage_free_chunk(alloc, L4_Address(node->fpage),
                sys$fp_end(node->fpage));
        TAILQ_REMOVE(&list, node, flist);
        sys$slab_cache_free(&fp_cache, node);
    }

    return;
}

int
sys$back_mem(vms$pointer base, vms$pointer end, vms$pointer pagesize)
{
    extern struct pd            freevms_pd;

    struct memsection           *ms;
    struct memsection           *backed;

    ms = sys$objtable_lookup((void*) base);
    PANIC(!(ms && (ms->flags & VMS$MEM_USER)));

    while(base < end)
    {
        backed = sys$pd_create_memsection(&freevms_pd, pagesize,
                0, VMS$MEM_NORMAL, pagesize);

        if (backed == NULL)
        {
            // FIXME: clean up the partially backed region
			FIXME;
            return(-1);
        }

        sys$memsection_page_map(ms, L4_Fpage(backed->base, pagesize),
                L4_Fpage(base, pagesize));
        base += pagesize;
    }

    return(0);
}
