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

static L4_Fpage_t       kip_area;

static vms$pointer      utcb_size;
static vms$pointer      utcb_size_log2;

void
jobctl$utcb_init(L4_KernelInterfacePage_t *kip)
{
    utcb_size = L4_UtcbSize(kip);
    utcb_size_log2 = kip->UtcbAreaInfo.X.a;

    // We map the kip into all area.
    kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2(kip));
    return;
}

static struct pd *
jobctl$pd_setup(struct pd *self, struct pd *parent, int max_threads)
{
	self->owner = parent;
	self->state = pd_empty;
	self->local_threadno = jobctl$bfl_new(max_threads);
	self->callback_buffer = NULL;
	self->cba = NULL;

	/*
	jobctl$thread_list_init(&self->threads);
	jobctl$pd_list_init(&self->pds);
	jobctl$session_p_list_init(&self->sessions);
	vms$memsection_list_init(&self->memsections);
	vms$eas_list_init(&self->eass);
	clist_list_init(&self->clists);
	self->quota = new_quota();
	set_quota(self->quota, QUOTA_INF);
	*/

	return(self);
}

void
jobctl$pd_init(struct vms$meminfo *meminfo)
{
	extern int 						vms$pd_initialized;
	extern struct pd				freevms_pd;
	extern struct memsection_list	internal_memsections;

    struct memsection_list      	*list;

    struct memsection_node      	*first_ms;
    struct memsection_node      	*next;
    struct memsection_node      	*node;

    list = &freevms_pd.memsections;

	// Setup freevms_pd with itself as parent.
	jobctl$pd_setup(&freevms_pd, &freevms_pd, NUMBER_OF_KERNEL_THREADS);
	vms$pd_initialized = 1;

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

	return;
}
