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

int
EXPORT(BTSearch)(GBTree const btree, BTKey const key, GBTObject *obj)
{
    BTKeyCount      lo;
    BTKeyCount      hi;
    BTKeyCount      mid;

    BTPage          *current;

    if (!btree)
    {
        return(BT_INVALID);
    }

    if (!(current=btree->root))
    {
        return(BT_NOT_FOUND);
    }

    for(;;)
    {
        lo = 0;
        hi = current->count;

        while(lo < hi)
        {
            mid = (lo + hi) / 2;

            if (BTKeyEQ(current->key[mid], key))
            {
                hi = mid+1;
                break;
            }
            else if (BTKeyGT(current->key[mid], key))
            {
                hi = mid;
            }
            else
            {
                lo = mid + 1;
            }

            if (current->isleaf)
            {
                break;
            }

            current = current->child[hi];
        }
    }

    (*obj) = (GBTObject)(current->child[hi]);

    if (BTObjMatch(*obj, key))
    {
        return(BT_OK);
    }
    else
    {
        return(BT_NOT_FOUND);
    }
}

