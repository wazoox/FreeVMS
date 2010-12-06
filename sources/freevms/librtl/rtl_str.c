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
rtl$strcmp()
{
}

static vms$pointer
rtl$strcpy(unsigned char *dest, const char *src, vms$pointer size)
{
    vms$pointer             length;

    length = 0;

    while(*src)
    {
        if (length == size)
        {
            return(length);
        }

        *dest++ = *src++;
        length++;
    }

    return(length);
}

void
rtl$strcpy(struct vms$string *str_dest, struct vms$string *str_src)
{
    return;
}

void
rtl$strcpy(struct vms$string *str_dest, const char *src)
{
    str_dest->length_trim = rtl$strcpy(str_dest->c, src,
            str_dest->length);
    return;
}
