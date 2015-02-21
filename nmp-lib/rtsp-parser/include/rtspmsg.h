/* *
 * This file is part of RTSP protocol parser library.
 *
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTIES OR REPRESENTATIONS; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.See the GNU General Public License
 * for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * */
#ifndef __GST_RTSP_MESSAGE_H__
#define __GST_RTSP_MESSAGE_H__

#include <glib.h>

#include "rtspdefs.h"

G_BEGIN_DECLS

/**
 * GstRTSPMsgType:
 * @GST_RTSP_MESSAGE_INVALID: invalid message type
 * @GST_RTSP_MESSAGE_REQUEST: RTSP request message
 * @GST_RTSP_MESSAGE_RESPONSE: RTSP response message
 * @GST_RTSP_MESSAGE_HTTP_REQUEST: HTTP request message. Since 0.10.25
 * @GST_RTSP_MESSAGE_HTTP_RESPONSE: HTTP response message. Since 0.10.25
 * @GST_RTSP_MESSAGE_DATA: data message
 *
 * The type of a message.
 */
typedef enum
{
  GST_RTSP_MESSAGE_INVALID,
  GST_RTSP_MESSAGE_REQUEST,
  GST_RTSP_MESSAGE_RESPONSE,
  GST_RTSP_MESSAGE_HTTP_REQUEST,
  GST_RTSP_MESSAGE_HTTP_RESPONSE,
  GST_RTSP_MESSAGE_DATA
} GstRTSPMsgType;

typedef struct _GstRTSPMessage GstRTSPMessage;

/**
 * GstRTSPMessage:
 * @type: the message type
 *
 * An RTSP message containing request, response or data messages. Depending on
 * the @type, the appropriate structure may be accessed.
 */
struct _GstRTSPMessage
{
  GstRTSPMsgType    type;

  union {
    struct {
      GstRTSPMethod      method;
      gchar             *uri;
      GstRTSPVersion     version;
    } request;
    struct {
      GstRTSPStatusCode  code;
      gchar             *reason;
      GstRTSPVersion     version;
    } response;
    struct {
      guint8             channel;
    } data;
  } type_data;

  /*< private >*/
  GArray        *hdr_fields;

  guint8        *body;
  guint          body_size;
};

/* memory management */
GstRTSPResult      gst_rtsp_message_new             (GstRTSPMessage **msg);
GstRTSPResult      gst_rtsp_message_init            (GstRTSPMessage *msg);
GstRTSPResult      gst_rtsp_message_unset           (GstRTSPMessage *msg);
GstRTSPResult      gst_rtsp_message_free            (GstRTSPMessage *msg);

GstRTSPMsgType     gst_rtsp_message_get_type        (GstRTSPMessage *msg);

/* request */
GstRTSPResult      gst_rtsp_message_new_request     (GstRTSPMessage **msg,
                                                     GstRTSPMethod method,
                                                     const gchar *uri);
GstRTSPResult      gst_rtsp_message_init_request    (GstRTSPMessage *msg,
                                                     GstRTSPMethod method,
                                                     const gchar *uri);
GstRTSPResult      gst_rtsp_message_parse_request   (GstRTSPMessage *msg,
                                                     GstRTSPMethod *method,
                                                     const gchar **uri,
						     GstRTSPVersion *version);

/* response */
GstRTSPResult      gst_rtsp_message_new_response    (GstRTSPMessage **msg,
                                                     GstRTSPStatusCode code,
                                                     const gchar *reason,
                                                     const GstRTSPMessage *request);
GstRTSPResult      gst_rtsp_message_init_response   (GstRTSPMessage *msg,
                                                     GstRTSPStatusCode code,
                                                     const gchar *reason,
                                                     const GstRTSPMessage *request);
GstRTSPResult      gst_rtsp_message_parse_response  (GstRTSPMessage *msg,
                                                     GstRTSPStatusCode *code,
                                                     const gchar **reason,
                                                     GstRTSPVersion *version);

/* data */
GstRTSPResult      gst_rtsp_message_new_data        (GstRTSPMessage **msg,
                                                     guint8 channel);
GstRTSPResult      gst_rtsp_message_init_data       (GstRTSPMessage *msg,
                                                     guint8 channel);
GstRTSPResult      gst_rtsp_message_parse_data      (GstRTSPMessage *msg,
                                                     guint8 *channel);

/* headers */
GstRTSPResult      gst_rtsp_message_add_header      (GstRTSPMessage *msg,
                                                     GstRTSPHeaderField field,
                                                     const gchar *value);
GstRTSPResult      gst_rtsp_message_take_header     (GstRTSPMessage *msg,
                                                     GstRTSPHeaderField field,
                                                     gchar *value);
GstRTSPResult      gst_rtsp_message_remove_header   (GstRTSPMessage *msg,
                                                     GstRTSPHeaderField field,
                                                     gint indx);
GstRTSPResult      gst_rtsp_message_get_header      (const GstRTSPMessage *msg,
                                                     GstRTSPHeaderField field,
                                                     gchar **value,
                                                     gint indx);
GstRTSPResult      gst_rtsp_message_append_headers  (const GstRTSPMessage *msg,
                                                     GString *str);

/* handling the body */
GstRTSPResult      gst_rtsp_message_set_body        (GstRTSPMessage *msg,
                                                     const guint8 *data,
                                                     guint size);
GstRTSPResult      gst_rtsp_message_take_body       (GstRTSPMessage *msg,
                                                     guint8 *data,
                                                     guint size);
GstRTSPResult      gst_rtsp_message_get_body        (const GstRTSPMessage *msg,
                                                     guint8 **data,
                                                     guint *size);
GstRTSPResult      gst_rtsp_message_steal_body      (GstRTSPMessage *msg,
                                                     guint8 **data,
                                                     guint *size);
/* mem dump */
GString *				gst_rtsp_message_to_string( GstRTSPMessage  * message);

/* debug */
GstRTSPResult      gst_rtsp_message_dump            (GstRTSPMessage *msg);

/* message dup */
GstRTSPMessage		*gst_rtsp_message_dup_request(GstRTSPMessage *request);

G_END_DECLS

#endif /* __GST_RTSP_MESSAGE_H__ */

