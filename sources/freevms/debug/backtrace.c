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
#include "./amd64.h"

#define get_registers(arch)			CONCAT(registers_, arch)
#define CONCAT(a, b)				XCAT(a, b)
#define XCAT(a, b)					a##b()

void backtrace(void)
{
	vms$pointer			sp;

	sp = get_registers(ARCH);

	/*
	void show_backtrace (void)
	{
		char name[256];
		unw_cursor_t cursor; unw_context_t uc;
		unw_word_t ip, sp, offp;

		unw_getcontext (&uc);
		unw_init_local (&cursor, &uc);

		while (unw_step(&cursor) > 0)
		{
			char file[256];
			int line = 0;

			name[0] = '\0';
			unw_get_proc_name (&cursor, name, 256, &offp);
			unw_get_reg (&cursor, UNW_REG_IP, &ip);
			unw_get_reg (&cursor, UNW_REG_SP, &sp);

			printf ("%s ip = %lx, sp = %lx\n", name, (long) ip, (long) sp);
		}
	}
	*/

	for(;;);
	// 0000000001037830
	// 0000000001037820
    return;
}

