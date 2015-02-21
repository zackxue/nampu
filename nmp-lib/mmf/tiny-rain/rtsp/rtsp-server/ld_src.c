#include "ld_src.h"
#include "alloc.h"

#include "rtsp_utc.h"

#define MAX_FMTP	256

void
__fill_media_info(ld_src *lds, media_info *msi, media_info_t *mi)
{
	stm_info *stm;
	unsigned char *sps_ptr, *pps_ptr;
	unsigned int sps_size, pps_size, fmtp_size;
	unsigned int profile_level_id = 0;
	char fmtp[MAX_FMTP];
	pic_parm_t *pic;

	memset(msi, 0, sizeof(*msi));
	if (mi->begin_time == mi->end_time)
	{
		strcpy(
			__str(msi->range),
			"npt=0.0-"
		);
	}
	else
	{
#if 0
		snprintf(
			__str(msi->range),
			RANGE_MAX,
			"npt=%.1f-%.1f",
			(double)mi->begin_time,
			(double)mi->end_time
		);
#else
        char beg[RTSP_UTC_STR_LEN];
        char end[RTSP_UTC_STR_LEN];

		snprintf(
			__str(msi->range),
			RANGE_MAX,
			"clock=%s-%s",
            rtsp_utc2str(mi->begin_time, beg),
            rtsp_utc2str(mi->end_time, end)
        );
#endif
	}

	strncpy(
		__str(msi->descp),
		"MEDIA FROM IPC/IPNC/DVR/NVR",
		MAX_DESCP_LEN - 1
	);

	strncpy(
		__str(msi->email),
		"wangx@szjxj.com",
		MAX_EMAIL_LEN - 1
	);

	strncpy(
		__str(msi->phone),
		"0755-27696901",
		MAX_PHONE_LEN - 1
	);

	if (mi->mask & MEDIA_VIDEO)
	{
		stm = &msi->stms[msi->n_stms];
		lds->idx[ST_VIDEO] = msi->n_stms;
		++msi->n_stms;

		stm->stm_type = ST_VIDEO;
		stm->clock_rate = 0;
		stm->bit_rate = mi->video.bitrate;	/* kb */
		stm->sample_rate = 90000/* mi->video.samples_per_sec * 1000 */;
		stm->bits_per_sample = 0;
		stm->frame_rate = mi->video.frame_rate;

		stm->video.pixel_width = 0;
		stm->video.pixel_height = 0;

		if (mi->video.enc_type == ENC_H264)
		{
			stm->fmt = H264;
			strcpy(
				__str(stm->encoding_name),
				"H264"
			);
		}
		else if (mi->video.enc_type == ENC_MPEG4)
		{
			stm->fmt = MPEG4;
			strcpy(
				__str(stm->encoding_name),
				"MPEG4"
			);
		}
		else
		{
			stm->fmt = H264;		/* ! */
			strcpy(
				__str(stm->encoding_name),
				"H264"
			);
		}

		pic = &mi->video.pic_parm;
		profile_level_id = 0;
		sps_ptr = NULL;
		pps_ptr = NULL;

		if (pic->sps_size)
		{
			sps_size = base64_encode(pic->sps, pic->sps_size, &sps_ptr, &sps_size);
			profile_level_id = (pic->sps[1] << 16) | (pic->sps[2] << 8) |(pic->sps[3]);
		}

		if (pic->pps_size)
		{
			pps_size = base64_encode(pic->pps, pic->pps_size, &pps_ptr, &pps_size);
		}

		snprintf(
			fmtp,
			MAX_FMTP,
			"packetization-mode=1;profile-level-id=%X;sprop-parameter-sets=%s,%s",
			profile_level_id,
			sps_ptr ? __str(sps_ptr) : "",
			pps_ptr ? __str(pps_ptr) : ""
		);

		if (sps_ptr)
		{
			base64_free(sps_ptr, sps_size);
		}

		if (pps_ptr)
		{
			base64_free(pps_ptr, pps_size);
		}

		fmtp_size = strlen(fmtp) + 1;
		stm->pri_fields[stm->fields].data = tr_alloc(fmtp_size);
		strcpy(stm->pri_fields[stm->fields].data, fmtp);
		stm->pri_fields[stm->fields].field_type = FIELD_FMTP;
		stm->pri_fields[stm->fields].size = fmtp_size;
		++stm->fields;
	}

	if (mi->mask & MEDIA_AUDIO)
	{
		stm = &msi->stms[msi->n_stms];
		lds->idx[ST_AUDIO] = msi->n_stms;
		++msi->n_stms;

		stm->stm_type = ST_AUDIO;
		stm->clock_rate = 0;
		stm->bit_rate = mi->audio.bitrate;
		stm->sample_rate = 8000/* mi->audio.samples_per_sec * 1000 */;
		stm->bits_per_sample = mi->audio.bits_per_sample;
		stm->frame_rate = mi->audio.frame_rate;

		stm->audio.audio_channels = mi->audio.channel_num;;

		if (mi->audio.enc_type == ENC_G711A)
		{
			stm->fmt = G711A;
			strcpy(
				__str(stm->encoding_name),
				"PCMA"
			);
		}
		else if (mi->audio.enc_type == ENC_G711U)
		{
			stm->fmt = G711U;
			strcpy(
				__str(stm->encoding_name),
				"PCMU"
			);
		}
		else
		{
			stm->fmt = G711A;		/* ! */
			strcpy(
				__str(stm->encoding_name),
				"PCMA"
			);
		}
	}
}


//:~ End
