#ifndef __NMP_RTSP_MSG_HANDLER_H__
#define __NMP_RTSP_MSG_HANDLER_H__

#include <rtspwatch.h>

G_BEGIN_DECLS

typedef struct _JpfRtspState JpfRtspState;
struct _JpfRtspState
{
	GstRTSPMessage      *request;
	GstRTSPMethod        method;
	GstRTSPUrl			*url;
	GstRTSPMessage      *response;
};

G_END_DECLS

#endif /* __NMP_RTSP_MSG_HANDLER_H__ */
