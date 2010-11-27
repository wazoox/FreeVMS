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

rfl_t
sys$rfl_new(void)
{
    return(ll_new());
}

int
sys$rfl_free(rfl_t rfl, vms$pointer val)
{
    return(sys$rfl_insert_range(rfl, val, val));
}

int
sys$rfl_insert_range(rfl_t rfl, vms$pointer from, vms$pointer to)
{
    int             added;

    struct ll       *temp;

    struct range    *range;
    struct range    *next_range;
    struct range    *new_range;

    added = 0;

    if (from > to)
    {
        // Can't have a range like this
        return(E_RFL_INV_RANGE);
    }

    // See we can append to existing
    for(temp = rfl->next; temp != rfl; temp = temp->next)
    {
        range = (struct range *) temp->data;

        // Check that the new range doesn't overlap with this existing range
        if (((from >= range->from) && (from <= range->to)) ||
                ((to >= range->from) && (to <= range->to)))
        {
            return(E_RFL_OVERLAP);
        }

        if (range->from == (to + 1))
        {
            // In this case can add to the start of this range
            range->from = from;
            added = 1;
            break;
        }

        if (range->to == (from - 1))
        {
            // In this case we can add to the end of this range
            next_range = (struct range *) temp->next->data;
            range->to = to;

            if ((next_range != NULL) &&
                    ((range->to + 1) == next_range->from))
            {
                // Merge with next range
                range->to = next_range->to;
                // Now delete the next range
                sys$free(next_range);
                ll_delete(temp->next);
            }

            added = 1;
            break;
        }

        if (range->from > to)
        {
            // In this case we need to insert it before the existing range, so
            // we break now and let the logic at the end add it.
            break;
        }
    }

    if (added == 0)
    {
        if ((new_range = (struct range *) sys$alloc(sizeof(struct range)))
                == NULL)
        {
            return(E_RFL_NOMEM);
        }

        ll_insert_before(temp, new_range);
        new_range->from = from;
        new_range->to = to;
    }

    return(RFL_SUCCESS);
}

vms$pointer
sys$rfl_alloc(rfl_t rfl)
{
    struct range            *range;

    vms$pointer             retval;

    if (rfl->next == rfl)
    {
        // This is the no items left case
        return(-1);
    }

    range = (struct range *) rfl->next->data;
    retval = range->from;

    if (range->from == range->to)
    {
        // None left in this range now, free resources
        sys$free(range);
        ll_delete(rfl->next);
    }
    else
    {
        // There are more left in the range, just increment the from value
        range->from++;
    }

    return(retval);
}
