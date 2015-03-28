#include <glib.h>

#ifndef __NMP_PENDING_QUEUE_H__
#define __NMP_PENDING_QUEUE_H__

G_BEGIN_DECLS

typedef gpointer PQN_DATA_T;
typedef void (*JpfPQNFunc)(PQN_DATA_T data_1, PQN_DATA_T data_2, PQN_DATA_T data_3);

typedef struct _JpfPQNode JpfPQNode;
struct _JpfPQNode
{
	PQN_DATA_T		data_1;
	PQN_DATA_T		data_2;
	PQN_DATA_T		data_3;

	JpfPQNFunc		func;
};


typedef struct _JpfPendingQueue JpfPendingQueue;
struct _JpfPendingQueue
{
	GList			*list;
};


void nmp_pq_init(JpfPendingQueue *pq);
gint nmp_pq_pending(JpfPendingQueue *pq, PQN_DATA_T data_1, 
	PQN_DATA_T data_2, PQN_DATA_T data_3, JpfPQNFunc fun);
void nmp_pq_graft(JpfPendingQueue *src, JpfPendingQueue *dst);
void nmp_pq_call_and_free(JpfPendingQueue *pq);


G_END_DECLS

#endif /* __NMP_PENDING_QUEUE_H__ */
