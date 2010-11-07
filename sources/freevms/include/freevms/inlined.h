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

// Inlined functions

static inline struct thread *
jobctl$thread_list_create_front( struct thread_list *list)
{
    return((struct thread *) dl_list_create_front((struct double_list *) list,
            sizeof(struct thread)));
}

static inline struct thread *
jobctl$thread_list_create_back(struct thread_list* list)
{
    return((struct thread *) dl_list_create_back((struct double_list *) list,
            sizeof(struct thread)));
}

static inline void
jobctl$thread_list_delete(struct thread * data)
{
    ll_delete((struct ll *) ((void**) data - 2));
    return;
}

static inline void
jobctl$thread_list_init(struct thread_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static inline void
jobctl$thread_list_clear(struct thread_list * list)
{
    dl_list_clear((struct double_list *) list);
    return;
}

static inline void
jobctl$eas_list_init(struct eas_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static inline void
jobctl$clist_list_init(struct clist_list *list)
{
    dl_list_clear((struct double_list *) list);
    return;
}

static inline void
vms$memsection_list_init(struct memsection_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static inline void
vms$eas_list_init(struct eas_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static inline void
jobctl$session_p_list_delete(struct session ** data)
{
    ll_delete((struct ll *) ((void**) data - 2));
    return;
}

static inline void
jobctl$session_p_list_init(struct session_p_list *list)
{
    dl_list_init((struct double_list *) list);
    return;
}

static inline void
jobctl$session_p_list_clear(struct session_p_list * list)
{
    dl_list_clear((struct double_list *) list);
    return;
}

static inline struct pd *
pd_list_create_front(struct pd_list* list)
{
    return((struct pd *) dl_list_create_front((struct double_list *) list,
            sizeof(struct pd)));
}

static inline struct pd *
pd_list_create_back(struct pd_list* list)
{
    return((struct pd *) dl_list_create_back((struct double_list *) list,
            sizeof(struct pd)));
}

static inline void
pd_list_delete(struct pd * data)
{
    ll_delete((struct ll *) ((void**) data - 2));
    return;
}

static inline void
pd_list_clear(struct pd_list * list)
{
    dl_list_clear((struct double_list *) list);
    return;
}
