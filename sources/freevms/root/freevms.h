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

#include "lib.h"
#include "l4io.h"

#include "l4/kip.h"
#include "l4/sigma0.h"
#include "l4/thread.h"
#include "l4/bootinfo.h"

#include "information.h"
#include "system.h"
#include "levels.h"

#define NULL ((void *) 0)

#define FREEVMS_VERSION "0.0.1"

#define notice(...) printf(__VA_ARGS__)

#define PANIC(a)  if (a) { \
        notice("Panic at %s(%d)\n", __FUNCTION__, __LINE__); \
        notice("Have a nice day !\n"); \
        while(1); } while(0)

void parsing(char *line, char *command, char *argument, int length);
