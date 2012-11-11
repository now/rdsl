/*
 * contents: Priority Queue ADT.
 * arch-tag: 00764151-c762-4585-bfb0-2149a2df3587
 *
 * Copyright (C) 2004 Nikolai Weibull <source@pcppopper.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H


typedef struct _PriorityQueue PriorityQueue;


PriorityQueue *priority_queue_new(CompareFunc compare, EqualFunc equal);
PriorityQueue *priority_queue_sized_new(CompareFunc compare,
					EqualFunc equal,
					size_t size);
void priority_queue_release(PriorityQueue *q);
void priority_queue_push(PriorityQueue *q, pointer data);
pointer priority_queue_pop(PriorityQueue *q);
inline int priority_queue_length(PriorityQueue *q);
inline bool priority_queue_empty(PriorityQueue *q);
bool priority_queue_contains(PriorityQueue *q, constpointer data);
void priority_queue_map(PriorityQueue *q, MapFunc lambda, pointer closure);


#endif /* PRIORITYQUEUE_H */



/* vim: set sts=0 sw=8 ts=8: */
