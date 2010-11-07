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

// Find the first application and try to execute if. This application shall
// bootstrap the rest of the system.

void
sys$init(L4_KernelInterfacePage_t *kip, struct vms$meminfo *meminfo,
        vms$pointer pagesize, char *root_device)
{
    extern struct pd                freevms_pd;

    struct initial_obj              *init_obj;
    struct initial_obj              *obj;
    struct pd                       *pd;
    struct thread                   *thread;

    struct memsection               *clist_section;
    struct memsection               *heap;
    struct memsection               *heap_phys;
    struct memsection               *stack;

    vms$pointer                     *init_vars;
    vms$pointer                     *user_stack;

    cap_t                           *clist;

    unsigned int                    i;
    int                             j;
    int                             r;

    notice(SYSBOOT_I_SYSBOOT "spawning VMS$INIT.SYS\n");

    // Find init (vms$init.sys)
    for(obj = meminfo->objects, i = 0; i < meminfo->num_objects; i++, obj++)
    {
        if (strstr(obj->name, "vms$init.sys") != NULL)
        {
            break;
        }
    }

    PANIC(i == meminfo->num_objects, notice("Couldn't find VMS$INIT.SYS!\n"));
    init_obj = obj;

    notice(SYSBOOT_I_SYSBOOT "creating VMS$INIT.SYS process descriptor\n");
    pd = jobctl$pd_create(&freevms_pd, 0);
    thread = jobctl$pd_create_thread(pd, -1);

    notice(SYSBOOT_I_SYSBOOT "creating VMS$INIT.SYS stack\n");
    notice(SYSBOOT_I_SYSBOOT "creating VMS$INIT.SYS heap\n");
    return;
}
