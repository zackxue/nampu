#include <string.h>
#include <assert.h>

#include "ctx.h"
#include "pack.h"

/* Single NAL:   
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |F|NRI|  type   |                                               |
 *     +-+-+-+-+-+-+-+-+                                               |
 *     |                                                               |
 *     |               Bytes 2..n of a Single NAL unit                 |
 *     |                                                               |
 *     |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                               :...OPTIONAL RTP padding        |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

int pack_snp(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    uint32_t data_s = 0;
    uint32_t hdr_s  = 0;
    ctx->parm._size(&ctx->parm, _uargs, &data_s, &hdr_s);
    
    if((nal == NULL) 
        || (pkt == NULL) 
        || (mem_data_s(&nal->mem) > data_s))
    {
        return -1;
    }

    ctx->array.count = 1;
    ctx->array.node[0].mem = ctx->parm._alloc(&ctx->parm, data_s, hdr_s);
    
    if(mem_data(&ctx->array.node[0].mem) == NULL)
    {
        return -1;
    }
    memcpy(mem_data(&ctx->array.node[0].mem), mem_data(&nal->mem), mem_data_s(&nal->mem));
   
    mem_data_s(&ctx->array.node[0].mem) = mem_data_s(&nal->mem);
    
    *pkt = &ctx->array;
        
    return ctx->array.count;
}
/* FU-A:
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     | FU indicator  |   FU header   |                               |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
 *     |                                                               |
 *     |                         FU payload                            |
 *     |                                                               |
 *     |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                               :...OPTIONAL RTP padding        |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
int pack_fu_a(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    uint32_t nalPayloadSize = mem_data_s(&nal->mem) - sizeof(nal_header_type);

    uint32_t buf_size = 0;
    uint32_t pad_size = 0;
    ctx->parm._size(&ctx->parm, _uargs, &buf_size, &pad_size);
    
    uint32_t fuaPayloadSize = buf_size - sizeof(fu_indicator_type) - sizeof(fu_header_type);
    
    /* To set the S bit and E bit at the same time is disallowed in FU-A*/
    if (nalPayloadSize <= fuaPayloadSize)
    {
        return -1;
    }

#if 0
    printf(":mem_data_s(&nal->mem):%d, sizeof(nal_header_type):%d, buf_size:%d, sizeof(fu_indicator_type):%d, sizeof(fu_header_type):%d\n"
            , mem_data_s(&nal->mem)
            , sizeof(nal_header_type)
            , buf_size
            , sizeof(fu_indicator_type)
            , sizeof(fu_header_type));
#endif

    uint32_t packetsNumber = (((nalPayloadSize % fuaPayloadSize) > 0)
                                ? ((nalPayloadSize / fuaPayloadSize) + 1)
                                : (nalPayloadSize / fuaPayloadSize));
    /* src */
    uint8_t *nalus = mem_data(&nal->mem);
    uint8_t *nalus_off = (uint8_t *)(nalus + sizeof(nal_header_type));
    
    uint32_t count   = 0;
    ctx->array.count = 0;

    //printf("\n\n............begin.....................\n");
    for (count = 0; count < packetsNumber; ++count)
    {
        /* user free */
        mem_t mem = ctx->parm._alloc(&ctx->parm, buf_size, pad_size);
        uint8_t *buf = mem_data(&mem);
        
        if(buf == NULL)
        {
            printf("error, parm._alloc\n");
            assert(0);
            break;
        }
#ifdef BIGENDIAN
        /*
         * (|0-1-2-3-4-5-6-7|                  |0-1-2-3-4-5-6-7|)
         * (|1|1|1|0|0|0|0|0| & NALU Header) | |0|0|0|0|0|1|1|1|)
         * (|F|NRI|   Type  | & NALU Header) | |F|NRI|  Type   |)
         */
        buf[0] = ((0x07 & nalus[0]) | 0xE0);
        buf[1] = 0x00;
        if (count == 0)
        {
            buf[1] = 0x01; /*Set the S bit,This means this is the start FU-A packet*/
        }
        else if ((count + 1) == packetsNumber)
        {
            buf[1] = 0x02; /*Set the E bit, This means this is the last FU-A packet*/
        }
        /*
         * set the Type field of one FU-A header
         * (|0-1-2-3-4-5-6-7|)
         * (|0|0|0|1|1|1|1|1| & NALU Header)
         * (|S|E|R|   Type  | & NALU Header)
         */
        buf[1] |= (0xF8 & nalus[0]);
#else
        /*
         * (|7-6-5-4-3-2-1-0|                  |7-6-5-4-3-2-1-0|)
         * (|1|1|1|0|0|0|0|0| & NALU Header) | |0|0|0|1|1|1|0|0|)
         * (|F|NRI|   Type  | & NALU Header) | |F|NRI|  Type   |)
         *  Type = 0x1C = 28 indicates this is an FU-A packet
         */
        buf[0] = 0x1C | (0xE0 & nalus[0]);
        buf[1] = 0x00;

        if (count == 0)
        {
            buf[1] = 0x80; /*Set the S bit,This means this is the start FU-A packet*/
        }
        else if ((count + 1) == packetsNumber)
        {
            buf[1] = 0x40;/*Set the E bit, This means this is the last FU-A packet*/
        }
        /*
         * set the Type field of one FU-A header
         * (|7-6-5-4-3-2-1-0|)
         * (|0|0|0|1|1|1|1|1| & NALU Header)
         * (|S|E|R|   Type  | & NALU Header)
         */
        buf[1] |= (0x1F & nalus[0]);
#endif /*BIGENDIAN*/
        if ((count + 1) < packetsNumber)
        {
            memcpy ((buf + 2*sizeof(uint8_t)), nalus_off, fuaPayloadSize);
            ctx->array.count = count+1;
            mem_data(&ctx->array.node[count].mem)  = buf;
            mem_data_s(&ctx->array.node[count].mem)= fuaPayloadSize + sizeof(fu_indicator_type) + sizeof(fu_header_type); 
            mem_hdr_s(&ctx->array.node[count].mem) = mem_hdr_s(&mem);
        }
        else
        {
            uint32_t remainingPayload = nalPayloadSize - count * fuaPayloadSize;
            memcpy ((buf + 2*sizeof(uint8_t)), nalus_off, remainingPayload);
            ctx->array.count = count+1;
            mem_data(&ctx->array.node[count].mem)  = buf;
            mem_data_s(&ctx->array.node[count].mem)= remainingPayload + sizeof(fu_indicator_type) + sizeof(fu_header_type);
            mem_hdr_s(&ctx->array.node[count].mem) = mem_hdr_s(&mem);
        }
        nalus_off = (uint8_t *)(nalus_off + fuaPayloadSize);
#if 0
        printf("fu-a: count:%d, mem.hdr_s:%d, mem.data_s:%d\n", count
                , mem_hdr_s(&ctx->array.node[count].mem)
                , mem_data_s(&ctx->array.node[count].mem));
#endif

#if 0
    printf("buf:%p, NALU[F:%d,NRI:%d,TYPE:%d] "
            , buf
            , (buf[0]>>7)
            , (buf[0]>>5)&0x3
            , (buf[0]&0x1F));

    uint8_t nal_type = (buf[0] & 0x1F);
    switch(nal_type)
    {
        case 28: case 29:          /* FU-A or FU-B */
            printf("FU-A[S:%d,E:%d,R:%d,TYPE:%d] "
                    , (buf[1]>>7)
                    , (buf[1]>>6)&0x01
                    , (buf[1]>>5)&0x01
                    , (buf[1]&0x1F));     
            break;
        default:
           break;  
    }
    printf("\n");
#endif
    }
    //printf("............end.....................\n");

    *pkt = &ctx->array;
    if(ctx->array.count == 0)
    {
        printf("error, ctx->array.count == 0\n");
        assert(0);
    }
    return ctx->array.count;
}
int pack_fu_b(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    return -1;
}

int pack_stap_a(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    return -1;
}
int pack_stap_b(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    return -1;
}

int pack_mtap(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    return -1;
}



int pack_audio(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    uint32_t data_s = 0;
    uint32_t hdr_s  = 0;
    ctx->parm._size(&ctx->parm, _uargs, &data_s, &hdr_s);
    
    if((nal == NULL) 
        || (pkt == NULL) 
        || (mem_data_s(&nal->mem) > data_s))
    {
        return -1;
    }

    ctx->array.count = 1;
    ctx->array.node[0].mem = ctx->parm._alloc(&ctx->parm, data_s, hdr_s);
    
    if(mem_data(&ctx->array.node[0].mem) == NULL)
    {
        return -1;
    }
    memcpy(mem_data(&ctx->array.node[0].mem), mem_data(&nal->mem), mem_data_s(&nal->mem));
   
    mem_data_s(&ctx->array.node[0].mem) = mem_data_s(&nal->mem);
    
    *pkt = &ctx->array;
        
    return ctx->array.count;    
}

int pack_eof(pack_ctx_t *ctx, node_t *nal, node_array_t **pkt, void *_uargs)
{
    uint32_t data_s = 0;
    uint32_t hdr_s  = 0;
    ctx->parm._size(&ctx->parm, _uargs, &data_s, &hdr_s);
    
    if((nal == NULL) 
        || (pkt == NULL) 
        || (mem_data_s(&nal->mem) > data_s))
    {
        return -1;
    }

    ctx->array.count = 1;
    ctx->array.node[0].mem = ctx->parm._alloc(&ctx->parm, data_s, hdr_s);
    
    if(mem_data(&ctx->array.node[0].mem) == NULL)
    {
        return -1;
    }
    memcpy(mem_data(&ctx->array.node[0].mem), mem_data(&nal->mem), mem_data_s(&nal->mem));
   
    mem_data_s(&ctx->array.node[0].mem) = mem_data_s(&nal->mem);
    
    *pkt = &ctx->array;
        
    return ctx->array.count;
}