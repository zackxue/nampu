#ifndef __h264pack_h__
#define __h264pack_h__ 

#include <sys/types.h>
#include <stdint.h>

#include "rtp.h"

/* pack hdr */
#define RTSP_CH_SIZE (4)
#define RTP_PAD_SIZE (RTSP_CH_SIZE+sizeof(rtp_pkt_t)) /* alignment */
#define RTP_EXT_PAD_SIZE (RTSP_CH_SIZE+sizeof(rtp_pkt_t)+sizeof(j_rtp_ext_t)+sizeof(j_rtp_av_ext_t))


#include "media_structs.h"

/* unpack hdr */
#define NAL_PAD_SIZE (4)
#define FRM_PAD_SIZE (sizeof(frame_t))

enum BLK_E {
    RTP_T       /* RTP_PAD_SIZE */
   ,NAL_T       /* NAL_PAD_SIZE */
   ,RTP_EXT_T   /* RTP_EXT_PAD_SIZE */
   ,FRM_T       /* FRM_PAD_SIZE */
};

typedef struct mem_s {
    uint32_t hdr_s;
    uint32_t data_s;
    uint8_t  *data;
}mem_t;

#define mem_hdr(m)    ((m)->data-(m)->hdr_s)
#define mem_data(m)   ((m)->data)
#define mem_size(m)   ((m)->hdr_s+(m)->data_s)
#define mem_data_s(m) ((m)->data_s)
#define mem_hdr_s(m)  ((m)->hdr_s)
#define mem_dup(d, m) do{(d)->hdr_s = (m)->hdr_s; (d)->data_s = (m)->data_s; (d)->data = (m)->data;}while(0)

/*
 * node defined;
 */
typedef struct node_s {
    uint32_t  times;
    uint16_t  don;
    uint16_t  type; /* 1: FRAME_I, 2: FRAME_P */
    mem_t     mem;
}node_t;

typedef struct node_array_s {
    int    count;
    node_t node[0];
}node_array_t;

/*
 * pack mode;
 */
typedef enum _PACK_MODE {
    PACK_MODE_FUA = 0x00 /* FU-A, STAP-A, Single NAL */
  , PACK_MODE_FUB = 0x01 /* FU-B, STAP-B */
  , PACK_MODE_BUTT
}PACK_MODE_E;

/*
 * mem alloc callback;
 */
typedef struct ctx_parm_s {
    void *udata0;
    void *udata1;
    int    (*_size)(struct ctx_parm_s  *parm, void *_uargs, int *data_s, int *hdr_s);
    mem_t  (*_alloc)(struct ctx_parm_s *parm, int data_s, int hdr_s);
    void   (*_free)(mem_t *mem);
}ctx_parm_t;

/*
 * handle defined;
 */
typedef void h264_pack_ctx_t;
typedef void h264_unpack_ctx_t;

/*
 * pack;
 */
h264_pack_ctx_t*
     h264_pack_new(int mode, ctx_parm_t *parm);
void h264_pack_del(h264_pack_ctx_t *ctx);

/* return: -1: err, >=0: pkt num */
int  h264_pack_pkt(h264_pack_ctx_t *ctx
        , node_t *nal
        , node_array_t **pkt    /* user free */
        , void *_uargs);

/*
 * unpack;
 */
h264_unpack_ctx_t*
     h264_unpack_new(ctx_parm_t *parm);
void h264_unpack_del(h264_unpack_ctx_t *ctx);


/* return: -1: err, >=0: nal num */
int  h264_unpack_nal(h264_unpack_ctx_t *ctx
        , node_t *pkt
        , node_t *nal           /* user free */
        , void *_uargs);

/*
 * |<- num(4)+9*[off(4)+size(4)] ->|<--- data --->|
 * return: -1: err, >=0: frm num
 */
int  h264_unpack_frm(h264_unpack_ctx_t *ctx
        , node_t *pkt
        , node_t *frm           /* user free */
        , void *_uargs);

#define NAL_INFO_SIZE     (4+9*(4+4))
#define NAL_OFF_NUM       (0)
#define NAL_OFF_I_OFF(i)  (4+(i)*(4+4))
#define NAL_OFF_I_SIZE(i) (NAL_OFF_I_OFF(i)+4)


#endif //__h264pack_h__
