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

static Header           base;
static Header           *_kr_alloc_freep = NULL;
#define freep           _kr_alloc_freep

static struct mutex     alloc_mutex;

void
sys$alloc_init(vms$pointer bss_p, vms$pointer top_p)
{
    vms$bss = bss_p;
    vms$top = top_p;
notice("vms$bss=$%016lX\n", vms$bss);
notice("vms$top=$%016lX\n", vms$top);

    sys$mutex_init(&alloc_mutex);
    return;
}

// sbrk equivalent
#define round_up(address, size)     ((((address) + (size - 1)) & (~(size - 1))))

static void *
sys$morecore(unsigned int nu)
{
    int                     r;

    Header                  *up;

    vms$pointer             cp;

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
    r = sys$back_mem((vms$pointer) up, ((vms$pointer) (up + nu)) - 1,
            vms$min_pagesize());

    if (r != 0)
    {
        return(NULL);
    }

    vms$bss += (nu * sizeof(Header));
notice("vms$bss=$%016lX\n", vms$bss);
    up->s.size = nu;

notice("<c1>\n");
    sys$free((void *) (up + 1));
notice("<c2>\n");
    return(_kr_alloc_freep);
}

void *
sys$alloc(vms$pointer nbytes)
{
    Header                  *p;
    Header                  *prevp;

    unsigned int            nunits;

notice("<a1>\n");
    sys$mutex_count_lock(&alloc_mutex);

    nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

    if ((prevp = freep) == NULL) // No free list yet
    {
notice("<a1.1>\n");
        prevp = &base;
        freep = prevp;
        base.s.ptr = freep;
        base.s.size = 0;
    }

notice("<a2>\n");
    for(p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
    {
notice("<a2.1> %p\n", p);
        if (p->s.size >= nunits)
        {
notice("<a2.2>\n");
            // big enough
            if (p->s.size == nunits)
            {
notice("<a2.3>\n");
                // exactly
                prevp->s.ptr = p->s.ptr;
            }
            else
            {   // allocate tail end
notice("<a2.4>\n");
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }

            freep = prevp;
            sys$mutex_count_unlock(&alloc_mutex);
notice("sys$alloc %ld %p\n", nbytes, (void *) (p + 1));
            return((void *) (p + 1));
        }

notice("<a3>\n");
        if (p == freep)
        {
notice("<a4>\n");
            // wrapped around free list
            if ((p = (Header *) sys$morecore(nunits)) == NULL)
            {
notice("<a4.1>\n");
                sys$mutex_count_unlock(&alloc_mutex);
                return(NULL);
            }
notice("<a5>\n");
        }
    }

    sys$mutex_count_unlock(&alloc_mutex);
	PANIC(1);
}

void
sys$free(void *ap)
{
    Header          *bp;
    Header          *p;

    if (ap == NULL)
    {
        return;
    }

notice("<b1>\n");
    sys$mutex_count_lock(&alloc_mutex);
notice("<b2>\n");
    bp = (Header *) ap - 1;     // point to block header

    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    {
//notice("p=%lu\n", p);
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
        {
            // freed block at start or end of arena
            break;
        }
    }

notice("<b3>\n");
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

notice("<b4>\n");
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

notice("<b5>\n");
    freep = p;
    sys$mutex_count_unlock(&alloc_mutex);

    return;
}
