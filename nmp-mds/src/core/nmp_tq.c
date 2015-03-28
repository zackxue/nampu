#include "nmp_tq.h"
#include "nmp_timer.h"

#define TQ_RUN_FREQ 	1

static LIST_HEAD(tq_list);
static GStaticMutex tq_mutex = G_STATIC_MUTEX_INIT;


static void
nmp_run_tq( void )
{
	LIST_HEAD tmp, *l;
	JpfTq *tq;

	g_static_mutex_lock(&tq_mutex);
	list_add(&tmp, &tq_list);
	list_del_init(&tq_list);
	g_static_mutex_unlock(&tq_mutex);

	while (!list_empty(&tmp))
	{
		l = tmp.next;
		list_del(l);

		tq = list_entry(l, JpfTq, list);
		if (tq->fun)
		{
			(*tq->fun)(tq->data);
		}

		g_static_mutex_lock(&tq_mutex);
		list_add(l, &tq_list);
		g_static_mutex_unlock(&tq_mutex);
	}
}


void
nmp_add_tq(JpfTq *tq)
{
	g_static_mutex_lock(&tq_mutex);
	list_add(&tq->list, &tq_list);
	g_static_mutex_unlock(&tq_mutex);
}


static gboolean
nmp_tq_timer(gpointer user_data)
{
	nmp_run_tq();
	return TRUE;
}


void
nmp_init_tq( void )
{
	nmp_set_timer(
		TQ_RUN_FREQ * 1000,
		nmp_tq_timer,
		NULL
	);	
}


//:~ End
