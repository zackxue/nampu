/*
 * nmp_etable.h
 *
 * This is for mod response waiting. thread who invokes
 * jpf_event_request() will sleep, and will not be woken
 * up till the response arriving, or timeout.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_EVENT_TABLE_H__
#define __NMP_EVENT_TABLE_H__

#include <glib.h>
#include "nmp_sysmsg.h"

#define EVENT_HASH_TABLE_SIZE       128
#define EVENT_WAIT_MILLISECONDS     8000

G_BEGIN_DECLS

typedef struct _NmpEventTable NmpEventTable;

NmpEventTable *jpf_event_table_new(gpointer owner);
void jpf_event_table_destroy(NmpEventTable *table);

/* send request, and wait */
gint jpf_event_request(NmpEventTable *t, NmpSysMsg **msg);

/* invoked by response giver */
gint jpf_event_response(NmpEventTable *table, NmpSysMsg *msg);

G_END_DECLS

#endif  //__NMP_EVENT_TABLE_H__
