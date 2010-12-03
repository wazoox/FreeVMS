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

rfl_t                  thread_list;

vms$pointer             utcb_size;
vms$pointer             utcb_size_log2;

L4_Fpage_t              kip_area;

static L4_Word_t        max_threadno;
static L4_Word_t        min_threadno;

#define JOBCTL$THREAD_PD_HASHSIZE   4096
struct hashtable        *l4tid_to_thread;

void
sys$utcb_init(L4_KernelInterfacePage_t *kip)
{
    utcb_size = L4_UtcbSize(kip);
    utcb_size_log2 = kip->UtcbAreaInfo.X.a;

    // We map the kip into all area.
    kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2(kip));
    return;
}

static inline void
sys$pd_list_init(struct pd_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static struct pd *
sys$pd_setup(struct pd *self, struct pd *parent, int max_threads)
{
    self->owner = parent;
    self->state = pd_empty;
    self->local_threadno = sys$bfl_new(max_threads);
    self->callback_buffer = NULL;
    self->cba = NULL;

    sys$thread_list_init(&self->threads);
    sys$pd_list_init(&self->pds);
    sys$session_p_list_init(&self->sessions);
    sys$memsection_list_init(&self->memsections);
    sys$eas_list_init(&self->eass);
    sys$clist_list_init(&self->clists);
    self->quota = sys$new_quota();
    sys$set_quota(self->quota, QUOTA$INF);

    return(self);
}

void
sys$pd_init(struct vms$meminfo *meminfo)
{
    extern int                      vms$pd_initialized;
    extern struct pd                freevms_pd;
    extern struct memsection_list   internal_memsections;

    struct memsection_list          *list;

    struct memsection_node          *first_ms;
    struct memsection_node          *next;
    struct memsection_node          *node;

    list = &freevms_pd.memsections;

    // Setup freevms_pd with itself as parent.
    sys$pd_setup(&freevms_pd, &freevms_pd, NUMBER_OF_KERNEL_THREADS);
    freevms_pd.state = pd_active;

    // Insert memsections used during bootstrapping into memsection list
    // and objtable.

    first_ms = internal_memsections.first;
    node = first_ms;

    do
    {
        next = node->next;
        objtable_insert(&node->data);
        node->next = (struct memsection_node *)list;
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
        node = next;
    } while(node != first_ms);

    vms$pd_initialized = 1;
    return;
}

void
sys$thread_init(L4_KernelInterfacePage_t *kip)
{
    int                 r;

    // L4_Myself() + 1 is the callback thread
    min_threadno = L4_ThreadNo(L4_Myself()) + 2;
    max_threadno = ((vms$pointer) 1) << L4_ThreadIdBits(kip);

    notice(JOBCTL_I_MAXTRD "setting maximal concurrent threads to %ld\n",
            (max_threadno - min_threadno) + 1);
    thread_list = sys$rfl_new();
    r = sys$rfl_insert_range(thread_list, min_threadno, max_threadno);
    PANIC(r != 0);

    l4tid_to_thread = hash_init(JOBCTL$THREAD_PD_HASHSIZE);
    return;
}

static struct memsection *
sys$setup_utcb_area(struct pd *self, void **base, L4_Fpage_t *area,
        int threads, vms$pointer pagesize)
{
    struct memsection       *utcb_obj;

    vms$pointer             area_size;
    vms$pointer             fpage_size;

    // Compute the size of UTCB.
    // We should look at the utcb min size and alignment
    // here, but for now we assume that page alignment is good enough
    area_size = sys$page_round_up(utcb_size * threads, vms$min_pagesize());

    // Find Fpage size.
    for(fpage_size = 1U; fpage_size < area_size; fpage_size = fpage_size << 1);

    utcb_obj = sys$pd_create_memsection(self, fpage_size, 0, VMS$MEM_UTCB,
            pagesize);

    if (!utcb_obj)
    {
        return(NULL);
    }

    if ((utcb_obj->base % fpage_size))
    {
        return(NULL);
    }

    (*base) = (void*) utcb_obj->base;
    (*area) = L4_Fpage((L4_Word_t) *base, area_size);

    return(utcb_obj);
}

struct pd *
sys$pd_create(struct pd *self, int max_threads, vms$pointer pagesize)
{
    struct pd       *new_pd;

    if (max_threads > JOBCTL$MAX_THREADS_PER_APD)
    {
        return(NULL);
    }

    if (!self)
    {
        return(NULL);
    }

    if (max_threads == 0)
    {
        max_threads = JOBCTL$MAX_THREADS_PER_APD;
    }

    if ((new_pd = sys$pd_list_create_back(&self->pds)) == NULL)
    {
        return(NULL);
    }

    sys$pd_setup(new_pd, self, max_threads);

    new_pd->utcb_memsection = sys$setup_utcb_area(new_pd,
            &new_pd->utcb_base, &new_pd->utcb_area, max_threads,
            pagesize);

    if (!new_pd->utcb_memsection)
    {
        return(NULL);
    }

    return(new_pd);
}
