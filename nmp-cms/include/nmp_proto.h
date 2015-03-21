/*
 * nmp_proto.h
 *
 * nmp private proto, private packet header.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_PROTO_H__
#define __NMP_PROTO_H__

#include <stdint.h>
#include "nmp_netproto.h"

#define NMP_PROTO_HEAD_MAGIC	0x6a786a2d

#define SET_PROTO_HEAD_M(head) \
	(head)->magic = htonl(NMP_PROTO_HEAD_MAGIC)
#define SET_PROTO_HEAD_S(head, seq) \
	(head)->seq = htonl(seq)
#define SET_PROTO_HEAD_L(head, len) \
	(head)->payload_len = htonl(len)
#define SET_PROTO_HEAD_P(head, packets) \
	(head)->tot_packets = packets
#define SET_PROTO_HEAD_N(head, no) \
	(head)->packet_no = no

#define VALID_PROTO_HEAD(head)	\
	ntohl((head)->magic) == NMP_PROTO_HEAD_MAGIC
#define GET_PROTO_HEAD_S(head) \
	ntohl((head)->seq)
#define GET_PROTO_HEAD_L(head) \
	ntohl((head)->payload_len)
#define GET_PROTO_PACKETS(head) \
	((head)->tot_packets)
#define GET_PROTO_PACKNO(head) \
	((head)->packet_no)

typedef struct _NmpProtoHead NmpProtoHead;
struct _NmpProtoHead
{
	uint32_t        magic;
	uint32_t   		seq;
	uint32_t        payload_len;
	uint8_t			tot_packets;
	uint8_t			packet_no;
	uint16_t		reserve;
};


extern NmpPacketProto nmp_packet_proto;
extern NmpPayloadProto nmp_xml_proto;

#endif	//__NMP_PROTO_H__
