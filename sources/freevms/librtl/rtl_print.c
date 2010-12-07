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
rtl$print(struct vms$string *fmt, void **arg)
{
	extern L4_ThreadId_t		roottask_tid;

    vms$string_initializer(str, 1024);

    L4_Msg_t            msg;
    L4_StringItem_t     si;

    if (fmt == NULL)
    {
        return;
    }

    rtl$sprint(&str, fmt, arg);

    if (str.length_trim <= MAX_STRINGITEM_LENGTH)
    {
        si = L4_StringItem(str.length_trim, (void *) str.c);

        L4_Clear(&msg);
        L4_Append(&msg, si);
        L4_Set_Label(&msg, SYSCALL$PRINT);
        L4_Load(&msg);

        L4_Call(roottask_tid);
    }

    return;
}
