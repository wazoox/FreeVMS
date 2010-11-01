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

static vms$pointer      vms$bss = 0x0;
static vms$pointer      vms$top = 0x0;

static Header			base;
static Header			*_kr_alloc_freep = NULL;
#define	freep			_kr_alloc_freep

static struct mutex		alloc_mutex;

void
vms$alloc_init(vms$pointer bss_p, vms$pointer top_p)
{
    vms$bss = bss_p;
    vms$top = top_p;

	lock$mutex_init(&alloc_mutex);
    return;
}

// sbrk equivalent
#define round_up(address, size)		((((address) + (size - 1)) & (~(size - 1))))

static void *
vms$morecore(unsigned int nu)
{
	int						r;

	Header					*up;

	vms$pointer				cp;

	cp = vms$bss;

	// we can only request more memory in pagesized chunks
	nu = round_up(nu, vms$min_pagesize() / sizeof(Header));

	if ((vms$bss + (nu * sizeof(Header))) > vms$top)
	{
		return(NULL);
	}

	up = (Header *) cp;

	// On this system we need to ensure we back memory
	// before touching it, or else we are in trouble
	// FIXME: we should check exactly how much to back here !
	r = vms$back_mem((vms$pointer) up, ((vms$pointer) (up + nu)) - 1);

	if (r != 0)
	{
		return(NULL);
	}

	vms$bss += (nu * sizeof(Header));
	up->s.size = nu;

	vms$free((void *) (up + 1));
	return(_kr_alloc_freep);
}

void *
vms$alloc(vms$pointer nbytes)
{
	Header					*p;
	Header					*prevp;

	unsigned int			nunits;

	lock$mutex_count_lock(&alloc_mutex);

	nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

	if ((prevp = freep) == NULL) // No free list yet
	{
		prevp = &base;
		freep = prevp;
		base.s.ptr = freep;
		base.s.size = 0;
	}

	for(p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
	{
		if (p->s.size >= nunits)
		{
			// big enough
			if (p->s.size == nunits)
			{
				// exactly
				prevp->s.ptr = p->s.ptr;
			}
			else
			{	// allocate tail end
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}

			freep = prevp;
			lock$mutex_count_unlock(&alloc_mutex);
			return((void *) (p + 1));
		}

		if (p == freep)
		{
			lock$mutex_count_unlock(&alloc_mutex);

			// wrapped around free list
			if ((p = (Header *) vms$morecore(nunits)) == NULL)
			{
				return(NULL);
			}

			lock$mutex_count_lock(&alloc_mutex);
		}
	}

	lock$mutex_count_unlock(&alloc_mutex);
	return(NULL);
}

void
vms$free(void *ap)
{
	Header			*bp;
	Header			*p;

	if (ap == NULL)
	{
		return;
	}

	lock$mutex_count_lock(&alloc_mutex);
	bp = (Header *) ap - 1;		// point to block header

	for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	{
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
		{
			// freed block at start or end of arena
			break;
		}
	}

	if ((bp + bp->s.size) == p->s.ptr)
	{
		// join to upper nbr
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else
	{
		bp->s.ptr = p->s.ptr;
	}

	if ((p + p->s.size) == bp)
	{
		// join to lower nbr
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	}
	else
	{
		p->s.ptr = bp;
	}

	freep = p;
	lock$mutex_count_unlock(&alloc_mutex);

	return;
}
