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

struct mutex
{
    L4_Word_t       holder;
    L4_Word_t       needed;
    L4_Word_t       count;
};

typedef struct mutex    *mutex_t;

extern "C"
{
    vms$pointer try_lock_amd64(vms$pointer mutex, vms$pointer myself);
}

static inline void
mutex_lock(mutex_t m)
{
    L4_Word_t       me;

    me = L4_Myself().raw;

int r;
	printf("1=%lx 2=%lx\n", (vms$pointer) m, (vms$pointer) me);
	printf("m->holder=%lx\n", m->holder);
    while(!(r=arch_specific(try_lock)((vms$pointer) m, (vms$pointer) me)))
    {
		printf("r=%d mutex->holder=%lx\n", r, m->holder);
        L4_ThreadSwitch(L4_nilthread);
    }

	printf("> %d\n", r);

    return;
}

static inline void
mutex_unlock(mutex_t mutex)
{
    mutex->holder = 0;

    if (mutex->needed)
    {
        mutex->needed = 0;
        L4_ThreadSwitch(L4_nilthread);
    }

    return;
}

void lock$mutex_init(mutex_t mutex);
void lock$mutex_count_lock(mutex_t mutex);
void lock$mutex_count_unlock(mutex_t mutex);

