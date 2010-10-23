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
sigma0_map_fpage(L4_Fpage_t virt_page, L4_Fpage_t phys_page)
{
    L4_ThreadId_t           tid;

    L4_MsgTag_t             tag;

    L4_Msg_t                msg;

    L4_MapItem_t            map;

    // Find Pager's ID
    tid = L4_Pager();
    L4_Set_Rights(&phys_page, L4_FullyAccessible);
    L4_Accept(L4_MapGrantItems(virt_page));
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, (L4_Word_t) phys_page.raw);
    L4_MsgAppendWord(&msg, (L4_Word_t) 0);
    L4_Set_Label(&msg.tag, SIGMA0_REQUEST_LABEL);
    L4_MsgLoad(&msg);

    tag = L4_Call(tid);

    PANIC(L4_IpcFailed(tag), notice(IPC_F_FAILED "IPC failed\n"));

    L4_MsgStore(tag, &msg);
    L4_MsgGetMapItem(&msg, 0, &map);

    PANIC(map.X.snd_fpage.raw == L4_Nilpage.raw,
            notice(MEM_F_REJMAP, "rejecting mapping\n"));

    return;
}

void
vms$sigma0_map(vms$pointer virt_addr, vms$pointer phys_addr, vms$pointer size)
{
    L4_Fpage_t              ppage;
    L4_Fpage_t              vpage;

    vms$pointer             pbase;
    vms$pointer             pend;
    vms$pointer             vbase;
    vms$pointer             vend;

    vms$debug(__func__);

    vbase = virt_addr;
    vend = (vbase + size) - 1;

    pbase = phys_addr;
    pend = (pbase + size) - 1;

    if (vbase < vend)
    {
        vpage = vms$biggest_fpage(vbase, vbase, vend);
        ppage = vms$biggest_fpage(pbase, pbase, pend);

        if (L4_Size(vpage) > L4_Size(ppage))
        {
            vpage = L4_Fpage(vbase, L4_Size(ppage));
        }
        else if (L4_Size(ppage) > L4_Size(vpage))
        {
            ppage = L4_Fpage(pbase, L4_Size(vpage));
        }

        sigma0_map_fpage(vpage, ppage);

        vbase += L4_Size(vpage);
        pbase += L4_Size(ppage);
    }

    return;
}
