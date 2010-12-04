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
sys$loop()
{
    int                     running;

    L4_ThreadId_t           partner;
    L4_MsgBuffer_t          buffer;
    L4_MsgTag_t             tag;
    L4_Msg_t                msg;
	L4_StringItem_t			string_item;

	unsigned char			*string;

    running = 1;

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace)
            + L4_StringItemsAcceptor);
    tag = L4_Wait(&partner);

    while(running)
    {
        L4_Clear(&msg);
        L4_Store(tag, &msg);

        if ((tag.raw & L4_REQUEST_MASK) == L4_PAGEFAULT)
        {
            sys$pagefault(partner, L4_Get(&msg, 0), L4_Get(&msg, 1),
                    tag.raw);
        }
        else
        {
            switch(L4_Label(tag))
            {
                case CALL$PRINT:
					// This memory is mapped by calling thread
					L4_StoreMRs(1, 2, string_item.raw);

					if ((string = (unsigned char *)
							sys$alloc((string_item.X.string_length + 1) *
							sizeof(unsigned char))) != NULL)
					{
						sys$memcopy((vms$pointer) string,
								(vms$pointer) string_item.X.str.string_ptr,
								string_item.X.string_length);
						string[string_item.X.string_length] = 0;
						notice("%s\n", string);
						sys$free(string);
					}
                    break;

                default:
                    PANIC(1, notice(IPC_F_UNKNOWN "unknown IPC from $%lX "
                            "with label $%lX\n", L4_ThreadNo(partner),
                            L4_Label(tag)));
            }
        }

        tag = L4_ReplyWait(partner, &partner);
    }

    return;
}
