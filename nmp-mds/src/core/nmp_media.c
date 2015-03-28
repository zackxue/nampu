#include <string.h>
#include "nmp_version.h"
#include "nmp_media.h"
#include "nmp_timer.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_scheduler.h"
#include "nmp_media_device.h"
#include "nmp_monotonic_timer.h"


#define MIN_MEDIA_PERMANET			10
#define RTSP_STEP_TIMEOUT			30	/* Never work */

static void nmp_media_on_timer(NmpRtspMedia *media);
static void nmp_rtsp_media_request_iframe(NmpRtspMedia *media,
	NmpMediaDevice *device);

static gint total_media_objects = 0;


static __inline__ void
nmp_rtsp_media_uri_free(NmpMediaUri *uri_info)
{
	g_free(uri_info->media_base);
	g_free(uri_info);
}


static gboolean
nmp_media_timer(gpointer user_data)
{
	NmpRtspMedia *media;
	G_ASSERT(user_data != NULL);

	media = (NmpRtspMedia*)user_data;
	nmp_media_on_timer(media);
	return FALSE;
}


static __inline__ NmpMediaSession *
nmp_rtsp_media_session_new( void )
{
	NmpMediaSession *s;

	s = g_new0(NmpMediaSession, 1);
	s->cseq = 0;
	s->stream = 0;
	s->state = NMP_SESSION_DESCRIBE;

	return s;
}


static __inline__ gint
nmp_rtsp_media_init(NmpRtspMedia *self)
{
	int index;

	self->ref_count = 2;	/* Timer and us*/
	self->lock = g_mutex_new();
	self->state = NMP_MEDIA_STAT_NEW;
	self->media_uri = NULL;
	self->session = nmp_rtsp_media_session_new();
	self->sink = NULL;
	self->src = NULL;
	self->sdp = NULL;
	self->streams = NSTREAMS;

	for (index = 0; index < NSTREAMS; ++index)
	{
		snprintf(self->track_map[index].name, MAX_TRACK_NAME_LEN,
			"track%d", index + 1);
	}

	self->ready_to_play = 0;
	self->device = NULL;
	self->timer = nmp_set_timer(
		MIN_MEDIA_PERMANET * 1000,
		nmp_media_timer,
		self
	);
	nmp_pq_init(&self->r_pending);

	g_atomic_int_add(&total_media_objects, 1);

	return 0;
}


static void
nmp_rtsp_media_finalize(NmpRtspMedia *media)
{
	g_atomic_int_add(&total_media_objects, -1);

	nmp_print(
		"Freeing media '%p', total %d left.",
		media,
		total_media_objects
	);

	BUG_ON(media->device);

	if (media->sdp)
		gst_sdp_message_free(media->sdp);

	if (media->session)
		g_free(media->session);

	if (media->media_uri)
		nmp_rtsp_media_uri_free(media->media_uri);

	g_mutex_free(media->lock);
}


static __inline__ gboolean
__nmp_rtsp_media_killed(NmpRtspMedia *media)
{
	return media->state == NMP_MEDIA_STAT_NULL;
}


__export gboolean
nmp_rtsp_media_destroyed(NmpRtspMedia *media)
{
	gboolean destroyed;
	G_ASSERT(media != NULL);
	
	g_mutex_lock(media->lock);
	destroyed = (media->state == NMP_MEDIA_STAT_NULL);
	g_mutex_unlock(media->lock);

	return destroyed;
}


static __inline__ gint
nmp_media_blockable(NmpRtspMedia *media)
{
	BUG_ON(!media || !media->media_uri);

	return media->media_uri->type != MS_LIVE;
}


static __inline__ gint
nmp_media_get_track_index(NmpRtspMedia *media, NmpMediaTrack *track)
{
	gint index;

	for (index = 0; index < media->streams; ++index)
	{
		if (!strcmp(media->track_map[index].name, track->name))
			return index;
	}

	nmp_warning(
		"Invalid client track '/%s' when get index.", track->name
	);

	return -1;
}


static __inline__ void
nmp_rtsp_media_map_track(NmpRtspMedia *media)
{
	const GstSDPMedia *m;
	const gchar *val;
	gint idx;

	for (idx = 0; idx < media->streams; ++idx)
	{
		m = gst_sdp_message_get_media(media->sdp, idx);
		val = gst_sdp_media_get_attribute_val(m, "control");
		if (val)
		{
			memset(&media->track_map[idx], 0, sizeof(NmpMediaTrack));
			strncpy(media->track_map[idx].name, val, MAX_TRACK_NAME_LEN - 1);
		}

		nmp_debug(
			"Map track '%s' to stream index '%d'",
			media->track_map[idx].name,
			idx
		);
	}

}


static __inline__ void
__nmp_rtsp_media_set_sdp_info(NmpRtspMedia *media, GstSDPMessage *sdp)
{
	if (media->sdp)
		gst_sdp_message_free(media->sdp);

	gst_sdp_message_set_session_name(sdp, NMP_MEDIA_SESS_IN_SDP);
	gst_sdp_message_set_information (sdp, NMP_MEDIA_INFO_IN_SDP);

	media->sdp = sdp;
	media->streams = gst_sdp_message_medias_len(sdp);

	nmp_print(
		"Sdp: media '%p' '%s/media-%d/channel=%2d&level=%d' has %d"
		" stream(s).",
		media,
		media->media_uri->device,
		media->media_uri->type,
		media->media_uri->channel,
		media->media_uri->rate_level,
		media->streams
	);

	nmp_rtsp_media_map_track(media);
}


__export void
nmp_rtsp_media_set_sdp_info(NmpRtspMedia *media, GstSDPMessage *sdp)
{
	G_ASSERT(media != NULL);
	
	g_mutex_lock(media->lock);
	__nmp_rtsp_media_set_sdp_info(media, sdp);
	g_mutex_unlock(media->lock);
}


static __inline__ void
__nmp_rtsp_media_get_sdp_info(NmpRtspMedia *media, guint8 **sdp, guint *size)
{
	gchar *sdp_string;
	gsize sdp_size;

	if (__nmp_rtsp_media_killed(media))
	{
		*sdp = NULL;
		return;
	}

	if (!media->sdp)
	{
		*sdp = NULL;
		return;		
	}

	sdp_string = gst_sdp_message_as_text(media->sdp);
	if (sdp_string)
	{
		sdp_size = strlen(sdp_string);

		*sdp = g_memdup(sdp_string, sdp_size);
		*size = sdp_size;
		g_free(sdp_string);
		return;
	}

	*sdp = NULL;
	return;
}


__export void
nmp_rtsp_media_get_sdp_info(NmpRtspMedia *media, guint8 **sdp, guint *size)
{
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);
	__nmp_rtsp_media_get_sdp_info(media, sdp, size);
	g_mutex_unlock(media->lock);
}


__export void
nmp_rtsp_media_set_session_id(NmpRtspMedia *media, gchar *sid)
{
	G_ASSERT(media != NULL && sid != NULL);
	G_ASSERT(media->session->state == NMP_SESSION_SETUP);

	BUG_ON(!media->session);
	if (!media->session->sid[0])
	{
		strncpy(media->session->sid, sid, MAX_SESSION_ID_LEN - 1);
		strtok(media->session->sid, ";");
	}
}


__export void
nmp_rtsp_media_set_session_seq(NmpRtspMedia *media, gint seq)
{
	G_ASSERT(media != NULL && media->session != NULL);

	media->session->cseq = seq;
	media->session->expire = nmp_get_monotonic_time() + RTSP_STEP_TIMEOUT;
}


__export NmpSessionState
nmp_rtsp_media_get_session_state(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL);

	BUG_ON(!media->session);
	return media->session->state;
}


__export gint
nmp_rtsp_media_session_expire(NmpRtspMedia *media)
{
	if (nmp_rtsp_media_get_session_state(media) != NMP_SESSION_DESCRIBE)
		return 0;

	if (nmp_get_monotonic_time() >= media->session->expire)
		return 1;

	return 0;
}


__export NmpSessionState
nmp_rtsp_media_session_state_next(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL);

	if (media->session->state < NMP_SESSION_OPTIONS)
	{
		if (media->session->state == NMP_SESSION_SETUP &&
			media->session->stream < media->streams - 1)
		{
			++media->session->stream;
		}
		else
			++media->session->state;
	}

	return media->session->state;
}


static __inline__ void
nmp_rtsp_media_set_teardown(NmpRtspMedia *media)
{
	media->session->state = NMP_SESSION_TEARDOWN;
}


__export gboolean
nmp_rtsp_media_has_teardown(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL);

	BUG_ON(!media->session);
	return media->session->state == NMP_SESSION_TEARDOWN;
}


__export void
nmp_rtsp_media_zombie_it(void *media)
{
	NmpRtspMedia *m = (NmpRtspMedia*)media;
	G_ASSERT(m != NULL);

	m->zombie = 1;	/* may be seen later by reader */
}


__export gint
nmp_rtsp_media_play_waiting(NmpRtspMedia *media)
{
	gint wait = 0;
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);
	if (!__nmp_rtsp_media_killed(media))
	{
		if (media->session->state == NMP_SESSION_PLAY &&
			!media->ready_to_play)
		{
			wait = 1;
		}
	}
	g_mutex_unlock(media->lock);

	return wait;
}


static gint
nmp_rtsp_media_recv_packet(gpointer _media, gint stream, gchar *buf, gint size)
{
	gint err = -EINVAL;
	NmpRtspMedia *media = (NmpRtspMedia*)_media;
	NmpMediaSinkers *sinkers = NULL;

	g_mutex_lock(media->lock);
	if (!__nmp_rtsp_media_killed(media))
	{
		if (media->sink)
		{
			sinkers = nmp_media_sinkers_ref(media->sink);
		}
	}
	g_mutex_unlock(media->lock);

	if (sinkers)
	{
		err = nmp_media_sinkers_recv_packet(sinkers, stream, buf, size);
		nmp_media_sinkers_unref(sinkers);
	}

	return err;
}


static __inline__ gint
nmp_rtsp_media_set_state(NmpRtspMedia *media, NmpMediaState state)
{
	NmpMediaSource *src = NULL;
	NmpPendingQueue pq;
	gint blockable;

	if (media->state == state)
		return 0;

	if (media->state == NMP_MEDIA_STAT_NULL)
	{
		nmp_warning(
			"Change 'NULL' state media '%p'", media
		);
		return -1;	/* can't be changed */
	}

	if (state == NMP_MEDIA_STAT_READY)
	{
		blockable = nmp_media_blockable(media);

		media->sink = nmp_media_sinkers_new(media, blockable);
		if (G_UNLIKELY(!media->sink))
			return -1;

		media->src = nmp_media_src_create_source(
			media,
			media->streams,
			media->mt == NMP_MT_UDP ? MEDIA_TRANS_UDP : MEDIA_TRANS_TCP,
			blockable
		);
		if (G_UNLIKELY(!media->src))
		{
			nmp_media_sinkers_unref(media->sink);
			media->sink = NULL;
			return -1;
		}

		nmp_media_src_set_recv_fun(media->src,
			nmp_rtsp_media_recv_packet);

		nmp_rtsp_media_ref(media);	/* media->src owns */
		nmp_media_src_attach(media->src);

		media->state = state;
		return 0;
	}

	if (state == NMP_MEDIA_STAT_NULL)
	{
		if (media->sink)
		{
			nmp_media_sinkers_unref(media->sink);
			media->sink = NULL;
		}

		src = media->src;
		media->src = NULL;
		media->state = state;
		nmp_pq_graft(&media->r_pending, &pq);

		g_mutex_unlock(media->lock);

		if (src)
		{
			/*
			 * 可能直接导致src的finalize()被调用.
			*/
			nmp_scheduler_del((GSource*)src);
			g_source_unref((GSource*)src);
		}

		nmp_pq_call_and_free(&pq);

		g_mutex_lock(media->lock);
		return 0;
	}

	return -1;
}


__export gint
nmp_rtsp_media_prepare(NmpRtspMedia *media, NmpDeviceMediaType mt)
{
	gint ret;
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);
	media->mt = mt;
	ret = nmp_rtsp_media_set_state(media, NMP_MEDIA_STAT_READY);
	g_mutex_unlock(media->lock);

	return ret;
}


static __inline__ gint
nmp_rtsp_media_kill(NmpRtspMedia *media)
{
	gint ret;
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);
	ret = nmp_rtsp_media_set_state(media, NMP_MEDIA_STAT_NULL);
	g_mutex_unlock(media->lock);

	return ret;
}


NmpRtspMedia *
nmp_rtsp_media_new( void )
{
	NmpRtspMedia *media;

	media = g_new0(NmpRtspMedia, 1);
	if (nmp_rtsp_media_init(media))
	{
		g_free(media);
		media = NULL;
	}

	return media;
}


void
nmp_rtsp_media_ref(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL &&
		g_atomic_int_get(&media->ref_count) > 0);

	g_atomic_int_inc(&media->ref_count);
}


void
nmp_rtsp_media_unref(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL &&
		g_atomic_int_get(&media->ref_count) > 0);

	if (g_atomic_int_dec_and_test(&media->ref_count))
	{
		nmp_rtsp_media_finalize(media);
		g_free(media);
	}
}


__export void
nmp_rtsp_media_kill_unref(NmpRtspMedia *media)
{
	G_ASSERT(media != NULL);

	nmp_rtsp_media_kill(media);
	nmp_rtsp_media_unref(media);
}


__export void
nmp_rtsp_media_attach_device(NmpRtspMedia *media, 
	gpointer device)
{
	G_ASSERT(media != NULL && device != NULL);

	g_mutex_lock(media->lock);

	BUG_ON(media->device);
	media->device = device;
	nmp_rtsp_device_ref((NmpMediaDevice*)device);

	g_mutex_unlock(media->lock);
}


__export void
nmp_rtsp_media_detach_device(NmpRtspMedia *media, 
	gpointer device)
{
	G_ASSERT(media != NULL && device != NULL);

	g_mutex_lock(media->lock);

	BUG_ON(!media->device);
	BUG_ON(media->device != device);
	media->device = NULL;

	g_mutex_unlock(media->lock);

	nmp_rtsp_device_unref((NmpMediaDevice*)device);
}


NmpMediaUri*
nmp_rtsp_media_uri_dup(const NmpMediaUri *uri)
{
	NmpMediaUri *media_uri;

	media_uri = g_new0(NmpMediaUri, 1);
	memcpy(media_uri, uri, sizeof(*uri));
	media_uri->media_base = g_strdup(uri->media_base);

	if (!uri->media_base)
	{
		if (uri->type == MS_LIVE)
		{
			media_uri->media_base = g_strdup_printf(
				"rtsp://localhost/dev=%s/media=%d/channel=%d&level=%d",
				media_uri->device,
				media_uri->type,
				media_uri->channel,
				media_uri->rate_level
			);
		}
		else
		{
			media_uri->media_base = g_strdup_printf(
				"rtsp://localhost/dev=%s/media=%d/%s",
				media_uri->device,
				media_uri->type,
				media_uri->mrl
			);
		}
	}

	return media_uri;
}


static __inline__ gint
nmp_rtsp_media_get_rtp_port(NmpRtspMedia *media, gint stream, gint *rtp)
{
	G_ASSERT(media != NULL);

	return nmp_media_src_get_rtp_port(media->src, stream, rtp);
}


#ifdef CONFIG_RTCP_SUPPORT

static __inline__ gint
nmp_rtsp_media_get_rtcp_port(NmpRtspMedia *media, gint stream, gint *rtcp)
{
	G_ASSERT(media != NULL);

	return nmp_media_src_get_rtcp_port(media->src, stream, rtcp);
}

#endif


NmpSinkerWatch *
nmp_media_create_sinker(NmpRtspMedia *media, GstRTSPLowerTrans trans,
	gint *err)
{
	NmpSinkerWatch *sinker;
	G_ASSERT(media != NULL);

	sinker = nmp_media_sinker_create(trans, media->streams, err);
	if (G_UNLIKELY(!sinker))
	{
		nmp_warning(
			"Create sinker for media '%p' failed.", media
		);
	}

	return sinker;
}


gint
nmp_media_set_sinker_transport(NmpRtspMedia *media, NmpSinkerWatch *sinker,
	NmpMediaTrack *track, gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p)
{
	gint ret = -1, i_stream;
	G_ASSERT(media != NULL && sinker != NULL && ct != NULL);

	g_mutex_lock(media->lock);
	if (!__nmp_rtsp_media_killed(media))
	{
		i_stream = nmp_media_get_track_index(media, track);
		ret = nmp_media_sinker_set_transport(sinker, i_stream, 
			client_Ip, ct, b_nat, p);
	}
	g_mutex_unlock(media->lock);

	return ret;
}


static __inline__ gint
__nmp_rtsp_media_add_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker)
{
	if (__nmp_rtsp_media_killed(media))
		return -E_MKILLED;

	if (media->sink)
	{
		media->timer = 0;
		nmp_media_sinkers_add(media->sink, sinker);
		return 0;
	}

	BUG();

	return -E_UNKNOWN;
}


gint
nmp_rtsp_media_add_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker)
{
	gint ret;
	G_ASSERT(media != NULL && sinker != NULL);

	g_mutex_lock(media->lock);
	ret = __nmp_rtsp_media_add_sinker(media, sinker);
	g_mutex_unlock(media->lock);

	return ret;
}


static __inline__ gboolean
__nmp_rtsp_media_del_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker,
	NmpMediaDevice **devp)
{
	gint left_sinkers;

	if (__nmp_rtsp_media_killed(media))
		return FALSE;

	if (media->sink)
	{
		left_sinkers = nmp_media_sinkers_del(media->sink, sinker);
		if (left_sinkers == 0)
		{
			nmp_rtsp_media_set_state(media, NMP_MEDIA_STAT_NULL);
			if (media->device)
			{
				*devp = media->device;
				nmp_rtsp_device_ref(*devp);
				return TRUE;
			}
			return FALSE;
		}
	}

	return FALSE;
}


void
nmp_rtsp_media_del_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker)
{
	gboolean need_teardown;
	NmpMediaDevice *device;
	G_ASSERT(media != NULL && sinker != NULL);

	g_mutex_lock(media->lock);
	need_teardown = __nmp_rtsp_media_del_sinker(media, sinker, &device);
	g_mutex_unlock(media->lock);

	if (need_teardown)
	{
		nmp_rtsp_media_die_announce(media, device);
		nmp_rtsp_device_remove_media(device, media);
		nmp_rtsp_device_unref(device);
	}
}


__export gint
nmp_rtsp_media_sinker_play(NmpRtspMedia *media, NmpSinkerWatch *sinker)
{
	NmpMediaDevice *device = NULL;
	gint rc = -E_MKILLED, fist_playing = 0;
	G_ASSERT(media != NULL && sinker != NULL);

	g_mutex_lock(media->lock);
	if (!__nmp_rtsp_media_killed(media))
	{
		rc = 0;
		nmp_media_sinker_play(sinker);

		fist_playing = !media->ready_to_play;
		media->ready_to_play = 1;

		device = (NmpMediaDevice*)media->device;
		if (device)
		{
			nmp_rtsp_device_ref(device);
		}
	}
	g_mutex_unlock(media->lock);

	if (device)
	{
		if (fist_playing)
			rc = nmp_rtsp_device_request(device, media);
		else
			nmp_rtsp_media_request_iframe(media, device);

		nmp_rtsp_device_unref(device);
	}

	return rc;
}


static void
nmp_media_on_timer(NmpRtspMedia *media)
{
	NmpMediaDevice *device = NULL;

	g_mutex_lock(media->lock);

	if (media->timer)
	{
		if (!__nmp_rtsp_media_killed(media))
		{
			nmp_print(
				"[Media-Timer] Media '%p' %d(secs) timeout.",
				media, MIN_MEDIA_PERMANET
			);

			nmp_rtsp_media_set_state(media, NMP_MEDIA_STAT_NULL);
			device = (NmpMediaDevice*)media->device;
			if (device)
			{
				nmp_rtsp_device_ref(device);
			}
		}
	}

	g_mutex_unlock(media->lock);

	if (device)
	{
		nmp_rtsp_media_die_announce(media, device);
		nmp_rtsp_device_remove_media(device, media);
		nmp_rtsp_device_unref(device);
	}

	nmp_rtsp_media_unref(media);
}


static __inline__ gchar*
nmp_rtsp_media_localhost(NmpRtspMedia *media)
{
	gchar *local_ip = NULL;

	local_ip = nmp_rtsp_device_get_localip(media->device);		
	if (!local_ip)
	{
		local_ip = g_strdup("");
	}

	return local_ip;
}


static __inline__ gint
nmp_rtsp_media_make_describe_request(GstRTSPMessage **request,
	NmpRtspMedia *media)
{
	GstRTSPResult res;

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_DESCRIBE,
			media->media_uri->media_base
		),
		no_describe_message
	);

	gst_rtsp_message_add_header(
		*request,
		GST_RTSP_HDR_ACCEPT,
		"application/sdp"
	);

	return 0;

no_describe_message:
	return -1;
}


static __inline__ gint
nmp_rtsp_media_make_setup_request(GstRTSPMessage **request,
	NmpRtspMedia *media)
{
	gint rtp_port, rtcp_port;
	GstRTSPResult res;
	gchar *transport;
	gchar *url;
	gchar *localhost;
	gint stream;

	stream = media->session->stream;
	nmp_rtsp_media_get_rtp_port(media, stream, &rtp_port);

#ifdef CONFIG_RTCP_SUPPORT
	nmp_rtsp_media_get_rtcp_port(media, stream, &rtcp_port);
#else
	rtcp_port = rtp_port + 1;
#endif

	url = g_strdup_printf(
		"%s/%s",
		media->media_uri->media_base,
		media->track_map[stream].name
	);

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_SETUP,
			url
		),
		no_setup_message
	);

	localhost = nmp_rtsp_media_localhost(media);

	if (media->mt == NMP_MT_UDP)
	{
		transport = g_strdup_printf(
			"RTP/AVP;unicast;client_port=%d-%d;destination=%s",
			rtp_port,
			rtcp_port,
			localhost
		);
	}
	else
	{
		transport = g_strdup_printf(
			"RTP/AVP/TCP;client_port=%d-%d;destination=%s",
			rtp_port,
			rtcp_port,
			localhost
		);		
	}

	if (stream)
	{
		gst_rtsp_message_add_header(
			*request,
			GST_RTSP_HDR_SESSION,
			media->session->sid
		);
	}

	gst_rtsp_message_add_header(
		*request,
		GST_RTSP_HDR_TRANSPORT,
		transport
	);

	nmp_print(
		"Setup media '%p', stream idx '%d', tranport:'%s'",
		media,
		stream,
		transport
	);

	g_free(transport);
	g_free(localhost);
	g_free(url);
	return 0;

no_setup_message:
	g_free(url);
	return -1;
}


static __inline__ gint
nmp_rtsp_media_make_play_request(GstRTSPMessage **request,
	NmpRtspMedia *media)
{
	gchar *url;
	GstRTSPResult res;

	url = g_strdup_printf("%s/", media->media_uri->media_base);
	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_PLAY,
			url
		),
		no_play_message
	);

	gst_rtsp_message_add_header(
		*request, 
		GST_RTSP_HDR_SESSION,
		media->session->sid
	);

	gst_rtsp_message_add_header(
		*request,
		GST_RTSP_HDR_RANGE,
		"npt=0.000-"
	);

	nmp_print(
		"Play media '%p', Session '%s'",
		media,
		media->session->sid
	);

	g_free(url);
	return 0;

no_play_message:
	g_free(url);
	return -1;
}


static __inline__ gint
nmp_rtsp_media_make_option_request(GstRTSPMessage **request,
	NmpRtspMedia *media)
{
	gchar *url;
	GstRTSPResult res;

	url = g_strdup_printf("%s/", media->media_uri->media_base);
	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_OPTIONS,
			url
		),
		no_option_message
	);

	gst_rtsp_message_add_header(
		*request, 
		GST_RTSP_HDR_SESSION,
		media->session->sid
	);

	g_free(url);
	return 0;

no_option_message:
	g_free(url);
	return -1;	
}


static __inline__ gint
nmp_rtsp_media_make_teardown_request(GstRTSPMessage **request,
	NmpRtspMedia *media)
{
	gchar *url;
	GstRTSPResult res;

	url = g_strdup_printf("%s/", media->media_uri->media_base);
	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_TEARDOWN,
			url
		),
		no_teardown_message
	);

	gst_rtsp_message_add_header(
		*request, 
		GST_RTSP_HDR_SESSION,
		media->session->sid
	);

	g_free(url);
	return 0;

no_teardown_message:
	g_free(url);
	return -1;
}


static __inline__ gint
nmp_rtsp_media_make_setparam_request(GstRTSPMessage **request,
	NmpRtspMedia *media, gint name, gpointer value)
{
	NmpSessionState state;
	GstRTSPResult res;

	state = nmp_rtsp_media_get_session_state(media);
	if (state <= NMP_SESSION_PLAY)
		return -1;

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request, 
			GST_RTSP_SET_PARAMETER,
			media->media_uri->media_base
		),
		no_setparam_message
	);

	gst_rtsp_message_add_header(
		*request,
		GST_RTSP_HDR_USER_AGENT,
		NMP_MEDIA_SERVER_AGENT
	);

	if (G_LIKELY(strlen(media->session->sid)))
	{
		gst_rtsp_message_add_header(
			*request, 
			GST_RTSP_HDR_SESSION,
			media->session->sid
		);
	}

	return 0;

no_setparam_message:
	return -1;
}


static __inline__ gint
__nmp_rtsp_media_make_request(NmpRtspMedia *media,
	GstRTSPMessage **request)
{
	gint ret = -1;
	NmpSessionState state;
	G_ASSERT(media != NULL && request != NULL);

	state = nmp_rtsp_media_get_session_state(media);
	switch (state) {
	case NMP_SESSION_DESCRIBE:
		ret = nmp_rtsp_media_make_describe_request(
			request,
			media
		);
		break;

	case NMP_SESSION_SETUP:
		ret = nmp_rtsp_media_make_setup_request(
			request,
			media
		);
		break;

	case NMP_SESSION_PLAY:
		ret = nmp_rtsp_media_make_play_request(
			request,
			media
		);
		break;

	case NMP_SESSION_OPTIONS:
		ret = nmp_rtsp_media_make_option_request(
			request,
			media
		);
		break;

	case NMP_SESSION_TEARDOWN:
		ret = nmp_rtsp_media_make_teardown_request(
			request,
			media
		);
		break;

	default: 
		break;
	}

	if (!ret)
	{
		gst_rtsp_message_add_header(
			*request, 
			GST_RTSP_HDR_USER_AGENT,
			NMP_MEDIA_SERVER_AGENT
		);
	}

	return ret;
}


gint
nmp_rtsp_media_make_request(NmpRtspMedia *media,
	GstRTSPMessage **request)
{
	gint err;
	G_ASSERT(media != NULL && request != NULL);

	g_mutex_lock(media->lock);
	err = __nmp_rtsp_media_make_request(media, request);
	g_mutex_unlock(media->lock);

	return err;
}


__export gint
nmp_rtsp_media_make_param_request(NmpRtspMedia *media,
	GstRTSPMessage **request, gint name, gpointer value)
{
	gint err = -E_MKILLED;

	g_mutex_lock(media->lock);
	if (!__nmp_rtsp_media_killed(media))
	{
		err = nmp_rtsp_media_make_setparam_request(
			request, media, name, value
		);
	}
	g_mutex_unlock(media->lock);

	return err;
}


__export void
nmp_rtsp_media_die_announce(NmpRtspMedia *media,
	gpointer device)
{
	G_ASSERT(media != NULL && device != NULL);

	if (media->session->sid[0])
	{
		nmp_rtsp_media_set_teardown(media);
		nmp_rtsp_device_request(
			(NmpMediaDevice*)device, media
		);

		nmp_print(
			"Media '%p' session '%s' die announce!",
			media,
			media->session->sid
		);
	}
	else
	{
		nmp_print(
			"Media '%p' die announce!", media
		);
	}
}


static __inline__ void
nmp_rtsp_media_request_iframe(NmpRtspMedia *media,
	NmpMediaDevice *device)
{
	nmp_rtsp_device_extend_request(
		device, media, REQUEST_DEVICE_IFRAME, NULL
	);
}


static __inline__ gint
nmp_rtsp_media_pending_request(NmpRtspMedia *media, PQN_DATA_T data_1, 
	PQN_DATA_T data_2, PQN_DATA_T data_3, NmpPQNFunc fun)
{
	return nmp_pq_pending(
		&media->r_pending, data_1, data_2, data_3, fun
	);
}


gint
nmp_rtsp_media_pending_request_state(NmpRtspMedia *media, NmpMediaState s,
	PQN_DATA_T data_1, PQN_DATA_T data_2, PQN_DATA_T data_3, NmpPQNFunc fun)
{
	gint ret = -E_MKILLED;	/* not needed */
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);

	if (!__nmp_rtsp_media_killed(media))
	{
		BUG_ON(!media->session);

		if (s >= media->session->state)
		{
			nmp_rtsp_media_pending_request(media, data_1, data_2, data_3, fun);
			ret = 0;
		}
		else
		{
			ret = -E_NOPENDING;
		}
	}

	g_mutex_unlock(media->lock);

	return ret;
}


void
nmp_rtsp_media_deal_pending_request(NmpRtspMedia *media)
{
	NmpPendingQueue pq;
	G_ASSERT(media != NULL);

	g_mutex_lock(media->lock);
	nmp_pq_graft(&media->r_pending, &pq);
	g_mutex_unlock(media->lock);

	nmp_pq_call_and_free(&pq);
}


//:~ End
