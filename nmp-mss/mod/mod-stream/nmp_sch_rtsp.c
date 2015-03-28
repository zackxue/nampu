#include "nmp_sch_rtsp.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "rtsp_client.h"
#include "rtp.h"
#include "nmp_sdl.h"
#include "nmp_clock.h"
#include "nmp_rec.h"

#define RTSP_SCH_SDL_MAX			(16*1024)
#define RTP_MTU						(14*1024)
#define FAKE_DEV					0xABABABAB
#define STRANGER_I_RTP_INTERVAL		100
#define MAX_AUDIO_I                 256

typedef struct __NmpRtspSchPriv NmpRtspSchPriv;
struct __NmpRtspSchPriv
{
	RTSP_client_session	*s;
	gint err_code;
	RTP_Cnf sdp_cnf0;	/* video */
	RTP_Cnf sdp_cnf1;	/* audio */
	RTP_Cnf sdp_cnf2;	/* othre */
	gchar *sdl_buff;
	gsize sdl_buff_pos;
	gsize sdl_buff_size;
	gint last_pack_type;
	gint audio_counter;
};


static void
nmp_rtsp_sch_exception_callback(void *u, gint why)
{
	nmp_warning(
		"sch: '%p' exception! err: '%d'.", u, why
	);
}


static __inline__ gint
nmp_rtsp_is_iframe_pkt(NmpSCh *sch, RTP_pkt *rtp)
{
	j_rtp_extend_header_t *ext_hdr;
	j_rtp_extend_info_t *ext;

	if (sch->stranger)
	{
		if (--sch->stranger_pri <= 0)
		{
			sch->stranger_pri = STRANGER_I_RTP_INTERVAL;
			return 1;
		}
		return 0;
	}

	if (!rtp->extension)
	{
		return 0;
	}

	ext_hdr = (j_rtp_extend_header_t*)(gchar*)RTP_PKT_DATA(rtp);
	ext = (j_rtp_extend_info_t*)((gchar*)RTP_PKT_DATA(rtp) + 
		sizeof(j_rtp_extend_header_t));

	return ext_hdr->frame_type == 1;
}


static void
nmp_rtsp_sch_data_callback(void *u, RTP_Frame *fr, RTP_Cnf *rc, RTP_Ext *re)
{
	NmpSCh *sch = (NmpSCh*)u;
	NmpRtspSchPriv *sch_priv = sch->private;
	RTP_pkt *rtp;
	gint ipack, total_len, ext_len;
	gchar *sdl, *p;
	NmpSdlExtV1 *ext;

	sdl = sch_priv->sdl_buff;

	if (fr)
	{
		if (!fr->data || fr->len >= RTP_MTU)
		{
			return;
		}

		if (fr->stm != STM_VIDEO)
		{
			if (fr->stm == STM_AUDIO)
			{
				memcpy(SDL_V1_GET_DATA(sdl), (gchar*)fr->data, fr->len);
				total_len = sizeof(NmpSdlV1) + fr->len;
				SDL_V1_SET_DEV(sdl, FAKE_DEV);
				SDL_SET_LEN(sdl, total_len);
				SDL_V1_SET_EXT(sdl, 0);
				SDL_V1_SET_EXTLEN(sdl, 0);
				SDL_V1_SET_TS1(sdl, nmp_clock_get_time());
				SDL_V1_SET_TS2(sdl, fr->ts);
				SDL_V1_SET_TAGS(sdl, SDL_V1_TAGS_AUDIO);
				if (++sch_priv->audio_counter >= MAX_AUDIO_I)
				{
					sch_priv->audio_counter = 0;
					nmp_sbuff_write(sch->sb, sdl, total_len, REC_IFRAME_TAG|sch->rec_type);
				}
				else
				{
					nmp_sbuff_write(sch->sb, sdl, total_len, sch->rec_type);
				}
				return;
			}

			return;
		}

		nmp_sch_set_rec_state(sch);
		sch_priv->audio_counter = 0;

		rtp = (RTP_pkt*)fr->data;
		ipack = nmp_rtsp_is_iframe_pkt(sch, rtp);
		if (ipack && (ipack ^ sch_priv->last_pack_type))		/* First Iframe piece */
		{
			ext = (NmpSdlExtV1*)SDL_V1_GET_DATA(sdl);

			p = (gchar*)ext->data;
			ext->len1 = sch_priv->sdp_cnf0.sps_len;
			memcpy(p, sch_priv->sdp_cnf0.sps, ext->len1);

			p += ext->len1;
			ext->len2 = sch_priv->sdp_cnf0.pli_len;
			memcpy(p, sch_priv->sdp_cnf0.pli, ext->len2);

			p += ext->len2;
			ext->len3 = sch_priv->sdp_cnf0.pri_len;
			memcpy(p, sch_priv->sdp_cnf0.pri, ext->len3);

			p += ext->len3;
			ext_len = sizeof(NmpSdlExtV1) + (p - (gchar*)ext->data);

			memcpy(SDL_V1_GET_DATA(sdl) + ext_len, (gchar*)fr->data, fr->len);
			total_len = sizeof(NmpSdlV1) + ext_len + fr->len;

			SDL_V1_SET_DEV(sdl, FAKE_DEV);
			SDL_SET_LEN(sdl, total_len);
			SDL_V1_SET_EXT(sdl, SDL_V1_EXTS_SPS);
			SDL_V1_SET_EXTLEN(sdl, ext_len);
			SDL_V1_SET_TS1(sdl, nmp_clock_get_time());
			SDL_V1_SET_TS2(sdl, fr->ts);
			SDL_V1_SET_TAGS(sdl, SDL_V1_TAGS_IFRAME);
			nmp_sbuff_write(sch->sb, sdl, total_len, REC_IFRAME_TAG|sch->rec_type);			//TODO
		}
		else
		{
			memcpy(SDL_V1_GET_DATA(sdl), (gchar*)fr->data, fr->len);
			total_len = sizeof(NmpSdlV1) + fr->len;
			SDL_V1_SET_DEV(sdl, FAKE_DEV);
			SDL_SET_LEN(sdl, total_len);
			SDL_V1_SET_EXT(sdl, 0);
			SDL_V1_SET_EXTLEN(sdl, 0);
			SDL_V1_SET_TS1(sdl, nmp_clock_get_time());
			SDL_V1_SET_TS2(sdl, fr->ts);
			SDL_V1_SET_TAGS(sdl, 0);
			nmp_sbuff_write(sch->sb, sdl, total_len, sch->rec_type);			//TODO
		}

		sch_priv->last_pack_type = ipack;
		return;
	}

	if (rc)
	{
		if (rc->stm == STM_VIDEO)
		{
			memcpy(&sch_priv->sdp_cnf0, rc, sizeof(*rc));
		}
		else if (rc->stm == STM_AUDIO)
		{
			memcpy(&sch_priv->sdp_cnf1, rc, sizeof(*rc));
			memcpy(sch_priv->sdp_cnf0.pri, sch_priv->sdp_cnf1.pri, sch_priv->sdp_cnf1.pri_len);
			sch_priv->sdp_cnf0.pri_len = sch_priv->sdp_cnf1.pri_len;
		}
		else
		{
			memcpy(&sch_priv->sdp_cnf2, rc, sizeof(*rc));
		}

		return;
	}
}


static gint
nmp_rtsp_sch_init(NmpSCh *sch)
{
	NmpRtspSchPriv *p;

	p = g_new0(NmpRtspSchPriv, 1);
	p->sdl_buff = g_malloc(RTSP_SCH_SDL_MAX);
	p->sdl_buff_size = RTSP_SCH_SDL_MAX;
	SDL_HEADER_INIT(p->sdl_buff);

	sch->private = p;

	return 0;
}


static gint
nmp_rtsp_sch_open(NmpSCh *sch)
{
	NmpRtspSchPriv *p;
	gint ret;

	static gsize init_rtsp_lib = FALSE;
	if (g_once_init_enter(&init_rtsp_lib))
	{
		rtsp_client_init();
		g_once_init_leave(&init_rtsp_lib, TRUE);
	}

	if (!sch->uri)
		return -EINVAL;

	p = (NmpRtspSchPriv*)sch->private;
	BUG_ON(!p);

	if (p->s)
	{
		rtsp_client_close_session(p->s);
	}

	p->s = rtsp_client_create_session();
	if (p->s)
	{
		ret = rtsp_client_open_session(p->s, sch->uri, L4_TCP);
		if (ret)
		{
			nmp_print(
				"rtsp-sch: open session '%s' failed, err '%d'.",
				sch->uri, ret
			);

			rtsp_client_close_session(p->s);
			p->s = NULL;
			return ret;
		}

		rtsp_client_set_callback(p->s, nmp_rtsp_sch_exception_callback,
			nmp_rtsp_sch_data_callback, sch);

		ret = rtsp_client_play_session(p->s, 0, 0);
		if (ret)
		{
			nmp_print(
				"rtsp-sch: play session '%s' failed, err '%d'.",
				sch->uri, ret
			);

			rtsp_client_close_session(p->s);
			p->s = NULL;
			return ret;		
		}

		nmp_print(
			"rtsp-sch: '%s@%s' open stream.", sch->guid.domain_id,
			sch->guid.guid
		);

		return 0;
	}

	return -ENOMEM;
}


static gint
nmp_rtsp_sch_close(NmpSCh *sch)
{
	NmpRtspSchPriv *p;

	p = (NmpRtspSchPriv*)sch->private;
	BUG_ON(!p);

	if (p->s)
	{
		nmp_print(
			"rtsp-sch: '%s, %s' close stream.", sch->guid.domain_id,
			sch->guid.guid
		);
		rtsp_client_close_session(p->s);
		p->s = NULL;
	}

	return 0;
}


static gint
nmp_rtsp_sch_finalize(NmpSCh *sch)
{
	NmpRtspSchPriv *pri = (NmpRtspSchPriv*)sch->private;

	nmp_rtsp_sch_close(sch);
	g_free(pri->sdl_buff);
	g_free(pri);
	return 0;
}


NmpSchOps sch_rtsp_ops =
{
	.init           = nmp_rtsp_sch_init,
	.open_stream    = nmp_rtsp_sch_open,
	.close_stream   = nmp_rtsp_sch_close,
	.fin            = nmp_rtsp_sch_finalize
};


//:~ End
