#ifndef __NMP_MDS_H__
#define __NMP_MDS_H__

#include "nmp_pool.h"

#define TICK_PER_SECOND		5

typedef struct __NmpMdsPool NmpMdsPool;
typedef struct __NmpMdsOps NmpMdsOps;
typedef struct __NmpMds NmpMds;


struct __NmpMdsOps
{
	gint (*init_pool)(NmpMdsPool *p);
	void (*fin_pool)(NmpMdsPool *p);

	gint (*init_inst)(NmpMds *mds);
	gint (*connect)(NmpMds *mds);
	gint (*disconnect)(NmpMds *mds);
	gint (*fin)(NmpMds *mds);
};


struct __NmpMdsPool
{
	NmpPool	parent;
	NmpMdsOps *mds_ops;
	gint	con_tokens;
	void	*private;
};


struct __NmpMds				/* stream channel */
{
	gchar		*id;
	gint		ref_count;

	NmpMdsOps 	*mds_ops;

	gint		state;
	gpointer	state_data;

	gchar		*ip;
	gint		port;

	gint		connected;

	gint		initialized;
	gpointer	private;

	NmpMdsPool	*pool;
	GMutex		*mutex;
};


NmpMdsPool *nmp_spool_mds_new(gint freq_msec, NmpMdsOps *ops);
gint nmp_spool_add_mds(NmpMdsPool *sp, gchar *id);
gint nmp_spool_add_mds_ip(NmpMdsPool *sp, gchar *id, gchar *ip, gint port);
gint nmp_spool_del_mds(NmpMdsPool *sp, gchar *id);
gint nmp_spool_get_conn_token(NmpMdsPool *sp);


void nmp_mds_ref(NmpMds *mds);
void nmp_mds_unref(NmpMds *mds);

#endif	/* __NMP_MDS_H__ */
