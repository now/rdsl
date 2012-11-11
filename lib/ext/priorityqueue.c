/*
 * contents: Priority queue ADT.
 * arch-tag: 04027689-27a6-4466-bd8f-5ff93da83289
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


#include <clear/internal.h>
#include <clear/mem.h>
#include "priorityqueue.h"


/* {{{1
 * The default initial size of a priority queue.  It's a question whether the
 * default should actually be zero (0), but at the moment it really doesn't
 * matter much.
 */
#define PRIORITY_QUEUE_INITIAL_SIZE	16


/* {{{1
 * Our priority queue structure.  ‘compare’ is the function we use for
 * determining the placement of data items we put in the queue and ‘equal’ is
 * used for checking if a data item is a member of the queue.  ‘heap’ is of
 * course our heap structure and ‘size’/‘len’ keep track of it.
 */
struct _PriorityQueue {
	CompareFunc compare;
	EqualFunc equal;
	pointer *heap;
	size_t len;
	size_t size;
};


/* {{{1
 * Create a new priority queue ADT using the given functions for the purposes
 * described above.
 */
PriorityQueue *
priority_queue_new(CompareFunc compare, EqualFunc equal)
{
	return priority_queue_sized_new(compare,
					equal,
					PRIORITY_QUEUE_INITIAL_SIZE);
}


/* {{{1
 * Create a new priority queue with a starting size.  ‘equal’ may be ‹null›,
 * but ‘compare’ must be non-‹null›.
 */
PriorityQueue *
priority_queue_sized_new(CompareFunc compare, EqualFunc equal, size_t size)
{
	invariant(compare != null);

	PriorityQueue *q = new_struct(PriorityQueue);
	q->compare = compare;
	q->equal = equal;
	q->size = size;
	q->heap = new_array(pointer, q->size + 1);
	q->len = 0;

	return q;
}


/* {{{1
 * Release all resources associated with the given priority queue.
 */
void
priority_queue_release(PriorityQueue *q)
{
	invariant(q != null);

	release(q->heap);
	release(q);
}


/* {{{1
 * Push ‘data’ on the priority queue.
 */
void
priority_queue_push(PriorityQueue *q, pointer data)
{
	invariant(q != null);

	/* first, resize to fit this element as well, if necessary */
	if (q->len > q->size) {
		q->size *= 2;
		q->heap = resize_array(q->heap, pointer, q->size + 1);
	}

	/* increase the number of elements in queue and add it */
	q->heap[++q->len] = data;

	/* then sift it upwards while it's smaller than its parents */
	for (size_t i = q->len, p;
	     i > 1 && q->compare(q->heap[p = i / 2], q->heap[i]) > 0;
	     i = p) {
		pointer tmp = q->heap[p];
		q->heap[p] = q->heap[i];
		q->heap[i] = tmp;
	}
}


/* {{{1
 * Pop the item with highest priority (sorted as the smallest item with
 * |PriorityQueue|->compare), off of the priority queue.
 */
pointer
priority_queue_pop(PriorityQueue *q)
{
	invariant(q != null);

	/* first, save the root so that we can return it later on */
	pointer root = q->heap[1];

	/* then, set the root to our last element, and sift it downwards */
	q->heap[1] = q->heap[q->len--];
	for (size_t i = 1, c; (c = 2 * i) <= q->len; i = c) {
		if (c + 1 <= q->len && q->heap[c + 1] < q->heap[c]) {
			c++;
		}
		if (q->compare(q->heap[i], q->heap[c]) > 0) {
			pointer tmp = q->heap[c];
			q->heap[c] = q->heap[i];
			q->heap[i] = tmp;
		} else {
			break;
		}
	}

	return root;
}


/* {{{1
 * Retrieve the number of items in the priority queue.
 */
inline int
priority_queue_length(PriorityQueue *q)
{
	invariant(q != null);

	return q->len;
}


/* {{{1
 * Check if the given priority queue is empty.
 */
inline bool
priority_queue_empty(PriorityQueue *q)
{
//	invariant(q != null);

	return q->len == 0;
}


/* {{{1
 * Check if the priority queue contains the given data.  This requires that the
 * priority queue was created with an non-‹null› ‘equal’ function.
 */
bool
priority_queue_contains(PriorityQueue *q, constpointer data)
{
	invariant(q != null);
	invariant(q->equal != null);

	if (q->len > 0 && q->equal(q->heap[1], data)) {
		return true;
	}

	for (size_t i = 1, c; (c = 2 * i) <= q->len; i = c) {
		if (q->equal(q->heap[c], data) ||
		    (c + 1 <= q->len && q->equal(q->heap[c + 1], data))) {
			return true;
		}
	}

	return false;
}


/* {{{1
 * Call the given function on each item of the given priority queue in order of
 * decreasing priority.
 */
void
priority_queue_map(PriorityQueue *q, MapFunc lambda, pointer closure)
{
	invariant(q != null);
	invariant(lambda != null);

	if (q->len > 0) {
		lambda(q->heap[1], closure);
	}

	for (size_t i = 1, c; (c = 2 * i) <= q->len; i = c) {
		lambda(q->heap[c], closure);
		if (c + 1 <= q->len) {
			lambda(q->heap[c + 1], closure);
		}
	}
}


/* }}}1 */



/* vim: set sts=0 sw=8 ts=8: */
