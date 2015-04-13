/*
 * nmp_list.c
 *
 * This file implements list.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_list.h"
#include "nmp_mem.h"


nmp_list_t *nmp_list_alloc( void )
{
	nmp_list_t *list = nmp_new(nmp_list_t, 1);
	return list;
}


nmp_list_t *nmp_list_add(nmp_list_t *list, void *data)
{
	nmp_list_t *new_list = nmp_list_alloc();

	new_list->data = data;
	new_list->next = list;
	new_list->prev = NULL;

	if (list)
		list->prev = new_list;

	return new_list;
}


nmp_list_t *nmp_list_add_tail(nmp_list_t *list, void *data)
{
	nmp_list_t *new_list = nmp_list_alloc();
	nmp_list_t *last;

	new_list->data = data;
	new_list->next = NULL;

	if (list)
	{
		last = nmp_list_last(list);
		new_list->prev = last;
		last->next = new_list;
		return list;
	}
	else
	{
		new_list->prev = NULL;
		return new_list;
	}
}


nmp_list_t *nmp_list_remove(nmp_list_t *list, void *data)
{
	nmp_list_t *head = list;

	while (list)
	{
		if (list->data != data)
			list = list->next;
		else
		{
			if (list->prev)
				list->prev->next = list->next;
			if (list->next)
				list->next->prev = list->prev;

			if (head == list)
				head = head->next;

			nmp_list_free_1(list);
			break;
		}
	}

	return head;
}


nmp_list_t *nmp_list_remove_all(nmp_list_t *list, void *data)
{
	nmp_list_t *next, *head = list;

	while (list)
	{
		if (list->data != data)
			list = list->next;
		else
		{
			next = list->next;

			if (list->prev)
				list->prev->next = next;
			else
				head = next;

			if (next)
				next->prev = list->prev;

			nmp_list_free_1(list);
			list = next;
		}
	}

	return head;	
}


nmp_list_t *nmp_list_remove_link(nmp_list_t *list, nmp_list_t *link)
{
	if (link)
	{
		if (link->next)
			link->next->prev = link->prev;
		if (link->prev)
			link->prev->next = link->next;

		if (link == list)
			list = list->next;

		link->next = NULL;
		link->prev = NULL;
	}

	return list;
}


nmp_list_t *nmp_list_delete_link(nmp_list_t *list, nmp_list_t *link)
{
	list = nmp_list_remove_link(list, link);
	nmp_list_free_1(link);

	return list;
}


nmp_list_t *nmp_list_concat(nmp_list_t *list1, nmp_list_t *list2)
{
	nmp_list_t *last;

	if (list2)
	{
		last = nmp_list_last(list1);
		if (last)
			last->next = list2;
		else
			list1 = list2;
		list2->prev = last;
	}

	return list1;	
}


nmp_list_t *nmp_list_find(nmp_list_t *list, void *data)
{
	while (list)
	{
		if (list->data == data)
			break;
		list = list->next;
	}

	return list;
}


nmp_list_t *nmp_list_find_custom(nmp_list_t *list, void *data,
	nmp_compare_custom func)
{
	while (list)
	{
		if (!(*func)(list->data, data))
			break;
		list = list->next;
	}

	return list;
}


void nmp_list_foreach(nmp_list_t *list, nmp_visit_custom func, void *data)
{
	nmp_list_t *next;

	while (list)
	{
		next = list->next;
		(*func)(list->data, data);
		list = next;
	}
}


void nmp_list_free_1(nmp_list_t *list)
{
	if (list)
		nmp_del(list, nmp_list_t, 1);
}


void nmp_list_free(nmp_list_t *list)
{
	nmp_list_t *next;

	while (list)
	{
		next = list->next;
		nmp_list_free_1(list);
		list = next;
	}
}



nmp_list_t *nmp_list_first(nmp_list_t *list)
{
	if (list)
	{
		while (list->prev)
			list = list->prev;
	}

	return list;
}


nmp_list_t *nmp_list_last(nmp_list_t *list)
{
	if (list)
	{
		while (list->next)
			list = list->next;
	}

	return list;
}


//: ~End
