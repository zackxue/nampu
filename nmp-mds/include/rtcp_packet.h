#ifndef __RTCP_PACKET_H__
#define __RTCP_PACKET_H__

#include <linux/types.h>
#include <glib.h>


typedef enum {
    RTCP_SR   = 200,
    RTCP_RR   = 201,
    RTCP_SDES = 202,        /* 源描述 */
    RTCP_BYE  = 203,
    RTCP_APP  = 204
}rtcp_type;


typedef enum {              /* 源描述类型 */
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8,
    RTCP_SDES_MAX   = 9
}rtcp_sdes_type;


typedef struct _rtcp_comm_header rtcp_comm_header;
struct _rtcp_comm_header {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    __u32 count:5;          /* 报告块计数 */
    __u32 padding:1;
    __u32 version:2;
#elif G_BYTE_ORDER == G_BIG_ENDIAN
    __u32 version:2;
    __u32 padding:1;
    __u32 count:5;
#else
# error "Invalid byte order!"
#endif
    __u32 packet_type:8;    /* RTCP报文类型 */
    __u16 length;           /* RTCP包长(含头，4字节为单位)-1*/
};


typedef struct _rtcp_src_desc rtcp_src_desc;
struct _rtcp_src_desc {     /* 源描述 */
    __u8 type;
    __u8 len;
    __u8 data[1];
};


typedef struct _rtcp_rr_block rtcp_rr_block;
struct _rtcp_rr_block {     /* 接收端报告块 */
    __u32 ssrc;             /* 源识别 */
    __u32 lost_rate:8;      /* 丢包率(自上次RR/SR) */
    __u32 lost:24;          /* 丢包数(自上次RR/SR，全部丢失则不发送RR) */
    __u32 last_seq;         /* 接收到的最高seq(低16位), 高16位扩展为周期计数 */
    __u32 jitter;           /* 间隔抖动 */
    __u32 lsr;              /* 上一个SR标识，NTP时间戳的中间32位 */
    __u32 dlsr;             /* 收到上一个SR到发送此RR之间的延时 */
};


typedef struct _rtcp_sr_header rtcp_sr_header;
struct _rtcp_sr_header {    /* 发送端报告 */
    rtcp_comm_header comm_h;
    __u32 ssrc;             /* 该SR发送者源标识 */
    __u32 ntp_hi;           /* 64位NTP */
    __u32 ntp_lo;
    __u32 rtp_ts;           /* RTP时间戳 */
    __u32 pkt_sent;         /* 已发送的RTP包数 */
    __u32 oct_sent;         /* 已发送的RTP字节数 */
    rtcp_rr_block rr_b[1];  /* 反馈接收端信息 */
};


typedef struct _rtcp_rr_header rtcp_rr_header;
struct _rtcp_rr_header {
    rtcp_comm_header comm_h;
    __u32 ssrc;             /* 该RR发送者源标识 */
    rtcp_rr_block rr_b[1];  /* 反馈接收端信息 */
};


typedef struct _rtcp_sdes_header rtcp_sdes_header;
struct _rtcp_sdes_header {  /* 源描述 */
    rtcp_comm_header comm_h;
    __u32 src;
    rtcp_src_desc descrip[1];
};


typedef struct _rtcp_bye_header rtcp_bye_header;
struct _rtcp_bye_header {   /* 离开通告 */
    rtcp_comm_header comm_h;
    __u32 src[1];           /* 源标识 */
    __u8 reason[1];         /* 离开原因 */
};


typedef struct _rtcp_app_header rtcp_app_header;
struct _rtcp_app_header {
    rtcp_comm_header comm_h;
    __u32 ssrc;             /* 源标识 */
    __u8 name[4];           /* 应用名称 */
    __u8 data[1];           /* 应用数据 */
};


guint32 get_rtcp_sr_src(gpointer sr);
guint32 get_rtcp_sr_lsr(gpointer sr);

void init_rtcp_rr_packet(rtcp_rr_header *rh);
void set_rtcp_rr_src(rtcp_rr_header *rh, guint32 src);
void set_rtcp_rr_ssrc(rtcp_rr_header *rh, guint32 ssrc);
void set_rtcp_rr_lost(rtcp_rr_header *rh, 
    guint8 lost_rate, guint32 lost_packets);
void set_rtcp_rr_jitter(rtcp_rr_header *rh, guint32 jitter);
void set_rtcp_rr_lastseq(rtcp_rr_header *rh, guint32 lastseq);
void set_rtcp_rr_lsr(rtcp_rr_header *rh, guint32 lsr);
void set_rtcp_rr_dlsr(rtcp_rr_header *rh, guint32 dlsr);

void init_rtcp_bye_packet(rtcp_bye_header *bh);
void set_rtcp_bye_src(rtcp_bye_header *bh, guint32 src);

void dump_rtcp_packet(gpointer packet, gint len);

#endif  //__RTCP_PACKET_H__
