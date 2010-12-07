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

#include "freevms/pager.h"

int
main(int argc, char **argv)
{
	void			*arg[16];

    vms$string_initializer(message, 80);
    rtl$strcpy(&message, RUN_S_STARTED "PAGER.SYS process started");
    rtl$print(&message, NULL);

	/* Instructions required to print arguments
	arg[0] = &argc;
	rtl$strcpy(&message, "%d");
	rtl$print(&message, arg);

	arg[0] = &(argv[0]);
	rtl$strcpy(&message, "%d");
	rtl$print(&message, arg);
	*/

    for(;;);
}
