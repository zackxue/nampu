/*
 * talk_api.h
 *
 * @data 2013-4-28
 */

#ifndef __TALK_API_H__
#define __TALK_API_H__

#include "media_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef talk_frame_hdr_t frame_t;
typedef void talk_handle_t;

typedef struct _talk_ops
{
    int (*open)(talk_handle_t *hdl, const char* device, const int channel, media_info_t *info);
    int (*close)(talk_handle_t *hdl);
    int (*recv)(talk_handle_t *hdl, char *data, int len);
}talk_ops_t;

int send_talk_data(talk_handle_t *hdl, frame_t *frm);

void set_user_data(talk_handle_t *hdl, void *u);
void *get_user_data(talk_handle_t *hdl);
int register_talk_ops(talk_ops_t *ops); /* 注册对讲的接口 */

talk_handle_t *talk_handle_ref(talk_handle_t *hdl);
void talk_handle_unref(talk_handle_t *hdl);

#ifdef __cplusplus
}
#endif

#endif