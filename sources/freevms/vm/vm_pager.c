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

static vms$pointer
vms$swap_size(vms$pointer actual_swap_size, vms$pointer requested_swap_size)
{
	vms$pointer					new_swap_size;

    vms$string_initializer(message, 80);

	void						*arg$print;

	rtl$strcpy(&message, PAGER_I_SWPSIZE "setting swapper size to %lu bytes");
	new_swap_size = actual_swap_size;

	if (actual_swap_size <= requested_swap_size)
	{
		new_swap_size = requested_swap_size;
	}
	else
	{
		FIXME;
	}

	arg$print = &new_swap_size;
	rtl$print(&message, &arg$print);

	return(new_swap_size);
}

int
main(int argc, char **argv)
{
	vms$pointer				new_swap_size;
	vms$pointer				swapper_base;
	vms$pointer				swap_size;

    vms$string_initializer(message, 80);

	void					*arg$print;

    rtl$strcpy(&message, RUN_S_STARTED "PAGER.SYS process started");
    rtl$print(&message, NULL);

	swapper_base = (vms$pointer) argv[0];
	arg$print = &swapper_base;

	rtl$strcpy(&message, PAGER_I_SWPADDR
			"setting swapper base address at $%016lX");
	rtl$print(&message, &arg$print);

	swap_size = 0;
	new_swap_size = vms$swap_size(swap_size, 0);
	swap_size = new_swap_size;

    return(0);
}
