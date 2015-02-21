#ifndef __RTP_H__
#define __RTP_H__

#include <linux/types.h>
#include <arpa/inet.h>
#include <glib.h>

typedef struct _rtp_pkt RTP_pkt;
struct _rtp_pkt {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    __u32   csrc_count:4;    /* 有贡献源个数 */
    __u32   extension:1; 
    __u32   padding:1;
    __u32   version:2; 
    __u32   payload_type:7;  /* 载荷类型 */
    __u32   marker:1;
#elif G_BYTE_ORDER == G_BIG_ENDIAN
    __u32   version:2;
    __u32   padding:1; 
    __u32   extension:1;
    __u32   csrc_count:4;
    __u32   marker:1;
    __u32   payload_type:7;
#else
    # error "Invalid byte order!"
#endif
    __u16   seq;
    __u32   timestamp;
    __u32   ssrc;            /* 同步源 */
	__u32	data[0];
};


typedef struct _rtp_ssrc_stats rtp_ssrc_stats;
struct _rtp_ssrc_stats {
    __u16 max_seq;         //!< highest seq number seen 
    __u32 cycles;          //!< shifted count of seq number cycles 
    __u32 base_seq;        //!< base seq number 
    __u32 bad_seq;         //!< last 'bad' seq number + 1 
    __u32 probation;       //!< sequ. pkts till source is valid 
    __u32 received;        //!< RTP pkts received 
    __u32 expected_prior;  //!< pkt expected at last interval 
    __u32 received_prior;  //!< pkt received al last interval 
    __u32 transit;         //!< relative trans time for prev pkt 
    double jitter;          //!< extimated jitter 
    GTimeVal lastrtp; //!< last RTP pkt reception time 
    GTimeVal lastsr;  //!< last RTCP SR pkt reception time 
    __u32 ntplastsr[2];    //!< last RTCP SR pkt NTP reception time 
    __u32 firstts;         //!< first pkt timestamp
    __u32 lastts;          //!< last pkt timestamp
    GTimeVal firsttv; //!< first pkt timeval 
};


typedef struct _rtp_session_state rtp_session_state;
struct _rtp_session_state {
    GTimeVal tp;      //!< the last time an RTCP pkt was transmitted
    GTimeVal tn;      //!< the next scheduled transmission time of an RTCP pkt
    __u32 pmembers;      //!< the estimated number of session members at time tm was last recomputed
    __u32 members;       //!< the most currente estimate for the number of the session members
    __u32 senders;       //!< the most currente estimate for the number of senders in the session
    double rtcp_bw;         //!< the target RTCP bandwidth
    __u8 we_sent;        //!< flag that is true if the app has sent data since the second previous RTCP Report was transmitted
    double avg_rtcp_size;   //!< the average Compound RTCP pkt size, in octets, over all RTCP pkts sent and received by this partecipant
    __u8 initial;        //!< the flag that is true if the app has not yet sent an RTCP pkt
};

#define RTP_PKT_CC(pkt)     (((RTP_pkt*)pkt)->csrc_count)
#define RTP_PKT_MARK(pkt)   (((RTP_pkt*)pkt)->marker)
#define RTP_PKT_PT(pkt)     (((RTP_pkt*)pkt)->payload_type)
#define RTP_PKT_SEQ(pkt)    ntohs(((RTP_pkt*)pkt)->seq)
#define RTP_PKT_TS(pkt)     ntohl(((RTP_pkt*)pkt)->timestamp)
#define RTP_PKT_SSRC(pkt)   ntohl(((RTP_pkt*)pkt)->ssrc)
#define RTP_PKT_DATA(pkt)   (&((RTP_pkt*)pkt)->data[0]  + ((RTP_pkt*)pkt)->csrc_count)

#define RTPPT_ISDYNAMIC(pt)    (pt >= 96)


#endif	/* __RTP_H__ */
