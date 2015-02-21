#include <assert.h>
#include <string.h>
#include "atomic.h"
#include "buf_pool.h"
#include "nalu.h"
#include "h264pack.h"
#include "rtp.h"

#include "ctx.h"

#include "h264_framer.h"

#include "buf_pl_array.h"

static int frm_mem_size(struct ctx_parm_s *parm
        , void *_uargs
        , int *data_s
        , int *hdr_s);
static mem_t
    frm_mem_alloc(struct ctx_parm_s *parm
            , int data_s 
            , int hdr_s);
static void  
    frm_mem_free(mem_t *mem);
    
static uint8_t*
    frm_mem_pdata(frm_mem_t *mem, uint32_t *l);
static frm_mem_t*  
    frm_mem_ref(frm_mem_t *mem);         
static void  
    frm_mem_unref(frm_mem_t *mem);
    
typedef struct _frm_mem_s {
    frm_mem_t  public;
    mem_t      private;
    atomic_t   ref_cnt;
}_frm_mem_t;

typedef struct _h264_frm_s {
    h264_unpack_ctx_t *unpack;
    buf_pool_t      *bf_pool;
}_h264_frm_t;


h264_frm_t *h264_frm_init(int32_t frm_size)
{
    /* new hdl */
    _h264_frm_t *fl = (_h264_frm_t*)calloc(1, sizeof(_h264_frm_t));

    if((!fl) || (frm_size <= (FRM_PAD_SIZE+NAL_INFO_SIZE))) return NULL;

    ctx_parm_t parm;

    printf("h264_frm_init >>>>>>> USE_RAW MALLOC\n");

    parm.udata1= (void*)frm_size;
    parm._size = frm_mem_size;
    parm._alloc= frm_mem_alloc;
    parm._free = frm_mem_free;

    fl->unpack   = h264_unpack_new(&parm);
    
    return fl;
}
void    h264_frm_fin(h264_frm_t *f)
{
    /* free hdl */
    assert(f);
    
    _h264_frm_t *fl = (_h264_frm_t*)f;
    
    if(fl->unpack)    h264_unpack_del(fl->unpack);

    free(fl);
}

static inline void _frm_fill_hdr(node_t *frm)
{
    frame_t *_frm = mem_hdr(&frm->mem);
    
    _frm->hdr.no       = 0;
    _frm->hdr.enc_type = ENC_H264;
    _frm->hdr.timestamp= frm->times;
    _frm->hdr.type     = frm->type;
    _frm->hdr.v.width  = 0;
    _frm->hdr.v.height = 0;
    _frm->hdr.size = mem_data_s(&frm->mem) - NAL_INFO_SIZE;
    _frm->data     = mem_data(&frm->mem) + NAL_INFO_SIZE;
    _frm->nal_desc = mem_data(&frm->mem);
}


int32_t h264_frm_fill(h264_frm_t *f, rtp_pkt_t *i, uint32_t size, frm_mem_t **o)
{
    _h264_frm_t *fl = (_h264_frm_t*)f;
    int skip = sizeof(rtp_pkt_t);
    node_t pkt;
    node_t frm;
       
    assert(f);
    
    if(i->extension)
    {
        rtp_ext_hdr_t *ext = (rtp_ext_hdr_t*)i->data;
        
        skip += sizeof(rtp_ext_hdr_t);
        skip += (ntohs(ext->length)*4);
    }

    pkt.times = ntohl(i->timestamp);
    pkt.mem.hdr_s = skip;
    pkt.mem.data_s= size - pkt.mem.hdr_s;
    pkt.mem.data  = (char*)i + pkt.mem.hdr_s;

    if(h264_unpack_frm(fl->unpack, &pkt, &frm, FRM_T) > 0)
    {
        _frm_mem_t *_mem = (_frm_mem_t*)calloc(1, sizeof(_frm_mem_t));
                
        atomic_set(&_mem->ref_cnt, 1);
        
        _mem->public.pdata   = frm_mem_pdata;
        _mem->public.ref     = frm_mem_ref;
        _mem->public.unref   = frm_mem_unref;

        _frm_fill_hdr(&frm);
        
        mem_dup(&_mem->private, &frm.mem);
        
        *o = _mem;
        return 1;
    }
    return 0;
}

static int frm_mem_size(struct ctx_parm_s *parm
        , void *_uargs
        , int *data_s
        , int *hdr_s)
{
    int t = (int)_uargs;
    switch(t)
    {
        case FRM_T:
            *data_s = (int32_t)parm->udata1 - FRM_PAD_SIZE;
            *hdr_s  = FRM_PAD_SIZE;            
            break;
        default:
            assert(0);
            break;
    }
    return *data_s;
}
static mem_t
    frm_mem_alloc(struct ctx_parm_s *parm
            , int data_s 
            , int hdr_s)
{
    mem_t mem;

    uint8_t *buf = (uint8_t*)malloc((int32_t)parm->udata1);

    mem.data_s = data_s;
    mem.hdr_s  = hdr_s;
    mem.data   = (buf != NULL)?(buf+hdr_s):NULL;
    
    if(!buf) assert(0);
    return mem;
}

static void  
    frm_mem_free(mem_t *mem)
{
    if(mem)
    {
        if(mem_data(mem))
        {
            free(mem_hdr(mem));
        }
    }
}

static uint8_t*
    frm_mem_pdata(frm_mem_t *mem, uint32_t *l)
{
    _frm_mem_t *_mem = (_frm_mem_t*)mem;
    if(_mem)
    {
        *l = mem_size(&_mem->private);
        return mem_hdr(&_mem->private);
    }
    return NULL;
}

static frm_mem_t*  
    frm_mem_ref(frm_mem_t *mem)
{
    _frm_mem_t *_mem = (_frm_mem_t*)mem;
    if(_mem)
    {
        atomic_inc(&_mem->ref_cnt);
    }
    return mem;
}
    
static void  
    frm_mem_unref(frm_mem_t *mem)
{
    _frm_mem_t *_mem = (_frm_mem_t*)mem;
    if(_mem)
    {
        if(atomic_dec_and_test_zero(&_mem->ref_cnt))
        {
            blk_mem_free(&_mem->private);
            free(_mem);
        }
    }
}
