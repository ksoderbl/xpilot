/* $Id: list.c,v 1.4 1998/04/20 10:52:16 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * A double linked list similar to the STL list, but implemented in C.
 */

#include <stdlib.h>
#include "list.h"

struct ListNode {
    struct ListNode	*next;
    struct ListNode	*prev;
    void		*data;
};
typedef struct ListNode list_node_t;

struct List {
    list_node_t		tail;
    int			size;
};
/* typedef struct List *list_t; */
/* typedef struct ListNode *list_iter_t; */

/* create a new list. */
list_t List_new(void)
{
    list_t list = (list_t) malloc(sizeof(*list));
    if (!list) {
	return NULL;
    }
    list->tail.next = &list->tail;
    list->tail.prev = &list->tail;
    list->tail.data = NULL;
    list->size = 0;
    return list;
}

/* delete a list. */
void List_delete(list_t list)
{
    if (list) {
	List_clear(list);
	list->tail.next = list->tail.prev = NULL;
	free(list);
    }
}

/* return a list iterator pointing to the first element of the list. */
list_iter_t List_begin(list_t list)
{
    return list->tail.next;
}

/* return a list iterator pointing to the one past the last element of the list. */
list_iter_t List_end(list_t list)
{
    return &list->tail;
}

/* return a pointer to the last list element. */
void* List_back(list_t list)
{
    return list->tail.prev->data;
}

/* return a pointer to the first list element. */
void* List_front(list_t list)
{
    return list->tail.next->data;
}

/* erase all elements from the list. */
void List_clear(list_t list)
{
    while (!List_empty(list)) {
	List_pop_front(list);
    }
}

/* return true if list is empty. */
int List_empty(list_t list)
{
    return (list->size == 0);
}

/* erase element at list position. */
list_iter_t List_erase(list_t list, list_iter_t pos)
{
    list_iter_t next, prev;

    if (pos == &list->tail) {
	return List_end(list);
    }

    next = pos->next;
    prev = pos->prev;
    prev->next = next;
    next->prev = prev;
    list->size--;

    pos->prev = NULL;
    pos->next = NULL;
    pos->data = NULL;
    free(pos);
    
    return next;
}

/* erase a range of list elements excluding last. */
list_iter_t List_erase_range(list_t list, list_iter_t first, list_iter_t last)
{
    while (first != last) {
	first = List_erase(list, first);
    }
    return first;
}

/* insert a new element into the list at position
 * and return new position or NULL on failure. */
list_iter_t List_insert(list_t list, list_iter_t pos, void *data)
{
    list_iter_t node = (list_iter_t) malloc(sizeof(*node));
    if (!node) {
	return NULL;
    }
    node->next = pos;
    node->prev = pos->prev;
    node->data = data;
    node->prev->next = node;
    node->next->prev = node;
    list->size++;
    return node;
}

/* remove the first element from the list and return a pointer to it. */
void* List_pop_front(list_t list)
{
    void *data = list->tail.next->data;
    List_erase(list, list->tail.next);
    return data;
}

/* remove the last element from the list and return a pointer to it. */
void* List_pop_back(list_t list)
{
    void *data = list->tail.prev->data;
    List_erase(list, list->tail.prev);
    return data;
}

/* add a new element to the beginning of the list.
 * and return the new position or NULL on failure. */
list_iter_t List_push_front(list_t list, void *data)
{
    return List_insert(list, list->tail.next, data);
}

/* append a new element at the end of the list.
 * and return the new position or NULL on failure. */
list_iter_t List_push_back(list_t list, void *data)
{
    return List_insert(list, &list->tail, data);
}

/* return the number of elements in the list. */
int List_size(list_t list)
{
    return list->size;
}

/* advance list iterator one position and return new position. */
list_iter_t List_iter_forward(list_iter_t *pos)
{
    (*pos) = (*pos)->next;
    return (*pos);
}

/* move list iterator one position backwards and return new position. */
list_iter_t List_iter_backward(list_iter_t *pos)
{
    (*pos) = (*pos)->prev;
    return (*pos);
}

/* return data at list position. */
void* List_iter_data(list_iter_t pos)
{
    return pos->data;
}


