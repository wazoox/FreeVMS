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

typedef long long               Align;  // for alignment to long long boundary

union header                            // block header
{
    struct
    {
        union header            *ptr;   // next block if on free list
        vms$pointer             size;   // size of this block
    } s;

    Align                       x;      // force alignment
};

typedef union header            Header;

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

struct clist;

struct cap_slot
{
    struct clist            *list;
    int                     pos;
};

#define SLAB_CACHE_INITIALIZER(sz, sc) \
        { (sz), TAILQ_HEAD_INITIALIZER((sc)->pools) }

struct memsection
{
    vms$pointer                 base;
    vms$pointer                 end;
    vms$pointer                 flags;
    vms$pointer                 phys_active;
    union
    {
        vms$pointer                     base;
        struct flist_head               list;
    } phys;
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

#define VMS$FPAGE_PERM_READ     1
#define VMS$FPAGE_PERM_WRITE    2
#define VMS$FPAGE_PERM_EXECUTE  4

int vms$back_mem(vms$pointer base, vms$pointer end, vms$pointer pagesize);
int vms$memsection_back(struct memsection *memsection);
int vms$memsection_page_map(struct memsection *self, L4_Fpage_t from_page,
        L4_Fpage_t to_page);
int vms$remove_chunk(struct memdesc *mem_desc, int pos, int max,
        vms$pointer low, vms$pointer high);

L4_Word_t vms$min_pagesize(void);
L4_Word_t vms$min_pagebits(void);

L4_Fpage_t vms$biggest_fpage(vms$pointer addr, vms$pointer base,
        vms$pointer end);

void *vms$alloc(vms$pointer nbytes);
void vms$alloc_init(vms$pointer bss_p, vms$pointer top_p);
void vms$bootstrap(struct vms$meminfo *mem_info, vms$pointer page_size);
void vms$fpage_clear_internal(struct fpage_alloc *alloc);
void vms$fpage_free_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end);
void vms$fpage_free_internal(struct fpage_alloc *alloc, vms$pointer base,
        unsigned long end);
void vms$fpage_free_list(struct fpage_alloc *alloc, struct flist_head list);
void vms$fpage_remove_chunk(struct fpage_alloc *alloc, vms$pointer base,
        vms$pointer end);
void vms$free(void *ptr);
void vms$init(L4_KernelInterfacePage_t *kip,
        struct vms$meminfo *MemInfo, vms$pointer page_size);
void vms$initmem(vms$pointer zone, vms$pointer len);
void vms$memcopy(vms$pointer dest, vms$pointer src, vms$pointer size);
void vms$populate_init_objects(struct vms$meminfo *mem_info,
        vms$pointer pagesize);
void vms$remove_virtmem(struct vms$meminfo *mem_info,
        vms$pointer base, vms$pointer end, vms$pointer page_size);
void vms$sigma0_map(vms$pointer virt_addr, vms$pointer phys_addr,
        vms$pointer size, unsigned int priv);
void vms$sigma0_map_fpage(L4_Fpage_t virt_page, L4_Fpage_t phys_page,
        unsigned int priv);
void *vms$slab_cache_alloc(struct slab_cache *sc);
void vms$slab_cache_free(struct slab_cache *sc, void *ptr);

struct flist_head vms$fpage_alloc_list(struct fpage_alloc *alloc,
        vms$pointer base, vms$pointer end, vms$pointer pagesize);

memsection *vms$objtable_lookup(void *addr);

vms$pointer vms$fpage_alloc_chunk(struct fpage_alloc *alloc, vms$pointer size);
vms$pointer vms$fpage_alloc_internal(struct fpage_alloc *alloc,
        vms$pointer size);
void vms$pagefault(L4_ThreadId_t caller, vms$pointer addr,
        vms$pointer ip, vms$pointer tag);
vms$pointer vms$page_round_down(vms$pointer address, vms$pointer page_size);
vms$pointer vms$page_round_up(vms$pointer address, vms$pointer page_size);

// Objtable's functions

struct sBTPage *ObjAllocPage(PagePool *pool);

void ObjFreePage(PagePool *pool, struct sBTPage *page);
void vms$objtable_init(void);

int objtable_insert(struct memsection *memsection);
int objtable_setup(struct memsection *ms, vms$pointer size, unsigned int flags,
        vms$pointer pagesize);
int objtable_setup_fixed(struct memsection *ms, vms$pointer size,
        vms$pointer base, unsigned int flags, vms$pointer pagesize);
int objtable_setup_internal(struct memsection *ms, vms$pointer size,
        vms$pointer base, unsigned int flags);
int objtable_setup_utcb(struct memsection *ms, vms$pointer size,
        unsigned int flags);

