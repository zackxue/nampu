#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include "nmp_version.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_media_device.h"
#include "nmp_media_factory.h"
#include "nmp_utils.h"

extern GstRtspWatchFuncs nmp_rtsp_watch_funcs;

__export NmpMediaDevice *
nmp_rtsp_device_new( void )
{
	NmpMediaDevice *device;

	device = g_new0(NmpMediaDevice, 1);
	device->ref_count = 1;
	device->state = NMP_DEV_STAT_NEW;
	device->media_type = NMP_MT_TCP;
	device->time_to_die = MAX_DEVICE_TTD_SEC;
	device->ttd = MAX_DEVICE_ACC_SEC;
	device->seq_generator = 1;
	device->device_lock = g_mutex_new();

	return device;
}


__export void
nmp_rtsp_device_set_info(NmpMediaDevice *device, gchar *id,
	gchar *ip, gint ttd)
{
	strncpy(device->id, id, MAX_DEVICE_ID_LEN - 1);
	strncpy(device->localip, ip, __MAX_IP_LEN - 1);
	device->time_to_die = ttd;
	device->ttd = device->time_to_die;
}


__export void
nmp_rtsp_device_set_media_type(NmpMediaDevice *device,
	NmpDeviceMediaType mt)
{
	device->media_type = mt;
}


__export void
nmp_rtsp_device_update_ttd(NmpMediaDevice *device)
{
	device->ttd = device->time_to_die;
}


gboolean
nmp_rtsp_device_check_id(NmpMediaDevice *device, const gchar *id, gint ttd)
{
	G_ASSERT(device != NULL);

	if (strcmp(device->id, id))
		return FALSE;

	device->time_to_die = ttd;
	device->ttd = device->time_to_die;

	return TRUE;
}


static __inline__ gint 
nmp_rtsp_device_generate_seq(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	return g_atomic_int_exchange_and_add(&device->seq_generator, 1);
}


static __inline__ void
nmp_rtsp_device_unref_all_medias(NmpMediaDevice *old, 
	GList *medias)
{
	NmpRtspMedia *m;
	GList *l;

	for (l = medias; l; l = g_list_next(l))
	{
		m = (NmpRtspMedia*)l->data;
		nmp_rtsp_media_detach_device(m, old);
		nmp_rtsp_media_kill_unref(m);
	}

	g_list_free(medias);
}


static __inline__ void
nmp_rtsp_device_free(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	nmp_print(
		"Freeing device: '%p', ID: '%s'.",
		device, device->id
	);

	g_mutex_free(device->device_lock);

	BUG_ON(device->live_medias);
	BUG_ON(device->watch);

	g_free(device);
}


__export NmpMediaDevice *
nmp_rtsp_device_ref(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL &&
		g_atomic_int_get(&device->ref_count) > 0);

	g_atomic_int_inc(&device->ref_count);
	return device;
}


__export void
nmp_rtsp_device_unref(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL && 
		g_atomic_int_get(&device->ref_count) > 0);

	if (g_atomic_int_dec_and_test(&device->ref_count))
	{
		if (device->finalize)
			(*device->finalize)(device);

		nmp_rtsp_device_free(device);
	}
}


static void
nmp_rtsp_device_watch_finalize(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);
	
	nmp_print(
		"Device '%p', ID: '%s' watch finalized.",
		 device, device->id
	);
	nmp_rtsp_device_unref(device);
}


__export gboolean
nmp_rtsp_device_accept(NmpMediaDevice *device, 
	GEvent *channel, gint *errp)
{
  	gint listen_sock, new_sock;
	struct sockaddr_in s;
	socklen_t len;

	listen_sock = g_event_fd(channel);

	bzero(&s, sizeof(s));
	len = sizeof(s);

	new_sock = accept(listen_sock, (struct sockaddr*)&s, &len);
	if (new_sock < 0)
	{
		*errp = -errno;
		return FALSE;
	}

	strncpy(device->devip, inet_ntoa(s.sin_addr), __MAX_IP_LEN - 1);	//@{Fix me , inet_ntoa() not-ts!}
	nmp_print(
		"Accept device '%p', IP '%s:%d'",
		device,
		device->devip,
		ntohs(s.sin_port)
	);

	set_fd_flags(new_sock, O_NONBLOCK);

	device->watch = gst_rtsp_watch_new_2(
		new_sock, 
		&nmp_rtsp_watch_funcs,
		nmp_rtsp_device_ref(device), 
		(GstRtspWatchFin)nmp_rtsp_device_watch_finalize
	);

	return TRUE;
}


__export gchar *
nmp_rtsp_device_get_localip(NmpMediaDevice *device)
{
	struct sockaddr_in s;
	socklen_t len;
	gint rfd;

	gchar ip[__MAX_IP_LEN];

	G_ASSERT(device != NULL);

	if (device->localip[0])
		return g_strdup(device->localip);

	if (device->watch)		/* !DMZ */
	{
		rfd = g_event_fd(device->watch);
		if (rfd < 0)
		{
			nmp_warning(
				"Get local ip failed, Invalid deivce '%s' rtsp connection fd",
				device->id
			);
			return NULL;
		}

		len = sizeof(s);
		if (getsockname(rfd, (struct sockaddr*)&s, &len) < 0 ||
			!inet_ntop(AF_INET, &s.sin_addr, ip, __MAX_IP_LEN))
		{
			nmp_warning(
				"Get device '%p' '%s' local ip failed, err: '%d'",
				device, device->id, -errno
			);
			return NULL;
		}

		return g_strdup(ip);
	}

	nmp_warning(
		"Get local ip failed, Invalid deivce '%s' rtsp connection",
		device->id
	);
	return NULL;
}


__export void
nmp_rtsp_device_attach(NmpMediaDevice *device, void *context)
{
	G_ASSERT(device != NULL);

	gst_rtsp_watch_attach_2(device->watch, context);
}


static __inline__ gint
__nmp_rtsp_device_watch_send_message(NmpMediaDevice *device,
	GstRTSPMessage *message)
{
	gint ret = -1;

	if (device->watch)
	{
		ret = 0;
		gst_rtsp_watch_send_message_2(device->watch, message);
	}
	return ret;
}


static __inline__ gint
nmp_rtsp_device_watch_send_response(NmpMediaDevice *device,
	GstRTSPMessage *message)
{
	gint ret;

	g_mutex_lock(device->device_lock);
	ret = __nmp_rtsp_device_watch_send_message(device, message);
	g_mutex_unlock(device->device_lock);

	return ret;
}


__export void
nmp_rtsp_device_send_response(NmpMediaDevice *device, 
	GstRTSPMessage *response)
{
	gst_rtsp_message_add_header(response, 
		GST_RTSP_HDR_SERVER, NMP_MEDIA_SERVER_LOGO);
	gst_rtsp_message_remove_header(response, GST_RTSP_HDR_SESSION, -1);
	nmp_rtsp_device_watch_send_response(device, response);
	gst_rtsp_message_unset(response);
}


static __inline__ void
nmp_rtsp_message_add_seq(GstRTSPMessage *request, gint seq)
{
	gchar *cseq;

	cseq = g_strdup_printf("%d", seq);

	gst_rtsp_message_add_header(
		request,
		GST_RTSP_HDR_CSEQ,
		cseq
	);	

	g_free(cseq);
}


static __inline__ gint
nmp_rtsp_device_send_request(NmpMediaDevice *device, NmpRtspMedia *media,
	GstRTSPMessage *request)
{
	gint seq, ret;

	seq = nmp_rtsp_device_generate_seq(device);
	nmp_rtsp_message_add_seq(request, seq);
	nmp_rtsp_media_set_session_seq(media, seq);	

	ret = __nmp_rtsp_device_watch_send_message(device, request);
	gst_rtsp_message_free(request);
	return ret;
}


static __inline__ gint
__nmp_rtsp_device_send_extend_request(NmpMediaDevice *device,
	NmpRtspMedia *media, GstRTSPMessage *request)
{
	gint ret, seq;

	seq = nmp_rtsp_device_generate_seq(device);
	nmp_rtsp_message_add_seq(request, seq);

	ret = __nmp_rtsp_device_watch_send_message(device, request);
	gst_rtsp_message_free(request);
	return ret;
}


static __inline__ NmpDeviceState
__nmp_rtsp_device_get_state(NmpMediaDevice *device)
{
	return device->state;
}


static __inline__ NmpDeviceState
nmp_rtsp_device_get_state(NmpMediaDevice *device)
{
	NmpDeviceState state;

	g_mutex_lock(device->device_lock);
	state = __nmp_rtsp_device_get_state(device);
	g_mutex_unlock(device->device_lock);

	return state;
}


__export gboolean
nmp_rtsp_device_is_new(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	return nmp_rtsp_device_get_state(device)
		== NMP_DEV_STAT_NEW;
}


static __inline__ gboolean
nmp_rtsp_device_is_ok(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	return __nmp_rtsp_device_get_state(device)
		== NMP_DEV_STAT_REGISTERED;
}


static __inline__ void
__nmp_rtsp_device_set_state(NmpMediaDevice *device,
	NmpDeviceState state)
{
	GEvent *watch = NULL;
	GList *list = NULL;

	if (device->state == state)
		return;

	if (device->state == NMP_DEV_STAT_ILLEGAL)
	{
		nmp_error(
			"Can't change a 'NMP_DEV_STAT_ILLEGAL' device's state."
		);
		FATAL_ERROR_EXIT;
	}

	if (state == NMP_DEV_STAT_ILLEGAL)
	{
		if (device->live_medias)
		{
			list = device->live_medias;
			device->live_medias = NULL;
		}

		if (device->watch)
		{
			watch = (GEvent*)device->watch;
			device->watch = NULL;
		}

		device->state= NMP_DEV_STAT_ILLEGAL;
		g_mutex_unlock(device->device_lock);

		if (watch)
		{
			/*
			 * !!g_event_unref()可能直接导致finalize().
			*/
			g_event_remove(watch);
			g_event_unref(watch);
		}

		if (list)
		{
			nmp_rtsp_device_unref_all_medias(
				device, list
			);
		}

		g_mutex_lock(device->device_lock);
		return;
	}

	device->state = state;
}


static __inline__ void
nmp_rtsp_device_set_state(NmpMediaDevice *device,
	NmpDeviceState state)
{
	g_mutex_lock(device->device_lock);
	__nmp_rtsp_device_set_state(device, state);
	g_mutex_unlock(device->device_lock);
}


void
nmp_rtsp_device_set_registered(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	nmp_rtsp_device_set_state(device,
		NMP_DEV_STAT_REGISTERED);
}


__export void
nmp_rtsp_device_set_illegal(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	nmp_rtsp_device_set_state(device, 
		NMP_DEV_STAT_ILLEGAL);
}


__export NmpRtspMediaFactory *
nmp_rtsp_device_get_media_factory(NmpMediaDevice *device)
{
	G_ASSERT(device != NULL);

	return jxj_device_factory;
}


static __inline__ gint
nmp_rtsp_device_run_rtsp(NmpMediaDevice *device, 
	NmpRtspMedia *media)
{
	gint ret;
	GstRTSPMessage *request = NULL;
	
	if (nmp_rtsp_media_get_session_state(media) ==
		NMP_SESSION_OPTIONS)
		return 0;

	if (nmp_rtsp_media_play_waiting(media))
		return 0;

	ret = nmp_rtsp_media_make_request(media, &request);
	if (G_UNLIKELY(ret))
		return ret;

	return nmp_rtsp_device_send_request(device, media, request);
}


static __inline__ gint
nmp_rtsp_device_run_session(NmpMediaDevice *device, 
	NmpRtspMedia *media)
{
	return nmp_rtsp_device_run_rtsp(device, media);
}


static __inline__ gint
nmp_rtsp_device_rerun_session(NmpMediaDevice *device, 
	NmpRtspMedia *media)
{
	//${TODO:reset media session state}
	return nmp_rtsp_device_run_rtsp(device, media);
}


static __inline__ void
nmp_rtsp_device_add_media(NmpMediaDevice *device,
	NmpRtspMedia *media)
{
	device->live_medias = g_list_append(
		device->live_medias,
		media
	);
	++device->media_count;

	nmp_rtsp_media_ref(media);
	nmp_rtsp_media_attach_device(media, device);
}


static __inline__ void
__nmp_rtsp_device_remove_media(NmpMediaDevice *device,
	NmpRtspMedia *media)
{
	GList *list;

	if (!nmp_rtsp_device_is_ok(device))
	{
		BUG_ON(device->live_medias);
		return;
	}

	list = g_list_find(device->live_medias, media);
	if (list)
	{
		device->live_medias = g_list_delete_link(
			device->live_medias,
			list
		);
		--device->media_count;

		nmp_rtsp_media_detach_device(media, device);
		nmp_rtsp_media_unref(media); /* 注意次序，dettach时要访问media 对象*/
	}
}


__export void
nmp_rtsp_device_remove_media(NmpMediaDevice *device,
	NmpRtspMedia *media)
{
	G_ASSERT(device != NULL && media != NULL);
	
	g_mutex_lock(device->device_lock);
	__nmp_rtsp_device_remove_media(device, media);
	g_mutex_unlock(device->device_lock);
}


static gint
nmp_rtsp_media_uri_equ(gconstpointer _media, gconstpointer _info)
{
	NmpRtspMedia *media = (NmpRtspMedia*)_media;
	NmpMediaUri *media_uri = media->media_uri;
	NmpMediaUri *info = (NmpMediaUri*)_info;

	if (G_UNLIKELY(media->zombie))
		return 1;

	if (media_uri->type == info->type &&
		media_uri->channel == info->channel &&
		media_uri->rate_level == info->rate_level &&
		media_uri->sequence == info->sequence)
	{
		return 0;
	}
	return 1;
}


static gint
nmp_rtsp_media_seq_equ(gconstpointer _media, gconstpointer _seq)
{
	NmpRtspMedia *media = (NmpRtspMedia*)_media;
	gint seq = (gint)_seq;

	if (media->session->cseq == seq)
		return 0;

	return 1;
}


static __inline__ gint
__nmp_rtsp_device_request(NmpMediaDevice *device, NmpRtspMedia *media)
{
	NmpSessionState s_state;
	gint err;

	s_state = nmp_rtsp_media_get_session_state(media);
	if (s_state == NMP_SESSION_SETUP)
	{
		err = nmp_rtsp_media_prepare(media, device->media_type);
		if (G_UNLIKELY(err))
			return err;
	}

	err = nmp_rtsp_device_run_session(device, media);
	return err;
}


__export gint
nmp_rtsp_device_request(NmpMediaDevice *device, NmpRtspMedia *media)
{
	gint err = -E_DKILLED;	/* device illegal */
	G_ASSERT(device != NULL && media != NULL);

	g_mutex_lock(device->device_lock);
	if (nmp_rtsp_device_is_ok(device))
	{
		err = __nmp_rtsp_device_request(device, media);
	}
	g_mutex_unlock(device->device_lock);

	return err;
}


__export gint
nmp_rtsp_device_extend_request(NmpMediaDevice *device, 
	NmpRtspMedia *media, gint name, gpointer value)
{
	GstRTSPMessage *request = NULL;
	gint err;
	G_ASSERT(device != NULL && media != NULL);

	err = nmp_rtsp_media_make_param_request(
		media, &request, name, value
	);
	if (G_UNLIKELY(err))
		return err;

	BUG_ON(request == NULL);
	err = -E_DKILLED;

	g_mutex_lock(device->device_lock);
	if (nmp_rtsp_device_is_ok(device))
	{
		err = __nmp_rtsp_device_send_extend_request(
			device, media, request);
	}
	g_mutex_unlock(device->device_lock);

	return err;
}


static __inline__ gint
nmp_rtsp_device_start_media(NmpMediaDevice *device, NmpRtspMedia *media)
{
	gint err;

	if (!(err = __nmp_rtsp_device_request(device, media)))
	{
		nmp_rtsp_device_add_media(device, media);
	}

	return err;
}


static __inline__ gint
nmp_rtsp_device_restart_media(NmpMediaDevice *device, NmpRtspMedia *media)
{
	if (nmp_rtsp_media_session_expire(media))
		nmp_rtsp_device_rerun_session(device, media);
	return 0;
}


static __inline__ NmpRtspMedia *
__nmp_rtsp_device_find_media(NmpMediaDevice *device, NmpMediaUri *media_uri)
{
	GList *list;
	NmpRtspMedia *media = NULL;

	list = g_list_find_custom(
		device->live_medias,
		media_uri,
		nmp_rtsp_media_uri_equ
	);

	if (list)
	{
		media = (NmpRtspMedia*)list->data;
		nmp_rtsp_media_ref(media);
	}

	return media;
}


static __inline__ NmpRtspMedia *
nmp_rtsp_device_get_media_by_seq(NmpMediaDevice *device, gint seq)
{
	GList *list;
	NmpRtspMedia *media = NULL;
	
	list = g_list_find_custom(
		device->live_medias,
		(gpointer)seq,
		nmp_rtsp_media_seq_equ
	);

	if (list)
	{
		media = (NmpRtspMedia*)list->data;
		nmp_rtsp_media_ref(media);
	}

	return media;		
}


__export NmpRtspMedia *
_nmp_rtsp_device_find_media(NmpMediaDevice *device, NmpMediaUri *media_uri)
{
	NmpRtspMediaFactory *factory;
	NmpRtspMedia *media;

	media = __nmp_rtsp_device_find_media(device, media_uri);
	if (!media)
	{
		factory = nmp_rtsp_device_get_media_factory(device);
		if (G_LIKELY(factory))
		{
			/* create new media */
			media = nmp_rtsp_media_factory_create_media(factory, media_uri);
			if (media)
			{
				if (nmp_rtsp_device_start_media(device, media))
				{
					g_mutex_unlock(device->device_lock);
					nmp_rtsp_media_kill_unref(media);
					g_mutex_lock(device->device_lock);
					media = NULL;
				}
			}
		}
	}
	else
	{
		nmp_rtsp_device_restart_media(device, media);
	}

	return media;
}


__export NmpRtspMedia *
nmp_rtsp_device_find_media(NmpMediaDevice *device, NmpMediaUri *media_uri)
{
	NmpRtspMedia *media = NULL;
	G_ASSERT(device != NULL && media_uri != NULL);

	g_mutex_lock(device->device_lock);
	if (nmp_rtsp_device_is_ok(device))
		media = _nmp_rtsp_device_find_media(device, media_uri);
	g_mutex_unlock(device->device_lock);

	return media;
}


__export NmpRtspMedia *
nmp_rtsp_device_get_media(NmpMediaDevice *device, gint seq)
{
	NmpRtspMedia *media = NULL;
	G_ASSERT(device != NULL);

	g_mutex_lock(device->device_lock);
	if (nmp_rtsp_device_is_ok(device))
		media = nmp_rtsp_device_get_media_by_seq(device, seq);
	g_mutex_unlock(device->device_lock);

	return media;
}

//:~ End
