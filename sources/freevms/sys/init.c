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

int vms$pd_initialized;

int
main(void)
{
    char                        *command_line;

#   define                      ROOT_DEVICE_LENGTH 80
    char                        root_device[ROOT_DEVICE_LENGTH];

    L4_BootRec_t                *boot_record;

    L4_Clock_t                  time;

    L4_Fpage_t                  kip_area;
    L4_Fpage_t                  threads_stack;

    L4_KernelInterfacePage_t    *kip;

    L4_ProcDesc_t               *main_proc_desc;

    L4_ThreadId_t               jobctl_tid;
    L4_ThreadId_t               name_tid;
    L4_ThreadId_t               pager_tid;
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
    L4_Word_t                   page_size;
    L4_Word_t                   pager_utcb;
    L4_Word_t                   running_system;
    L4_Word_t                   utcb_size;

    struct vms$meminfo          mem_info;

    dbg$sigma0(0);
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
    notice(RUN_S_PROC_ID "identification of created process is %08X\n",
            (unsigned int) L4_ThreadNo(root_tid));
    s0_tid = L4_GlobalId(kip->ThreadInfo.X.UserBase, 1);

    notice(SYSBOOT_I_SYSBOOT "booting main processor\n");

    for(page_bits = 0; !((1 << page_bits) & L4_PageSizeMask(kip)); page_bits++);
    page_size = (1 << page_bits);
    notice(SYSBOOT_I_SYSBOOT "computing page size: %d bytes\n",
            (int) page_size);

    // Map kip

    kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2(kip));
    utcb_size = L4_UtcbSize(kip);
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

    L4_Sigma0_GetPage(L4_nilthread, L4_Fpage(L4_BootInfo(kip), page_size));

    boot_info = L4_BootInfo((void *) kip);
    num_boot_info_entries = L4_BootInfo_Entries((void *) boot_info);
    boot_record = L4_BootInfo_FirstEntry((void *) boot_info);

    for(i = 2; i < num_boot_info_entries; i++)
    {
        PANIC(L4_BootRec_Type(boot_record) != L4_BootInfo_SimpleExec);
        command_line = L4_SimpleExec_Cmdline(boot_record);

        if (strstr(command_line, "vmskernel.sys") != NULL)
        {
            break;
        }

        boot_record = L4_BootRec_Next(boot_record);
    }

    PANIC(L4_BootRec_Type(boot_record) != L4_BootInfo_SimpleExec);
    command_line = L4_SimpleExec_Cmdline(boot_record);
    notice(SYSBOOT_I_SYSBOOT "parsing command line: %s\n", command_line);
    parsing(command_line, (char *) " root", root_device, ROOT_DEVICE_LENGTH);
    notice(SYSBOOT_I_SYSBOOT "selecting root device: %s\n", root_device);

    threads_stack = L4_Sigma0_GetPage(s0_tid,
            L4_FpageLog2(THREAD_STACK_BASE, 16));
    vms$initmem(THREAD_STACK_BASE, 1UL << 16);
    
    // Starting virtual memory subsystem
    vms$init(kip, &mem_info, (unsigned int) page_size);
    vms$bootstrap(&mem_info, (unsigned int) page_size);

    // A thread must have a pager. This pager requires a
    // specific thread to handle pagefault protocol.
    // This thread is created by hand because there is no memory management
    // to manage thread.

    pager_utcb = L4_MyLocalId().raw;
    pager_utcb = (pager_utcb & (~(utcb_size - 1))) + utcb_size;
    pager_tid = L4_GlobalId(L4_ThreadNo(root_tid) + 1, 1);
    L4_ThreadControl(pager_tid, root_tid, root_tid, root_tid,
            (void *) pager_utcb);
    PANIC(L4_ErrorCode(),
            notice("ERR=%s\n", L4_ErrorCode_String(L4_ErrorCode())));

    L4_Start(pager_tid, (L4_Word_t) ((THREAD_STACK_BASE + 4096)
            - (2 * L4_SIZEOFWORD)), (L4_Word_t) vms$pager);

    PANIC(L4_ErrorCode(),
            notice("ERR=%s\n", L4_ErrorCode_String(L4_ErrorCode())));
    L4_Call(pager_tid);
    notice(RUN_S_PROC_ID "identification of created process is %08X\n",
            (unsigned int) L4_ThreadNo(pager_tid));

    // Starting job controller
    jobctl_tid = L4_GlobalId(L4_ThreadNo(root_tid) + 2, 1);
    notice(SYSBOOT_I_SYSBOOT "spawning job controller\n");
    notice(RUN_S_PROC_ID "identification of created process is %08X\n",
            (unsigned int) L4_ThreadNo(jobctl_tid));
    // tid = jobctl$create(JOBCTL$THREAD/PROC)
    // jobctl$delete(tid)
    // jobctl$schedule()

    name_tid = L4_GlobalId(L4_ThreadNo(root_tid) + 3, 1);
    notice(SYSBOOT_I_SYSBOOT "spawning name service\n");
    notice(RUN_S_PROC_ID "identification of created process is %08X\n",
            (unsigned int) L4_ThreadNo(name_tid));

    //passer le thread_id aux serveurs créés.

    // Adding to name server:
    // - SYS$KERNEL
    // - SYS$PAGER
    // - SYS$NMSRV
    // - SYS$JOBCTL

    switch(num_boot_info_entries - 3)
    {
        case 0:
            break;

        case 1:
            notice(SYSBOOT_I_SYSBOOT "trying to load %d driver\n",
                    (int) (num_boot_info_entries - 3));
            break;

        default:
            notice(SYSBOOT_I_SYSBOOT "trying to load %d drivers\n",
                    (int) (num_boot_info_entries - 3));
            break;
    }

    for(; i < num_boot_info_entries; i++)
    {
        boot_record = L4_BootRec_Next(boot_record);
        PANIC(L4_BootRec_Type(boot_record) != L4_BootInfo_Module);
        notice(SYSBOOT_I_SYSBOOT "loading %s\n",
                L4_Module_Cmdline(boot_record));
        /*
        elf_loader();
        notice(SYSBOOT_I_SYSBOOT "address %016lX:%016lX\n",
                (long unsigned int) L4_Module_Start(boot_record),
                (long unsigned int) L4_Module_Size(boot_record));
                */
    }

    notice(SYSBOOT_I_SYSBOOT "freeing kernel memory\n");

    notice(SYSBOOT_I_SYSBOOT "trying to mount root filesystem\n");
    notice(MOUNT_I_MOUNTED "SYS$ROOT mounted on _%s:\n", root_device);
    notice(SYSBOOT_I_SYSBOOT "trying to read SYS$ROOT:[VMS$COMMON.SYSMGR]"
            "VMSKERNEL.CNF\n");
    notice(SYSBOOT_I_SYSBOOT "trying to read SYS$ROOT:[VMS$COMMON.SYSMGR]"
            "VMSSWAPPER.CNF\n");

    /*
     * Chargement des modules
     * start_task()
     */

    time = L4_SystemClock();
    notice(STDRV_I_STARTUP "FreeVMS startup begun at %d, %d (UTC)\n",
            time.X.low, time.X.high);
    notice("\n");

    notice("The FreeVMS system is now executing the site-specific "
            "startup commands.\n");
    notice("\n");

    notice(SYSBOOT_I_SYSBOOT "executing SYS$ROOT:[VMS$COMMON.SYSMGR]"
            "SYSTARTUP_VMS.COM\n");
/*
    notice(SYSBOOT_I_SYSBOOT "creating logical name "
            "SYS$SYSTEM=SYS$ROOT:[VMS$COMMON]\n");
    notice(SYSBOOT_I_SYSBOOT "creating logical name "
            "SYS$MANAGER=SYS$SYSTEM:[.SYSMGR]\n");

    notice(SYSBOOT_I_SYSBOOT "SYS$SYSTEM:PAGEFILE.SYS\n");
    notice(SYSBOOT_I_SYSBOOT "SYS$SYSTEM:PAGEFILE.SYS\n");
    notice(SYSBOOT_I_SYSBOOT "SYS$SYSTEM:SWAPFILE.SYS\n");
    */


    /*
    notice("%%DCL-S-SPAWNED, process SYSTEM_1 spawned\n");
    */

    L4_KDB_Enter("Debug");
    running_system = 1;

    while(running_system == 1);
    {
    }

    notice(">>> System halted\n");

    return(0);
}
