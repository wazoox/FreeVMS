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

void *
vms$slab_cache_alloc(struct slab_cache *sc)
{
	int						length;

	struct memsection		*pool;
	struct slab				*slab;

	unsigned char			*ptr;

	TAILQ_FOREACH(pool, &sc->pools, pools)
	{
		if (!TAILQ_EMPTY(&pool->slabs))
		{
			break;
		}
	}

	if (pool == NULL)
	{
		//pool = vms$memsection_create_cache(sc);
	}

	if (pool == NULL)
	{
		return(NULL);
	}

	slab = TAILQ_FIRST(&pool->slabs);
	TAILQ_REMOVE(&pool->slabs, TAILQ_FIRST(&pool->slabs), slabs);

	length = sc->slab_size;

	while(length > 0)
	{
		(*ptr) = '\0';
		length--;
		ptr++;
	}

	return(slab);
}

void
vms$slab_cache_free(struct slab_cache *sc, void *ptr)
{
	struct memsection		*pool;
	struct slab				*slab;

	slab = (struct slab *) ptr;
	// TODO
	//pool = objtable_lookup((void *) ((vms$pointer) ptr));

	//TAILQ_INSERT_TAIL(&(pool->slabs), slab, slabs);
	return;
}
