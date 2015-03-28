#ifndef __NMP_GU_H__
#define __NMP_GU_H__

#include <glib.h>
#include "nmp_rec.h"
#include "nmp_policy.h"
#include "nmp_msg.h"


typedef struct __NmpTrigRecCtl NmpTrigRecCtl;
struct __NmpTrigRecCtl	/* triggered record, alarm/manual */
{
	time_t	start;
	time_t	lasts;
};


typedef struct __NmpGu NmpGu;
struct __NmpGu
{
	NmpGuid	guid;

	gint	state;		/* gu info state */
	gpointer	state_data;

	gint	level;		/* rate level */
	gint	last_level;	/* last used */
	gint	transport;	/* udp/rtp over rtsp */
	gint	rec_state;	/* rec-type submitted */

	NmpPolicy	*time_policy;
	gint	policy_stale;		/* policy stale state */

	NmpTrigRecCtl	*alarm;
	NmpTrigRecCtl	*manual;
};


typedef void (*NmpEventHandler)(gpointer priv, NmpMsg *msg);

typedef struct __NmpGuPool NmpGuPool;
struct __NmpGuPool
{
	guint	size;

	GHashTable	*hash_table;
	GMutex	*mutex;

	guint	timer;
	gint	dead_gus;
	gint	ready;

	NmpEventHandler ev_handler;
	gpointer ev_private;

	gint policy_tokens;
	FILE	*dump_fp;
	gint	 dump_total;
};

NmpGuPool *nmp_new_gu_pool( void );

gint nmp_gu_pool_set_ready(NmpGuPool *p);

gint nmp_gu_pool_add_gu(NmpGuPool *p, NmpGuid *guid);
gint nmp_gu_pool_del_gu(NmpGuPool *p, NmpGuid *guid);

gint nmp_gu_pool_add_policy(NmpGuPool *p, NmpGuid *guid, gint level,
	NmpPolicy *policy);

gint nmp_gu_pool_flush_policy(NmpGuPool *p, NmpGuid *guid);
gint nmp_gu_pool_flush_all_policy(NmpGuPool *p);

gint nmp_gu_pool_get_record_status(NmpGuPool *p, NmpGuid *guid,
	gint *recording);

void nmp_gu_pool_set_handler(NmpGuPool *p, NmpEventHandler handler,
	gpointer priv);

gint nmp_gu_pool_start_alarm(NmpGuPool *p, NmpGuid *guid, gint seconds);
gint nmp_gu_pool_start_manual(NmpGuPool *p, NmpGuid *guid, gint seconds);

#endif	/* __NMP_GU_H__ */
