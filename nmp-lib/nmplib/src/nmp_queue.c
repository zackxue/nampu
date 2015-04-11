//#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "nmp_queue.h"
#include "nmp_mem.h"

#ifdef _DMALLOC
#include "dmalloc.h"
#endif

void nmp_queue_init(nmp_queue_t *queue)
{
	NMP_ASSERT(queue != NULL);

	memset(queue, 0, sizeof(*queue));
}


nmp_queue_t *nmp_queue_new( void )
{
	nmp_queue_t *queue = nmp_new(nmp_queue_t, 1);
	nmp_queue_init(queue);
	return queue;
}


void nmp_queue_free(nmp_queue_t *queue)
{
	NMP_ASSERT(queue != NULL);

	nmp_list_free(queue->head);
	nmp_del(queue, nmp_queue_t, 1);
}


void nmp_queue_clear(nmp_queue_t *queue)
{
	NMP_ASSERT(queue != NULL);

	nmp_list_free(queue->head);
	nmp_queue_init(queue);
}


unsigned nmp_queue_length(nmp_queue_t *queue)
{
	NMP_ASSERT(queue != NULL);

	return queue->count;
}


void nmp_queue_push_tail(nmp_queue_t *queue, void *data)
{
	NMP_ASSERT(queue != NULL);

	queue->tail = nmp_list_add_tail(queue->tail, data);
	if (queue->tail->next)
		queue->tail = queue->tail->next;
	else
		queue->head = queue->tail;
	++queue->count;
}


void* nmp_queue_pop_head(nmp_queue_t *queue)
{
	nmp_list_t *node;
	void *data;
	NMP_ASSERT(queue != NULL);

	if (queue->head)
	{
		node = queue->head;
		data = nmp_list_data(node);

		queue->head = nmp_list_next(node);
		if (queue->head)
			queue->head->prev = NULL;
		else
			queue->tail = NULL;

		--queue->count;
		nmp_list_free_1(node);
		return data;
	}

	return NULL;
}


void *nmp_queue_pop_tail(nmp_queue_t *queue)
{
	nmp_list_t *node;
	void *data;
	
	NMP_ASSERT(queue != NULL);

	if(!queue->tail)	return NULL;

	node = queue->tail ;
	data = nmp_list_data(node);
	
	if(queue->tail == queue->tail )
	{
		queue->head  = queue->tail = NULL; 
	}
	else
	{
		queue->tail = nmp_list_prev(node);
	}
	--queue->count;
	nmp_list_free_1(node);
	return data;
}

void nmp_queue_push_head(nmp_queue_t *queue, void *data)
{
	NMP_ASSERT(queue != NULL);

	if(queue->head)
	{
		queue->head = nmp_list_add(queue->head, data);
	}
	else
	{
		queue->head = nmp_list_add(queue->head, data);
		queue->tail = queue->head;
	}
	++queue->count;
}

void nmp_queue_foreach(nmp_queue_t *queue, nmp_visit_custom func, void *data)
{
	NMP_ASSERT(queue != NULL);
	
	if(queue->head)
	{
		nmp_list_foreach(queue->head, func, data);
	}
}



//:~ End
