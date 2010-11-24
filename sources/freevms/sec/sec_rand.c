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

static int mt[624];
static int index = 0;
static int initialized = 0;

// Mersenne twister random generator

void
rand$init(int seed)
{
    int             i;

    mt[0] = seed;

    for(i = 1; i < 624; i++)
    {
        mt[i] = 0xffffffff & (1812433253U * (mt[i - 1] ^ (mt[i - 1] >> 30)));
    }

    initialized = 1;
    return;
}

static void
rand$generate_number()
{
    int             i;
    int             y;

    if (initialized == 0)
    {
        rand$init(1);
    }

    for(i = 0; i < 624; i++)
    {
        y = (mt[i] & 0x80000000) + (mt[(i + 1) % 624] & 0x7fffffff);
        mt[i] = mt[(i + 397) % 624] ^ (y >> 1);

        if ((y % 2) == 1)
        {
            mt[i] = mt[i] ^ 2567483615U;
        }
    }

    return;
}

vms$pointer
rand$extract_number()
{
    int             y;

    if (index == 0)
    {
        rand$generate_number();
    }

    y = mt[index];
    y = y ^ (y >> 11);
    y = y ^ ((y << 7) & 2636928640U);
    y = y ^ ((y << 15) & 4022730752U);
    y = y ^ (y >> 18);
    index = (index + 1) % 624;

    return((vms$pointer) y);
}

vms$pointer
rand$extract_number(unsigned int size)
{
    unsigned int    i;

    unsigned char   *c;

    vms$pointer     r;
    vms$pointer     t;

    c = (unsigned char *) &r;

    for(i = 0; i < size / 4; i++)
    {
        t = rand$extract_number();

        (*c) = t & 0xff; c++; t >>= 2;
        (*c) = t & 0xff; c++; t >>= 2;
        (*c) = t & 0xff; c++; t >>= 2;
        (*c) = t & 0xff; c++; t >>= 2;
    }

    return(r);
}
