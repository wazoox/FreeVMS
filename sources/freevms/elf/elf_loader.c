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

// elf charge un exécutable à l'adresse virtuelle $100000000000 (512 Mo)
// L'adresse de base d'un exécutable est de $1000000000. Il est
// chargé par cette fonction. La mémoire basse est réservée au noyau.
// Utiliser un grantitem pour mapper l'exécutable
//
// If thread != NULL, image is granted to new address space.

vms$pointer
elf$loader(struct thread *thread, vms$pointer start, vms$pointer end,
        cap_t *clist, vms$pointer *pos)
{
    ehdr_t              *eh;
    phdr_t              *ph;

    struct memsection   *memsection;

    vms$pointer         dst_end;
    vms$pointer         dst_start;
    vms$pointer         i;
    vms$pointer         src_end;
    vms$pointer         src_start;

    eh = (ehdr_t *) start;

    PANIC((eh->type != 2) || (eh->phoff == 0),
            notice(ELF_F_FMT "cannot load binary stream\n"));

    for(i = 0; i < eh->phnum; i++)
    {
        ph = (phdr_t*) (start + eh->phoff + (eh->phentsize * i));

        if (ph->msize < ph->fsize)
        {
            ph->msize = ph->fsize;
        }

        if (ph->type == PT_LOAD)
        {
            src_start = start + ph->offset;
            src_end = src_start + ph->msize;
            dst_start = ph->paddr;
            dst_end = dst_start + ph->msize;

            // Allocating memory
            memsection = vms$pd_create_memsection(thread->owner,
                    vms$page_round_up((dst_end - dst_start) + 1,
                    vms$min_pagesize()),
                    vms$page_round_down(dst_start, vms$min_pagesize()),
                    VMS$MEM_FIXED, vms$min_pagesize());
            clist[(*pos)++] = sec$create_capability((vms$pointer)
                    memsection, CAP$MEMSECTION);
            vms$memcopy(dst_start, src_start, ph->fsize);
            vms$initmem(dst_start + ph->fsize, ph->msize - ph->fsize);
        }
    }

    return(eh->entry);
}

