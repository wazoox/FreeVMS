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

    while(1)
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
        }

        if (current->isleaf)
        {
            break;
        }

        current = current->child[hi];
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

/*
 * Insert kpromoted with child ppromoted at index hi in page current,
 * ppromoted is kpromoted's right child.
 */

static int
BTInsertPage(BTPage * const current, BTKeyCount const hi,
        BTKey const kpromoted, BTPage * const ppromoted)
{
    BTKeyCount          i;

    for(i = current->count - 1; i >= max(hi, 0); i--)
    {
        current->key[i + 1] = current->key[i];
        current->child[i + 2] = current->child[i + 1];
    }

    if (hi < 0)
    {
        current->key[0] = BTGetObjKey((GBTObject)(current->child[0]));
        current->child[1] = current->child[0];
        current->child[0] = ppromoted;
    }
    else
    {
        current->key[hi] = kpromoted;
        current->child[hi+1] = ppromoted;
    }

    current->count++;

    return(BT_OK);
}

/*
 * Insert obj in the b+tree with b-tree index pointed by current,
 * return:
 *   BT_PROMOTION : if current splitted to two pages, 
 *       ppromoted will point to the right child of key kpromoted
 *       (while current will hold the left child)
 *   BT_OK : if insert succesfull
 *   BT_DUPLICATE : if there has been another object with the same key in tree 
 *   BT_ALLOC_fail : if there's not enough memory to allocate a new page
 *       (the upper level index can be inconsistent with the lower level)
 */

static int
BTInsert(PagePool *pool, BTPage * const current, GBTObject const obj,
        BTPage ** const ppromoted, BTKey * const kpromoted,
        GBTObject *ngb)
{
    BTKey               key;
    BTKey               kmed;

    BTKeyCount          hi;
    BTKeyCount          i;
    BTKeyCount          j;
    BTKeyCount          lo;
    BTKeyCount          mid;

    BTPage              *newpage;

    int                 in_old;
    int                 retval;

    if (current == NULL)
    {
        (*kpromoted) = BTGetObjKey(obj);
        (*ppromoted) = NULL;

        return(BT_PROMOTION);
    }

    key = BTGetObjKey(obj);
    lo = 0;
    hi = current->count;

    while(lo < hi)
    {
        mid = (lo + hi) / 2;

        if (BTKeyEQ(current->key[mid], key))
        {
            return(BT_DUPLICATE);
        }
        else if (BTKeyGT(current->key[mid], key))
        {
            hi = mid;
        }
        else
        {
            lo = mid + 1;
        }
    }

#   ifdef BT_HAVE_INTERVALS
    if ((hi < current->count) && (BTObjMatch(obj,current->key[hi])))
    {
        return(BT_OVERLAP);
    }
#   endif

    if (current->isleaf)
    {
#       ifdef BT_HAVE_INTERVALS
        if (BTObjMatch((GBTObject)(current->child[hi]), key) ||
                (!hi && BTOverlaps(obj,(GBTObject)(current->child[0]))))
        {
            return(BT_OVERLAP);
        }
#       endif

        (*ppromoted) = (BTPage *) obj;
        (*kpromoted) = BTGetObjKey(obj);
        retval = BT_PROMOTION;
        (*ngb) = (GBTObject)(current->child[hi]);

        if (!hi && BTKeyLT(key,BTGetObjKey((GBTObject)(current->child[0]))))
        {
            hi = -1;
        }
    }
    else
    {
        retval = BTInsert(pool, current->child[hi], obj, ppromoted,
                kpromoted, ngb);
    }

    if (retval == BT_PROMOTION)
    {
        if (current->count < BT_MAXKEY)
        {
            return(BTInsertPage(current, hi, *kpromoted, *ppromoted));
        }

        // split the page BT_MINKEY keys on left (org) and
        // BT_MAXKEY-BT_MINKEY on right

        if ((newpage = (BTPage *) EXPORT(AllocPage)(pool)) == NULL)
        {
            return(BT_ALLOC_fail);
        }

        newpage->isleaf = current->isleaf;
        newpage->count = BT_MAXKEY - BT_MINKEY;
        current->count = BT_MINKEY;

        if ((in_old = (hi < current->count)))
        {
            // insert promoted key in the left (original) page
            current->count--;
            kmed = current->key[current->count];
            current->key[current->count] = 0;
        }
        else
        {
            // insert promoted key in the right (new) page
            newpage->count--;

            if (hi == current->count)
            {
                kmed = *kpromoted;
            }
            else
            {
                kmed = current->key[current->count];
                current->key[current->count] = 0;
            }
        }

        newpage->child[0] = current->child[current->count+1];
        current->child[current->count+1] = NULL;

        for(j = current->count + 1, i = 0; i < newpage->count; i++, j++)
        {
            newpage->key[i] = current->key[j];
            newpage->child[i + 1] = current->child[j + 1];
            current->key[j] = 0;
            current->child[j + 1] = NULL;
        }

        for (; i < BT_MAXKEY; i++)
        {
            newpage->key[i] = 0;
            newpage->child[i + 1] = NULL;
        }

        if (in_old)
        {
            retval = BTInsertPage(current, hi, *kpromoted, *ppromoted);
        }
        else
        {
            hi -= current->count + 1;
            retval = BTInsertPage(newpage, hi, *kpromoted, *ppromoted);
        }

        (*kpromoted) = kmed;
        (*ppromoted) = newpage;

        return(BT_PROMOTION);
    }

    return(retval);
}

int
EXPORT(BTIns)(GBTree const btree, GBTObject const obj, GBTObject *ngb)
{
    BTKey               kpromoted;

    BTPage              *newroot;
    BTPage              *ppromoted;

    int                 i;
    int                 retval;

    if (btree == NULL)
    {
        // No tree
        return(BT_INVALID);
    }

    if (btree->root == NULL)
    {
        // Empty tree
        newroot = (BTPage *) EXPORT(AllocPage)(btree->pool);

        if (newroot == NULL)
        {
            return(BT_ALLOC_fail);
        }

        for(i = 0; i < BT_MAXKEY; i++)
        {
            newroot->child[i] = NULL;
            newroot->key[i] = 0;
        }

        // Numbers of children is key+1, so ensure the last child is initialised
        newroot->child[i] = NULL;
        newroot->count = 0;
        newroot->isleaf = 1;
        newroot->child[0] = (BTPage *) obj;
        btree->root = newroot;
        btree->depth = 1;
        (*ngb) = NULL;

        return(BT_OK);
    }

    if ((retval = BTInsert(btree->pool, btree->root, obj,
            &ppromoted, &kpromoted, ngb)) == BT_PROMOTION)
    {
        newroot = (BTPage *) EXPORT(AllocPage)(btree->pool);

        if (newroot == NULL)
        {
            return(BT_ALLOC_fail);
        }

        for(i = 0; i < BT_MAXKEY; i++)
        {
            newroot->child[i] = NULL;
            newroot->key[i] = 0;
        }

        newroot->child[i] = NULL;
        newroot->count = 1;
        newroot->isleaf = 0;
        newroot->child[0] = btree->root;
        newroot->child[1] = ppromoted;
        newroot->key[0] = kpromoted;
        btree->root = newroot;
        btree->depth++;
        retval = BT_OK;
    }

    return(0);
}

