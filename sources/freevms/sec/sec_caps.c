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

cap_t
sec$create_capability(vms$pointer reference, enum cap_type type)
{
	cap_t			cap;

	cap.ref.obj = reference;
	cap.type = type;
	cap.passwd = rand$extract_number(sizeof(cap.passwd));

	return(cap);
}

static cap_t
sec$validate_access(vms$pointer ref, struct pd *pd)
{
	cap_t				cap;
	cap_t				*temp;

	unsigned int		i;
	unsigned int		iid_bits = 3;

	vms$pointer			master_ref;

	struct clist_node	*clists;

	INVALID_CAP(cap);
	iid_bits = 3;
	master_ref = (ref >> iid_bits) << iid_bits;

	for (clists = pd->clists.first;
			clists->next != pd->clists.first;
			clists = clists->next)
	{
		temp = clists->data.clist;

		if (temp == NULL)
		{
			break;
		}

		for(i = 0; (i < clists->data.length) && IS_VALID_CAP(temp[i]); i++)
		{
			if ((temp[i].ref.obj == ref) || (temp[i].ref.obj == master_ref))
			{
				cap = temp[i];
				goto found;
			}
		}
	}

found:
	return cap;
}

int
sec$check(L4_ThreadId_t tid, vms$pointer ref)
{
	cap_t				cap;

	struct thread		*thread;
	struct pd			*pd;
	struct memsection	*memsection;

	thread = jobctl$thread_lookup(tid);

	if (thread == NULL)
	{
		return(-1);
	}

	pd = thread->owner;
	memsection = vms$objtable_lookup((void *) ref);

	if (memsection == NULL)
	{
		return(-2);
	}

	cap = sec$validate_access(ref, pd);

	if (IS_VALID_CAP(cap))
	{
		return(0);
	}
	else
	{
		return(-1);
	}

	return(0);
}
