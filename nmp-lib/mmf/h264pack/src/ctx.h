#ifndef __ctx_h__
#define __ctx_h__

#include "h264pack.h"

/* RTP Header
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|   CC  |M|      PT     |         sequence number       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |            synchronization source (SSRC) identifier           |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 */
#define RTP_HEADER_SIZE   12

/* Generic Nal header
 *
 *  +---------------+
 *  |0|1|2|3|4|5|6|7|
 *  +-+-+-+-+-+-+-+-+
 *  |F|NRI|  Type   |
 *  +---------------+
 *
 */
struct header_octet
{
#ifdef BIGENDIAN
    uint8_t f   :1;   /*Bit 0    */
    uint8_t nri :2;   /*Bit 1 ~ 2*/
    uint8_t type:5;   /*Bit 3 ~ 7*/
#else
    uint8_t type:5;   /*Bit 3 ~ 7*/
    uint8_t nri :2;   /*Bit 1 ~ 2*/
    uint8_t f   :1;   /*Bit 0    */
#endif
}__attribute__((packed));
typedef struct header_octet nal_header_type;
typedef struct header_octet fu_indicator_type;
typedef struct header_octet stap_header_type;
typedef struct header_octet mtap_header_type;

/*  FU header
 *  +---------------+
 *  |0|1|2|3|4|5|6|7|
 *  +-+-+-+-+-+-+-+-+
 *  |S|E|R|  Type   |
 *  +---------------+
 */
struct header_fu
{
#ifdef BIGENDIAN
    uint8_t s   :1; /* Bit 0    */
    uint8_t e   :1; /* Bit 1    */
    uint8_t r   :1; /* Bit 2    */
    uint8_t type:5; /* Bit 3 ~ 7*/
#else
    uint8_t type:5; /* Bit 3 ~ 7*/
    uint8_t r   :1; /* Bit 2    */
    uint8_t e   :1; /* Bit 1    */
    uint8_t s   :1; /* Bit 0    */
#endif
}__attribute__((packed));
typedef struct header_fu fu_header_type;

/* -------------------------- */

struct pack_ctx_s;
typedef struct pack_ctx_s pack_ctx_t; 
typedef struct pack_func_s {
    int(*snp)(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
    int(*fu)(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
    int(*stap)(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);
    int(*mtap)(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs);    
}pack_func_t;

struct pack_ctx_s {
    ctx_parm_t parm;
    PACK_MODE_E mode;
    pack_func_t pack;
    #define MAX_NODE_NUM 400
    node_array_t array;
};


typedef struct unpack_ctx_s {
    ctx_parm_t  parm;
    uint8_t     cur_pkt_type;
    uint8_t     cur_pkt_is_begin;
    uint8_t     cur_pkt_is_comp; 
    uint8_t     cur_nal_type; 
    uint8_t     cur_frm_type;
    uint32_t    cur_times;
    mem_t       nal_mem;
    int         nal_buf_size;
    int         nal_buf_off;
}unpack_ctx_t;

#endif //__ctx_h__
