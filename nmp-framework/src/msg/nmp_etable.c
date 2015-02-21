/*
 * jpf_etable.c
 *
 * routines for response waiting.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#include "nmp_etable.h"
#include "nmp_appmod.h"
#include "nmp_errno.h"
#include "nmp_wait.h"
#include "nmp_list_head.h"
#include "nmp_debug.h"
#include "nmp_memory.h"

typedef struct _JpfEvent JpfEvent;
struct _JpfEvent    /* Event description */
{
    LIST_HEAD   event_list;
    gint        event_id;
    guint       event_seq;
    gpointer    event_data;

    JpfWait     *wait;  /* For waking up */
};

typedef gint (*JpfEventHash)(JpfEvent *event);


struct _JpfEventTable       /* Event Hash Table */
{
    LIST_HEAD       bucket[EVENT_HASH_TABLE_SIZE];
    JpfEventHash    hash_fn;    /* Hash Func */
    guint           seq_generator;          
    GStaticMutex    lock;

    gpointer        *owner; /* owner */

#ifdef JPF_DEBUG
    gint            events;
#endif
};


static gint
jpf_event_hash(JpfEvent *event)
{
    return event->event_seq;
}


static __inline__ void
jpf_event_table_init(JpfEventTable *t, gpointer owner)
{
    gint i;
    
    for (i = 0; i < EVENT_HASH_TABLE_SIZE; ++i)
        INIT_LIST_HEAD(&t->bucket[i]);

    g_static_mutex_init(&t->lock);
    t->hash_fn = jpf_event_hash;
    t->owner = owner;

#ifdef JPF_DEBUG
    t->events = 0;
#endif
}


JpfEventTable *
jpf_event_table_new(gpointer owner)
{
    JpfEventTable *t;
    G_ASSERT(owner != NULL);

    t = jpf_new0(JpfEventTable, 1);
    if (G_UNLIKELY(!t))
        return NULL;

    jpf_event_table_init(t, owner);

    return t;
}


static __inline__ JpfEvent *
jpf_event_new( void )
{
    JpfEvent *event;

    event = jpf_new0(JpfEvent, 1);
    if (G_UNLIKELY(!event))
        return NULL;

    event->wait = jpf_wait_new();
    if (G_UNLIKELY(!event->wait))
    {
        jpf_free(event);
        return NULL;
    }

    return event;
}


static __inline__ void
jpf_event_release(JpfEvent *event)
{
    G_ASSERT(event != NULL);

    BUG_ON(!event->wait);
    jpf_wait_free(event->wait);
    jpf_free(event);
}


void
jpf_event_table_destroy(JpfEventTable *table)
{//:TODO
    FATAL_ERROR_EXIT;
}



static __inline__ gint
jpf_event_add_to_table(JpfEventTable *table, JpfEvent *event,
    JpfSysMsg *msg)
{
    LIST_HEAD *head;
    gint key;

    g_static_mutex_lock(&table->lock);

    event->event_seq = ++table->seq_generator;
    event->event_id = MSG_GETID(msg);
    MSG_SEQ(msg) = event->event_seq;
    key = (*table->hash_fn)(event);
    head = &table->bucket[key%EVENT_HASH_TABLE_SIZE];
    list_add2(&event->event_list, head);

    g_static_mutex_unlock(&table->lock);
    return 0;
}


static __inline__ void
jpf_event_del_from_table(JpfEventTable *table, JpfEvent *event)
{
    LIST_HEAD *head, *l;
    gint key, found = 0;
    G_ASSERT(table != NULL && event != NULL);

    g_static_mutex_lock(&table->lock);
    
    key = (*table->hash_fn)(event);
    head = &table->bucket[key%EVENT_HASH_TABLE_SIZE];

    list_for_each(l, head)
    {
        JpfEvent *e = list_entry(l, JpfEvent, event_list);
        if (e == event)
        {
            found = 1;
            list_del(&e->event_list);

#ifdef JPF_DEBUG
            --table->events;
#endif
            break;
        }
    }

    g_static_mutex_unlock(&table->lock);

    BUG_ON(!found);
}


static __inline__ void
__jpf_event_wait(JpfEvent *event)
{
    GTimeVal abs_time;
    G_ASSERT(event != NULL);

    g_get_current_time(&abs_time);
    g_time_val_add(&abs_time, EVENT_WAIT_MILLISECONDS * 1000);

    jpf_wait_timed_waiting(event->wait, &abs_time); 
}


static __inline__ void
jpf_event_wait(JpfEvent *event)
{
    G_ASSERT(event != NULL);

    jpf_wait_begin(event->wait);
    if (!jpf_wait_is_cond_true(event->wait))
    {
    	__jpf_event_wait(event);
    }
    jpf_wait_end(event->wait);
}


static __inline__ void
jpf_event_wakeup(JpfEvent *event)
{
    jpf_wait_begin(event->wait);
    jpf_wait_set_cond_true(event->wait);
    jpf_wait_wakeup(event->wait);
    jpf_wait_end(event->wait);  
}


gint
jpf_event_request(JpfEventTable *t, JpfSysMsg **msg)
{
    JpfEvent*event;
    gint ret;
    G_ASSERT(t != NULL && msg != NULL && *msg != NULL);

    event = jpf_event_new();
    if (G_UNLIKELY(!event))
        return -E_NOMEM;

    ret = jpf_event_add_to_table(t, event, *msg);
    if (G_LIKELY(!ret))
    {
        ret = jpf_app_mod_snd((JpfAppMod*)t->owner, *msg);
        if (G_LIKELY(!ret))
            jpf_event_wait(event);

        jpf_event_del_from_table(t, event);
    }

    if (G_LIKELY(!ret))
        *msg = event->event_data;

    jpf_event_release(event);
    return ret;
}


gint
jpf_event_response(JpfEventTable *table, JpfSysMsg *msg)
{
    LIST_HEAD *head, *l;
    JpfEvent *event;
    guint seq, not_found = 1;

    g_static_mutex_lock(&table->lock);

    seq = MSG_SEQ(msg); 
    head = &table->bucket[seq%EVENT_HASH_TABLE_SIZE];

    list_for_each(l, head)
    {
        event = list_entry(l, JpfEvent, event_list);
        if (event->event_seq == seq)
        {
            if (G_UNLIKELY(event->event_id != MSG_GETID(msg)))
            {
                jpf_warning(
                    "<JpfEventTable> event_id:%d not-equ msg_id:%d!",
                    event->event_id, MSG_GETID(msg)
                );
                continue;
            }

            not_found = 0;
            event->event_data = msg;
            jpf_event_wakeup(event);
            break;
        }
    }

    g_static_mutex_unlock(&table->lock);

    return not_found;
}


//:~ End
