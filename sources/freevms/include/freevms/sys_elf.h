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

struct ehdr_t
{
    unsigned char       ident[16];
    L4_Word16_t         type;
    L4_Word16_t         machine;
    L4_Word16_t         version;
    L4_Word_t           entry;
    L4_Word_t           phoff;
    L4_Word_t           shoff;
    L4_Word32_t         flags;
    L4_Word16_t         ehsize;
    L4_Word16_t         phentsize;
    L4_Word16_t         phnum;
    L4_Word16_t         shentsize;
    L4_Word16_t         shnum;
    L4_Word16_t         shstrndx;
};

struct phdr_t
{
    L4_Word32_t         type;
    L4_Word32_t         flags;
    L4_Word_t           offset;
    L4_Word_t           vaddr;
    L4_Word_t           paddr;
    L4_Word_t           fsize;
    L4_Word_t           msize;
    L4_Word_t           align;
};

enum phdr_type_e
{
    PT_LOAD = 1     // Loadable program segment
};

vms$pointer sys$elf_loader(struct thread *thread, vms$pointer start,
        vms$pointer end, cap_t *clist, vms$pointer *pos);
