/* $Id: list.h,v 1.2 1998/04/16 11:00:08 bert Exp $
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

#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

/*
 * A double linked list similar to the STL list, but implemented in C.
 */

typedef struct List *list_t;
typedef struct ListNode *list_iter_t;

/* create a new list and return the new list or NULL on failure. */
list_t		List_new(void);

/* delete a list. */
void		List_delete(list_t);

/* return a list iterator pointing to the first element of the list. */
list_iter_t	List_begin(list_t);

/* return a list iterator pointing to the one past the last element of the list. */
list_iter_t	List_end(list_t);

/* return a pointer to the last list element. */
void* 		List_back(list_t);

/* return a pointer to the first list element. */
void* 		List_front(list_t);

/* erase all elements from the list. */
void		List_clear(list_t);

/* return true if list is empty. */
int		List_empty(list_t);

/* erase element at list position. */
list_iter_t	List_erase(list_t, list_iter_t);

/* erase a range of list elements excluding last. */
list_iter_t	List_erase_range(list_t alist, list_iter_t first, list_iter_t last);

/* insert a new element into the list at position
 * and return the new position or NULL on failure. */
list_iter_t	List_insert(list_t alist, list_iter_t position, void *element);

/* remove the first element from the list and return a pointer to it. */
void*		List_pop_front(list_t);

/* remove the last element from the list and return a pointer to it. */
void*		List_pop_back(list_t);

/* add a new element to the beginning of the list.
 * and return the new position or NULL on failure. */
list_iter_t List_push_front(list_t list, void *data);

/* append a new element at the end of the list.
 * and return the new position or NULL on failure. */
list_iter_t List_push_back(list_t list, void *data);

/* return the number of elements in the list. */
int		List_size(list_t);

/* advance list iterator one position and return new position. */
list_iter_t	List_iter_forward(list_iter_t *pos);

/* move list iterator one position backwards and return new position. */
list_iter_t	List_iter_backward(list_iter_t *pos);

/* return data at list position. */
void*		List_iter_data(list_iter_t pos);

/* macros to reduce typing. */
#define LI_FORWARD(pos_)	List_iter_forward(&(pos_))
#define LI_BACKWARD(pos_)	List_iter_backward(&(pos_))
#define LI_DATA(pos_)		List_iter_data((pos_))

#endif
