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

struct vms$string
{
    vms$pointer             length;
    vms$pointer             length_trim;
    unsigned char           *c;
};

#define vms$string_initializer(s, l) \
    struct vms$string s;                        \
    unsigned char static_##s[l];                \
    s.length = l;                               \
    s.length_trim = 0;                          \
    s.c = &(static_##s[0])

int rtl$strcmp(struct vms$string *s1, struct vms$string *s2);
void rtl$strcpy(struct vms$string *s1, struct vms$string *s2);
void rtl$strcpy(struct vms$string *s1, const char *s2);

extern "C"
{
	void __attribute__((noreturn)) __bootstrap(int argc, char **argv);
	void __attribute__((noreturn)) exit(int v);
}

int main(int argc , char **argv);

// FIXME
int str$len(struct vms$string, struct vms$string);
int str$lentrim(struct vms$string, struct vms$string);

void rtl$print(struct vms$string *fmt, void **arg);
void rtl$sprint(struct vms$string *str, struct vms$string *fmt, void **arg);
