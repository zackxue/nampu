/*
 * nmp_list.h
 *
 * This file describes list interfaces
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_LIST_H__
#define __NMP_LIST_H__

#include "nmp_types.h"
#include "nmp_macros.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _nmp_list_t nmp_list_t;

struct _nmp_list_t
{
	void *data;
	nmp_list_t *next, *prev;
};

nmp_list_t *nmp_list_add(nmp_list_t *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_add_tail(nmp_list_t *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_remove(nmp_list_t *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_remove_all(nmp_list_t *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_remove_link(nmp_list_t *list, nmp_list_t *link) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_delete_link(nmp_list_t *list, nmp_list_t *link) __ATTR_WARN_UNUSED_RETSULT__;
nmp_list_t *nmp_list_concat(nmp_list_t *list1, nmp_list_t *list2);
nmp_list_t *nmp_list_find(nmp_list_t *list, void *data);
nmp_list_t *nmp_list_find_custom(nmp_list_t *list, void *data, nmp_compare_custom func);

void nmp_list_foreach(nmp_list_t *list, nmp_visit_custom func, void *data);
void nmp_list_free_1(nmp_list_t *list);    /* 释放单个结点 */
void nmp_list_free(nmp_list_t *list);      /* 释放所有结点 */

nmp_list_t *nmp_list_first(nmp_list_t *list);
nmp_list_t *nmp_list_last(nmp_list_t *list);

#define nmp_list_data(list) (((nmp_list_t*)list)->data)
#define nmp_list_next(list) (list ? ((nmp_list_t*)list)->next : NULL)
#define nmp_list_prev(list) (list ? ((nmp_list_t*)list)->prev : NULL)

#ifdef __cplusplus
}
#endif

#endif	/* __NMP_LIST_H__ */
