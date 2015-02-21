/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_RTSP_CLIENT_H__
#define __TINY_RAIN_RTSP_CLIENT_H__

#include "network_client.h"

BEGIN_NAMESPACE

enum
{	/* rtsp msg types that handled by tiny-rain */
	TR_RTSP_UNKNOWN,
	TR_RTSP_OPTION,
	TR_RTSP_DESC,
	TR_RTSP_SETUP,
	TR_RTSP_PLAY,
	TR_RTSP_PAUSE,
	TR_RTSP_TEARDOWN,
	TR_RTSP_SETPARM,
	TR_RTSP_GETPARM,
	TR_RTSP_DATA,
	TR_RTSP_RESPONSE
};

typedef enum
{	/* rtsp server state machine, states */
	STATE_INIT = 0,
	STATE_READY,
	STATE_PLAYING,
	STATE_RECORDING
}rtsp_state;

typedef struct __rtsp_client rtsp_client;
typedef struct __rtsp_client_ops rtsp_client_ops;

struct __rtsp_client
{
	network_client client_base;
	rtsp_client_ops	*ops;
};


struct __rtsp_client_ops
{
	int32_t (*init)(rtsp_client *rc);
	void	(*fin)(rtsp_client *rc);
	proto_parser *(*create_proto_parser)(rtsp_client *rc);
	void 	(*release_proto_parser)(rtsp_client *rc, proto_parser *parser);
	void	(*kill)(rtsp_client *rc);
	session *(*create_session)(rtsp_client *rc, void *p);
	uint32_t (*recognize)(rtsp_client *rc, msg *req);
	int32_t	(*on_option)(rtsp_client *rc, msg *req);
	int32_t (*on_desc)(rtsp_client *rc, msg *req);
	int32_t (*on_setup)(rtsp_client *rc, msg *req);
	int32_t (*on_play)(rtsp_client *rc, msg *req);
    int32_t (*on_pause)(rtsp_client *rc, msg *req);
	int32_t (*on_teardown)(rtsp_client *rc, msg *req);
	int32_t (*on_setparm)(rtsp_client *rc, msg *req);
	int32_t (*on_getparm)(rtsp_client *rc, msg *req);
	int32_t (*on_data)(rtsp_client *rc, msg *req);
	void    (*on_closed)(rtsp_client *rc);
	void    (*on_error)(rtsp_client *rc, int32_t err);
};


rtsp_client *rtsp_client_new(uint32_t size, rtsp_client_ops *ops,
	uint32_t factory, void *io);

int32_t rtsp_client_send_msg(rtsp_client *rc, msg *res, uint32_t seq);

END_NAMESPACE

#endif	//__TINY_RAIN_RTSP_CLIENT_H__
