#include <string.h>
#include "tiny_rain.h"
#include "server.h"
#include "tr_avs.h"
#include "tr_log.h"
#include "version.h"
#include "ldl_src.h"
#include "ldh_src.h"


ls_avs_ops *lso = NULL;
hs_avs_ops *hso = NULL;

static ls_avs_ops live_av_ops = { NULL };
static hs_avs_ops histroy_av_ops = { NULL };
static tr_server *avs_rtsp_server = NULL;

void
avs_media_set_u(avs_media *m, void *u)
{
	ld_src *ld = (ld_src*)m;
	ld->u = u;
}


void *
avs_media_get_u(avs_media *m)
{
	ld_src *ld = (ld_src*)m;
	return ld->u;
}


int32_t
avs_media_fill(avs_media *m, frame_t *frm)
{
	ld_src *lds = (ld_src*)m;
	ldl_src *ldl = (ldl_src*)lds;

	if (!lds->ldl)
		return -EPERM;

	return ldl_src_consume(ldl, frm);
}


void
avs_media_kill(avs_media *m)
{
	ld_src *ld = (ld_src*)m;
	ld->break_off = 1;
}


avs_media *
avs_media_ref(avs_media *m)
{
	media_src *ms = (media_src*)m;

	if (ms)
	{
		media_src_ref(ms);
	}

	return m;
}


void
avs_media_unref(avs_media *m)
{
	media_src *ms = (media_src*)m;

	if (ms)
	{
		media_src_unref(ms);
	}
}


frame_t *
avs_alloc_frame(uint32_t data_size, uint32_t nal_count)
{
	if (data_size > MAX_FRAME_SIZE || nal_count > MAX_NALS_COUNT)
		return NULL;

	return ldh_alloc_frame(data_size, nal_count);
}


void
avs_free_frame(frame_t *frame)
{
	if (frame)
	{
		ldh_free_frame(frame);
	}
}


int32_t
avs_init( void )
{
	tiny_rain_init();
	return 0;
}


int32_t
avs_register_ops(ls_avs_ops *_lso, hs_avs_ops *_hso)
{
	if (_lso)
	{
		if (lso)
			return -EEXIST;

		if (!_lso->probe || !_lso->open || !_lso->close)
			return -EINVAL;
	}

	if (_hso)
	{
		if (hso)
			return -EEXIST;

		if (!_hso->probe || !_hso->open || !_hso->pull || !_hso->close)
			return -EINVAL;
	}

	if (_lso)
	{
		lso = &live_av_ops;
		memcpy(lso, _lso, sizeof(*lso));
	}

	if (_hso)
	{
		
		hso = &histroy_av_ops;
		memcpy(hso, _hso, sizeof(*hso));
	}

	return 0;
}


int32_t
avs_start(uint16_t port)
{
	int32_t err;

	if (!avs_rtsp_server)
	{
		avs_rtsp_server = tr_create_server(0);
		if (!avs_rtsp_server)
		{
			LOG_W(
				"avs_start()->tr_create_server() failed."
			);
			return -ENOMEM;
		}
	}

	err = server_bind_port(avs_rtsp_server, port);
	if (err)
	{
		LOG_W(
			"avs_start()->server_bind_port(%d) failed, err:'%d'.",
			port, err
		);
		return err;
	}

	start_server(avs_rtsp_server);
	return 0;
}


void
avs_stop( void )
{
}


tr_server*
avs_get_server( void )
{
	return avs_rtsp_server;
}


int32_t
avs_set_stream_ports_range(uint16_t low, uint16_t hi)
{
	return 0;
}


int32_t
avs_get_version(uint32_t *major, uint32_t *minor)
{
	return sscanf(VERSION_NUM, "%d.%d", major, minor) != 2;
}


int32_t
avs_log_set_handler(log_handler lf)
{
	tr_log_set_handler((log_handler_t)lf);
	return 0;
}


int32_t
avs_log_set_verbose(int8_t level)
{
	tr_log_set_verbose(level);
	return 0;
}


//:~ End
