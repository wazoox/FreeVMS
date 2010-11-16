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
	int						running;

	L4_ThreadId_t			partner;
	L4_MsgTag_t				tag;
	L4_Msg_t				msg;

	running = 1;
	tag = L4_Wait(&partner);

	while(running)
	{
		L4_Clear(&msg);
		L4_Store(tag, &msg);

		if ((tag.raw & L4_REQUEST_MASK) == L4_PAGEFAULT)
		{
			vms$pagefault(partner, L4_Get(&msg, 0), L4_Get(&msg, 1),
					tag.raw);
		}

		L4_ReplyWait(partner, &partner);
	}

    return;
}
