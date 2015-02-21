#include "frame_filter.h"
#include "tr_log.h"
#include "rtp_msg.h"
#include "media_stm.h"

typedef struct __video_frame_filter video_frame_filter;
struct __video_frame_filter
{
	filter __super;
	h264_fl_op_t *handle;
};


static int32_t
vff_init(filter *f)
{
	video_frame_filter *vff = (video_frame_filter*)f;

	vff->handle = h264_fl_op_init(RTP_MTU);
	if (!vff->handle)
		return -EPERM;

	return 0;
}


static void
vff_fin(filter *f)
{
	video_frame_filter *vff = (video_frame_filter*)f;

	h264_fl_op_fin(vff->handle);
}


static int32_t
vff_cal(filter *f, void *i, uint32_t i_size)
{
	video_frame_filter *vff = (video_frame_filter*)f;

	return h264_fl_op_count(vff->handle, (frame_t*)i);
}


static int32_t
vff_fill(filter *f, void *i, uint32_t i_size)
{
	video_frame_filter *vff = (video_frame_filter*)f;

	return h264_fl_op_fill(vff->handle, (frame_t*)i, 0);
}


static int32_t
vff_pull(filter *f, msg **o)
{
	int32_t err;
	rtp_mem_t *rtp;
	uint32_t size;
	video_frame_filter *vff = (video_frame_filter*)f;

	err = h264_fl_op_pull(vff->handle, &rtp, &size);
	if (!err)
	{
		*o = rtp_mem_s_to_msg(rtp);
	}

	return err;
}


static filter_ops vff_filter_ops =
{
	.filter_size = sizeof(video_frame_filter),
	.init	= vff_init,
	.fin	= vff_fin,
	.cal	= vff_cal,
	.fill	= vff_fill,
	.pull	= vff_pull
};


media_filter *
frame_filter_new( void )
{
	media_filter *mf;
	int32_t err;

	mf = media_filter_new();
	if (mf)
	{
		err = media_filter_register(mf, ST_VIDEO, &vff_filter_ops);
		if (err)
		{
			LOG_W(
				"frame_filter_new()->media_filter_register(VIDEO) failed."
			);
			media_filter_release(mf);
			return NULL;
		}

		err = media_filter_register(mf, ST_AUDIO, &vff_filter_ops);
		if (err)
		{
			LOG_W(
				"frame_filter_new()->media_filter_register(AUDIO) failed."
			);
			media_filter_release(mf);
			return NULL;
		}
	}

	return mf;
}


//:~ End
