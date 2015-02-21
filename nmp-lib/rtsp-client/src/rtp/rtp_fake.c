#include <standard_frame.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include "rtsp_client.h"
#include "rtsp.h"
#include "rtpparser.h"

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))


struct rtp_fake_parser_private
{
	gchar	*conf;
	gsize	conf_len;
};


static gint
rtp_fake_init_parser(RTP_Session *rtp_s, RTP_Parser *parser, gint pt)
{
	const gchar *attr_val;
	gchar *attr_copy, *sps, *pli, *pri;
	RTP_Cnf cnf;

	memset(&cnf, 0, sizeof(cnf));
	cnf.stm = rtp_s->stm_type;

	if (rtp_s->stm_type == STM_VIDEO)
	{
	#define SPS "sprop-parameter-sets="
	#define PLI "profile-level-id="
		attr_val = gst_sdp_media_get_attribute_val(rtp_s->m, "fmtp");
		if (attr_val)
		{
			attr_copy = g_strdup(attr_val);
	
			sps = strstr(attr_copy, SPS);
			pli = strstr(attr_copy, PLI);
	
			if (sps)
			{
				strtok(sps, ";");
				strncpy(cnf.sps, sps + strlen(SPS), SDP_ATTR_SIZE - 1);
				cnf.sps_len = strlen(cnf.sps) + 1;
			}
	
			if (pli)
			{
				strtok(pli, ";");
				strncpy(cnf.pli, pli + strlen(PLI), SDP_ATTR_SIZE - 1);
				cnf.pli_len = strlen(cnf.pli) + 1;
			}
	
			g_free(attr_copy);
		}
	#undef SPS
	
		else
	
		{
	#define SDP_MEDIA_ATTR	"pri"
	#define SDP_MEDIA_ATTR_DC "decode-header="
			attr_val = gst_sdp_media_get_attribute_val(rtp_s->m, SDP_MEDIA_ATTR);
			if (attr_val)
			{
				attr_copy = g_strdup(attr_val);
	
				pri = strstr(attr_copy, SDP_MEDIA_ATTR_DC);
				if (pri)
				{
					strtok(pri, ";");
					strncpy(cnf.pri, pri + strlen(SDP_MEDIA_ATTR_DC), SDP_ATTR_SIZE - 1);
					cnf.pri_len = strlen(cnf.pri) + 1;
				}
	
				g_free(attr_copy);
			}
	#undef SDP_MEDIA_ATTR
	#undef SDP_MEDIA_ATTR_DC
		}
	}
	else if (rtp_s->stm_type == STM_AUDIO)
	{
	#define SDP_AUDIO_RTPMAP	"rtpmap"
		attr_val = gst_sdp_media_get_attribute_val(rtp_s->m, SDP_AUDIO_RTPMAP);
		if (attr_val)
		{
			strncpy(cnf.pri, attr_val, SDP_ATTR_SIZE - 1);
			cnf.pri_len = strlen(cnf.pri) + 1;
		}
	#undef SDP_AUDIO_RTPMAP
	}

	INVOKE_DAT_HOOK_CNF((RTSP_client_session*)rtp_s->rtsp_s, &cnf)
	return 0;
}


static gint
rtp_fake_parse(RTP_Session *rtp_s, gchar *rtp, gint size, void *frame)
{
//	RTP_Parser *parser = rtp_s->parser;	
	RTP_pkt	*pkt = (RTP_pkt*)rtp;
	RTP_Frame *fr = (RTP_Frame*)frame;

	fr->data = (guint8*)pkt;
	fr->len = size;

	return RTP_PKT_OK;
}


static gint
rtp_fake_uninit_parser(RTP_Session *rtp_s)
{
	return 0;
}


gchar* rtp_fake_parser_mime[] = {"RTP-FAKE", NULL};

RTP_Parser_Template rtp_fake_parser_template =
{
	rtp_fake_parser_mime,
	rtp_fake_init_parser,
	rtp_fake_parse,
	rtp_fake_uninit_parser
};

//:~ End
