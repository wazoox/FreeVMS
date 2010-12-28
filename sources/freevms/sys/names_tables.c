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

// All devices are registered is a tree

#define SIZE_OF_NAMES_ALPHABET     256

static device_node      *devices;
static int              indirection[SIZE_OF_NAMES_ALPHABET];

void
names$init(void)
{
    unsigned int            i;

    notice(NSR_I_INIT "initializing names tree\n");

    for(i = 0; i < SIZE_OF_NAMES_ALPHABET; indirection[i++] = 0);

    devices = NULL;
    i = 0;

    // Characters : A-Z, 0-9 and '.'

    indirection['A'] = i++;
    indirection['B'] = i++;
    indirection['C'] = i++;
    indirection['D'] = i++;
    indirection['E'] = i++;
    indirection['F'] = i++;
    indirection['G'] = i++;
    indirection['H'] = i++;
    indirection['I'] = i++;
    indirection['J'] = i++;
    indirection['K'] = i++;
    indirection['L'] = i++;
    indirection['M'] = i++;
    indirection['N'] = i++;
    indirection['O'] = i++;
    indirection['P'] = i++;
    indirection['Q'] = i++;
    indirection['R'] = i++;
    indirection['S'] = i++;
    indirection['T'] = i++;
    indirection['U'] = i++;
    indirection['V'] = i++;
    indirection['W'] = i++;
    indirection['X'] = i++;
    indirection['Y'] = i++;
    indirection['Z'] = i++;

    indirection['0'] = i++;
    indirection['1'] = i++;
    indirection['2'] = i++;
    indirection['3'] = i++;
    indirection['4'] = i++;
    indirection['5'] = i++;
    indirection['6'] = i++;
    indirection['7'] = i++;
    indirection['8'] = i++;
    indirection['9'] = i++;

    indirection['.'] = i++;

    return;
}
