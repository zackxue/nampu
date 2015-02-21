/*
 * j_netproto.h
 *
 * This file describes interfaces of net protocol parser. 
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
 *
*/
#ifndef __JPF_NETPROTO_H__
#define __JPF_NETPROTO_H__

typedef struct _JNetPackInfo JNetPackInfo;

/*
 * Payload information.
*/
struct _JNetPackInfo
{
	unsigned	total_packets;			//@{total packets, for fragmentation}
	unsigned	packet_no;				//@{current packet no, for fragmentation}

    char        *start;                 //@{start of payload}
    size_t      size;                   //@{payload size}
    void        *private_from_low_layer; //@{private data from packet header}
};


typedef struct _JPayloadProto JPayloadProto;
struct _JPayloadProto
{
    /*
     * parse the payload information, it is always in XML format. return
     * the internal msg object.
     * $Ret: !NULL, binary format of payload.
     *        NULL, ignored.
    */
	void *(*get_payload)(char *start, size_t size, void *from_lower);


    /*
     * format payload information from internal data to net transferable data.
     * @Ret: < 0, errno;
     *       >=0, payload size
    */
	int (*put_payload)(void *pack_data,  char buf[], size_t size);

	/*
	 * if the msg object return by .get_payload() delivers failed, we destroy
	 * it. these are all invoked with #watch->lock held.
	*/
	void (*destroy_pointer)(void *msg, int err);
};


typedef struct _JPacketProto JPacketProto;
struct _JPacketProto
{
    /*
     * check whether a full packet has come, if so, return
     * the length of the packet.
     * @Ret: > 0 a full packet length, the packet will be received.
     *       = 0 data not enough for a packet, nothing to do.
     *       < 0 error, it will be reported to upper layer. 
    */
    int (*check)(char *start, char *end);

    /*
     * unpack, return the payload information by #info.
     * @Ret:  0 success, payload() will be invoked. 
     *       !0 error, it will be reported to upper layer.
    */
    int (*unpack)(char *start, char *end,  JNetPackInfo *info);

    /*
     * create net protocol packet.
     * @ret: < 0, errno
     *       >=0, packet head len
    */
    int (*pack)(void *pack_data, size_t payload_len,
    	char buff[], size_t buff_size);
};

#endif  //__JPF_NETPROTO_H__
