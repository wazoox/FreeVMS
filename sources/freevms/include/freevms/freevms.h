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

#include <stdarg.h>

#include "libearly/lib.h"
#include "libearly/l4io.h"

#define arch_specific(func)         CONCAT(func, ARCH)
#define CONCAT(a, b)                XCAT(a, _, b)
#define XCAT(a, c, b)               a##c##b

// L4 interfaces
#include "l4/arch.h"
#include "l4/bootinfo.h"
#include "l4/ipc.h"
#include "l4/kip.h"
#include "l4/kcp.h"
#include "l4/schedule.h"
#include "l4/sigma0.h"
#include "l4/space.h"
#include "l4/thread.h"

#include "freevms/sys_arch.h"
#include "freevms/syscalls.h"

#ifdef AMD64
#   define ARCH     amd64
#   include         "freevms/amd64.h"
#endif

typedef L4_Word64_t     vms$pointer;

// FreeVMS messagesœ
#include "freevms/fatal.h"
#include "freevms/information.h"
#include "freevms/system.h"
#include "freevms/levels.h"

#include "freevms/tailq.h"
#include "freevms/b_plus_tree.h"

// FreeVMS subsystems
#include "freevms/sys_dev.h"
#include "freevms/sys_mem.h"
#include "freevms/sys_names.h"
#include "freevms/sys_thread.h"
#include "freevms/sys_mutex.h"
#include "freevms/sys_quota.h"
#include "freevms/sys_sec.h"
#include "freevms/sys_elf.h"
#include "freevms/sys_inlined.h"
#include "freevms/librtl.h"

// Defines
#define NULL                            ((vms$pointer) 0)
#define FREEVMS_VERSION                 "0.4.0"
#define THREAD_STACK_BASE               (0xF00000UL)
#define NUMBER_OF_KERNEL_THREADS        256
#define MAX_STRINGITEM_LENGTH           4096

// Address
#define UTCB(x)                 ((void*) (L4_Address(utcb_area) + \
                                        ((x) * utcb_size)))

// Macros
#define notice(...) printf(__VA_ARGS__)

#define max(a, b)   ((a < b) ? b : a)
#define min(a, b)   ((a < b) ? a : b)

// Debug capabilities
const char *dbg$symbol(vms$pointer address);

void dbg$backtrace(void);
void dbg$sigma0(int level);

extern int dbg$virtual_memory;
extern int dbg$sys_pagefault;
extern int dbg$vms_pagefault;

#define ERROR_PRINT_L4 notice("L4 microkernel error: %s [%lx]\n", \
        L4_ErrorCode_String(L4_ErrorCode()), L4_ErrorCode())

#define FIXME do { notice("TO BE FIXED ! %s line %d\n", __FILE__, __LINE__); } \
        while(0)
#define PANIC(a, ...) { if (a) { \
        __VA_ARGS__; \
        notice("\nPanic at %s, %s line %d\n", __FUNCTION__, __FILE__, \
                __LINE__); \
        notice("Have a nice day !\n\n"); \
    dbg$backtrace(); \
    while(1) L4_KDB_Enter("Panic"); } } while(0)

#define L4_SIZEOFWORD       (sizeof(L4_Word_t) * 8)
#define L4_REQUEST_MASK     (~((~0UL) >> (L4_SIZEOFWORD - 20)))
#define L4_PAGEFAULT        (-2UL << 20)
#define L4_IO_PAGEFAULT     (-8UL << 20)

// Prototypes
void sys$init(L4_KernelInterfacePage_t *kip, struct vms$meminfo *meminfo,
        vms$pointer pagesize, char *root_device);
void sys$pager(L4_KernelInterfacePage_t *kip, struct vms$meminfo *meminfo,
        vms$pointer pagesize, char *root_device);
void sys$loop();
void sys$parsing(char *line, char *command, char *argument, int length);

extern "C"
{
    void freevms_main(void);
}
