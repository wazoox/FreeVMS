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

struct hashtable *
hash_init(unsigned int size)
{
    struct hashtable        *tablestruct;

    unsigned int            counter;

    // Our hash function only works with power-of-2 bucket sizes for speed.
    PANIC((size & (size -1)) != 0);

    if ((tablestruct = (struct hashtable *)
            vms$alloc(sizeof(struct hashtable))) == NULL)
    {
        return(NULL);
    }

    if ((tablestruct->table = (struct hashentry **)
            vms$alloc(size * sizeof(struct hashentry *))) == NULL)
    {
        vms$free(tablestruct);
        return(NULL);
    }

    for(counter = 0; counter < size; counter++)
    {
        tablestruct->table[counter] = NULL;
    }

    tablestruct->size = size;
    tablestruct->spares = NULL;

    return(tablestruct);
}
