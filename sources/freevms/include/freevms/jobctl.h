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

struct thread
{
		struct pd			*owner;
		void				*utcb;
};

struct thread_node
{
	struct thread_node		*next;
	struct thread_node		*prev;
	struct thread			data;
};

struct thread_list
{
	struct thread_node		*first;
	struct thread_node		*last;
};

enum pd_state
{
	pd_empty,			// newly created
	pd_active,			// has active threads
	pd_suspended		// last thread was removed, but not deleted yet
};

typedef unsigned long		bfl_word;
#define BITS_PER_BFL_WORD	((int) (sizeof(bfl_word) * 8))

struct bfl
{
	unsigned int			curpos;
	unsigned int			len;
	unsigned int			array_size;

	bfl_word				bitarray[];
};

typedef struct bfl			*bfl_t;

bfl_t jobctl$bfl_new(vms$pointer size);
void jobctl$bfl_free(bfl_t rfl, vms$pointer val);
vms$pointer jobctl$bfl_alloc(bfl_t rfl);

struct pd
{
    struct pd               *owner;			// Our owner
	enum pd_state			state;

	struct memsection		*callback_buffer;
	struct cb_alloc_handle	*cba;

	// PD information
    struct memsection_list  memsections;

	// Thread information
	bfl_t					local_threadno; // Freelist for UTCBs
	struct thread_list		threads;
};

struct memsection *vms$pd_create_memsection(struct pd *self, vms$pointer size,
        vms$pointer base, unsigned int flags);

void jobctl$utcb_init(L4_KernelInterfacePage_t *kip);
void jobctl$pd_init(struct vms$meminfo *meminfo);
