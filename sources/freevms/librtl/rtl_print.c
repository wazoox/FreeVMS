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

int
rtl$print(const char *fmt, int size, ...)
{
    char                str[1024];

    L4_Msg_t            msg;
    L4_StringItem_t     si;

    va_list     ap;

    if (fmt == NULL)
    {
        return(0);
    }

    va_start(ap, size);
    //vsnprintf(str, size, fmt, ap);
    va_end(ap);

    si = L4_StringItem(1024, (void *) str);

    L4_Clear(&msg);
    L4_Append(&msg, si);
    L4_Set_Label(&msg, 1);
    L4_Load(&msg);

    L4_Call(L4_Pager());

    return(0);
}
