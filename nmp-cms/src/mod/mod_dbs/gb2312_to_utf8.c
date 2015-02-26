/* this file depend on zh_CN[][] table
*  created by nosword 2007-08-30
*  function: convert gb2312 to utf8
*/
//#include "zteComm.h"
//#include <unistd.h>
//#include <sys/socket.h>
//#include <stdint.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <sys/types.h>
//#include <pthread.h>
//#include "commLib.h"
//#include "paramLib.h"
//#include "streamLib.h"
//#include "net_socket.h"
//#include "ptzLib.h"
//#include "av_stream/av_rtsp.h"
#include "gb2312_to_utf8.h"

extern uint16_t zh_CN[128][128];

static int unicode_to_utf8(uint32_t u,utf8_t *utf8);
/***********************************************************************
* input : gb2312 encoding
* output : utf8 encoding
************************************************************************/
int gb2312_to_utf8(const uint8_t *in_buf,uint32_t in_len,uint8_t *out_buf)
{
	int ii,jj;
	int x,y;
	utf8_t utf8;
	uint32_t u;
	int len = 0;
	const uint8_t *p_in = in_buf;
	for(ii=0;ii<in_len;ii++)
	{
		memset(&utf8,0,sizeof(utf8));
		if(p_in[ii]>0xa0)
		{
			x= p_in[ii++]- 0xa0;
			y= p_in[ii] - 0xa0;
			u = zh_CN[x][y];
		}
		else
		{
			u = p_in[ii];
		}
		unicode_to_utf8(u,&utf8);
		for(jj=0;jj<utf8.count;jj++)
		{
			out_buf[len++]=utf8.u[jj];
		}
	}
	out_buf[len]='\0';
	return len;
}
static int unicode_to_utf8(uint32_t u,utf8_t *utf8)
{
	if(u<=0x7f)
	{
		utf8->u[0]=u;
		utf8->count =1;
	}
	else if(u<= 0x7FF)
	{
		utf8->u[0]= (u>>6)|0xB0;
		utf8->u[1] =(u&0x3F)|0x80;
		utf8->count=2;
	}
	else if(u<=0xFFFF)
	{
		utf8->u[0]=(u>>12)|0xE0;
		utf8->u[1]=((u>>6)&0x3F)|0x80;
		utf8->u[2]=(u&0x3F)|0x80;
		utf8->count= 3;
	}
	else if(u<0x1FFFFF)
	{
		utf8->u[0]=(u>>18)|0xF0;
		utf8->u[1]=((u>>12)&0x3F)|0x80;
		utf8->u[2]=((u>>6)&0x3F)|0x80;
		utf8->u[3]=(u&0x3F)|0x80;
		utf8->count=4;
	}
//	printf("utf8: %x %x %x\n",utf8->u[0],utf8->u[1],utf8->u[2]);
	return 0;
}



