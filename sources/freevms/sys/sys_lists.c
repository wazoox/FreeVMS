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
dl_list_create_front(struct double_list *dl, int payload)
{
    struct ll           *n;

    n = (struct ll *) sys$alloc(sizeof(struct ll) + payload);

    if (n != NULL)
    {
        n->prev = (struct ll *) dl;
        dl->head->prev = n;
        n->next = dl->head;
        dl->head = n;

        return(&n->data);
    }

    return(NULL);
}

void *
dl_list_create_back(struct double_list *dl, int payload)
{
    struct ll       *n;

    n = (struct ll *) sys$alloc(sizeof(struct ll) + payload);

    if (n != NULL)
    {
        n->next = (struct ll *) dl;
        dl->tail->next = n;
        n->prev = dl->tail;
        dl->tail = n;

        return(&n->data);
    }

    return(NULL);
}

void
dl_list_init(struct double_list *dl)
{
    dl->head = (struct ll *) dl;
    dl->tail = (struct ll *) dl;

    return;
}

void
dl_list_clear(struct double_list *list)
{
    struct ll       *iter;
    struct ll       *next;

    iter = list->head;

    while(iter)
    {
        next = iter->next;
        sys$free(iter->data);
        sys$free(iter);
    }

    list->head = 0;
    list->tail = 0;

    return;
}

struct ll *
ll_new(void)
{
    struct ll   *ll;

    if ((ll = (struct ll *) sys$alloc(sizeof(struct ll))) != NULL)
    {
        ll->next = ll;
        ll->prev = ll;
        ll->data = NULL;
    }

    return(ll);
}

struct ll *
ll_delete(struct ll *ll)
{
    struct ll       *next;

    next = ll->next;
    ll->next->prev = ll->prev;
    ll->prev->next = ll->next;
    sys$free(ll);

    return(next);
}

struct ll *
ll_insert_before(struct ll *ll, void *data)
{
    struct ll       *n;

    if ((n = (struct ll *) sys$alloc(sizeof(struct ll))) != NULL)
    {
        n->next = ll;
        ll->prev->next = n;
        n->prev = ll->prev;
        ll->prev = n;
        n->data = data;
    }

    return(n);
}
