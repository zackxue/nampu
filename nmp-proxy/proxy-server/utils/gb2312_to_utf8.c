
#include "gb2312_to_utf8.h"


typedef struct __utf8_t_ utf8_t;

struct __utf8_t_
{
    uint8_t u[4];
    int count;
}__attribute__((packed));

extern uint16_t zh_CN[128][128];

static int unicode_to_utf8(uint32_t u, utf8_t *utf8)
{
    if(u<=0x7f)
    {
        utf8->u[0] = u;
        utf8->count= 1;
    }
    else if(u<= 0x7FF)
    {
        utf8->u[0] =  (u>>6)|0xB0;
        utf8->u[1] =(u&0x3F)|0x80;
        utf8->count= 2;
    }
    else if(u<=0xFFFF)
    {
        utf8->u[0] = (u>>12)|0xE0;
        utf8->u[1] = ((u>>6)&0x3F)|0x80;
        utf8->u[2] = (u&0x3F)|0x80;
        utf8->count= 3;
    }
    else if(u<0x1FFFFF)
    {
        utf8->u[0] = (u>>18)|0xF0;
        utf8->u[1] = ((u>>12)&0x3F)|0x80;
        utf8->u[2] = ((u>>6)&0x3F)|0x80;
        utf8->u[3] = (u&0x3F)|0x80;
        utf8->count= 4;
    }

    return 0;
}

int gb2312_to_utf8(const uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf)
{
    int x, y;
    int ii, jj;
    utf8_t utf8;
    uint32_t u;

    int len = 0;
    const uint8_t *p_in = in_buf;

    for(ii=0; ii<in_len; ii++)
    {
        memset(&utf8,0,sizeof(utf8));
        if(p_in[ii]>0xa0)
        {
            x = p_in[ii++]- 0xa0;
            y = p_in[ii] - 0xa0;
            u = zh_CN[x][y];
        }
        else
        {
            u = p_in[ii];
        }
        unicode_to_utf8(u,&utf8);
        for(jj=0; jj<utf8.count; jj++)
        {
            out_buf[len++] = utf8.u[jj];
        }
    }
    out_buf[len] = '\0';
    return len;
}



