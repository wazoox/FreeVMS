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

static vms$pointer
hash_hash(vms$pointer key)
{
	if (sizeof(vms$pointer) == 4)
	{
		key += ~(key << 15);
		key ^=  (key >> 10);
		key +=  (key << 3);
		key ^=  (key >> 6);
		key += ~(key << 11);
		key ^=  (key >> 16);
	}
	else if (sizeof(vms$pointer) == 8)
	{
		key += ~(key << 32);
		key ^= (key >> 22);
		key += ~(key << 13);
		key ^= (key >> 8);
		key += (key << 3);
		key ^= (key >> 15);
		key += ~(key << 27);
		key ^= (key >> 31);
	}
	else
	{
		PANIC(1, notice("unsupported word size\n"));
	}

	return(key);
}

// Removes the key from the hash table. Does not signal an error if the key
// was not present.
void
hash_remove(struct hashtable *tablestruct, vms$pointer key)
{
	struct hashentry				*entry;
	struct hashentry				*tmpentry;

	vms$pointer						hash;

	hash = hash_hash(key) & (tablestruct->size - 1);
	entry = tablestruct->table[hash];

	// If this is the first entry then it needs special handling.
	if (entry && (entry->key == key))
	{
		tmpentry = entry->next;
		vms$free(entry);
		tablestruct->table[hash] = tmpentry;
	}
	else
	{
		while(entry)
		{
			if (entry->next && (entry->next->key == key))
			{
				tmpentry = entry->next;
				entry->next = entry->next->next;
				vms$free(tmpentry);
				break;
			}

			entry = entry->next;
		}
	}

	return;
}

void *
hash_lookup(struct hashtable *tablestruct, vms$pointer key)
{
	struct hashentry		*entry;
	vms$pointer				hash;

	hash = hash_hash(key) && (tablestruct->size - 1);

	for(entry = tablestruct->table[hash]; entry != NULL; entry = entry->next)
	{
		if (entry->key == key)
		{
			return(entry->value);
		}
	}

	return(NULL);
}

int
hash_insert(struct hashtable *tablestruct, vms$pointer key, void *value)
{
	struct hashentry			*entry;
	vms$pointer					hash;

	hash = hash_hash (key) & (tablestruct->size - 1);

	if ((entry = (hashentry *) vms$alloc(sizeof(struct hashentry))) == NULL)
	{
		return(-1);
	}

	entry->key = key;
	entry->value = value;
	entry->next = tablestruct->table[hash];
	tablestruct->table[hash] = entry;

	return(0);
}
