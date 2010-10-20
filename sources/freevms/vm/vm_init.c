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

/*
================================================================================
  Read a memory descriptor from KIP structure. Return true if it is physical
  and if we have informations about it.
================================================================================
*/

static int
vms$find_memory_info(L4_KernelInterfacePage_t *kip, int pos,
        L4_Word_t *low, L4_Word_t *high, L4_Word_t *type)
{
    L4_MemoryDesc_t     *mem_desc;

    mem_desc = L4_MemoryDesc(kip, pos);
    PANIC(mem_desc == NULL);

    (*low) = L4_MemoryDescLow(mem_desc);
    (*high) = L4_MemoryDescHigh(mem_desc);
    (*type) = mem_desc->x.type;
    PANIC((*high) <= (*low));

    return(mem_desc->x.v == 0);
}

static int
vms$remove_chunk(struct memdesc *mem_desc, int pos, int max,
        unsigned long int low, unsigned long int high)
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

static char *
vms$strncpy(char *dest, const char *src, int n)
{
    char *ret = dest;

    do
    {
        if (!n--) return ret;
    } while ((*dest++ = *src++));

    while (n--) *dest++ = 0;
    return ret;
}

void
vms$add_initial_object(struct initial_obj *objs, const char *name,
        unsigned long int base, unsigned long int end,
        unsigned long int entry, char flags)
{
    if (name == NULL)
    {
        name = "";
    }

    vms$strncpy(objs->name, name, INITIAL_NAME_MAX);
    objs->name[INITIAL_NAME_MAX - 1] = '\0';
    objs->base = base;
    objs->end = end;
    objs->entry = entry;
    objs->flags = flags;

    return;
}

static unsigned int
vms$find_initial_objects(L4_KernelInterfacePage_t *kip,
        unsigned int max, struct initial_obj *initial_objs)
{
    L4_KernelConfigurationPage_t    *kcp;

    L4_Word_t                       high;
    L4_Word_t                       low;
    L4_Word_t                       type;

    unsigned int                    count;
    unsigned int                    i;

    count = 0;

    kcp = (L4_KernelConfigurationPage_t *) kip;

    // Find all reserved objects
    for(i = 0; i < kip->MemoryInfo.n; i++)
    {
        if (vms$find_memory_info(kip, i, &low, &high, &type) != 0)
        {
            if (type == L4_ReservedMemoryType)
            {
                vms$add_initial_object(initial_objs++, "L4 object", low, high,
                        0, VMS$IOF_RESERVED | VMS$IOF_PHYS);
                if ((++count) == max) goto overflow;
            }
        }
    }

    vms$add_initial_object(initial_objs++, "L4 sigma0", kcp->sigma0.low,
            kcp->sigma0.high, 0, VMS$IOF_RESERVED | VMS$IOF_PHYS);
    if ((++count) == max) goto overflow;

    //count += vms$bootinfo_find_initial_objects(max - count, initial_objs);

overflow:
    return(0);
}

static unsigned int
vms$find_memory_region(L4_KernelInterfacePage_t *kip,
        unsigned int max, int memory_type, int except_type,
        struct memdesc *mem_desc)
{
    int                 covered;
    int                 mem_desc_type;

    L4_Word_t           high;
    L4_Word_t           low;
    L4_Word_t           type;

    unsigned int        i;
    unsigned int        j;
    unsigned int        pos;

    pos = 0;

    for(i = 0; i < kip->MemoryInfo.n; i++)
    {
        if (vms$find_memory_info(kip, i, &low, &high, &type))
        { // Physical memory
            switch(type)
            {
                case L4_ConventionalMemoryType:
                    mem_desc_type = VMS$MEM_RAM;
                    break;

                case L4_SharedMemoryType:
                case L4_DedicatedMemoryType:
                    mem_desc_type = VMS$MEM_IO;
                    break;

                default:
                    mem_desc_type = VMS$MEM_OTHER;
                    break;
            }
        }
        else
        { // No physical memory
            mem_desc_type = VMS$MEM_VM;
        }

        if (mem_desc_type & memory_type)
        {
            // Add it to the array

            covered = 0;
            PANIC(pos >= (max - 1));

            for(j = 0; j < pos; j++)
            {
                if ((low >= mem_desc[j].base) &&
                        (low < mem_desc[j].end) &&
                        (high >= mem_desc[j].end))
                {
                    mem_desc[j].end = high;
                    covered = 1;
                    break;
                }
                else if ((low <= mem_desc[j].base) &&
                        (high <= mem_desc[j].end) &&
                        (high > mem_desc[j].base))
                {
                    mem_desc[j].base = high + 1;
                    covered = 1;
                    break;
                }
                else if ((low > mem_desc[j].base) &&
                        (low < mem_desc[j].end) &&
                        (high < mem_desc[j].end) &&
                        (high > mem_desc[j].base))
                {
                    covered = 1;
                    break;
                }
            }

            if (covered == 0)
            {
                mem_desc[pos].base = low;
                mem_desc[pos].end = high;
                pos++;
            }
        }
        else if (mem_desc_type & except_type)
        {
            pos = vms$remove_chunk(mem_desc, pos, max, low, high);
        }
    }

    // Return number of actual descriptors we have copied.
    return(pos);
}

void
vms$vm_init(L4_KernelInterfacePage_t *kip, struct vms$meminfo *MemInfo)
{
    static struct initial_obj       static_objects[NUM_MI_OBJECTS];

    static struct memdesc           static_regions[NUM_MI_REGIONS];
    static struct memdesc           static_io_regions[NUM_MI_IOREGIONS];
    static struct memdesc           static_vm_regions[NUM_MI_VMREGIONS];

    MemInfo->regions = static_regions;
    MemInfo->max_regions = NUM_MI_REGIONS;
    MemInfo->num_regions = vms$find_memory_region(kip,
            NUM_MI_REGIONS, VMS$MEM_RAM, VMS$MEM_IO, static_regions);

    MemInfo->io_regions = static_io_regions;
    MemInfo->max_io_regions = NUM_MI_IOREGIONS;
    MemInfo->num_io_regions = vms$find_memory_region(kip,
            NUM_MI_IOREGIONS, VMS$MEM_IO, VMS$MEM_RAM, static_io_regions);

    MemInfo->vm_regions = static_vm_regions;
    MemInfo->max_vm_regions = NUM_MI_VMREGIONS;
    MemInfo->num_vm_regions = vms$find_memory_region(kip,
            NUM_MI_VMREGIONS, VMS$MEM_VM, 0, static_vm_regions);

    // Create a guard page

    MemInfo->num_vm_regions = vms$remove_chunk(MemInfo->vm_regions,
            MemInfo->num_vm_regions, NUM_MI_VMREGIONS, 0, 0xfff);

    MemInfo->objects = static_objects;
    MemInfo->max_objects = NUM_MI_OBJECTS;
    MemInfo->num_objects = vms$find_initial_objects(kip,
            NUM_MI_OBJECTS, static_objects);

    return;
}
