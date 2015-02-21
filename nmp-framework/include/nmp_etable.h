/*
 * nmp_etable.h
 *
 * This is for mod response waiting. thread who invokes
 * jpf_event_request() will sleep, and will not be woken
 * up till the response arriving, or timeout.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __JPF_EVENT_TABLE_H__
#define __JPF_EVENT_TABLE_H__

#include <glib.h>
#include "nmp_sysmsg.h"

#define EVENT_HASH_TABLE_SIZE       128
#define EVENT_WAIT_MILLISECONDS     8000

G_BEGIN_DECLS

typedef struct _JpfEventTable JpfEventTable;

JpfEventTable *jpf_event_table_new(gpointer owner);
void jpf_event_table_destroy(JpfEventTable *table);

/* send request, and wait */
gint jpf_event_request(JpfEventTable *t, JpfSysMsg **msg);

/* invoked by response giver */
gint jpf_event_response(JpfEventTable *table, JpfSysMsg *msg);

G_END_DECLS

#endif  //__JPF_EVENT_TABLE_H__
