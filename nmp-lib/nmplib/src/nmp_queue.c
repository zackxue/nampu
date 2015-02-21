//#include <stdlib.h>
#include <string.h>
#include "nmp_queue.h"
#include "nmp_mem.h"

#ifdef _DMALLOC
#include "dmalloc.h"
#endif

void j_queue_init(JQueue *queue)
{
	J_ASSERT(queue != NULL);

	memset(queue, 0, sizeof(*queue));
}


JQueue *j_queue_new( void )
{
	JQueue *queue = j_new(JQueue, 1);
	j_queue_init(queue);
	return queue;
}


void j_queue_free(JQueue *queue)
{
	J_ASSERT(queue != NULL);

	j_list_free(queue->head);
	j_del(queue, JQueue, 1);
}


void j_queue_clear(JQueue *queue)
{
	J_ASSERT(queue != NULL);

	j_list_free(queue->head);
	j_queue_init(queue);
}


unsigned j_queue_length(JQueue *queue)
{
	J_ASSERT(queue != NULL);

	return queue->count;
}


void j_queue_push_tail(JQueue *queue, void *data)
{
	J_ASSERT(queue != NULL);

	queue->tail = j_list_add_tail(queue->tail, data);
	if (queue->tail->next)
		queue->tail = queue->tail->next;
	else
		queue->head = queue->tail;
	++queue->count;
}


void* j_queue_pop_head(JQueue *queue)
{
	JList *node;
	void *data;
	J_ASSERT(queue != NULL);

	if (queue->head)
	{
		node = queue->head;
		data = j_list_data(node);

		queue->head = j_list_next(node);
		if (queue->head)
			queue->head->prev = NULL;
		else
			queue->tail = NULL;

		--queue->count;
		j_list_free_1(node);
		return data;
	}

	return NULL;
}


void *j_queue_pop_tail(JQueue *queue)
{
	JList *node;
	void *data;
	
	J_ASSERT(queue != NULL);

	if(!queue->tail)	return NULL;

	node = queue->tail ;
	data = j_list_data(node);
	
	if(queue->tail == queue->tail )
	{
		queue->head  = queue->tail = NULL; 
	}
	else
	{
		queue->tail = j_list_prev(node);
	}
	--queue->count;
	j_list_free_1(node);
	return data;
}

void j_queue_push_head(JQueue *queue, void *data)
{
	J_ASSERT(queue != NULL);

	if(queue->head)
	{
		queue->head = j_list_add(queue->head, data);
	}
	else
	{
		queue->head = j_list_add(queue->head, data);
		queue->tail = queue->head;
	}
	++queue->count;
}
void j_queue_foreach(JQueue *queue, JVisitCustom func, void *data)
{
	J_ASSERT(queue != NULL);
	
	if(queue->head)
	{
		j_list_foreach(queue->head, func, data);
	}
}



//:~ End
