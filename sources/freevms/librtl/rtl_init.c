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

/*
 * argv[0] = roottask_tid
 * argv[1] = parent_tid;
 */

L4_ThreadId_t                   roottask_tid;
L4_ThreadId_t                   parent_tid;

static struct vms$string        message;

/*
 * Start a thread with a pre-existing pager
 */

void
__bootstrap(int argc, char **argv)
{
    int                 l;
    int                 v;

    unsigned char       *p;

    roottask_tid.raw = (vms$pointer) argv[0];
    parent_tid.raw = (vms$pointer) argv[1];

    p = (unsigned char *) argv[2];

    if (p != NULL)
    {
        l = 0;

        while(*p)
        {
            p++;
            l++;
        }
    }
    else
    {
        l = 0;
    }

    message.c = (unsigned char *) argv[2];
    message.length = l;
    message.length_trim = l;

    argc -= 3;
    argv += 3;

    v = main(argc, argv);
    exit(v);
}

/*
 * Start a process
 * - create an address space
 * - launch pager
 * - launch executable as child thread
 */

void
__bootstrap_process(int argc, char **argv)
{
    int                 l;
    int                 v;

    unsigned char       *p;

    roottask_tid.raw = (vms$pointer) argv[0];
    parent_tid.raw = (vms$pointer) argv[1];

    p = (unsigned char *) argv[2];

    if (p != NULL)
    {
        l = 0;

        while(*p)
        {
            p++;
            l++;
        }
    }
    else
    {
        l = 0;
    }

    message.c = (unsigned char *) argv[2];
    message.length = l;
    message.length_trim = l;

    argc -= 3;
    argv += 3;

    v = main(argc, argv);
    exit(v);
}

void
exit(int v)
{
    L4_Msg_t            msg;

    // Send IPC to parent (value returned by task)

    L4_Clear(&msg);
    L4_Append(&msg, (L4_Word_t) v);
    L4_Set_Label(&msg, SYSCALL$EXIT_VALUE);
    L4_Load(&msg);
    L4_Call(parent_tid);

    // Print message

    if (message.length_trim != 0)
    {
        rtl$print(&message, NULL);
    }

    // Send IPC to roottask to stop thread.

    L4_Clear(&msg);
    L4_Append(&msg, L4_Myself().raw);
    L4_Set_Label(&msg, SYSCALL$KILL_THREAD);
    L4_Load(&msg);
    L4_Call(roottask_tid);

    while(1);
}
