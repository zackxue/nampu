#include "pq.h"
#include "alloc.h"

void pq_init(pq *q)
{
	INIT_LIST_HEAD(&q->head);
}


int32_t pq_pend(pq *q, void *parm1, void *parm2,
	void *parm3, pq_fun f)
{
	int32_t err = -ENOMEM;
	pq_node *n = (pq_node*)tr_alloc(sizeof(*n));

	if (n)
	{
		INIT_LIST_HEAD(&n->list);
		n->parm1 = parm1;
		n->parm2 = parm2;
		n->parm3 = parm3;
		n->fun = f;

		list_add_tail(&n->list, &q->head);
		err = 0;
	}

	return err;
}


void pq_call_and_clear(pq *q)
{
	struct list_head *l;
	pq_node *n;

	while (!list_empty(&q->head))
	{
		l = q->head.next;
		list_del(l);

		n = list_entry(l, pq_node, list);
		if (n->fun)
		{
			(*n->fun)(n->parm1, n->parm2, n->parm3);
		}

		tr_free(n, sizeof(*n));
	}
}


void pq_graft(pq *src, pq *dst)
{
	list_add(&dst->head, &src->head);
	list_del_init(&src->head);
}


//:~ End