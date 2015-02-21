#include <string.h>
#include <assert.h>

#include "pack.h"
#include "ctx.h"

h264_pack_ctx_t*
     h264_pack_new(int mode, ctx_parm_t *parm)
{
    pack_ctx_t *_ctx = (pack_ctx_t*)calloc(1, sizeof(pack_ctx_t)+sizeof(node_t)*MAX_NODE_NUM);
    _ctx->mode = (mode < PACK_MODE_FUA || mode >= PACK_MODE_BUTT)?PACK_MODE_FUA:mode;
    
    _ctx->parm = *parm;
    
    if(_ctx->mode == PACK_MODE_FUA)
    {
        _ctx->pack.snp   = pack_snp;
        _ctx->pack.fu    = pack_fu_a;
        _ctx->pack.stap  = pack_stap_a;
        _ctx->pack.mtap  = pack_mtap;
    }
    else if(_ctx->mode == PACK_MODE_FUB)
    {
        _ctx->pack.snp   = pack_snp;
        _ctx->pack.fu    = pack_fu_b;
        _ctx->pack.stap  = pack_stap_b;
        _ctx->pack.mtap  = pack_mtap;        
    }

    return _ctx;
}
void h264_pack_del(h264_pack_ctx_t *ctx)
{
    if(ctx)
    {
        free(ctx);
    }
}
int  h264_pack_pkt(h264_pack_ctx_t *ctx
        , node_t *nal
        , node_array_t **pkt
        , void *_uargs)
{
    assert(ctx && nal && pkt);

    pack_ctx_t *_ctx = (pack_ctx_t*)ctx;

    if(_ctx->pack.snp(_ctx, nal, pkt, _uargs) >= 0)
    {
        return (*pkt)->count;
    }
    else if(_ctx->pack.fu(_ctx, nal, pkt, _uargs) >= 0)
    {
        return (*pkt)->count;
    }
    else
    {
        assert(0);
        return -1;
    }
    
    return -1;
}
h264_unpack_ctx_t*
     h264_unpack_new(ctx_parm_t *parm)
{
    unpack_ctx_t *_ctx = (unpack_ctx_t*)calloc(1, sizeof(unpack_ctx_t));
    _ctx->parm = *parm;
    return _ctx;
}
void h264_unpack_del(h264_unpack_ctx_t *ctx)
{
    if(ctx)
    {
        free(ctx);
    }
}


/*
 * 1-23   NAL unit  Single NAL unit packet per H.264   5.6
 * 24     STAP-A    Single-time aggregation packet     5.7.1
 * 25     STAP-B    Single-time aggregation packet     5.7.1
 * 26     MTAP16    Multi-time aggregation packet      5.7.2
 * 27     MTAP24    Multi-time aggregation packet      5.7.2
 * 28     FU-A      Fragmentation unit                 5.8
 * 29     FU-B      Fragmentation unit                 5.8
 */
#define HDR_TYPE(p) (p->mem.data[0] & 0x1F)

inline int get_hdr_size(unpack_ctx_t *ctx
        , node_t *pkt               /* RTP 负载 */
        , uint8_t *fu_nal_hdr)      /* FU分节时需要重新计算NAL头 */
{
    int n = 0;
    uint8_t *hdr_start   = mem_data(&pkt->mem);
    int      pkt_size    = mem_data_s(&pkt->mem);
    ctx->cur_pkt_type = ctx->cur_nal_type = (hdr_start[0] & 0x1F);

    switch (ctx->cur_pkt_type)
    {
        case 24:                   /* STAP-A */
            n = 1;                 /* discard the type byte */
            break;
        case 25: case 26: case 27: /* STAP-B, MTAP16, or MTAP24 */
            n = 3;                 /* discard the type byte, and the initial DON */
            break;
        case 28: case 29:          /* FU-A or FU-B */
            {
                /* For these NALUs, the first two bytes are the FU indicator and the FU header. */
                /* If the start bit is set, we reconstruct the original NAL header: */
                uint8_t startBit = (hdr_start[1] & 0x80);
                uint8_t endBit   = (hdr_start[1] & 0x40);
                if(startBit)
                {
                    n = 1;
                    if(pkt_size < n) return -1;
#if 0
                    hdr_start[1] = (hdr_start[0]&0xE0)+(hdr_start[1]&0x1F);
#else
                    /* 禁止修改输入缓冲中的数据 */
                    *fu_nal_hdr = (hdr_start[0]&0xE0)+(hdr_start[1]&0x1F);
#endif
                    ctx->cur_pkt_is_begin = 1;
                }
                else
                {
                    /* If the startbit is not set, both the FU indicator and header */
                    /* can be discarded */
                    n = 2;
                    if(pkt_size < n) return -1;
                    ctx->cur_pkt_is_begin = 0;
                }
                ctx->cur_pkt_is_comp = (endBit != 0);
                ctx->cur_nal_type = hdr_start[1]&0x1F;
            }
            break;
        default:
            /* This packet contains one or more complete, decodable NAL units */
            ctx->cur_pkt_is_begin = ctx->cur_pkt_is_comp = 1;
            break;
    }
     
    return n;
}


int  h264_unpack_nal(h264_unpack_ctx_t *ctx
        , node_t *pkt
        , node_t *nal
        , void *_uargs)
{
    int ret = 0;
    int skip = 0;
    uint8_t fu_nal_hdr = 0;

    unpack_ctx_t *_ctx = (unpack_ctx_t*)ctx;

    _ctx->cur_pkt_is_begin= 0;
    _ctx->cur_pkt_is_comp = 0;

    if((skip = get_hdr_size(_ctx, pkt, &fu_nal_hdr)) < 0)
    {
        /* pkt err */
        ret = 1;
        goto __error;
    }

    if((_ctx->cur_pkt_type != HDR_TYPE(pkt))
       || (_ctx->cur_times != pkt->times)
       || (mem_data(&_ctx->nal_mem) == NULL))
    {
        /* reset && alloc nal */
        int hdr_s, data_s;

        if(mem_data(&_ctx->nal_mem))
        {
            printf("%s ===> err: skip next nal, lost cur nal;\n", __FUNCTION__);
            _ctx->parm._free(&_ctx->nal_mem);
            mem_data(&_ctx->nal_mem) = NULL;
        }
        _ctx->parm._size(&_ctx->parm, _uargs, &data_s, &hdr_s);
        printf("data_s: %d, hdr_s: %d\n", data_s, hdr_s);
        _ctx->nal_buf_size = data_s;
        _ctx->nal_buf_off  = 0;
        _ctx->nal_mem = _ctx->parm._alloc(&_ctx->parm, data_s, hdr_s);
    }

    _ctx->cur_times = pkt->times;

    /* memcpy pkt to nal; */

    if((mem_data_s(&pkt->mem)-skip) > (_ctx->nal_buf_size - _ctx->nal_buf_off))
    {
        /* size not enough */
        ret = -2;
        goto __error;
    }

    if(fu_nal_hdr)
    {
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = fu_nal_hdr;
        skip++;
    }

    memcpy(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off, mem_data(&pkt->mem)+skip, mem_data_s(&pkt->mem)-skip);
    _ctx->nal_buf_off += (mem_data_s(&pkt->mem)-skip);

    if(_ctx->cur_pkt_is_comp)
    {
        nal->times= _ctx->cur_times;
        mem_dup(&nal->mem, &_ctx->nal_mem);
        mem_data_s(&nal->mem) = _ctx->nal_buf_off;
        /* push nal to user */
        mem_data(&_ctx->nal_mem) = NULL;
        ret = 1;
    }
    return ret;
__error:
    printf("%s ===> err: %d;\n", __FUNCTION__, ret);
    if(mem_data(&_ctx->nal_mem))
    {
        _ctx->parm._free(&_ctx->nal_mem);
        mem_data(&_ctx->nal_mem) = NULL;
    }
    return ret;
}


/*
 * |<- num(4)+9*[off(4)+size(4)] ->|<--- data --->|
 * return: -1: err, >=0: frm num
 */
int  h264_unpack_frm(h264_unpack_ctx_t *ctx
        , node_t *pkt
        , node_t *frm
        , void *_uargs)
{
    int ret = 0;
    int skip = 0;
    uint8_t  fu_nal_hdr = 0;

    unpack_ctx_t *_ctx = (unpack_ctx_t*)ctx;

    _ctx->cur_pkt_is_begin= 0;
    _ctx->cur_pkt_is_comp = 0;

    if((skip = get_hdr_size(_ctx, pkt, &fu_nal_hdr)) < 0)
    {
        /* pkt err */
        ret = -1;
        goto __error;
    }

    if( (_ctx->cur_times != pkt->times)
       || (mem_data(&_ctx->nal_mem) == NULL))
    {
        int hdr_s, data_s;

        if(mem_data(&_ctx->nal_mem))
        {
            /* warning...... no seq in pkt */
            /* dup frm, push to user */
            frm->times= _ctx->cur_times;
            frm->type = _ctx->cur_frm_type;
            mem_dup(&frm->mem, &_ctx->nal_mem);
            mem_data_s(&frm->mem) = _ctx->nal_buf_off;
#if 0
            printf("%s ===> OK: push frame nal_num:%d, size(%d+%d=%d)\n"
                    , __FUNCTION__
                    , *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_NUM))
                    , NAL_INFO_SIZE  
                    , _ctx->nal_buf_off - NAL_INFO_SIZE 
                    , _ctx->nal_buf_off);
#endif
            mem_data(&_ctx->nal_mem) = NULL;
            ret = 1;
        }
        /* reset && alloc frm */
        _ctx->parm._size(&_ctx->parm, _uargs, &data_s, &hdr_s);
        _ctx->nal_buf_size = data_s;
        _ctx->nal_buf_off  = NAL_INFO_SIZE;
        _ctx->nal_mem      = _ctx->parm._alloc(&_ctx->parm, data_s, hdr_s);
        *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_NUM)) = 0;
    }
    _ctx->cur_times = pkt->times;

    int i = *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_NUM));

    /* memcpy pkt to ctx frm; */

    if((mem_data_s(&pkt->mem)-skip + 4) > (_ctx->nal_buf_size - _ctx->nal_buf_off))
    {
        /* size not enough */
        ret = -2;
        goto __error;
    }
    
    if(_ctx->cur_pkt_is_begin)
    {
        
        if(*((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_NUM)) == 0)
        {
            if(   _ctx->cur_nal_type == 7   /* sps */
               || _ctx->cur_nal_type == 8   /* pps */
               || _ctx->cur_nal_type == 6)  /* sei */
                _ctx->cur_frm_type  = 1;    /* FRAME_I */
            else
                _ctx->cur_frm_type  = 2;    /* FRAME_P */
        }
        
        //printf("%s ===> nal_idx:%d begin:\n", __FUNCTION__, i);
        *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_I_OFF(i)))  = _ctx->nal_buf_off;
        *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_I_SIZE(i))) = 4; 
        
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = 0x00;
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = 0x00;
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = 0x00;
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = 0x01;
    }

    (*(int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_I_SIZE(i))) += (mem_data_s(&pkt->mem)-skip);

    if(fu_nal_hdr)
    {
        *(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off++) = fu_nal_hdr;
        skip++;
    }
    
    memcpy(mem_data(&_ctx->nal_mem)+_ctx->nal_buf_off, mem_data(&pkt->mem)+skip, mem_data_s(&pkt->mem)-skip);
    _ctx->nal_buf_off += (mem_data_s(&pkt->mem)-skip);

    if(_ctx->cur_pkt_is_comp)
    {
        //printf("%s ===> nal_idx:%d end, size:%d\n", __FUNCTION__, i, *((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_I_SIZE(i))));
        (*((int*)(mem_data(&_ctx->nal_mem)+NAL_OFF_NUM)))++;
    }

    return ret;
__error:
    printf("%s ===> err: %d;\n", __FUNCTION__, ret);
    if(mem_data(&_ctx->nal_mem))
    {
        _ctx->parm._free(&_ctx->nal_mem);
        mem_data(&_ctx->nal_mem) = NULL;
    }
    return ret;
}

