/*
 * jpf_fragments.c
 *
 * Routines for packet defragmentation. 
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
*/

#include <string.h>
#include "nmp_fragments.h"
#include "nmp_debug.h"
#include "nmp_timer.h"
#include "nmp_netproto.h"

#define MAX_PACKET_FRAGMENTS            64
#define MAX_DEFRAG_TIMEOUT_S            10
#define MAX_DEFRAG_TIMEOUT_M            (MAX_DEFRAG_TIMEOUT_S * 1000)

typedef struct _HmPacketFrags HmPacketFrags;
struct _HmPacketFrags
{
    gpointer            packet_identity;    /* packet seq num */
    guint               total;              /* total packets */
    guint               eat;                /* packets already enqueue */
    gsize               total_size;         /* total packet size */

    GList               *packets;           /* fragmental packets list */

    guint               timer_id;           /* time expires */
};


typedef struct _HmDefrags HmDefrags;
struct _HmDefrags
{
    GList               *pl;                /* packet frags list */
    GStaticMutex        lock;               /* lock */
};


static __inline__ void
jpf_net_packet_release_frags(HmPacketFrags *frags);


static HmDefrags packet_fragments = 
{
    NULL,
    G_STATIC_MUTEX_INIT
};


static __inline__ gboolean
__jpf_net_packet_del_frags(HmDefrags *fragments, 
    HmPacketFrags *frags)
{
    GList *list;

    list = g_list_find(fragments->pl, frags);
    if (list)
    {
        jpf_net_packet_release_frags(frags);
        fragments->pl = g_list_delete_link(fragments->pl, list);
        return TRUE;
    }

    return FALSE;
}


static __inline__ gboolean
jpf_net_packet_del_frags(HmDefrags *fragments, 
    HmPacketFrags *frags)
{
    gboolean ret;
    G_ASSERT(fragments != NULL && frags != NULL);

    g_static_mutex_lock(&fragments->lock);
    ret = __jpf_net_packet_del_frags(fragments, frags);
    g_static_mutex_unlock(&fragments->lock);

    return ret;
}


static gboolean
jpf_net_packet_defrag_timer(gpointer user_data)
{
    HmPacketFrags *frags = (HmPacketFrags*)user_data;

    jpf_net_packet_del_frags(&packet_fragments, frags); 
    return FALSE;
}


static __inline__ gint
jpf_net_packet_sanity_test(HmPacketFrags *frags)
{
    JpfNetPackInfo *npi;
    GList *list;
    guint no = 1;
    gsize size = 0;

    list = frags->packets;
    while (list != NULL)
    {
        npi = (JpfNetPackInfo*)list->data;

        if (npi->packet_no != no)
            return 1;
        ++no;
        size += npi->size;
	list = g_list_next(list);
    }

    return size != frags->total_size;
}


static __inline__ JpfNetPackInfo *
jpf_net_packet_new_npi(gsize total_size)
{
    JpfNetPackInfo *npi;

    npi = g_new0(JpfNetPackInfo, 1);
    if (G_UNLIKELY(!npi))
        return NULL;    /* glib has its own oom facility */

    npi->start = g_malloc(total_size);
    if (G_UNLIKELY(!npi->start))
    {
        g_free(npi);
        return NULL;
    }

    npi->size = total_size;
    npi->total_packets = 1;
    npi->packet_no = 1;

    return npi;
}


static __inline__ JpfNetPackInfo *
jpf_net_packet_copy_npi(JpfNetPackInfo *npi)
{
    JpfNetPackInfo *n;

    if (G_UNLIKELY(!npi))
        return NULL;

    n = jpf_net_packet_new_npi(npi->size);
    if (G_UNLIKELY(!n))
        return NULL;

    n->total_packets = npi->total_packets;
    n->packet_no = npi->packet_no;
    n->private_from_low_layer = npi->private_from_low_layer;
    memcpy(n->start, npi->start, npi->size);

    return n;
}


void
jpf_net_packet_release_npi(JpfNetPackInfo *npi)
{
    G_ASSERT(npi != NULL);

    g_free(npi->start);
    g_free(npi);
}


static __inline__ HmPacketFrags *
jpf_net_packet_new_frags(gpointer identity, guint total_packet)
{
    HmPacketFrags *frags;

    frags = g_new0(HmPacketFrags, 1);
    if (G_UNLIKELY(!frags))
        return NULL;

    frags->packet_identity = identity;
    frags->total = total_packet;
    frags->timer_id = jpf_set_timer(
        MAX_DEFRAG_TIMEOUT_M,
        jpf_net_packet_defrag_timer,
        frags
    );

    return frags;
}


static __inline__ void
jpf_net_packet_release_frags(HmPacketFrags *frags)
{
    GList *list;
    G_ASSERT(frags != NULL);

    list = frags->packets;
    while (list != NULL)
    {
        jpf_net_packet_release_npi(
            (JpfNetPackInfo*)list->data
        );
        list = g_list_next(list);
    }

    g_list_free(frags->packets);
    jpf_del_timer(frags->timer_id);
    g_free(frags);
}


static __inline__ JpfNetPackInfo *
jpf_net_packet_linearize(HmPacketFrags *frags)
{
    gint pos = 0;
    GList *list;
    JpfNetPackInfo *npi, *pi;
    gpointer identity = NULL;

    if (jpf_net_packet_sanity_test(frags))
    {
        jpf_warning(
            "packet-%u sanity test failed!", 
            (guint)frags->packet_identity
        );
        return NULL;
    }

    npi = jpf_net_packet_new_npi(frags->total_size);
    if (npi)
    {
        list = frags->packets;
        while (list != NULL)
        {
            pi = (JpfNetPackInfo*)list->data;
            memcpy(&npi->start[pos], pi->start, pi->size);
            pos += pi->size;

            if (!identity)
                identity = pi->private_from_low_layer;

            list = g_list_next(list);
        }

        npi->private_from_low_layer = identity;
    }

    return npi;
}


static gint
jpf_net_packet_find_next_one(gconstpointer a, gconstpointer b)
{
    JpfNetPackInfo *npi = (JpfNetPackInfo*)a;
    guint no = (guint)b;

    if (npi->packet_no > no)
        return 0;

    return 1;
}


static __inline__ JpfNetPackInfo *
jpf_net_packet_enqueue(HmPacketFrags *frags, JpfNetPackInfo *np)
{
    JpfNetPackInfo *np_copy, *np_ret;
    GList *next;

    np_copy = jpf_net_packet_copy_npi(np);
    if (G_UNLIKELY(!np_copy))
    {
        jpf_warning(
            "copy npi failed while enqueue packet!"
        );

        return np;  /* return the original one */
    }

    if (!frags->eat)
    {
        frags->packets = g_list_prepend(
            frags->packets,
            np_copy
        );
    }
    else
    {
        next = g_list_find_custom(
            frags->packets,
            (gpointer)np->packet_no,
            jpf_net_packet_find_next_one
        );

        frags->packets = g_list_insert_before(
            frags->packets,
            next,
            np_copy
        );
    }

    ++frags->eat;
    frags->total_size += np->size;

    if (frags->eat >= frags->total)
    {
        np_ret = jpf_net_packet_linearize(frags);
        if (!np_ret)
            np_ret = np;
        return np_ret;
    }

    return NULL;
}


static gint
jpf_net_packet_find_frags(gconstpointer a, gconstpointer b)
{
    HmPacketFrags *frags = (HmPacketFrags*)a;

    if (frags->packet_identity == b)
        return 0;

    return 1;
}


static __inline__ JpfNetPackInfo *
__jpf_net_packet_cached(HmDefrags *fragments, JpfNetPackInfo *net_pack)
{
    JpfNetPackInfo *np;
    GList *list;
    HmPacketFrags *frags;
    
    list = g_list_find_custom(
        fragments->pl,
        net_pack->private_from_low_layer,
        jpf_net_packet_find_frags
    );

    if (list)
    {
        frags = (HmPacketFrags*)list->data;
        np = jpf_net_packet_enqueue(frags, net_pack);
        if (np)
        {
            jpf_net_packet_release_frags(frags);

            fragments->pl = g_list_delete_link(
                fragments->pl,
                list
            );

            if (np == net_pack)
                np = NULL;      /* always means error occurred */
        }

        return np;
    }
    else
    {
        if (net_pack->packet_no != 1)
        {
            jpf_print(
                "received a fragmentary packet, drop!"
            );
            return NULL;
        }

        if (net_pack->total_packets > MAX_PACKET_FRAGMENTS)
        {
            jpf_warning(
                "too many fragments, drop!"
            );

            return NULL;
        }

        frags = jpf_net_packet_new_frags(
            net_pack->private_from_low_layer,
            net_pack->total_packets
        );

        if (G_UNLIKELY(!frags))
        {
            jpf_warning(
                "alloc frags object failed!"
            );
            return NULL;
        }

        np = jpf_net_packet_enqueue(frags, net_pack);
        if (np)
        {
            jpf_net_packet_release_frags(frags);

            if (np == net_pack)
                np = NULL;
        }
	else
	{
	    fragments->pl = g_list_prepend(
	        fragments->pl,
	        frags
	    );
	}

        return np;
    }
}


static __inline__ JpfNetPackInfo *
jpf_net_packet_cached(HmDefrags *fragments, JpfNetPackInfo *net_pack)
{
    JpfNetPackInfo *pack;

    g_static_mutex_lock(&fragments->lock);
    pack = __jpf_net_packet_cached(fragments, net_pack);
    g_static_mutex_unlock(&fragments->lock);

    return pack;
}


JpfNetPackInfo *
jpf_net_packet_defrag(JpfNetPackInfo *net_packet)
{
    G_ASSERT(net_packet != NULL);

    if (net_packet->total_packets <= 1)
        return net_packet;

   if (G_UNLIKELY(net_packet->packet_no > net_packet->total_packets))
   {
       jpf_print(
	    "received a out of order packet %d-%d",
	    net_packet->total_packets,
	    net_packet->packet_no
		);

   		return NULL;
   }

    return jpf_net_packet_cached(&packet_fragments, net_packet);
}


//:~ End
