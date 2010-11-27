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

static int
find_first_set(bfl_word x)
{
    int             ret;

    for(ret = 0; ret < BITS_PER_BFL_WORD; ret++)
    {
        if (x & (bfl_word) 1)
        {
            return(ret);
        }

        x >>= (bfl_word) 1;
    }

    return(-1);
}

bfl_t
sys$bfl_new(vms$pointer size)
{
    bfl_t                   bfl;

    vms$pointer             array_size;
    vms$pointer             i;

    array_size = (size / BITS_PER_BFL_WORD) + 1;

    // Allocate enough space for the header and the bit array needed
    if ((bfl = (bfl_t) sys$alloc(sizeof(struct bfl) + (array_size) *
            sizeof(bfl_word))) == NULL)
    {
        return(NULL);
    }

    bfl->curpos = 0;
    bfl->len = array_size;

    // Set all as allocated
    sys$initmem((vms$pointer) bfl->bitarray, array_size * sizeof(bfl_word));

    // Now free the ones we have
    // FIXME: This is a terribly ineffecient way to do this
    for(i = 0; i < size; i++)
    {
        sys$bfl_free(bfl, i);
    }

    return(bfl);
}

void
sys$bfl_free(bfl_t bfl, vms$pointer val)
{
    vms$pointer         idx;

    idx = (val / BITS_PER_BFL_WORD);

    PANIC(idx >= bfl->len);
    bfl->bitarray[idx] |= ((bfl_word) 1) << (val % BITS_PER_BFL_WORD);

    return;
}

vms$pointer
sys$bfl_alloc(bfl_t bfl)
{
    unsigned int                found;
    unsigned int                i;
    unsigned int                pos;

    found = 0;

    for(i = bfl->curpos; i < bfl->len; i++)
    {
        if (bfl->bitarray[i] != 0)
        {
            found = 1;
            break;
        }
    }

    if (found == 0)
    {
        for(i = 0; i < bfl->curpos; i++)
        {
            if (bfl->bitarray[i] != 0)
            {
                found = 1;
                break;
            }
        }
    }

    if (found == 0)
    {
        return(-1);
    }

    pos = find_first_set(bfl->bitarray[i]);
    bfl->bitarray[i] &= ~(((bfl_word) 1) << pos);

    if (bfl->bitarray == 0)
    {
        bfl->curpos = (bfl->curpos + 1) % bfl->len;
    }

    return((i * BITS_PER_BFL_WORD) + pos);
}
