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

static void
jobctl$thread_free(L4_ThreadId_t thread)
{
    // Remove thread->pd mapping
    hash_remove(l4tid_to_thread, thread.raw);
    // Add thread back to free pool
    rfl_free(thread_list, L4_ThreadNo(thread));

    return;
}

static int
jobctl$thread_alloc(struct thread *thread)
{
    int         thread_no;

    thread_no = rfl_alloc(thread_list);

    if (thread_no == -1)
    {
        // Run out of threads
        return(1);
    }

    thread->id = L4_GlobalId(thread_no, 1);
    PANIC(hash_lookup(l4tid_to_thread, thread->id.raw) != NULL);
    hash_insert(l4tid_to_thread, thread->id.raw, thread);

    return(0);
}

static int
jobctl$thread_setup(struct thread *self, int priority)
{
    extern L4_Fpage_t   kip_area;

    int                 r;
    int                 ret;

    L4_Word_t           ignore;

    struct pd           *pd;

    ret = 1;
    PANIC(self == NULL);

    // Note: These don't allocate any resources
    jobctl$session_p_list_init(&self->client_sessions);
    jobctl$session_p_list_init(&self->server_sessions);

    if (priority == -1)
    {
        priority = 100;
    }

    if (pd->state != pd_suspended)
    {
        self->eas = NULL;
        // Allocate a thread id
        r = jobctl$thread_alloc(self);

        if (r != 0)
        {
            // Can't allocate a new thread id
            return(1);
        }
    }

    if (pd->state == pd_empty)
    {
        // Create thread in a new address space
        r = L4_ThreadControl(self->id, self->id, L4_Myself(),
                L4_nilthread, self->utcb);
        PANIC(r == 0, ERROR_PRINT_L4);

        r = L4_SpaceControl(self->id, 0, kip_area, pd->utcb_area, L4_anythread,
                &ignore);
        PANIC(r == 0, ERROR_PRINT_L4);
    }
    else
    {
        r = L4_ThreadControl(self->id, pd->threads.first->data.id,
                L4_Myself(), L4_nilthread, (void*) -1);
        PANIC(r == 0, ERROR_PRINT_L4);
    }

    // Set priority
    L4_Set_Priority(self->id, priority);

    // Activate new thread
    if (pd->state != pd_suspended)
    {
        r = L4_ThreadControl(self->id, pd_l4_space(pd), pd_l4_space(pd),
                L4_Myself(), L4_anythread, L4_anythread,
                self->utcb);

        if (r != 1)
        {
            if (L4_ErrorCode() == L4_ErrNoMem)
            {
                // L4 has run out of memory... this is probably very bad,
                // but we want to keep going for as long as we can.
                goto thread_error_state;
            }
            else
            {
                PANIC(1, ERROR_PRINT_L4);
            }
        }
        else
        {
            ret = 0;
        }
    }

    pd->state = pd_active;
    return(ret);

thread_error_state:
    // Here we clean up anything we have allocated
    if (pd->state != pd_suspended)
    {
        jobctl$thread_free(self->id);
    }

    return(1);
}

struct thread *
jobctl$pd_create_thread(struct pd* self, int priority)
{
    extern vms$pointer      utcb_size;

    int                     local_tid;

    struct thread           *thread;

    PANIC(self == NULL);

    switch(self->state)
    {
        case pd_empty:
        case pd_active:
            thread = jobctl$thread_list_create_back(&self->threads);

            if (thread == NULL)
            {
                return(NULL);
            }

            local_tid = jobctl$bfl_alloc(self->local_threadno);

            if (local_tid == -1)
            {
                goto thread_error_state_1;
            }

            thread->owner = self;
            thread->utcb = (void*) ((vms$pointer) self->utcb_base +
                    (utcb_size * local_tid));
            break;

        case pd_suspended:
            thread = &self->threads.first->data;
            break;

        default:
            PANIC(1, notice("pd corruption!\n"));
            return 0;
    }

    if (jobctl$thread_setup(thread, priority) != 0)
    {
        // Need to clean up
        goto thread_error_state_2;
    }

    self->state = pd_active;
    return(thread);

thread_error_state_1:
    jobctl$thread_list_delete(thread);
    return(NULL);

thread_error_state_2:
    if (self->state != pd_suspended)
    {
        jobctl$bfl_free(self->local_threadno, local_tid);
        jobctl$thread_list_delete(thread);
    }

    return(NULL);
}
