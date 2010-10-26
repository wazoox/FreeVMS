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

        for(bits = 0; bits < sizeof(L4_Word_t) * 8; min_pagebits++)
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

L4_Word_t
vms$min_pagesize(void)
{
    return(1 << vms$min_pagebits());
}

vms$pointer
vms$page_round_down(vms$pointer address, unsigned int page_size)
{
    return(address & (~(((vms$pointer) page_size) - 1)));
}

vms$pointer
vms$page_round_up(vms$pointer address, unsigned int page_size)
{
    return((address + (((vms$pointer) page_size) - 1)) &
			(~(((vms$pointer) page_size) - 1)));
}

static inline int
vms$fp_order(L4_Fpage_t fpage)
{
    static int                      order = 0;

    if (order == 0)
    {
        order = L4_SizeLog2(fpage) - vms$min_pagebits();
    }

    return(order);
}

L4_Fpage_t
vms$biggest_fpage(vms$pointer addr, vms$pointer base, vms$pointer end)
{
    vms$pointer             bits;
    vms$pointer             next_addr;
    vms$pointer             next_size;

    if ((end < base) || (((end - base) + 1) < vms$min_pagebits()))
    {
        return(L4_Nilpage);
    }

    for(bits = vms$min_pagebits(); bits < sizeof(L4_Word_t) * 8; bits++)
    {
        next_addr = addr >> (bits + 1) << (bits + 1);
        next_size = 1 << (bits + 1);

        if ((next_addr < base) || (((next_addr + next_size) - 1) > end))
        {
            break;
        }
    }

    return(L4_FpageLog2(addr >> bits << bits, bits));
}

void
vms$fpage_clear_internal(struct fpage_alloc *alloc)
{
    struct fpage_list       *fpage;

    if (alloc->internal.active)
    {
        while(alloc->internal.base < alloc->internal.end)
        {
notice("while 1 %lx %lx\n", alloc->internal.base, alloc->internal.end);
            if ((fpage = (struct fpage_list *) vms$slab_cache_alloc(&fp_cache))
                    == (struct fpage_list *) NULL)
            {
                vms$debug("vms$slab_cache_alloc returns NULL !");
                return;
            }
vms$debug("while 2");

            fpage->fpage = vms$biggest_fpage(alloc->internal.base,
                    alloc->internal.base, alloc->internal.end);
            TAILQ_INSERT_TAIL(&alloc->flist[vms$fp_order(fpage->fpage)],
                    fpage, flist);
            alloc->internal.base += L4_Size(fpage->fpage);
vms$debug("while 3");
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

static inline vms$pointer
vms$fp_end(L4_Fpage_t fpage)
{
    return((L4_Address(fpage) + L4_Size(fpage)) - 1);
}

vms$pointer
vms$fpage_alloc_internal(struct fpage_alloc *alloc, int size)
{
    unsigned int            i;

    struct fpage_list       *node;

    vms$pointer             mem;

    if (alloc->internal.active == 0)
    {
        for(i = 0; TAILQ_EMPTY(&alloc->flist[i]) && (i < MAX_FPAGE_ORDER); i++);
        PANIC(i == MAX_FPAGE_ORDER, notice(MEM_F_OUTMEM "out of memory\n"));

        node = TAILQ_FIRST(&alloc->flist[i]);
        alloc->internal.base = L4_Address(node->fpage);
        alloc->internal.end = vms$fp_end(node->fpage);
        alloc->internal.active = 1;
        TAILQ_REMOVE(&alloc->flist[i], node, flist);
        vms$slab_cache_free(&fp_cache, node);
    }

    if (((alloc->internal.end - alloc->internal.base) + 1) < (vms$pointer) size)
    {
        notice(MEM_I_NOTEMEM "not enough memory\n");
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
vms$remove_virtmem(struct vms$meminfo *mem_info,
        vms$pointer base, unsigned long end, unsigned int page_size)
{
    mem_info->num_vm_regions = vms$remove_chunk(mem_info->vm_regions,
            mem_info->num_vm_regions, mem_info->max_vm_regions,
            vms$page_round_down(base, page_size),
            vms$page_round_up(end, page_size) - 1);
}

int
vms$remove_chunk(struct memdesc *mem_desc, int pos, int max,
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
                (high > mem_desc[j].end) && (high > mem_desc[j].base))
        {
            // Chunk slips region
            PANIC(pos >= (max - 1))

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
vms$fpage_free_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end)
{
    if (alloc->internal.active)
    {
        notice("<1>\n");
        vms$fpage_clear_internal(alloc);
        notice("<2>\n");
    }

        notice("<3>\n");
    vms$fpage_free_internal(alloc, base, end);
        notice("<4>\n");
    vms$fpage_clear_internal(alloc);
        notice("<5>\n");

    return;
}
