/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_PENDING_QUEUE_H__
#define __TINY_RAIN_PENDING_QUEUE_H__

#include "def.h"
#include "list.h"

BEGIN_NAMESPACE

typedef void (*pq_fun)(void *parm1, void *parm2, void *parm3);

typedef struct __pq_node pq_node;
struct __pq_node
{
	struct list_head list;
	void *parm1, *parm2, *parm3;
	pq_fun fun;
};

typedef struct __pq pq;
struct __pq
{
	struct list_head head;
};

void pq_init(pq *q);
int32_t pq_pend(pq *q, void *parm1, void *parm2, void *parm3, pq_fun f);
void pq_call_and_clear(pq *q);
void pq_graft(pq *src, pq *dst);

END_NAMESPACE

#endif	//__TINY_RAIN_PENDING_QUEUE_H__
