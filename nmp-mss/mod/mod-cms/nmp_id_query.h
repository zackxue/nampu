#ifndef __GUID_QUERY_H__
#define __GUID_QUERY_H__

#include "nmp_guid.h"

G_BEGIN_DECLS


enum
{
	STATE_IDLE,
	STATE_INPROGRESS,
	STATE_RETRY_DELAY
};


typedef struct __NmpIDQueryBlock NmpIDQueryBlock;
struct __NmpIDQueryBlock
{
	/* guids querying */
	NmpGuidSet			*old_set;				/* global unique id set*/
	NmpGuidSet			*new_set;

	gint				running;

	guint				tick_counter;

	gint				state;
	gint				state_ttl;					/* change state when reach 0 */

	gint				total_guids;
	gint				got_guids;
	gint				next_row;

	GMutex				*mutex;
};


NmpIDQueryBlock *nmp_id_qb_new( void );
gint nmp_id_qb_add_entry(NmpIDQueryBlock *qb, gchar *domain, gchar *guid);
void nmp_id_qb_start(NmpIDQueryBlock *qb);
void nmp_id_qb_stop(NmpIDQueryBlock *qb);
void nmp_id_qb_delay(NmpIDQueryBlock *qb);
void nmp_id_qb_tick(NmpIDQueryBlock *qb);
gint nmp_id_qb_set_total(NmpIDQueryBlock *qb, gint total);
gint nmp_id_qb_query_check(NmpIDQueryBlock *qb, gint *start_row, gint *num);
NmpDiffSet *nmp_id_qb_get_diffset(NmpIDQueryBlock *qb);

G_END_DECLS


#endif 	/* __GUID_QUERY_H__ */
