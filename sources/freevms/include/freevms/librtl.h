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

// string descriptor

#define vms$string(s, l) \
struct vms$string \
{ \
    vms$pointer             length; \
    vms$pointer             length_trim; \
    unsigned char           c[l]; \
} s; s.length = l; s.length_trim = 0

int str$cmp(struct vms$string, struct vms$string);
int str$cpy(struct vms$string, struct vms$string);
int str$cpy(struct vms$string, const unsigned char *s);
int str$len(struct vms$string, struct vms$string);
int str$lentrim(struct vms$string, struct vms$string);

int rtl$print(const char *fmt, int size, ...);
