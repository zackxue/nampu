/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#ifndef ___RTP_H__
#define ___RTP_H__

#include <glib.h>

#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>

#include <netembryo/wsocket.h>

#include "bufferqueue.h"
#include "evsched.h"
#include "rtspparse.h"
#include "rtsp.h"
#include "media.h"

struct Track;
struct RTSP_Client;
struct RTSP_Range;
struct RTSP_session;

#define RTP_DEFAULT_PORT 5004
#define BUFFERED_FRAMES_DEFAULT 16

typedef enum {
	RTP_OVER_UDP,
	RTP_OVER_TCP,
	RTP_OVER_RTSP
}RTP_transmode;

typedef struct {
    gint RTP;
    gint RTCP;
} port_pair;

typedef struct RTP_transport {
	RTP_transmode trans_mode;
    Sock *rtp_sock;
    Sock *rtcp_sock;
    gint rtp_ch, rtcp_ch;
    GEvent *rtp_writer;
    GEvent *rtcp_reader;
    GEvent *passby;
} RTP_transport;

typedef struct RTP_session {
	gint	ref_count;

    /** Multicast session (treated in a special way) */
    gboolean multicast;
	gboolean playing;	/* is playing */

    guint16 start_seq;
    guint16 seq_no;

    guint32 start_rtptime;
    guint32 ssrc;
    guint32 last_packet_send_time;

    struct RTSP_Range *range;
    gdouble send_time;
    gdouble last_timestamp;

    /** URI of the resouce for RTP-Info */
    gchar *uri;

    /** Pointer to the currently-selected track */
    struct Track *track;

#ifdef HAVE_METADATA
    void *metadata;
#endif

    /**
     * @brief Pool of one thread for filling up data for the session
     *
     * This is a pool consisting of exactly one thread that is used to
     * fill up the session with data when it's running low.
     *
     * Since we do want to do this asynchronously but we don't really
     * want race conditions (and they would anyway just end up waiting
     * on the same lock), there is no need to allow multiple threads
     * to do the same thing here.
     *
     * Please note that this is created during @ref rtp_session_resume
     * rather than during @ref rtp_session_new, and deleted during
     * @ref rtp_session_pause (and eventually during @ref
     * rtp_session_free), so that we don't have reading threads to go
     * around during seeks.
     */
    GThreadPool *fill_pool;

    /**
     * @brief Consumer for the track buffer queue
     *
     * This provides the interface between the RTP session and the
     * source of the data to send over the network.
     */
    BufferQueue_Consumer *consumer;

    RTSP_Client *client;
    RTSP_session *rtsp;

    guint32 octet_count;
    guint32 pkt_count;

    RTP_transport transport;

	/**
	 * For live stream waking up.
	*/
	gint	fd[2];	/* pipe() */

	struct RTP_session *audio_rtp;
} RTP_session;


typedef enum {
    SR = 200,
    RR = 201,
    SDES = 202,
    BYE = 203,
    APP = 204
}rtcp_pkt_type;


gint get_port_pair(void *srv, port_pair * pair);
gint put_port_pair(void *srv, port_pair * pair);

void rtp_over_tcp_prepare(RTP_transport *transport);

void rtp_transport_close(RTP_transport *transport);

/**
 * @}
 */

/**
 * @defgroup rtp_session RTP session management functions
 * @{
 */
RTP_session *rtp_session_new(RTSP_Client *client, RTSP_session *rtsp_s,
                             RTP_transport *transport, GstRTSPUrl *uri,
                             Track *tr);

void rtp_session_rtps_resume(GSList *, struct RTSP_Range *range);
void rtp_session_rtps_pause(GSList *);

void rtp_session_handle_sending(RTP_session *session);

void rtp_session_kill(RTP_session *rtp_s);
RTP_session *rtp_session_ref(RTP_session *rtp_s);
void rtp_session_unref(RTP_session *rtp_s);


#endif // ___RTP_H__
