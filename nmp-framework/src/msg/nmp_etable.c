/*
 * nmp_etable.c
 *
 * routines for response waiting.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_etable.h"
#include "nmp_appmod.h"
#include "nmp_share_errno.h"
#include "nmp_share_wait.h"
#include "nmp_list_head.h"
#include "nmp_share_debug.h"
#include "nmp_memory.h"

typedef struct _NmpEvent NmpEvent;
struct _NmpEvent    /* Event description */
{
    LIST_HEAD   event_list;
    gint        event_id;
    guint       event_seq;
    gpointer    event_data;

    NmpWait     *wait;  /* For waking up */
};

typedef gint (*NmpEventHash)(NmpEvent *event);


struct _NmpEventTable       /* Event Hash Table */
{
    LIST_HEAD       bucket[EVENT_HASH_TABLE_SIZE];
    NmpEventHash    hash_fn;    /* Hash Func */
    guint           seq_generator;          
    GStaticMutex    lock;

    gpointer        *owner; /* owner */

#ifdef NMP_DEBUG
    gint            events;
#endif
};


static gint
nmp_event_hash(NmpEvent *event)
{
    return event->event_seq;
}


static __inline__ void
nmp_event_table_init(NmpEventTable *t, gpointer owner)
{
    gint i;
    
    for (i = 0; i < EVENT_HASH_TABLE_SIZE; ++i)
        INIT_LIST_HEAD(&t->bucket[i]);

    g_static_mutex_init(&t->lock);
    t->hash_fn = nmp_event_hash;
    t->owner = owner;

#ifdef NMP_DEBUG
    t->events = 0;
#endif
}


NmpEventTable *
nmp_event_table_new(gpointer owner)
{
    NmpEventTable *t;
    G_ASSERT(owner != NULL);

    t = nmp_new0(NmpEventTable, 1);
    if (G_UNLIKELY(!t))
        return NULL;

    nmp_event_table_init(t, owner);

    return t;
}


static __inline__ NmpEvent *
nmp_event_new( void )
{
    NmpEvent *event;

    event = nmp_new0(NmpEvent, 1);
    if (G_UNLIKELY(!event))
        return NULL;

    event->wait = nmp_wait_new();
    if (G_UNLIKELY(!event->wait))
    {
        nmp_free(event);
        return NULL;
    }

    return event;
}


static __inline__ void
nmp_event_release(NmpEvent *event)
{
    G_ASSERT(event != NULL);

    BUG_ON(!event->wait);
    nmp_wait_free(event->wait);
    nmp_free(event);
}


void
nmp_event_table_destroy(NmpEventTable *table)
{//:TODO
    FATAL_ERROR_EXIT;
}



static __inline__ gint
nmp_event_add_to_table(NmpEventTable *table, NmpEvent *event,
    NmpSysMsg *msg)
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
nmp_event_del_from_table(NmpEventTable *table, NmpEvent *event)
{
    LIST_HEAD *head, *l;
    gint key, found = 0;
    G_ASSERT(table != NULL && event != NULL);

    g_static_mutex_lock(&table->lock);
    
    key = (*table->hash_fn)(event);
    head = &table->bucket[key%EVENT_HASH_TABLE_SIZE];

    list_for_each(l, head)
    {
        NmpEvent *e = list_entry(l, NmpEvent, event_list);
        if (e == event)
        {
            found = 1;
            list_del(&e->event_list);

#ifdef NMP_DEBUG
            --table->events;
#endif
            break;
        }
    }

    g_static_mutex_unlock(&table->lock);

    BUG_ON(!found);
}


static __inline__ void
__nmp_event_wait(NmpEvent *event)
{
    GTimeVal abs_time;
    G_ASSERT(event != NULL);

    g_get_current_time(&abs_time);
    g_time_val_add(&abs_time, EVENT_WAIT_MILLISECONDS * 1000);

    nmp_wait_timed_waiting(event->wait, &abs_time); 
}


static __inline__ void
nmp_event_wait(NmpEvent *event)
{
    G_ASSERT(event != NULL);

    nmp_wait_begin(event->wait);
    if (!nmp_wait_is_cond_true(event->wait))
    {
    	__nmp_event_wait(event);
    }
    nmp_wait_end(event->wait);
}


static __inline__ void
nmp_event_wakeup(NmpEvent *event)
{
    nmp_wait_begin(event->wait);
    nmp_wait_set_cond_true(event->wait);
    nmp_wait_wakeup(event->wait);
    nmp_wait_end(event->wait);  
}


gint
nmp_event_request(NmpEventTable *t, NmpSysMsg **msg)
{
    NmpEvent*event;
    gint ret;
    G_ASSERT(t != NULL && msg != NULL && *msg != NULL);

    event = nmp_event_new();
    if (G_UNLIKELY(!event))
        return -E_NOMEM;

    ret = nmp_event_add_to_table(t, event, *msg);
    if (G_LIKELY(!ret))
    {
        ret = nmp_app_mod_snd((NmpAppMod*)t->owner, *msg);
        if (G_LIKELY(!ret))
            nmp_event_wait(event);

        nmp_event_del_from_table(t, event);
    }

    if (G_LIKELY(!ret))
        *msg = event->event_data;

    nmp_event_release(event);
    return ret;
}


gint
nmp_event_response(NmpEventTable *table, NmpSysMsg *msg)
{
    LIST_HEAD *head, *l;
    NmpEvent *event;
    guint seq, not_found = 1;

    g_static_mutex_lock(&table->lock);

    seq = MSG_SEQ(msg); 
    head = &table->bucket[seq%EVENT_HASH_TABLE_SIZE];

    list_for_each(l, head)
    {
        event = list_entry(l, NmpEvent, event_list);
        if (event->event_seq == seq)
        {
            if (G_UNLIKELY(event->event_id != MSG_GETID(msg)))
            {
                nmp_warning(
                    "<NmpEventTable> event_id:%d not-equ msg_id:%d!",
                    event->event_id, MSG_GETID(msg)
                );
                continue;
            }

            not_found = 0;
            event->event_data = msg;
            nmp_event_wakeup(event);
            break;
        }
    }

    g_static_mutex_unlock(&table->lock);

    return not_found;
}


//:~ End
