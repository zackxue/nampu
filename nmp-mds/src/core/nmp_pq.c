#include "nmp_pq.h"

void
nmp_pq_init(JpfPendingQueue *pq)
{
	g_assert(pq != NULL);

	pq->list = NULL;
}


gint
nmp_pq_pending(JpfPendingQueue *pq, PQN_DATA_T data_1, 
	PQN_DATA_T data_2, PQN_DATA_T data_3, JpfPQNFunc fun)
{
	JpfPQNode *node;

	node = g_new0(JpfPQNode, 1);
	node->data_1 = data_1;
	node->data_2 = data_2;
	node->data_3 = data_3;
	node->func = fun;

	pq->list = g_list_append(pq->list, node);
	return 0;
}


void
nmp_pq_graft(JpfPendingQueue *src, JpfPendingQueue *dst)
{
	g_assert(src != NULL && dst != NULL);

	dst->list = src->list;
	src->list = NULL;
}


static __inline__ void
__nmp_pq_call(JpfPendingQueue *pq, gboolean free)
{
	JpfPQNode *n;
	GList *list;

	list = pq->list;
	while (list)
	{
		n = (JpfPQNode*)list->data;
		(*n->func)(n->data_1, n->data_2, n->data_3);

		if (free)
			g_free(n);

		list = g_list_next(list);
	}

	if (free)
		g_list_free(pq->list);
}


void
nmp_pq_call_and_free(JpfPendingQueue *pq)
{
	g_assert(pq != NULL);

	__nmp_pq_call(pq, TRUE);
}


//:~ End
