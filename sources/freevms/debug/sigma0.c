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
dbg$sigma0(void)
{
    L4_Msg_t                msg;

    L4_MsgTag_t             tag;

    L4_ThreadId_t           sigma0;

#   define L4_S0EXT_VERBOSE     (1)
#   define L4_SIGMA0_EXT        (((vms$pointer) -1001) << 4)

    sigma0 = L4_Pager();
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, (L4_Word_t) L4_S0EXT_VERBOSE);
    L4_MsgAppendWord(&msg, (L4_Word_t) DEBUG_SIGMA0);
    L4_Set_Label(&msg.tag, L4_SIGMA0_EXT);
    L4_MsgLoad(&msg);

    tag = L4_Send(sigma0);
    PANIC(L4_IpcFailed(tag), notice(IPC_F_FAILED "IPC failed\n"));

    return;
}

