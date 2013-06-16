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

void
sys$parsing(char *line, char *command, char *argument, int length)
{
    char    *ptr;

    int     i;

    for(i = 0; i < length; argument[i++] = 0);
    PANIC((ptr = strstr(line, command)) == NULL,
            notice(SYSBOOT_F_PARAM "%s parameter not found", command));
	PANIC((ptr != line) && ((*(ptr - 1)) != ' '),
            notice(SYSBOOT_F_PARAM "%s parameter not found", command));

    i = 0; while((*ptr) != '=') { ptr++; PANIC(i >= length); }
    ptr++; while((*ptr) == ' ') { ptr++; PANIC(i >= length); }

    i = 0;
    while(((*ptr) != ' ') && ((*ptr != 0)))
    {
        argument[i++] = *ptr++;
        PANIC(i >= length);
    }

    ptr = argument;
    while(*ptr)
    {
        if (((*ptr) >= 'a') && (*ptr <= 'z'))
            (*ptr) = (*ptr) - ('a' - 'A');
        ptr++;
    }

    return;
}
