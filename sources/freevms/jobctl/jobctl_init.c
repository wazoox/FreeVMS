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

static L4_Fpage_t       kip_area;

static vms$pointer      utcb_size;
static vms$pointer      utcb_size_log2;

void
jobctl$utcb_init(L4_KernelInterfacePage_t *kip)
{
    utcb_size = L4_UtcbSize(kip);
    utcb_size_log2 = kip->UtcbAreaInfo.X.a;

    // We map the kip into all area.
    kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2(kip));
    return;
}

void
jobctl$pd_init(struct vms$meminfo *meminfo)
{
    struct memsection_list      *list;

    struct memsection_node      *first_ms;
    struct memsection_node      *next;
    struct memsection_node      *node;

    //list = &freevms_pd.memsections;
}
