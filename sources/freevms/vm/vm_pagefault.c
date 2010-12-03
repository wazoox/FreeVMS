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

#include "freevms/pager.h"

static TAILQ_HEAD(mapped_pages, mapped_page)    mapped_pages_head;

void
vms$pagefault_init()
{
    TAILQ_INIT(&mapped_pages_head);
    return;
}

struct mapped_page *
vms$add_mapped_page(struct pd *owner, L4_Fpage_t pfpage, L4_Fpage_t vfpage)
{
    struct mapped_page          *mp;

    /*
    if ((mp = (struct mapped_page *) vms$alloc(sizeof(struct mapped_page)))
            == ((struct mapped_page *) NULL))
    {
        return((struct mapped_page *) NULL);
    }
    */

    mp->owner = owner;
    mp->phys = pfpage;
    mp->virt = vfpage;
    mp->flags = VMS$MAPPED;

    return(mp);
}

void
vms$remove_mapped_page(struct pd *owner)
{  
}

void
vms$pagefault(L4_ThreadId_t caller, vms$pointer addr, vms$pointer ip,
        vms$pointer tag)
{
    // fpage [...........0RWX]

    L4_Fpage_t              pfpage;
    L4_Fpage_t              vfpage;

    L4_Msg_t                msg;

    struct memsection       *memsection;

    struct thread           *thread;

    vms$pointer             priv;
    vms$pointer             ref;
    vms$pointer             size;

    // Read privileges
    priv = (tag & 0xf0000) >> 16;

//notice("vms$pagefault(addr:%lx) from %lx [priv=%lx]\n", addr, caller, priv);
    // Find memory section it belongs too
    /*
    if ((memsection = vms$objtable_lookup((void *) addr)) == NULL)
    {
        notice(MEM_F_MEMSEC "no memory section\n");
        PANIC(1);
        goto fail;
    }

    ref = (vms$pointer) memsection + priv;

dbg$sigma0(10000000);
notice("ref=%lx\n", ref);
    if (sec$check(caller, ref) == 0)
    {
        extern fpage_alloc          pm_alloc;
        vms$pointer phys;
        vms$pointer virt;

notice("memsection %lx %lx\n", memsection->base, memsection->end);
notice("flags %lx\n", memsection->flags);

        size = vms$min_pagesize();
        virt = vms$page_round_down(addr, size);

        phys = vms$fpage_alloc_chunk(&pm_alloc, size);

        if (phys == INVALID_ADDR)
        {
            notice(MEM_F_OUTMEM "out of memory at IP=$%016lX, TID=$%lX\n",
                    ip, jobctl$threadno(L4_ThreadNo(caller)));
            goto fail;
        }
notice("V=%lx P=%lx\n", virt, phys);
notice("addr=%lx size=%lx\n", virt, size);
notice("Priv=%d %d\n", priv, L4_FullyAccessible);
        pfpage = L4_Fpage(phys, size);
        vfpage = L4_Fpage(virt, size);

        vms$sigma0_map_fpage(vfpage, pfpage, L4_FullyAccessible);
        // Why priv does not work instead of L4_FullyAccessible ?
        L4_Set_Rights(&pfpage, L4_FullyAccessible);
        L4_Clear(&msg);
        L4_Append(&msg, L4_MapItem(vfpage, virt));
        L4_Load(&msg);
    }
    else
    {
        notice(MEM_F_SECFLD "security check failed\n");
        goto fail;
    }
    */

    return;

fail:
    /*
    L4_Clear(&msg);
    L4_Append(&msg, L4_MapItem(L4_Nilpage, L4_Address(L4_Nilpage)));
    L4_Load(&msg);

    L4_Stop(caller);
    thread = jobctl$thread_lookup(caller);
    jobctl$thread_delete(thread);
    */
    return;
}
