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

#include "freevms.h"

static L4_KernelInterfacePage_t *kip;

int
main(void)
{
    char                        *CommandLine;
    char                        *ptr;

#   define                      RootDeviceLength 80
    char                        RootDevice[RootDeviceLength];

    L4_BootRec_t                *BootRecord;

    L4_Clock_t                  time;

    L4_Fpage_t                  kip_area;
    L4_Fpage_t                  utcb_area;

    L4_ProcDesc_t               *MainProcDesc;

    L4_ThreadId_t               roottid;
    L4_ThreadId_t               s0tid;

    L4_Word_t                   page_bits;
    L4_Word_t                   page_size;
    L4_Word_t                   utcb_size;

    L4_Word_t                   ApiFlags;
    L4_Word_t                   i;
    L4_Word_t                   j;
    L4_Word_t                   KernelId;
    L4_Word_t                   KernelInterface;
    L4_Word_t                   NumBootInfoEntries;
    L4_Word_t                   NumProcessors;
    L4_Word_t                   RunningSystem;

    void                        *BootInfo;

    notice("\n");
    notice(">>> FreeVMS %s (TM)\n", FREEVMS_VERSION);
    notice("\n");

    notice(SYSBOOT_I_SYSBOOT "booting main processor\n");

    kip = (L4_KernelInterfacePage_t *) L4_KernelInterface(&KernelInterface,
            &ApiFlags, &KernelId);

    for(page_bits = 0; !((1 << page_bits) & L4_PageSizeMask(kip)); page_bits++);
    page_size = (1 << page_bits);
    notice(SYSBOOT_I_SYSBOOT "computing page size: %d bytes\n",
            (int) page_size);

    roottid = L4_Myself();
    notice(SYSBOOT_I_SYSBOOT "launching kernel\n");
    notice(RUN_S_PROC_ID "identification of created process is %08X\n",
            (unsigned int) roottid.global.raw);
    s0tid = L4_GlobalId(kip->ThreadInfo.X.UserBase, 1);

    NumProcessors = L4_NumProcessors((void *) kip);

    switch(NumProcessors - 1)
    {
        case 0:
            break;

        case 1:
            notice(SYSBOOT_I_SYSBOOT "booting %d secondary processor\n",
                    (int) (NumProcessors - 1));
            break;

        default:
            notice(SYSBOOT_I_SYSBOOT "booting %d secondary processors\n",
                    (int) (NumProcessors - 1));
            break;
    }

    for(i = 0; i < NumProcessors; i++)
    {
        MainProcDesc = L4_ProcDesc((void *) kip, i);
        notice(SYSBOOT_I_SYSBOOT "CPU%d EXTFREQ=%d MHz, INTFREQ=%d MHz\n",
                (int) i, (int) (MainProcDesc->X.ExternalFreq / 1000),
                (int) (MainProcDesc->X.InternalFreq / 1000));
    }

    L4_Sigma0_GetPage(L4_nilthread, L4_Fpage(L4_BootInfo(kip),
            ((sizeof(BootInfo) / page_size) + 1) * page_size));

    BootInfo = (void *) L4_BootInfo((void *) kip);
    NumBootInfoEntries = L4_BootInfo_Entries(BootInfo);
    BootRecord = L4_BootInfo_FirstEntry(BootInfo);

    for(i = 2; i < NumBootInfoEntries; i++)
    {
        PANIC(L4_BootRec_Type(BootRecord) != L4_BootInfo_SimpleExec);
        CommandLine = L4_SimpleExec_Cmdline(BootRecord);

        if (strstr(CommandLine, "vmskernel.sys") != NULL)
        {
            break;
        }

        BootRecord = L4_BootRec_Next(BootRecord);
    }

    PANIC(L4_BootRec_Type(BootRecord) != L4_BootInfo_SimpleExec);

    CommandLine = L4_SimpleExec_Cmdline(BootRecord);
    notice(SYSBOOT_I_SYSBOOT "parsing command line: %s\n", CommandLine);
    parsing(CommandLine, " root", RootDevice, RootDeviceLength);
    notice(SYSBOOT_I_SYSBOOT "selecting root device: %s\n", RootDevice);

    switch(NumBootInfoEntries - 3)
    {
        case 0:
            break;

        case 1:
            notice(SYSBOOT_I_SYSBOOT "trying to load %d driver\n",
                    (int) (NumBootInfoEntries - 3));
            break;

        default:
            notice(SYSBOOT_I_SYSBOOT "trying to load %d drivers\n",
                    (int) (NumBootInfoEntries - 3));
            break;
    }

    for(; i < NumBootInfoEntries; i++)
    {
        BootRecord = L4_BootRec_Next(BootRecord);
        PANIC(L4_BootRec_Type(BootRecord) != L4_BootInfo_Module);
        notice(SYSBOOT_I_SYSBOOT "loading %s\n", L4_Module_Cmdline(BootRecord));
        /*
        elf_loader();
        notice(SYSBOOT_I_SYSBOOT "address %016lX:%016lX\n",
                (long unsigned int) L4_Module_Start(BootRecord),
                (long unsigned int) L4_Module_Size(BootRecord));
                */
    }

    kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));
    //utcb_area = L4_FpageLog2

    time = L4_SystemClock();
    notice(STDRV_I_STARTUP "FreeVMS startup begun at %d, %d (UTC)\n",
            (L4_Word32_t) time.X.low, (L4_Word32_t) time.X.high);

    notice(SYSBOOT_I_SYSBOOT "trying to mount root filesystem\n");
    notice(MOUNT_I_MOUNTED "SYS$ROOT mounted on _%s:\n", RootDevice);
    notice(SYSBOOT_I_SYSBOOT "trying to read SYS$ROOT:[VMS$COMMON.SYSMGR]"
            "VMSKERNEL.CNF\n");
    notice(SYSBOOT_I_SYSBOOT "trying to read SYS$ROOT:[VMS$COMMON.SYSMGR]"
            "VMSSWAPPER.CNF\n");

    /*
     * Chargement des modules
     * start_task()
     */

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

    RunningSystem = 1;

    while(RunningSystem == 1);
    {
    }

    notice(">>> System halted\n");

    return(0);
}
