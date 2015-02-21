#include <unistd.h>
#include <stdint.h>

#include "h264pack.h"
#include "nalu.h"
#include "ctx.h"

int nalu_find (node_t *nalus, int *count, char *bitst, int st_size)
{
    uint32_t index = *count;
    uint8_t * bs = (uint8_t *)bitst;
    uint32_t * head;
    while ((uint32_t)bs <= ((uint32_t)bitst + st_size - 4))
    {
#if 0
        head = (uint32_t *)bs;
#ifdef BIGENDIAN
        if (head[0] == 0x00000001)
#else
        if (head[0] == 0x01000000)
#endif

#else
        if(bs[0]==0x00 && bs[1]==0x00 && bs[2]==0x00 && bs[3]==0x01)
#endif
        {
            bs += 4;
            mem_data(&nalus[index].mem) = bs;
            nalus[index].times = 0;
            nalus[index].don = 0;
            if ((index - (*count)) > 0)/*Not the first NALU in this stream*/
            {
                mem_data_s(&nalus[index - 1].mem) 
                    = (size_t)(mem_data(&nalus[index].mem) - mem_data(&nalus[index - 1].mem)) - 4;
                mem_hdr_s(&nalus[index - 1].mem) = 0;
            }
            ++ index;
        }
        else if (bs[3] != 0)
        {
            bs += 4;
        }
        else if (bs[2] != 0)
        {
            bs += 3;
        }
        else if (bs[1] != 0)
        {
            bs += 2;
        }
        else
        {
            bs += 1;
        }
    }
    /*Calculate the size of the last NALU in this stream*/
    if (index > *count)
    {
        mem_data_s(&nalus[index - 1].mem) 
            = (size_t)(((uint32_t)bitst + st_size) - (uint32_t)mem_data(&nalus[index - 1].mem));
        *count = index;
    }
    else
    {
        printf ("Nothing found!\n");
        return -1;
    }
    return 0;
}


typedef struct _nalu_s {
    uint8_t *data;
    uint32_t size;
}nalu_t;

int nalu_get_sps_pps(char *bitst, int st_size, pic_parm_t *parm)
{
    nalu_t nalus[16];
    uint32_t count = 0;
    uint32_t index = 0;
    uint8_t  *bs = (uint8_t *)bitst;
    uint32_t *head;
    while ((uint32_t)bs <= ((uint32_t)bitst + st_size - 4))
    {
#if 0
        head = (uint32_t *)bs;
#ifdef BIGENDIAN
        if (head[0] == 0x00000001)
#else
        if (head[0] == 0x01000000)
#endif

#else
        if(bs[0]==0x00 && bs[1]==0x00 && bs[2]==0x00 && bs[3]==0x01)
#endif
        {
            if(index >= 16) break;
            
            bs += 4;
            nalus[index].data = bs;
            
            if ((index - count) > 0)/*Not the first NALU in this stream*/
            {
                nal_header_type *hdr = (nal_header_type*)nalus[index-1].data; 
                nalus[index-1].size = nalus[index].data - nalus[index-1].data - 4;
                if(hdr->type == 7)
                {
                    parm->sps_size = (nalus[index-1].size > sizeof(parm->sps))
                                    ?sizeof(parm->sps):nalus[index-1].size;
                    memcpy(parm->sps, hdr, parm->sps_size);
                }
                else if(hdr->type == 8)
                {
                    parm->pps_size = (nalus[index-1].size > sizeof(parm->pps))
                                    ?sizeof(parm->pps):nalus[index-1].size;
                    memcpy(parm->pps, hdr, parm->pps_size);
                }
            }
            ++index;
        }
        else if (bs[3] != 0)
        {
            bs += 4;
        }
        else if (bs[2] != 0)
        {
            bs += 3;
        }
        else if (bs[1] != 0)
        {
            bs += 2;
        }
        else
        {
            bs += 1;
        }
    }
    /*Calculate the size of the last NALU in this stream*/
    if (index > count)
    {
        nal_header_type *hdr = (nal_header_type*)nalus[index-1].data; 
        nalus[index-1].size = nalus[index].data - nalus[index-1].data - 4;
        if(hdr->type == 7)
        {
            parm->sps_size = (nalus[index-1].size > sizeof(parm->sps))
                            ?sizeof(parm->sps):nalus[index-1].size;
            memcpy(parm->sps, hdr, parm->sps_size);
        }
        else if(hdr->type == 8)
        {
            parm->pps_size = (nalus[index-1].size > sizeof(parm->pps))
                            ?sizeof(parm->pps):nalus[index-1].size;
            memcpy(parm->pps, hdr, parm->pps_size);
        }
        count = index;
    }
    else
    {
        printf ("Nothing found!\n");
        return -1;
    }
    return 0;
}
