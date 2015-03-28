#ifndef __NMP_RTP_BUFFER_H__
#define __NMP_RTP_BUFFER_H__

#include <glib.h>

#define WRAP_HEAD_LEN		sizeof(JpfRtpWrap)

typedef struct _JpfRtpWrap JpfRtpWrap;
struct _JpfRtpWrap
{
#ifndef NDEBUG
	gint	magic;
#endif
	gint	length;
	gchar	rtp[0];
};

extern JpfRingBufferOps rtp_buffer_ops;

#endif	//__NMP_RTP_BUFFER_H__
