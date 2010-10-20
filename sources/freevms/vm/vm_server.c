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
vms$pager()
{
    L4_Fpage_t                  fpage;

    L4_KernelInterfacePage_t    *kip;

    L4_Msg_t                    msg;

    L4_MsgTag_t                 tag;

    L4_ThreadId_t               tid;

    L4_Word_t                   api_flags;
    L4_Word_t                   faddr;
    L4_Word_t                   fip;
    L4_Word_t                   kernel_id;
    L4_Word_t                   kernel_interface;
    L4_Word_t                   page_bits;

    kip = (L4_KernelInterfacePage_t *) L4_KernelInterface(&kernel_interface,
            &api_flags, &kernel_id);
    for(page_bits = 0; !((1 << page_bits) & L4_PageSizeMask(kip)); page_bits++);

    notice(SYSBOOT_I_SYSBOOT "spawning virtual memory system "
            "with executive privileges\n");

    L4_Wait(&tid);

    for(;;)
    {
        L4_Store(tag, &msg);

        PANIC((L4_UntypedWords (tag) != 2) ||
                (L4_TypedWords (tag) != 0) ||
                (!L4_IpcSucceeded (tag)),
                notice("Malformed pagefault IPC from %p (tag=%p)\n",
                (void *) tid.raw, (void *) tag.raw));

        faddr = L4_Get(&msg, 0);
        fip = L4_Get(&msg, 1);

#       ifdef AMD64
        L4_Fpage_t  fp;

        fp.raw = faddr;

         if (((tag.raw & L4_REQUEST_MASK) == L4_IO_PAGEFAULT) &&
                L4_IsIoFpage(fp))
         {
             fpage = fp;
         }
         else
         {
             fpage = L4_FpageLog2(faddr, page_bits);
         }
#       endif

        L4_Clear(&msg);
        L4_Append(&msg, L4_MapItem (fpage + L4_FullyAccessible, faddr));
        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }

    return;
}
