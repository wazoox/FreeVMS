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

void
vms$sigma0_map_fpage(L4_Fpage_t virt_page, L4_Fpage_t phys_page,
		unsigned int priv)
{
    L4_ThreadId_t           tid;

    L4_MsgTag_t             tag;

    L4_Msg_t                msg;

    L4_MapItem_t            map;

    // Find Pager's ID
    tid = L4_Pager();
    L4_Set_Rights(&phys_page, priv);
    L4_Accept(L4_MapGrantItems(virt_page));
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, (L4_Word_t) phys_page.raw);
    L4_MsgAppendWord(&msg, (L4_Word_t) 0);
    L4_Set_Label(&msg.tag, SIGMA0_REQUEST_LABEL);
    L4_MsgLoad(&msg);

    tag = L4_Call(tid);

    PANIC(L4_IpcFailed(tag), notice(IPC_F_FAILED "IPC failed (error %ld: %s)\n",
            L4_ErrorCode(), L4_ErrorCode_String(L4_ErrorCode())));

    L4_MsgStore(tag, &msg);
    L4_MsgGetMapItem(&msg, 0, &map);

    if (dbg$virtual_memory == 1)
    {
        if (map.X.snd_fpage.raw == L4_Nilpage.raw)
        {
            notice(MEM_I_REJMAP "rejecting mapping\n");
            notice(MEM_I_REJMAP "virtual  $%016lX - $%016lX\n",
                    L4_Address(virt_page), L4_Address(virt_page)
                    + (L4_Size(virt_page) - 1));
            notice(MEM_I_REJMAP "physical $%016lX - $%016lX\n",
                    L4_Address(phys_page), L4_Address(phys_page)
                    + (L4_Size(phys_page) - 1));
        }
        else
        {
            notice(MEM_I_ACCMAP "accepting mapping\n");
            notice(MEM_I_ACCMAP "virtual  $%016lX - $%016lX\n",
                    L4_Address(virt_page), L4_Address(virt_page)
                    + (L4_Size(virt_page) - 1));
            notice(MEM_I_ACCMAP "physical $%016lX - $%016lX\n",
                    L4_Address(phys_page), L4_Address(phys_page)
                    + (L4_Size(phys_page) - 1));
        }
    }

    return;
}

void
vms$sigma0_map(vms$pointer virt_addr, vms$pointer phys_addr, vms$pointer size,
		unsigned int priv)
{
    L4_Fpage_t              ppage;
    L4_Fpage_t              vpage;

    vms$pointer             pbase;
    vms$pointer             pend;
    vms$pointer             vbase;
    vms$pointer             vend;

    vbase = virt_addr;
    vend = vbase + (size - 1);

    pbase = phys_addr;
    pend = pbase + (size - 1);

    if (vbase < vend)
    {
        /* ???? We should map a Fpage greater than size !
        vpage = vms$biggest_fpage(vbase, vbase, vend);
        ppage = vms$biggest_fpage(pbase, pbase, pend);
        */

        vpage = L4_Fpage(vbase, (vend - vbase) + 1);
        ppage = L4_Fpage(pbase, (pend - pbase) + 1);

        PANIC(L4_IsNilFpage(vpage) || L4_IsNilFpage(ppage));

        if (L4_Size(vpage) > L4_Size(ppage))
        {
            vpage = L4_Fpage(vbase, L4_Size(ppage));
        }
        else if (L4_Size(ppage) > L4_Size(vpage))
        {
            ppage = L4_Fpage(pbase, L4_Size(vpage));
        }

        vms$sigma0_map_fpage(vpage, ppage, L4_FullyAccessible);

        vbase += L4_Size(vpage);
        pbase += L4_Size(ppage);
    }

    return;
}
