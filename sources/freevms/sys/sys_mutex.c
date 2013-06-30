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
sys$mutex_init(mutex_t mutex)
{
    mutex->holder = 0;
    mutex->needed = 0;
    mutex->count = 0;
	mutex->internal = 0;

    return;
}

void
sys$mutex_count_lock(mutex_t mutex)
{
	L4_Word_t		me;

	me = L4_Myself().raw;

notice("me=%lu sizeof(me)=%d\n", me, sizeof me);
notice("mutex before mutex_lock((mutex_t) &(mutex->internal)) %lu\n", mutex->internal);
    mutex_lock((mutex_t) &(mutex->internal));
notice("mutex after mutex_lock((mutex_t) &(mutex->internal)) %lu\n", mutex->internal);

	if (mutex->holder == L4_nilthread.raw)
	{
notice("mutex before mutex_lock(mutex) %lu\n", mutex->holder);
		mutex_lock(mutex);
notice("mutex after mutex_lock(mutex) %lu\n", mutex->holder);
	}

	if (me == mutex->holder)
	{
    	mutex->count++;
	}

	mutex_unlock((mutex_t) &(mutex->internal));
notice("mutex after mutex_unlock((mutex_t) &(mutex->internal)) %lu\n", mutex->internal);

    return;
}

void
sys$mutex_count_unlock(mutex_t mutex)
{
	L4_Word_t		me;

	me = L4_Myself().raw;

notice("<1>\n");
	mutex_lock((mutex_t) &(mutex->internal));
notice("<2>\n");

	if (me == mutex->holder)
	{
notice("<2a>\n");
		mutex->count--;

		if (mutex->count == 0)
		{
notice("<2b>\n");
			mutex_unlock(mutex);
notice("<2c>\n");
		}
	}

notice("<3>\n");
	mutex_unlock((mutex_t) &(mutex->internal));
notice("<4>\n");

    return;
}
