#ifndef __h264_filter_h__
#define __h264_filter_h__

#include "media_structs.h"


typedef struct rtp_mem_s  rtp_mem_t;
struct rtp_mem_s {
    uint8_t*   (*pdata)(rtp_mem_t *o, uint32_t *l);
    rtp_mem_t* (*ref)(rtp_mem_t *o);
    void       (*unref)(rtp_mem_t *o);
};

typedef void* h264_fl_op_t;
h264_fl_op_t *h264_fl_op_init(int32_t rtp_mtu);
void    h264_fl_op_fin(h264_fl_op_t *f);
int32_t h264_fl_op_count(h264_fl_op_t *f, frame_t* frm);
int32_t h264_fl_op_fill(h264_fl_op_t *f, frame_t *i, uint32_t i_size);
int32_t h264_fl_op_pull(h264_fl_op_t *f, rtp_mem_t **o, uint32_t *size);

#endif //__h264_filter_h__
