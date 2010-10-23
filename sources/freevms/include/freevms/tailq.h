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

/*
 * Author: Alex Webster
 */

#ifndef TAILQ_H
#define TAILQ_H

#define TAILQ_CONCAT(head1, head2, name) \
    do { \
	    if ((head2)->tqh_first != (typeof((head2)->tqh_first)) NULL) { \
		(head1)->tqh_last->name.tqe_next = (head2)->tqh_first; \
		(head2)->tqh_first->name.tqe_prev = (head1)->tqh_last; \
		(head1)->tqh_last = (head2)->tqh_last; \
		(head2)->tqh_first = (typeof((head2)->tqh_first)) NULL; \
		(head2)->tqh_last = (typeof((head2)->tqh_last)) NULL; \
		} \
    } while (0)

#define TAILQ_EMPTY(head) \
    ((head)->tqh_first == (typeof((head)->tqh_first)) NULL)

#define TAILQ_ENTRY(type) \
    struct { \
	    struct type *tqe_next; \
	    struct type *tqe_prev; \
    }

#define TAILQ_FIRST(head) \
    ((head)->tqh_first)

#define TAILQ_FOREACH(var, head, name) \
    for ((var) = (head)->tqh_first; (var); (var) = (var)->name.tqe_next)

#define TAILQ_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_first; (var) && (tmp) = (var)->name.tqe_next; (var) = (tmp))

#define TAILQ_FOREACH_REVERSE(var, head) \
    for ((var) = (head)->tqh_last; (var); (var) = (var)->name.tqe_prev)

#define TAILQ_FOREACH_REVERSE_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_last; (var) && (tmp) = (var)->name.tqe_prev; \
			(var) = (tmp))

#define TAILQ_HEAD(headname, type) \
    struct headname { \
	    struct type *tqh_first; \
	    struct type *tqh_last; \
    }

// Modifications from { .tqh_first = NULL, .tqh_last = NULL }
#define TAILQ_HEAD_INITIALIZER(head) \
    { (head).tqh_first = (typeof((head).tqh_first)) NULL, \
			(head).tqh_last = (typeof((head).tqh_last)) NULL }

#define TAILQ_INIT(head) \
    do { \
	    (head)->tqh_first = (typeof((head)->tqh_first)) NULL; \
	    (head)->tqh_last = (typeof((head)->tqh_last)) NULL; \
    } while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, name) \
    do { \
	    (elm)->name.tqe_next = (listelm)->name.tqe_next; \
	    (elm)->name.tqe_prev = (listelm); \
	    (listelm)->name.tqe_next = (elm); \
	    if ((elm)->name.tqe_next != (typeof((elm)->name.tqe_next)) NULL) { \
		    (elm)->name.tqe_next->name.tqe_prev = (elm); \
	    } else { \
		    (head)->tqh_last = (elm); \
	    } \
    } while (0)

#define TAILQ_INSERT_BEFORE(head, listelm, elm, name) \
    do { \
	(elm)->name.tqe_next = (listelm); \
	(elm)->name.tqe_prev = (listelm)->name.tqe_prev; \
	(listelm)->name.tqe_prev = (elm); \
	if ((elm)->name.tqe_prev != (typeof((elm)->name.tqe_prev)) NULL) { \
		(elm)->name.tqe_prev->name.tqe_next = (elm); \
		} else { \
			(head)->tqh_first = (elm); \
		} \
    } while (0)

#define TAILQ_INSERT_HEAD(head, elm, name) \
    do { \
	    if (__builtin_types_compatible_p(typeof((head)->tqh_first), \
					typeof(elm))) { \
		    (head) = __builtin_choose_expr(0, NULL, "I'm a sad panda"); \
	    } \
	    (elm)->name.tqe_next = (head)->tqh_first; \
	    (elm)->name.tqe_prev = (typeof((elm)->name.tqe_prev)) NULL; \
	    if ((head)->tqh_first != (typeof((head)->tqh_first)) NULL) { \
		    (head)->tqh_first->name.tqe_prev = (elm); \
	    } \
	    if ((elm)->name.tqe_next == (typeof((elm)->name.tqe_next)) NULL) { \
		    (head)->tqh_last = (elm); \
	    } \
	    (head)->tqh_first = (elm); \
    } while (0)

#define TAILQ_INSERT_TAIL(head, elm, name) \
    do { \
	    (elm)->name.tqe_next = (typeof((elm)->name.tqe_next)) NULL; \
	    (elm)->name.tqe_prev = (head)->tqh_last; \
	    if ((head)->tqh_last != (typeof((head)->tqh_last)) NULL) { \
		    (head)->tqh_last->name.tqe_next = (elm); \
	    } \
	    if ((elm)->name.tqe_prev == (typeof((elm)->name.tqe_prev)) NULL) { \
		    (head)->tqh_first = (elm); \
	    } \
	    (head)->tqh_last = (elm); \
    } while (0)

#define TAILQ_LAST(head) \
    ((head)->tqh_last)

#define TAILQ_NEXT(elm, name) \
    ((elm)->name.tqe_next)

#define TAILQ_PREV(elm, name) \
    ((elm)->name.tqe_prev)

#define TAILQ_REMOVE(head, elm, name) \
    do { \
	    if ((elm)->name.tqe_next != (typeof((elm)->name.tqe_next)) NULL) { \
		    (elm)->name.tqe_next->name.tqe_prev = (elm)->name.tqe_prev; \
	    } \
	    else { \
		    (head)->tqh_last = (elm)->name.tqe_prev; \
	    } \
	    if ((elm)->name.tqe_prev != (typeof((elm)->name.tqe_prev)) NULL) { \
		    (elm)->name.tqe_prev->name.tqe_next = (elm)->name.tqe_next; \
	    } \
	    else { \
		    (head)->tqh_first = (elm)->name.tqe_next; \
	    } \
    } while (0)

#endif
