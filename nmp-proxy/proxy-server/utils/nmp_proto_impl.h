/*
 *          file: nmp_proto_impl.h
 *          description:–≠“ÈΩ‚Œˆ≤„
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __NMP_PROTO_IMPL_H__
#define __NMP_PROTO_IMPL_H__

#include <stdint.h>

#include "nmp_net.h"
#include "nmp_msg_impl.h"

#define NMP_PROTO_HEAD_MAGIC    0x6a786a2d

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

#define VALID_PROTO_HEAD(head)  \
    ntohl((head)->magic) == NMP_PROTO_HEAD_MAGIC
#define GET_PROTO_HEAD_S(head) \
    ntohl((head)->seq)
#define GET_PROTO_HEAD_L(head) \
    ntohl((head)->payload_len)
#define GET_PROTO_PACKETS(head) \
    ((head)->tot_packets)
#define GET_PROTO_PACKNO(head) \
    ((head)->packet_no)


typedef enum
{
    PACK_PTL_REGISTER,
    PACK_PTL_HEARTBEAT,
    PACK_CFG_REGISTER,
    PACK_CFG_HEARTBEAT,
    PACK_PLT_MDSCHANGED,
    PACK_PLT_GETMDSINFO,
}PACKID_E;

typedef struct _nmp_proto_head nmp_proto_head_t;

struct _nmp_proto_head
{
    uint32_t magic;
    uint32_t seq;
    uint32_t payload_len;
    uint8_t  tot_packets;
    uint8_t  packet_no;
    uint16_t reserve;
};

typedef struct packet_opt packet_opt_t;

struct packet_opt
{
    nmp_2proto_t proto;
    msg_t *(*create_special_packet)(void *init_data, PACKID_E id, int seq);
    int    (*check_special_packet)(msg_t *msg, PACKID_E id);
    int    (*process_special_packet)(msg_t *msg, void *init_data, PACKID_E id);
};

extern packet_opt_t pkt_opt;


#endif  //__NMP_PROTO_IMPL_H__


