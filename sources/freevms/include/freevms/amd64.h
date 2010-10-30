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

typedef struct
{
    L4_Word_t   start;
    L4_Word_t   end;
    char        *cmdline;
    L4_Word_t   entry;
} mbi_module_t;

typedef struct
{
    struct
    {
        L4_BITFIELD6(L4_Word_t,
                mem         :1,
                bootdev     :1,
                cmdline     :1,
                mods        :1,
                syms        :2,
                mmap        :1);
    } flags;

    // Memory info in KB, valid if flags.mem is set.
    L4_Word_t       mem_lower;
    L4_Word_t       mem_upper;

    // Valid if flags.bootdev is set
    L4_Word_t       boot_device;

    // The kernel command line.  Valid if flags.cmdline is set.
    char            *cmdline;

    // Module info.  Valid if flags.mods is set.
    L4_Word_t       modcount;
    mbi_module_t    *mods;

    // Kernel symbol info.  Valid if either one of flags:syms is set.
    L4_Word_t       syms[4];

    // BIOS memory map.  Valid if flags:mmap is set.
    L4_Word_t       mmap_length;
    L4_Word_t       mmap_addr;

    L4_Word_t       drives_length;
    L4_Word_t       drives_addr;
    L4_Word_t       config_table;
    L4_Word_t       boot_loader_name;
    L4_Word_t       apm_table;
    L4_Word_t       vbe[4];
} mbi_t;
