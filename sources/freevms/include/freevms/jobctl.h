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

#define JOBCTL$MAX_THREADS_PER_APD 256

struct ll {
    struct ll               *next;
    struct ll               *prev;
    void                    *data;
};

struct double_list
{
    struct ll               *head;
    struct ll               *tail;
};

void *dl_list_create_front(struct double_list *list, int payload);
void *dl_list_create_back(struct double_list *dl, int payload);
void dl_list_clear(struct double_list *list);
void dl_list_init(struct double_list *dl);

struct ll *ll_delete (struct ll *ll);
struct ll *ll_insert_before (struct ll *ll, void *data);
struct ll *ll_new(void);

typedef vms$pointer objref_t;
typedef objref_t memsection_ref_t;
typedef objref_t thread_ref_t;
typedef objref_t pd_ref_t;
typedef objref_t session_ref_t;
typedef objref_t eas_ref_t;
typedef objref_t hw_ref_t;
typedef objref_t quota_ref_t;

enum cap_type
{
    CAP$OBJ,
    CAP$MEMSECTION,
    CAP$THREAD,
    CAP$PD,
    CAP$SESSION,
    CAP$EAS,
    CAP$HW,
    CAP$QUOTA
};

typedef struct
{
    union
    {
        objref_t            obj;
        memsection_ref_t    memsection;
        thread_ref_t        thread;
        pd_ref_t            pd;
        session_ref_t       session;
        eas_ref_t           eas;
        hw_ref_t            hw;
        quota_ref_t         quota;
    } ref;

    enum cap_type           type;
    vms$pointer             passwd;
} cap_t;

struct session_p_list
{
    struct session_p_node   *first;
    struct session_p_node   *last;
};

struct session_p_node
{
    struct session_p_node   *next;
    struct session_p_node   *prev;
    struct session          *data;
};

struct session
{
    struct pd               *owner;
    struct thread           *client;
    struct thread           *server;
    void                    *call_buf;
    void                    *return_buf;
    struct memsection       *clist;
    struct session          **owner_node;
    struct session          **server_node;
    struct session          **client_node;
};

struct thread
{
    struct pd               *owner;
    void                    *utcb;
    struct eas              *eas; // if not NULL, indicates an external thread,
                                  // in a different address space.
    L4_ThreadId_t           id;
    struct session_p_list   client_sessions;
    struct session_p_list   server_sessions;
    //struct exception      exception[2];
};

struct thread_node
{
    struct thread_node      *next;
    struct thread_node      *prev;
    struct thread           data;
};

struct thread_list
{
    struct thread_node      *first;
    struct thread_node      *last;
};

enum pd_state
{
    pd_empty,           // newly created
    pd_active,          // has active threads
    pd_suspended        // last thread was removed, but not deleted yet
};

typedef unsigned long       bfl_word;
#define BITS_PER_BFL_WORD   ((int) (sizeof(bfl_word) * 8))

struct bfl
{
    unsigned int            curpos;
    unsigned int            len;
    unsigned int            array_size;

    bfl_word                bitarray[];
};

typedef struct bfl          *bfl_t;

bfl_t jobctl$bfl_new(vms$pointer size);
void jobctl$bfl_free(bfl_t rfl, vms$pointer val);
vms$pointer jobctl$bfl_alloc(bfl_t rfl);

// Describes external address space

enum eas_state
{
    eas_empty,          // newly created
    eas_active,         // has active threads
    eas_suspended       // last thread was removed, but not deleted yet
};

struct eas
{
    struct pd               *owner;
    struct thread_list      threads;
    L4_Fpage_t              kip;
    L4_Fpage_t              utcb_area;
    L4_ThreadId_t           redirector;
    enum eas_state          state;
};

struct eas_list
{
    struct eas_node         *first;
    struct eas_node         *last;
};

struct eas_node
{
    struct eas_node         *next;
    struct eas_node         *prev;
    struct eas              data;
};

struct clist_list
{
    struct clist_node       *first;
    struct clist_node       *last;
};

struct clist_info
{
    cap_t                   *clist;
    unsigned int            type;
    vms$pointer             length;
};

struct clist_node
{
    struct clist_node       *next;
    struct clist_node       *prev;
    struct clist_info       data;
};

struct pd_list
{
    struct pd_node          *first;
    struct pd_node          *last;
};

struct pd
{
    struct pd               *owner;         // Our owner
    enum pd_state           state;

    struct memsection       *callback_buffer;
    struct cb_alloc_handle  *cba;

    // PD information
    struct pd_list          pds;
    struct memsection_list  memsections;
    struct session_p_list   sessions;
    struct eas_list         eass;

    // UTCB info
    struct memsection       *utcb_memsection;
    void                    *utcb_base;
    L4_Fpage_t              utcb_area;

    // Thread information
    bfl_t                   local_threadno; // Freelist for UTCBs
    struct thread_list      threads;

    // Clist information
    struct clist_list       clists;

    // Quota information
    struct quota            *quota;
};

struct pd_node
{
    struct pd_node          *next;
    struct pd_node          *prev;
    struct pd               data;
};

struct range
{
    vms$pointer             from;
    vms$pointer             to;
};

typedef struct ll           *rfl_t;

#define RFL_SUCCESS 0
#define E_RFL_INV_RANGE -1
#define E_RFL_OVERLAP -2
#define E_RFL_NOMEM -3

rfl_t jobctl$rfl_new(void);
int jobctl$rfl_insert_range(rfl_t rfl, vms$pointer from, vms$pointer to);

struct hashtable
{
    struct hashentry        **table;
    unsigned int            size;
    struct hashentry        *spares;
};

struct hashentry
{
    struct hashentry        *next;
    vms$pointer             key;
    void                    *value;
};

struct hashtable *hash_init(unsigned int size);

struct memsection *vms$pd_create_memsection(struct pd *self, vms$pointer size,
        vms$pointer base, unsigned int flags);

void jobctl$utcb_init(L4_KernelInterfacePage_t *kip);
struct pd *jobctl$pd_create(struct pd *self, int max_threads);
void jobctl$pd_init(struct vms$meminfo *meminfo);
struct thread *jobctl$pd_create_thread(struct pd* self, int priority);
void jobctl$thread_init(L4_KernelInterfacePage_t *kip);
