#include <glib.h>

#ifndef __NMP_PENDING_QUEUE_H__
#define __NMP_PENDING_QUEUE_H__

G_BEGIN_DECLS

typedef gpointer PQN_DATA_T;
typedef void (*NmpPQNFunc)(PQN_DATA_T data_1, PQN_DATA_T data_2, PQN_DATA_T data_3);

typedef struct _NmpPQNode NmpPQNode;
struct _NmpPQNode
{
	PQN_DATA_T		data_1;
	PQN_DATA_T		data_2;
	PQN_DATA_T		data_3;

	NmpPQNFunc		func;
};


typedef struct _NmpPendingQueue NmpPendingQueue;
struct _NmpPendingQueue
{
	GList			*list;
};


void nmp_pq_init(NmpPendingQueue *pq);
gint nmp_pq_pending(NmpPendingQueue *pq, PQN_DATA_T data_1, 
	PQN_DATA_T data_2, PQN_DATA_T data_3, NmpPQNFunc fun);
void nmp_pq_graft(NmpPendingQueue *src, NmpPendingQueue *dst);
void nmp_pq_call_and_free(NmpPendingQueue *pq);


G_END_DECLS

#endif /* __NMP_PENDING_QUEUE_H__ */
