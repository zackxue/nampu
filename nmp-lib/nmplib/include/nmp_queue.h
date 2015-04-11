#ifndef __NMP_QUEUE_H__
#define __NMP_QUEUE_H__

#include "nmp_list.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nmp_queue nmp_queue_t;
struct _nmp_queue
{
	nmp_list_t	*head;
	nmp_list_t	*tail;
	unsigned    count;		
};

nmp_queue_t *nmp_queue_new( void );
void nmp_queue_init(nmp_queue_t *queue);
void nmp_queue_free(nmp_queue_t *queue);
void nmp_queue_clear(nmp_queue_t *queue);
unsigned nmp_queue_length(nmp_queue_t *queue);
void nmp_queue_push_tail(nmp_queue_t *queue, void *data);
void* nmp_queue_pop_head(nmp_queue_t *queue);

void *nmp_queue_pop_tail(nmp_queue_t *queue);
void nmp_queue_push_head(nmp_queue_t *queue, void *data);
void nmp_queue_foreach(nmp_queue_t *queue, nmp_visit_custom func, void *data);


#ifdef __cplusplus
}
#endif

#endif	/* __NMP_QUEUE_H__ */
