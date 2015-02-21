#ifndef __J_QUEUE_H__
#define __J_QUEUE_H__

#include "nmp_list.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _JQueue JQueue;
struct _JQueue
{
	JList	*head;
	JList	*tail;
	unsigned count;		
};

JQueue *j_queue_new( void );
void j_queue_init(JQueue *queue);
void j_queue_free(JQueue *queue);
void j_queue_clear(JQueue *queue);
unsigned j_queue_length(JQueue *queue);
void j_queue_push_tail(JQueue *queue, void *data);
void* j_queue_pop_head(JQueue *queue);

void *j_queue_pop_tail(JQueue *queue);
void j_queue_push_head(JQueue *queue, void *data);
void j_queue_foreach(JQueue *queue, JVisitCustom func, void *data);


#ifdef __cplusplus
}
#endif

#endif	/* __J_QUEUE_H__ */
