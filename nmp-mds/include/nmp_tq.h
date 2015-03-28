#ifndef __NMP_TQ_H__
#define __NMP_TQ_H__

#include <glib.h>
#include "nmp_list_head.h"

#define TQ_INIT(tq, func, v) \
do {\
	INIT_LIST_HEAD(&(tq)->list); \
	(tq)->fun = func; \
	(tq)->data = v; \
} while (0)

typedef void (*tq_func)(void *parm);

typedef struct _JpfTq JpfTq;
struct _JpfTq
{
	LIST_HEAD list;
	tq_func fun;
	void *data;
};

void nmp_init_tq( void );
void nmp_add_tq(JpfTq *tq);

#endif 	/* __NMP_TQ_H__ */
