#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "atomic.h"
#include "buf_pool.h"
#include "nalu.h"
#include "h264pack.h"
#include "rtp.h"

#include "ctx.h"

#include "h264_filter.h"

#include "buf_pl_array.h"


typedef struct _rtp_mem_s {
    rtp_mem_t  public;
    mem_t      private;
    atomic_t   ref_cnt;
}_rtp_mem_t;

typedef struct pack_hdl_s {
    h264_pack_ctx_t *pack;
    buf_pool_t      *bf_pool;
    node_array_t    *rtp_array;
    uint32_t        rtp_r;
    uint16_t        rtp_seq_no; 
}pack_hdl_t;

typedef struct h264_filter_s {
    int _super;
    pack_hdl_t hdl;
}h264_filter_t;

static int rtp_hdr_fill(pack_hdl_t *hdl
        , char *rtp_buf
        , int pad_size
        , frame_hdr_t *frm_hdr
        , int marker);

int blk_mem_size(struct ctx_parm_s *parm
        , void *_uargs
        , int *data_s
        , int *hdr_s);
mem_t
    blk_mem_alloc(struct ctx_parm_s *parm
            , int data_s 
            , int hdr_s);
void  
    blk_mem_free(mem_t *mem);


static uint8_t*
    rtp_mem_pdata(rtp_mem_t *mem, uint32_t *l);
static rtp_mem_t*  
    rtp_mem_ref(rtp_mem_t *mem);
static void  
    rtp_mem_unref(rtp_mem_t *mem);

////////////////////////////////

h264_fl_op_t *h264_fl_op_init(int32_t rtp_mtu)
{
    /* new hdl */
    
    h264_filter_t *fl = (h264_filter_t*)calloc(1, sizeof(h264_filter_t));

    if((!fl) || (rtp_mtu <= RTP_EXT_PAD_SIZE)) return NULL;

    ctx_parm_t parm;
#ifdef USE_BUF_POOL
    printf(">>>>>>> USE_BUF_POOL\n");
    parm.udata0 = buf_pl_get(rtp_mtu);
    assert(parm.udata0);
#else
    printf(">>>>>>> USE_RAW MALLOC\n");
#endif
    parm.udata1= (void*)rtp_mtu;
    parm._size = blk_mem_size;
    parm._alloc= blk_mem_alloc;
    parm._free = blk_mem_free;

#ifdef USE_BUF_POOL
    fl->hdl.bf_pool  = (buf_pool_t*)parm.udata0;
#endif
    fl->hdl.pack     = h264_pack_new(PACK_MODE_FUA, &parm);
    #define RTP_ARRAY_NUM 400
    fl->hdl.rtp_array= (node_array_t*)calloc(1, sizeof(node_array_t)
                                                +sizeof(node_t)*RTP_ARRAY_NUM);
    fl->hdl.rtp_r = fl->hdl.rtp_array->count = 0;
    
    return fl;
}
void    h264_fl_op_fin(h264_fl_op_t *f)
{
    /* free hdl */
    assert(f);
    
    h264_filter_t *fl = (h264_filter_t*)f;
    
    if(fl->hdl.pack)    h264_pack_del(fl->hdl.pack);
#ifdef USE_BUF_POOL
    if(fl->hdl.bf_pool) buf_pl_rel(fl->hdl.bf_pool);
#endif
    if(fl->hdl.rtp_array) free(fl->hdl.rtp_array);
    free(fl); 
}

int32_t h264_fl_op_count(h264_fl_op_t *f, frame_t* frm)
{
    assert(f);
    
    h264_filter_t *fl = (h264_filter_t*)f;
    pack_ctx_t *ctx = (pack_ctx_t*)fl->hdl.pack;
    
    int rtp_payload_size = ((int32_t)ctx->parm.udata1 - RTP_EXT_PAD_SIZE);
    return frm->hdr.size + ((frm->hdr.size + (rtp_payload_size-1))/rtp_payload_size + 2) * RTP_EXT_PAD_SIZE;
}

int32_t h264_fl_op_fill(h264_fl_op_t *f, frame_t *i, uint32_t i_size)
{
    /* input frame && pack pkt*/
    assert(f && i);
    
    int i1 = 0, k = 0, count = 0;

    frame_t *frame = (frame_t*)i;
    h264_filter_t *fl = (h264_filter_t*)f;    
    
    #define IN_NAL_NUM 100
    node_t  nalus[IN_NAL_NUM];
    int     rtp_pad_type = RTP_EXT_T;
    node_array_t *array = NULL;
    
    /* reset point */
    if(fl->hdl.rtp_r != fl->hdl.rtp_array->count)
    {
        printf("error, fl->hdl.rtp_r != fl->hdl.rtp_array->count\n");
        assert(0);
    }
    fl->hdl.rtp_r = fl->hdl.rtp_array->count = 0;

    nalus[0].times = frame->hdr.timestamp;
    nalus[0].mem.hdr_s = 0;
    nalus[0].mem.data_s= frame->hdr.size; 
    nalus[0].mem.data  = frame->data; 

    switch(frame->hdr.type)
    {
        case FRAME_A:  
             if(pack_audio(fl->hdl.pack, &nalus[0], &array, (void*)rtp_pad_type) > 0)
             {
                k = 0;
                {
                    rtp_hdr_fill(&fl->hdl
                            , mem_hdr(&array->node[k].mem) + RTSP_CH_SIZE
                            , mem_hdr_s(&array->node[k].mem)
                            , &frame->hdr
                            , 1);
                }
                assert((fl->hdl.rtp_array->count + array->count) < RTP_ARRAY_NUM);
                memcpy( &fl->hdl.rtp_array->node[fl->hdl.rtp_array->count]
                        , &array->node[0]
                        , array->count*sizeof(array->node[0]));
                fl->hdl.rtp_array->count += array->count;
             }
            return 0;
            break;
        case FRAME_EOF:
             if(pack_eof(fl->hdl.pack, &nalus[0], &array, (void*)rtp_pad_type) > 0)
             {
                k = 0;
                {
                    rtp_hdr_fill(&fl->hdl
                            , mem_hdr(&array->node[k].mem) + RTSP_CH_SIZE
                            , mem_hdr_s(&array->node[k].mem)
                            , &frame->hdr
                            , 1);
                }
                assert((fl->hdl.rtp_array->count + array->count) < RTP_ARRAY_NUM);
                memcpy( &fl->hdl.rtp_array->node[fl->hdl.rtp_array->count]
                        , &array->node[0]
                        , array->count*sizeof(array->node[0]));
                fl->hdl.rtp_array->count += array->count;
             }          
            return 0;
            break;
        default:
            break;
    }

    if((frame->nal_desc != NULL) 
            && (frame->nal_desc->nal_num > 0) 
            && (frame->nal_desc->nal_num < IN_NAL_NUM))
    {
        for(i1 = 0; i1 < frame->nal_desc->nal_num; i1++)
        {
            nalus[i1].times = frame->hdr.timestamp;
            nalus[i1].mem.hdr_s = 0;
            nalus[i1].mem.data_s= frame->nal_desc->nal[i1].nal_size - 4 /* skip nal startcode */;
            nalus[i1].mem.data  = frame->nal_desc->nal[i1].nal_off + frame->data + 4/* skip nal startcode */;
        }
        count = frame->nal_desc->nal_num;
    }
    else
    {
        nalu_find (nalus, &count, frame->data, frame->hdr.size);
    }


    //printf("\n\n---------------------------\n");
    for(i1 = 0; i1 < count; i1++)
    {
        rtp_pad_type = (i1==0)?RTP_EXT_T:RTP_T;
#if 0  
        uint8_t *buf = mem_data(&nalus[i1].mem); 
        printf("fill nal => NALU[F:%d,NRI:%d,TYPE:%d] \n"
            , (buf[0]>>7)
            , (buf[0]>>5)&0x3
            , (buf[0]&0x1F));
#endif  
        if(h264_pack_pkt(fl->hdl.pack, &nalus[i1], &array, (void*)rtp_pad_type) < 0)
        {
            continue;
        }

        for(k = 0; k < array->count; k++)
        {
            rtp_hdr_fill(&fl->hdl
                    , mem_hdr(&array->node[k].mem) + RTSP_CH_SIZE
                    , mem_hdr_s(&array->node[k].mem)
                    , &frame->hdr
                    , ((i1==(count-1))&&(k==(array->count-1)))?1:0);
        }
        assert((fl->hdl.rtp_array->count + array->count) < RTP_ARRAY_NUM);
        memcpy( &fl->hdl.rtp_array->node[fl->hdl.rtp_array->count]
                , &array->node[0]
                , array->count*sizeof(array->node[0]));
        fl->hdl.rtp_array->count += array->count;
    }
    //printf("\n---------------------------\n");
    //printf("fill => frame_no:%d, rtp_num:%d\n", frame->hdr.no, fl->hdl.rtp_array->count);

    return 0;
}




int32_t h264_fl_op_pull(h264_fl_op_t *f, rtp_mem_t **o, uint32_t *size)
{
    /* output pkg */
    assert(f);

    h264_filter_t *fl = (h264_filter_t*)f;
    
    if(fl->hdl.rtp_r >= fl->hdl.rtp_array->count)
    {
       return -EAGAIN;
    }

    _rtp_mem_t *_mem = (_rtp_mem_t*)calloc(1, sizeof(_rtp_mem_t));
    if(_mem)
    {
        atomic_set(&_mem->ref_cnt, 1);
        _mem->public.pdata   = rtp_mem_pdata;
        _mem->public.ref     = rtp_mem_ref;
        _mem->public.unref   = rtp_mem_unref;
        mem_dup(&_mem->private, &fl->hdl.rtp_array->node[fl->hdl.rtp_r].mem);
        *o = (rtp_mem_t*)_mem;
        *size = mem_size(&_mem->private) - RTSP_CH_SIZE;
        fl->hdl.rtp_r++;
        if(mem_hdr_s(&_mem->private) > RTP_EXT_PAD_SIZE)
        {
            printf("error, mem_hdr_s(&_mem->private):%d\n", mem_hdr_s(&_mem->private));
            assert(0);
        }
    }
    else
    {
        *o = NULL;
        *size = 0;
    }

    return 0;
}

////////////////////////////////////////////////

static int rtp_hdr_fill(pack_hdl_t *hdl
        , char *rtp_buf
        , int pad_size
        , frame_hdr_t *frm_hdr
        , int marker)
{
    if(rtp_buf == NULL) return -1;
    /* rtp hdr */
    rtp_pkt_t *rtp_hdr    = (rtp_pkt_t*)rtp_buf;
    rtp_hdr->version      = 2;
    rtp_hdr->padding      = 0;
    rtp_hdr->extension    = 0;
    rtp_hdr->csrc_count   = 0;
    rtp_hdr->marker       = marker & 0x1;
    rtp_hdr->payload_type = (frm_hdr->enc_type==ENC_H264)
                            ?(96&0x7f)
                            :((frm_hdr->enc_type==ENC_G711A)?(8&0x7f):(0&0x7f));
    rtp_hdr->seq          = htons(++hdl->rtp_seq_no);
    rtp_hdr->timestamp    = (frm_hdr->enc_type==ENC_H264)
                            ?htonl(frm_hdr->timestamp*90)
                            :htonl(frm_hdr->timestamp*8);
    rtp_hdr->ssrc         = htonl(rtp_hdr->payload_type);
    if(pad_size == RTP_EXT_PAD_SIZE)
    {
        rtp_hdr->extension = 1;
        /* rtp ext */
        j_rtp_ext_t *ext = (j_rtp_ext_t*)rtp_hdr->data;
        ext->version     = 0x01;
        ext->frame_type  = frm_hdr->type;
        ext->reserved    = 0x00;
        ext->size        = htons(sizeof(j_rtp_av_ext_t)/4);
        /* rtp ext data */
        j_rtp_av_ext_t *jext = (j_rtp_av_ext_t*)ext->data;
        jext->v.width        = htons(frm_hdr->v.width);
        jext->v.height       = htons(frm_hdr->v.height);
        jext->v.frame_num    = htonl(frm_hdr->no);
        jext->v.frame_length = htonl(frm_hdr->size);
        jext->v.utc_time     = htonl(frm_hdr->v.utc_time);
    }
    return 0;
}


int blk_mem_size(struct ctx_parm_s *parm
        , void *_uargs
        , int *data_s
        , int *hdr_s)
{
    int t = (int)_uargs;
    switch(t)
    {
        case RTP_T:
            *data_s = (int32_t)parm->udata1 - RTP_PAD_SIZE;
            *hdr_s  = RTP_PAD_SIZE;
            break;
        case NAL_T:
            *data_s = (int32_t)parm->udata1 - NAL_PAD_SIZE;
            *hdr_s  = NAL_PAD_SIZE;
            break;
        case RTP_EXT_T:
            *data_s = (int32_t)parm->udata1 - RTP_EXT_PAD_SIZE;
            *hdr_s  = RTP_EXT_PAD_SIZE;
			break;
        case FRM_T:
            *data_s = (int32_t)parm->udata1 - FRM_PAD_SIZE;
            *hdr_s  = FRM_PAD_SIZE;            
            break;
    }
    return *data_s;
}
mem_t
    blk_mem_alloc(struct ctx_parm_s *parm
            , int data_s 
            , int hdr_s)
{
    mem_t mem;

#ifdef USE_BUF_POOL
    uint8_t *buf = (uint8_t*)buf_new(parm->udata0); 
#else
    uint8_t *buf = (uint8_t*)malloc((int32_t)parm->udata1);
#endif
    mem.data_s = data_s;
    mem.hdr_s  = hdr_s;
    mem.data   = (buf != NULL)?(buf+hdr_s):NULL;
    
    if(!buf) assert(0);

    return mem;
}

void  
    blk_mem_free(mem_t *mem)
{
    if(mem)
    {
        if(mem_data(mem))
        {
            #ifdef USE_BUF_POOL
            buf_unref(mem_hdr(mem));
            #else
            free(mem_hdr(mem));
            #endif
        }
    }
}


static uint8_t*
    rtp_mem_pdata(rtp_mem_t *mem, uint32_t *l)
{
    _rtp_mem_t *_mem = (_rtp_mem_t*)mem;
    if(_mem)
    {
        *l = mem_size(&_mem->private) - RTSP_CH_SIZE;
        return mem_hdr(&_mem->private) + RTSP_CH_SIZE;
    }
    return NULL;
}

static rtp_mem_t*  
    rtp_mem_ref(rtp_mem_t *mem)
{
    _rtp_mem_t *_mem = (_rtp_mem_t*)mem;
    if(_mem)
    {
        atomic_inc(&_mem->ref_cnt);
    }
    return mem;
}
    
static void  
    rtp_mem_unref(rtp_mem_t *mem)
{
    _rtp_mem_t *_mem = (_rtp_mem_t*)mem;
    if(_mem)
    {
        if(atomic_dec_and_test_zero(&_mem->ref_cnt))
        {
            blk_mem_free(&_mem->private);
            free(_mem);
        }
    }
}
