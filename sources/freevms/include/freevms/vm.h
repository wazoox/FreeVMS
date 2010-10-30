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

#ifdef DEBUG_VM
#   define vms$debug(msg) notice(DBG_I_VMS "%s\n", msg);
#else
#   define vms$debug(msg)
#endif

struct memdesc
{
    vms$pointer         base;
    vms$pointer         end;
};

struct initial_obj
{
#   define              INITIAL_NAME_MAX    80
    char                name[INITIAL_NAME_MAX];
    char                flags;

    vms$pointer         base;
    vms$pointer         end;
    vms$pointer         entry;
};

struct vms$meminfo
{
    unsigned int        num_regions;
    unsigned int        max_regions;
    struct memdesc      *regions;

    unsigned int        num_io_regions;
    unsigned int        max_io_regions;
    struct memdesc      *io_regions;

    unsigned int        num_vm_regions;
    unsigned int        max_vm_regions;
    struct memdesc      *vm_regions;

    unsigned int        num_objects;
    unsigned int        max_objects;
    struct initial_obj  *objects;
};

#define     VMS$HEAP_SIZE   (4 * 1024 * 1024)

#define     VMS$MEM_RAM     1
#define     VMS$MEM_IO      2
#define     VMS$MEM_VM      4
#define     VMS$MEM_OTHER   8

#define     VMS$MEM_NORMAL      0x1
#define     VMS$MEM_FIXED       0x2
#define     VMS$MEM_UTCB        0x4
#define     VMS$MEM_USER        0x8

#define     VMS$MEM_VALID_USER_FLAGS \
        (VMS$MEM_NORMAL | VMS$EM_FIXED | VMS$MEM_UTCB | VMS$MEM_USER )

#define     VMS$MEM_INTERNAL    0x10

#define     NUM_MI_REGIONS      1024
#define     NUM_MI_IOREGIONS    1024
#define     NUM_MI_VMREGIONS    1024
#define     NUM_MI_OBJECTS      1024

#define     VMS$IOF_RESERVED    0x01    // Reserved by the kernel
#define     VMS$IOF_APP         0x02    // Application
#define     VMS$IOF_ROOT        0x04    // Root server
#define     VMS$IOF_BOOT        0x08    // Boot info
#define     VMS$IOF_VIRT        0x10    // Set if memory is physical
#define     VMS$IOF_PHYS        0x20    // Set if memory is virtual
// Both VMS$IOF_VIRT and VMS$IOF_PHYS can be set for direct mapped.

#define     MAX_FPAGE_ORDER     (sizeof(L4_Word_t) * 8)

#define     INVALID_ADDR        (~((vms$pointer) 0))

#define     SIGMA0_REQUEST_LABEL (((vms$pointer) -6) << 4)

struct fpage_list
{
    L4_Fpage_t                  fpage;
    TAILQ_ENTRY(fpage_list)     flist;
};

TAILQ_HEAD(flist_head, fpage_list);

struct fpage_alloc
{
    struct flist_head       flist[MAX_FPAGE_ORDER + 1];
    struct
    {
        vms$pointer         base;
        vms$pointer         end;
        int                 active;
    } internal;
};

struct slab
{
    TAILQ_ENTRY(slab)       slabs;
};

struct slab_cache
{
    vms$pointer                         slab_size;
    TAILQ_HEAD(sc_head, memsection)     pools;
};

#define SLAB_CACHE_INITIALIZER(sz, sc) \
        { (sz), TAILQ_HEAD_INITIALIZER((sc)->pools) }

struct memsection
{
    vms$pointer                 magic;
    vms$pointer                 base;
    vms$pointer                 end;
    vms$pointer                 memory_attributes;
    vms$pointer                 flags;
    vms$pointer                 phys_active;
    union
    {
        vms$pointer                     base;
        struct flist_head               list;
        TAILQ_HEAD(ml_head, map_list)   mappings;
    } phys;
    struct pd                   *owner;
    struct thread               *server;
    struct slab_cache           *slab_cache;
    TAILQ_ENTRY(memsection)     pools;
    TAILQ_HEAD(sl_head, slab)   slabs;
};

struct memsection_list
{
    struct memsection_node      *first;
    struct memsection_node      *last;
};

struct memsection_node
{
    struct memsection_node      *next;
    struct memsection_node      *prev;
    struct memsection           data;
};

int memsection_back(struct memsection *memsection);
int vms$remove_chunk(struct memdesc *mem_desc, int pos, int max,
        vms$pointer low, vms$pointer high);

L4_Word_t vms$min_pagesize(void);
L4_Word_t vms$min_pagebits(void);

L4_Fpage_t vms$biggest_fpage(vms$pointer addr, vms$pointer base,
        vms$pointer end);

void vms$sigma0_map(vms$pointer virt_addr, vms$pointer phys_addr,
        vms$pointer size);
void vms$sigma0_map_fpage(L4_Fpage_t virt_page, L4_Fpage_t phys_page);
void vms$bootstrap(struct vms$meminfo *mem_info, unsigned int page_size);
void vms$fpage_clear_internal(struct fpage_alloc *alloc);
void vms$fpage_free_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end);
void vms$fpage_free_internal(struct fpage_alloc *alloc, vms$pointer base,
        unsigned long end);
void vms$fpage_free_list(struct fpage_alloc *alloc, struct flist_head list);
void vms$init(L4_KernelInterfacePage_t *kip,
        struct vms$meminfo *MemInfo, unsigned int page_size);
void vms$initmem(vms$pointer zone, vms$pointer len);
void vms$pager(void);
void vms$remove_virtmem(struct vms$meminfo *mem_info,
        vms$pointer base, unsigned long end, unsigned int page_size);
void *vms$slab_cache_alloc(struct slab_cache *sc);
void vms$slab_cache_free(struct slab_cache *sc, void *ptr);

struct flist_head vms$fpage_alloc_list(struct fpage_alloc *alloc,
        vms$pointer base, vms$pointer end);

struct memsection *vms$objtable_lookup(void *addr);

vms$pointer vms$fpage_alloc_chunk(struct fpage_alloc *alloc,
        unsigned int size);
vms$pointer vms$fpage_alloc_internal(struct fpage_alloc *alloc,
        unsigned int size);
vms$pointer vms$page_round_down(vms$pointer address, unsigned int page_size);
vms$pointer vms$page_round_up(vms$pointer address, unsigned int page_size);

struct sBTPage *ObjAllocPage(PagePool *pool);
void ObjFreePage(PagePool *pool, struct sBTPage *page);
