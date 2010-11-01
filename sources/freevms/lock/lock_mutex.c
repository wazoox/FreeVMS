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
lock$mutex_init(mutex_t mutex)
{
    mutex->holder = 0;
    mutex->needed = 0;
    mutex->count = 0;

    return;
}

void
lock$mutex_count_lock(mutex_t mutex)
{
    mutex_lock(mutex);
    mutex->count++;

    return;
}

void
lock$mutex_count_unlock(mutex_t mutex)
{
    mutex->count--;

    if (mutex->count == 0)
    {
        mutex_unlock(mutex);
    }

    return;
}
