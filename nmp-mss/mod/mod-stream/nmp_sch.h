#ifndef __NMP_STREAM_CHANNEL_H__
#define __NMP_STREAM_CHANNEL_H__

#include "nmp_sbuff.h"
#include "nmp_pool.h"

#define TICK_PER_SECOND		5
#define EC_STM_NOT_FOUND	404

enum {
	ST_PREPARE, ST_URL, ST_REQUEST, ST_RECORD, ST_DYING
};

typedef struct __NmpSchPool NmpSchPool;
typedef struct __NmpSchOps NmpSchOps;
typedef struct __NmpSCh NmpSCh;


struct __NmpSchOps
{
	gint (*init)(NmpSCh *sch);
	gint (*open_stream)(NmpSCh *sch);
	gint (*close_stream)(NmpSCh *sch);
	gint (*fin)(NmpSCh *sch);
};


struct __NmpSchPool
{
	NmpPool	parent;
	NmpSchOps *sch_ops;		/* ops template */
	gint	uri_tokens;
	gint	stm_tokens;
};


struct __NmpSCh				/* stream channel */
{
	NmpGuid		guid;		/* channel id */
	gint		ref_count;

	NmpSchOps 	*sch_ops;

	gint		state;
	gpointer	state_data;

	gint		level;
	gchar		*uri;
	gint		rec_type;

	gint		recording;
	gint		err_code;

	gint		stranger;		/* hik? dah? */
	gint		stranger_pri;

	gint		local_flags;	/* hd group etc. */
	NmpSBuff	*sb;

	gint		initialized;	/* init()*/
	gpointer	private;

	NmpSchPool	*pool;
	GMutex		*mutex;
};


NmpSchPool *nmp_spool_new(gint freq_msec, NmpSchOps *ops);

gint nmp_spool_add_sch(NmpSchPool *sp, NmpGuid *guid,
	gint rec, gint hd_grp, gint level);

gint nmp_spool_add_sch_uri(NmpSchPool *sp, NmpGuid *guid,
	gchar *uri);

gint nmp_spool_del_sch(NmpSchPool *sp, NmpGuid *guid);

gint nmp_spool_get_uri_token(NmpSchPool *sp);
gint nmp_spool_get_stm_token(NmpSchPool *sp);

gint nmp_sch_set_rec_state(NmpSCh *sch);
void nmp_spool_get_record_status(NmpSchPool *sp, NmpGuid *guid,
	gint *status, gint *status_code);

#endif	/* __NMP_STREAM_CHANNEL_H__ */
