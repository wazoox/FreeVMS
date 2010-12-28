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

// Global variables
int                     vms$pd_initialized = 0;
int                     dbg$virtual_memory;
int                     dbg$sys_pagefault;
int                     dbg$vms_pagefault;

struct pd               freevms_pd;

void
freevms_main(void)
{
    char                        *command_line;

#   define                      ROOT_DEVICE_LENGTH 80
    char                        root_device[ROOT_DEVICE_LENGTH];
#   define                      CONSOLE_DEVICE_LENGTH 80
    char                        console_device[CONSOLE_DEVICE_LENGTH];

    L4_BootRec_t                *boot_record;

    L4_KernelInterfacePage_t    *kip;

    L4_ProcDesc_t               *main_proc_desc;

    L4_ThreadId_t               root_tid;
    L4_ThreadId_t               s0_tid;

    L4_Word_t                   api_flags;
    L4_Word_t                   boot_info;
    L4_Word_t                   i;
    L4_Word_t                   kernel_id;
    L4_Word_t                   kernel_interface;
    L4_Word_t                   num_boot_info_entries;
    L4_Word_t                   num_processors;
    L4_Word_t                   page_bits;
    L4_Word_t                   pagesize;

    struct vms$meminfo          mem_info;

    vms$pd_initialized = 0;

    notice("\n");
    notice(">>> FreeVMS %s (R)\n", FREEVMS_VERSION);
    notice("\n");

    kip = (L4_KernelInterfacePage_t *) L4_KernelInterface(&kernel_interface,
            &api_flags, &kernel_id);

    notice(SYSBOOT_I_SYSBOOT "leaving kernel privileges\n");
    notice(SYSBOOT_I_SYSBOOT "launching FreeVMS kernel with executive "
            "privileges\n");
    root_tid = L4_Myself();
    s0_tid = L4_GlobalId(kip->ThreadInfo.X.UserBase, 1);

    notice(SYSBOOT_I_SYSBOOT "booting main processor\n");

    for(page_bits = 0; !((1 << page_bits) & L4_PageSizeMask(kip)); page_bits++);
    pagesize = (((vms$pointer) 1) << page_bits);
    notice(SYSBOOT_I_SYSBOOT "computing page size: %d bytes\n",
            (int) pagesize);

    num_processors = L4_NumProcessors((void *) kip);

    switch(num_processors - 1)
    {
        case 0:
            break;

        case 1:
            notice(SYSBOOT_I_SYSBOOT "booting %d secondary processor\n",
                    (int) (num_processors - 1));
            break;

        default:
            notice(SYSBOOT_I_SYSBOOT "booting %d secondary processors\n",
                    (int) (num_processors - 1));
            break;
    }

    for(i = 0; i < num_processors; i++)
    {
        main_proc_desc = L4_ProcDesc((void *) kip, i);
        notice(SYSBOOT_I_SYSBOOT "CPU%d EXTFREQ=%d MHz, INTFREQ=%d MHz\n",
                (int) i, (int) (main_proc_desc->X.ExternalFreq / 1000),
                (int) (main_proc_desc->X.InternalFreq / 1000));
    }

    L4_Sigma0_GetPage(L4_nilthread, L4_Fpage(L4_BootInfo(kip), pagesize));

    boot_info = L4_BootInfo((void *) kip);
    num_boot_info_entries = L4_BootInfo_Entries((void *) boot_info);
    boot_record = L4_BootInfo_FirstEntry((void *) boot_info);

    for(i = 2; i < num_boot_info_entries; i++)
    {
        PANIC(L4_BootRec_Type(boot_record) != L4_BootInfo_SimpleExec);
        command_line = L4_SimpleExec_Cmdline(boot_record);

        if ((strstr(command_line, "vmskernel.sys") != NULL) && (i == 3))
        {
            break;
        }

        boot_record = L4_BootRec_Next(boot_record);
    }

    PANIC(L4_BootRec_Type(boot_record) != L4_BootInfo_SimpleExec);
    command_line = L4_SimpleExec_Cmdline(boot_record);
    notice(SYSBOOT_I_SYSBOOT "parsing command line: %s\n", command_line);
    sys$parsing(command_line, (char *) " root", root_device,
            ROOT_DEVICE_LENGTH);
    notice(SYSBOOT_I_SYSBOOT "selecting root device: %s\n", root_device);
    sys$parsing(command_line, (char *) " console", console_device,
            CONSOLE_DEVICE_LENGTH);
    notice(SYSBOOT_I_SYSBOOT "selecting console device: %s\n", console_device);

    dbg$virtual_memory = (strstr(command_line, " dbg$virtual_memory") != NULL)
            ? 1 : 0;
    dbg$sys_pagefault = (strstr(command_line, " dbg$sys_pagefault") != NULL)
            ? 1 : 0;
    dbg$vms_pagefault = (strstr(command_line, " dbg$vms_pagefault") != NULL)
            ? 1 : 0;

    // Starting virtual memory subsystem
    sys$mem_init(kip, &mem_info, pagesize);
    sys$bootstrap(&mem_info, pagesize);
    sys$objtable_init();
    sys$utcb_init(kip);
    sys$pd_init(&mem_info);
    sys$thread_init(kip);
    sys$populate_init_objects(&mem_info, pagesize);

    dev$init();
    names$init();

    sys$pager(kip, &mem_info, pagesize, root_device);
    sys$init(kip, &mem_info, pagesize, root_device);
    sys$loop();

    notice(">>> System halted\n");
    return;
}
