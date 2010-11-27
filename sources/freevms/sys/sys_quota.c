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

struct quota *
sys$new_quota(void)
{
    struct quota        *q;

    if ((q = (struct quota *) sys$alloc(sizeof(struct quota))) == NULL)
    {
        return(NULL);
    }

    sys$initmem((vms$pointer) q, sizeof(struct quota));
    return(q);
}

void
sys$set_quota(struct quota * q, int max)
{
    PANIC(q == NULL);

    q->max_pages = max;
    return;
}
