#ifndef __h264_framer_h__
#define __h264_framer_h__

/* frame obj */
typedef struct frm_mem_s  frm_mem_t;
struct frm_mem_s {
    /* return frame_t* , l = sizeof(frame_t) + nal_desc_t(n=9) + video_data */
    uint8_t*   (*pdata)(frm_mem_t *o, uint32_t *l);
    frm_mem_t* (*ref)(frm_mem_t *o);
    void       (*unref)(frm_mem_t *o);
};

typedef void* h264_frm_t;

/* new */
h264_frm_t *h264_frm_init(int32_t frm_size);

/* del */
void    h264_frm_fin(h264_frm_t *f);

/*
 * fill
 * return: -1: err, 0: incomplete£¬ 1: complete;
 */
int32_t h264_frm_fill(h264_frm_t *f, rtp_pkt_t *i, uint32_t size, frm_mem_t **o);


#endif //__h264_framer_h__
