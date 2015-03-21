/*
 * nmp_netproto.h
 *
 * This file describes interfaces of net protocol parser. 
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
*/
#ifndef __NMP_NETPROTO_H__
#define __NMP_NETPROTO_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _NmpNetPackInfo NmpNetPackInfo;

/*
 * Payload information.
*/
struct _NmpNetPackInfo
{
	guint				total_packets;			//@{total packets, for fragmentation}
	guint				packet_no;				//@{current packet no, for fragmentation}

    gchar               *start;                 //@{start of payload}
    gsize               size;                   //@{payload size}
    gpointer            private_from_low_layer; //@{private data from packet header}
};


typedef struct _NmpPayloadProto NmpPayloadProto;
struct _NmpPayloadProto
{
    /*
     * parse the payload information, it is always in XML format. return
     * the internal msg object.
     * $Ret: !NULL, binary format of payload.
     *        NULL, ignored.
    */
	gpointer (*get_payload)(gchar *start, gsize size, gpointer from_lower);


    /*
     * format payload information from internal data to net transferable data.
     * @Ret: < 0, errno;
     *       >=0, payload size
    */
	gint (*put_payload)(gpointer pack_data,  gchar buf[], gsize size);

	/*
	 * if the msg object return by .get_payload() delivers failed, we destroy
	 * it. these are all invoked with #watch->lock held.
	*/
	void (*destroy_pointer)(gpointer msg, gint err);
};


typedef struct _NmpPacketProto NmpPacketProto;
struct _NmpPacketProto
{
    /*
     * check whether a full packet has come, if so, return
     * the length of the packet.
     * @Ret: > 0 a full packet length, the packet will be received.
     *       = 0 data not enough for a packet, nothing to do.
     *       < 0 error, it will be reported to upper layer. 
    */
    gint (*check)(gchar *start, gchar *end);

    /*
     * unpack, return the payload information by #info.
     * @Ret:  0 success, payload() will be invoked. 
     *       !0 error, it will be reported to upper layer.
    */
    gint (*unpack)(gchar *start, gchar *end,  NmpNetPackInfo *info);

    /*
     * create net protocol packet.
     * @ret: < 0, errno
     *       >=0, packet head len
    */
    gint (*pack)(gpointer pack_data, gsize payload_len,
    	gchar buff[], gsize buff_size);
};


G_END_DECLS

#endif  //__NMP_NETPROTO_H__
