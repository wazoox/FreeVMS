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

#define DEBUG_VM

#include "libearly/lib.h"
#include "libearly/l4io.h"

// L4 interfaces
#include "l4/arch.h"
#include "l4/bootinfo.h"
#include "l4/ipc.h"
#include "l4/kip.h"
#include "l4/kcp.h"
#include "l4/sigma0.h"
#include "l4/space.h"
#include "l4/thread.h"

#include "freevms/arch.h"

typedef L4_Word64_t     vms$pointer;

// FreeVMS messagesœ
#include "freevms/fatal.h"
#include "freevms/information.h"
#include "freevms/system.h"
#include "freevms/levels.h"

#include "freevms/tailq.h"
#include "freevms/b_plus_tree.h"

// FreeVMS subsystems
#include "freevms/vm.h"
#include "freevms/pd.h"

// Defines
#define NULL                            ((vms$pointer) 0)
#define FREEVMS_VERSION                 "0.0.1"
#define THREAD_STACK_BASE               (0xF00000UL)

// Address
#define UTCB(x)                 ((void*) (L4_Address(utcb_area) + \
                                        ((x) * utcb_size)))

// Macros
#define notice(...) printf(__VA_ARGS__)

#define max(a, b)   ((a < b) ? b : a)
#define min(a, b)   ((a < b) ? a : b)

const char *dbg$symbol(vms$pointer address);

void dbg$backtrace(void);
void dbg$sigma0(int level);

#define PANIC(a, ...) { if (a) { \
        __VA_ARGS__; \
        notice("\nPanic at %s, %s line %d\n", __FUNCTION__, __FILE__, \
                __LINE__); \
        notice("Have a nice day !\n\n"); \
    dbg$backtrace(); \
    while(1); } } while(0)

#define L4_SIZEOFWORD       (sizeof(L4_Word_t) * 8)
#define L4_REQUEST_MASK     (~((~0UL) >> (L4_SIZEOFWORD - 20)))
#define L4_IO_PAGEFAULT     (-8UL << 20)

// Prototypes
void parsing(char *line, char *command, char *argument, int length);
