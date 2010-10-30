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

struct fpage_alloc pm_alloc;
struct fpage_alloc vm_alloc;

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

static char *
vms$strncat(char *s1, const char *s2, int n)
{
    char *s = s1;

    while (*s != '\0') s++;

    while (n != 0 && (*s = *s2++) != '\0')
    {
        n--;
        s++;
    }

    if (*s != '\0') *s = '\0';
    return s1;
}

void
vms$add_initial_object(struct initial_obj *objs, const char *name,
        vms$pointer base, vms$pointer end,
        vms$pointer entry, char flags)
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
vms$bootinfo_find_initial_objects(L4_KernelInterfacePage_t *kip,
        unsigned int max, struct initial_obj *initial_objs)
{
    char                        data_name[INITIAL_NAME_MAX];

    L4_BootRec_t                *record;

    L4_Word_t                   dsize;
    L4_Word_t                   dstart;
    L4_Word_t                   tsize;
    L4_Word_t                   tstart;
    L4_Word_t                   type;

    unsigned int                count;
    unsigned int                flags;
    unsigned int                num_recs;
    unsigned int                objects;

    void                        *bootinfo;

#   define DIT_INITIAL          32L // Root task

    bootinfo = (void*) L4_BootInfo(kip);
    count = 0;

    // Check bootinfo validity

    if (L4_BootInfo_Valid(bootinfo) == 0)
    {
        return(0);
    }

    num_recs = L4_BootInfo_Entries(bootinfo);
    record = L4_BootInfo_FirstEntry(bootinfo);

    while(num_recs > 0)
    {
        PANIC(record == NULL);
        type = L4_BootRec_Type(record);
        objects = 0;

        switch(type)
        {
            case L4_BootInfo_Module:
                vms$add_initial_object(initial_objs++,
                        L4_Module_Cmdline(record),
                        L4_Module_Start(record),
                        (L4_Module_Start(record) + L4_Module_Size(record)) - 1,
                        0, VMS$IOF_APP | VMS$IOF_PHYS);
                objects = 1;
                break;

            case L4_BootInfo_SimpleExec:
                if (L4_SimpleExec_Flags(record) & DIT_INITIAL)
                {
                    flags = VMS$IOF_ROOT | VMS$IOF_VIRT;
                }
                else
                {
                    flags = VMS$IOF_APP;
                }

                tstart = L4_SimpleExec_TextVstart(record);
                tsize = L4_SimpleExec_TextSize(record);
                dstart = L4_SimpleExec_DataVstart(record);
                dsize = L4_SimpleExec_DataSize(record);

                vms$add_initial_object(initial_objs++,
                        L4_SimpleExec_Cmdline(record),
                        tstart, (tstart + tsize) - 1,
                        L4_SimpleExec_InitialIP(record),
                        flags | VMS$IOF_PHYS);
                objects = 1;

                if ((!((dstart < (tstart + tsize) && dstart >= tstart)))
                        && (dsize != 0))
                {
                    vms$strncpy(data_name, L4_SimpleExec_Cmdline(record),
                            INITIAL_NAME_MAX - 5);
                    vms$strncat(data_name, ".data", INITIAL_NAME_MAX);

                    vms$add_initial_object(initial_objs++, data_name,
                            dstart, dstart + dsize - 1, 0,
                            VMS$IOF_ROOT | VMS$IOF_VIRT | VMS$IOF_PHYS);
                    objects++;
                }

                break;

            case L4_BootInfo_Multiboot:
                vms$add_initial_object(initial_objs++, "MultiBoot information",
                        L4_MBI_Address(record), L4_MBI_Address(record) + 0xFFF,
                        0, VMS$IOF_PHYS | VMS$IOF_BOOT | VMS$IOF_VIRT);
                objects = 1;
                break;

            default:
                PANIC(1);
                break;
        }

        if (objects > 0)
        {
            count += objects;

            if (count == max) goto overflow;
        }

        record = L4_BootRec_Next(record);
        num_recs--;
    }

    vms$add_initial_object(initial_objs++, "Boot",
            (vms$pointer) bootinfo,
            (vms$pointer) bootinfo + (L4_BootInfo_Size(bootinfo) - 1),
            0, VMS$IOF_BOOT | VMS$IOF_PHYS | VMS$IOF_VIRT);
    count++;

overflow:
    return count;
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
                vms$add_initial_object(initial_objs++, "L4 Object", low, high,
                        0, VMS$IOF_RESERVED | VMS$IOF_PHYS);
                if ((++count) == max) goto overflow;
            }
        }
    }

    vms$add_initial_object(initial_objs++, "L4 Sigma0", kcp->sigma0.low,
            kcp->sigma0.high, 0, VMS$IOF_RESERVED | VMS$IOF_PHYS);
    if ((++count) == max) goto overflow;

    count += vms$bootinfo_find_initial_objects(kip, max - count, initial_objs);

overflow:
    return(count);
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

static void
vms$set_flags(struct vms$meminfo *mem_info, char match, char set)
{
    unsigned int        i;

    for(i = 0; i < mem_info->num_objects; i++)
    {
        if (mem_info->objects[i].flags & match)
        {
            mem_info->objects[i].flags |= set;
        }
    }

    return;
}

void
vms$init(L4_KernelInterfacePage_t *kip, struct vms$meminfo *mem_info,
        unsigned int page_size)
{
    static struct initial_obj       static_objects[NUM_MI_OBJECTS];

    static struct memdesc           static_regions[NUM_MI_REGIONS];
    static struct memdesc           static_io_regions[NUM_MI_IOREGIONS];
    static struct memdesc           static_vm_regions[NUM_MI_VMREGIONS];

    unsigned int                    i;

    notice(SYSBOOT_I_SYSBOOT "initializing virtual memory\n");

    mem_info->regions = static_regions;
    mem_info->max_regions = NUM_MI_REGIONS;
    mem_info->num_regions = vms$find_memory_region(kip,
            NUM_MI_REGIONS, VMS$MEM_RAM, VMS$MEM_IO, static_regions);

    mem_info->io_regions = static_io_regions;
    mem_info->max_io_regions = NUM_MI_IOREGIONS;
    mem_info->num_io_regions = vms$find_memory_region(kip,
            NUM_MI_IOREGIONS, VMS$MEM_IO, VMS$MEM_RAM, static_io_regions);

    mem_info->vm_regions = static_vm_regions;
    mem_info->max_vm_regions = NUM_MI_VMREGIONS;
    mem_info->num_vm_regions = vms$find_memory_region(kip,
            NUM_MI_VMREGIONS, VMS$MEM_VM, 0, static_vm_regions);

    // Create a guard page

    mem_info->num_vm_regions = vms$remove_chunk(mem_info->vm_regions,
            mem_info->num_vm_regions, NUM_MI_VMREGIONS, 0, page_size - 1);

    mem_info->objects = static_objects;
    mem_info->max_objects = NUM_MI_OBJECTS;
    mem_info->num_objects = vms$find_initial_objects(kip,
            NUM_MI_OBJECTS, static_objects);

    // Remove any initial objects from free physical memory

    for(i = 0; i < mem_info->num_objects; i++)
    {
        if (mem_info->objects[i].flags & VMS$IOF_PHYS)
        {
            mem_info->num_regions = vms$remove_chunk(mem_info->regions,
                    mem_info->num_regions, NUM_MI_REGIONS,
                    vms$page_round_down(mem_info->objects[i].base, page_size),
                    vms$page_round_up(mem_info->objects[i].end, page_size) - 1);
        }
    }

    vms$set_flags(mem_info, VMS$IOF_APP, VMS$IOF_VIRT);

    for(i = 0; i < mem_info->num_regions; i++)
    {
        notice(MEM_I_AREA "$%016lX - $%016lX: physical memory\n",
                mem_info->regions[i].base, mem_info->regions[i].end);
    }

    for(i = 0; i < mem_info->num_vm_regions; i++)
    {
        notice(MEM_I_AREA "$%016lX - $%016lX: virtual memory\n",
                mem_info->vm_regions[i].base, mem_info->vm_regions[i].end);
    }

    for(i = 0; i < mem_info->num_io_regions; i++)
    {
        notice(MEM_I_AREA "$%016lX - $%016lX: mapped IO\n",
                mem_info->io_regions[i].base, mem_info->io_regions[i].end);
    }

    for(i = 0; i < mem_info->num_objects; i++)
    {
        if (mem_info->objects[i].flags & VMS$IOF_ROOT)
        {
            notice(MEM_I_AREA "$%016lX - $%016lX: kernel\n",
                    mem_info->objects[i].base, mem_info->objects[i].end);
        }
        else if (mem_info->objects[i].flags & VMS$IOF_RESERVED)
        {
            notice(MEM_I_AREA "$%016lX - $%016lX: reserved by kernel\n",
                    mem_info->objects[i].base, mem_info->objects[i].end);
        }
        else if (mem_info->objects[i].flags & VMS$IOF_BOOT)
        {
            notice(MEM_I_AREA "$%016lX - $%016lX: boot information\n",
                    mem_info->objects[i].base, mem_info->objects[i].end);
        }
        else
        {
            notice(MEM_I_AREA "$%016lX - $%016lX: modules\n",
                    mem_info->objects[i].base, mem_info->objects[i].end);
        }
    }

    return;
}

void
vms$bootstrap(struct vms$meminfo *mem_info, unsigned int page_size)
{
    struct memsection       *heap;

    unsigned int            i;

    vms$pointer             base;
    vms$pointer             end;

    notice(SYSBOOT_I_SYSBOOT "reserving memory for preloaded objects\n");

    // Initialization
    pm_alloc.internal.base = 0;
    pm_alloc.internal.end = 0;
    pm_alloc.internal.active = 0;

    vm_alloc.internal.base = 0;
    vm_alloc.internal.end = 0;
    vm_alloc.internal.active = 0;

    for(i = 0; i <= MAX_FPAGE_ORDER; i++)
    {
        TAILQ_INIT(&vm_alloc.flist[i]);
        TAILQ_INIT(&pm_alloc.flist[i]);
    }

    // Bootimage objects are removed from free virtual memory.
    for(i = 0; i < mem_info->num_objects; i++)
    {
        if (mem_info->objects[i].flags & VMS$IOF_VIRT)
        {
            notice(MEM_I_ALLOC "allocating $%016lX - $%016lX\n",
                    mem_info->objects[i].base, mem_info->objects[i].end);
            vms$remove_virtmem(mem_info, mem_info->objects[i].base,
                    mem_info->objects[i].end, page_size);
        }
    }

    // Free up som virtual memory to bootstrap the fpage allocator.
    for(i = 0; i < mem_info->num_vm_regions; i++)
    {
        base = vms$page_round_up(mem_info->vm_regions[i].base, page_size);
        end = vms$page_round_down(mem_info->vm_regions[i].end + 1, page_size)
            - 1;

        if ((end - (base + 1)) >= (2 * page_size))
        {
            notice(MEM_I_FALLOC "bootstrapping Fpage allocator at virtual "
                    "addresses\n");
            notice(MEM_I_FALLOC "$%016lX - $%016lX\n", base, end);
            vms$fpage_free_internal(&vm_alloc, base, end);
            mem_info->vm_regions[i].end = mem_info->vm_regions[i].base;
            break;
        }
    }

    PANIC(i >= mem_info->num_regions);

    // We need to make sure the first chunk of physical memory we free
    // is at least 2 * page_size to bootstrap the slab allocators for
    // memsections and the fpage lists.

    for(i = 0; i < mem_info->num_regions; i++)
    {
        base = vms$page_round_up(mem_info->regions[i].base, page_size);
        end = vms$page_round_down(mem_info->regions[i].end + 1, page_size) - 1;

        if (((end - base) + 1) >= (2 * page_size))
        {
            notice(MEM_I_SALLOC "bootstrapping Slab allocator at physical "
                    "addresses\n");
            notice(MEM_I_SALLOC "$%016lX - $%016lX\n", base, end);
            vms$fpage_free_chunk(&pm_alloc, base, end);
            mem_info->regions[i].end = mem_info->regions[i].base;
            break;
        }
    }

    PANIC(i >= mem_info->num_regions);

    // Base and end may not be aligned, but we need them to be aligned. If
    // the area is less than a page than we should not add it to the free list.

    for(i = 0; i < mem_info->num_regions; i++)
    {
        if (mem_info->regions[i].base == mem_info->regions[i].end)
        {
            continue;
        }

        base = vms$page_round_up(mem_info->regions[i].base, page_size);
        end = vms$page_round_down(mem_info->regions[i].end + 1, page_size) - 1;

        if (base < end)
        {
            notice(MEM_I_FREE "freeing region $%016lX - $%016lX\n",
                    base, end);
            vms$fpage_free_chunk(&pm_alloc, base, end);
        }
    }

    vms$fpage_clear_internal(&vm_alloc);

    // Initialize VM allocator

    for(i = 0; i < mem_info->num_vm_regions; i++)
    {
        if (mem_info->vm_regions[i].base < mem_info->vm_regions[i].end)
        {
            notice(MEM_I_VALLOC "adding $%016lX - $%016lX to VM allocator\n",
                    mem_info->vm_regions[i].base, mem_info->vm_regions[i].end);
            vms$fpage_free_chunk(&vm_alloc, mem_info->vm_regions[i].base,
                    mem_info->vm_regions[i].end);
        }
    }

    // Setup the kernel heap

    heap = vms$pd_create_memsection((struct pd *) NULL, VMS$HEAP_SIZE, 0,
            VMS$MEM_NORMAL | VMS$MEM_USER);

    PANIC(heap == NULL);
    /*
    vms$alloc_init((void *)heap->base, (void *)heap->end);
    */
    return;
}
