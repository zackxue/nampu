/*
 * j_list.h
 *
 * This file describes list interfaces
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __J_LIST_H__
#define __J_LIST_H__

#include "nmp_types.h"
#include "nmp_macros.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _JList JList;

struct _JList
{
	void *data;
	JList *next, *prev;
};

JList *j_list_add(JList *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_add_tail(JList *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_remove(JList *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_remove_all(JList *list, void *data) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_remove_link(JList *list, JList *link) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_delete_link(JList *list, JList *link) __ATTR_WARN_UNUSED_RETSULT__;
JList *j_list_concat(JList *list1, JList *list2);
JList *j_list_find(JList *list, void *data);
JList *j_list_find_custom(JList *list, void *data, JCompareCustom func);

void j_list_foreach(JList *list, JVisitCustom func, void *data);
void j_list_free_1(JList *list);    /* 释放单个结点 */
void j_list_free(JList *list);      /* 释放所有结点 */

JList *j_list_first(JList *list);
JList *j_list_last(JList *list);

#define j_list_data(list) (((JList*)list)->data)
#define j_list_next(list) (list ? ((JList*)list)->next : NULL)
#define j_list_prev(list) (list ? ((JList*)list)->prev : NULL)

#ifdef __cplusplus
}
#endif

#endif	/* __J_LIST_H__ */
