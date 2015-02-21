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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "base64.h"
#include "rtspctx.h"

#ifdef G_OS_WIN32
#define READ_SOCKET(fd, buf, len) recv (fd, (char *)buf, len, MSG_DONTWAIT)
#define WRITE_SOCKET(fd, buf, len) send (fd, (const char *)buf, len, SEND_FLAGS)
#define SETSOCKOPT(sock, level, name, val, len) setsockopt (sock, level, name, (const char *)val, len)
#define CLOSE_SOCKET(sock) closesocket (sock)
#define ERRNO_IS_EAGAIN (WSAGetLastError () == WSAEWOULDBLOCK)
#define ERRNO_IS_EINTR (WSAGetLastError () == WSAEINTR)
/* According to Microsoft's connect() documentation this one returns
 * WSAEWOULDBLOCK and not WSAEINPROGRESS. */
#define ERRNO_IS_EINPROGRESS (WSAGetLastError () == WSAEWOULDBLOCK)
#else
#define READ_SOCKET(fd, buf, len) read (fd, buf, len)
#define WRITE_SOCKET(fd, buf, len) send (fd, buf, len, SEND_FLAGS)
#define SETSOCKOPT(sock, level, name, val, len) setsockopt (sock, level, name, val, len)
#define CLOSE_SOCKET(sock) close (sock)
#define ERRNO_IS_EAGAIN (errno == EAGAIN)
#define ERRNO_IS_EINTR (errno == EINTR)
#define ERRNO_IS_EINPROGRESS (errno == EINPROGRESS)
#endif

#define TUNNELID_LEN   24

typedef enum
{
  TUNNEL_STATE_NONE,
  TUNNEL_STATE_GET,
  TUNNEL_STATE_POST,
  TUNNEL_STATE_COMPLETE
} GstRTSPTunnelState;

typedef struct
{
  gint state;
  guint save;
  guchar out[3];                /* the size must be evenly divisible by 3 */
  guint cout;
  guint coutl;
} DecodeCtx;

struct _GstRTSPContext
{
  gboolean manual_http;			/* 是否使用HTTP */

  gchar tunnelid[TUNNELID_LEN];
  gboolean tunneled;
  GstRTSPTunnelState tstate;

  gint read_ahead;

  gchar *initial_buffer;		/* 在使用此结构接收数据之前，已经被预先接收出的数据 */
  gsize initial_buffer_offset;

  gchar *rcv_buffer;			/* rtsp信令接收缓冲区 */
  gsize rcv_buffer_size; 		/* 缓冲区总长度 */
  gsize rcv_buffer_offset;		/* 有效数据起始 */
  gsize rcv_buffer_bytes;		/* 有效数据长度 */
 
 
  /* Session state */
  gchar session_id[512];        /* session id cache */
  gint timeout;                 /* session timeout in seconds */

  DecodeCtx ctx;
  DecodeCtx *ctxp;				/* we need base64 decoding for the readfd */
};

enum
{
  STATE_START = 0,
  STATE_DATA_HEADER,
  STATE_DATA_BODY,
  STATE_READ_LINES,
  STATE_END,
  STATE_LAST
};

enum
{
  READ_AHEAD_EOH = -1,          /* end of headers */
  READ_AHEAD_CRLF = -2,
  READ_AHEAD_CRLFCR = -3
};

void
build_init(GstRTSPBuilder *builder)
{
	memset(builder, 0, sizeof(*builder));
	builder->state = STATE_START;
}

void
build_reset(GstRTSPBuilder *builder)
{
  g_free (builder->body_data);
  memset (builder, 0, sizeof (GstRTSPBuilder));
}

GstRTSPBuilder *
gst_rtsp_builder_new( void )
{
	GstRTSPBuilder *builder;

	builder = g_new0(GstRTSPBuilder, 1);
	build_init(builder);
	return builder;
}

void
gst_rtsp_builder_free(GstRTSPBuilder *builder)
{
	build_reset(builder);
	g_free(builder);
}

static gint
fill_raw_bytes (GstRTSPContext * rtsp_ctx, guint8 * buffer, guint size)
{
  gint out = 0;

  if (G_UNLIKELY (rtsp_ctx->initial_buffer != NULL)) {
    gsize left = strlen (&rtsp_ctx->initial_buffer[rtsp_ctx->initial_buffer_offset]);

    out = MIN (left, size);
    memcpy (buffer, &rtsp_ctx->initial_buffer[rtsp_ctx->initial_buffer_offset], out);

    if (left == (gsize) out) {
      g_free (rtsp_ctx->initial_buffer);
      rtsp_ctx->initial_buffer = NULL;
      rtsp_ctx->initial_buffer_offset = 0;
    } else
      rtsp_ctx->initial_buffer_offset += out;
  }

  if (G_LIKELY (size > (guint) out)) {
    gint to_r = size - out;
	if (rtsp_ctx->rcv_buffer_bytes) {
		to_r = MIN(to_r, rtsp_ctx->rcv_buffer_bytes);
		memcpy(&buffer[out], &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_offset], to_r);
		rtsp_ctx->rcv_buffer_offset += to_r;
		rtsp_ctx->rcv_buffer_bytes -= to_r;
		return out + to_r;
	} else {
		errno = EAGAIN;
		return -1;
	}
  }

  return out;
}

static gint
fill_bytes (GstRTSPContext * rtsp_ctx, guint8 * buffer, guint size)
{
  DecodeCtx *ctx = rtsp_ctx->ctxp;
  gint out = 0;

  if (ctx) {
    while (size > 0) {
      guint8 in[sizeof (ctx->out) * 4 / 3];
      gint r;

      while (size > 0 && ctx->cout < ctx->coutl) {
        /* we have some leftover bytes */
        *buffer++ = ctx->out[ctx->cout++];
        size--;
        out++;
      }

      /* got what we needed? */
      if (size == 0)
        break;

      /* try to read more bytes */
      r = fill_raw_bytes (rtsp_ctx, in, sizeof (in));
      if (r <= 0) {
        if (out == 0)
          out = r;
        break;
      }

      ctx->cout = 0;
      ctx->coutl =
          g_base64_decode_step ((gchar *) in, r, ctx->out, &ctx->state,
          &ctx->save);
    }
  } else {
    out = fill_raw_bytes (rtsp_ctx, buffer, size);
  }

  return out;

}

static GstRTSPResult
read_bytes (GstRTSPContext * rtsp_ctx, guint8 * buffer, guint * idx, guint size)
{
  guint left;

  if (G_UNLIKELY (*idx > size))
    return GST_RTSP_ERROR;

  left = size - *idx;

  while (left) {
    gint r;

    r = fill_bytes (rtsp_ctx, &buffer[*idx], left);
    if (G_UNLIKELY (r == 0)) {
      return GST_RTSP_EEOF;
    } else if (G_UNLIKELY (r < 0)) {
      if (ERRNO_IS_EAGAIN)
        return GST_RTSP_EINTR;
      if (!ERRNO_IS_EINTR)
        return GST_RTSP_ESYS;
    } else {
      left -= r;
      *idx += r;
    }
  }
  return GST_RTSP_OK;
}


/* The code below tries to handle clients using \r, \n or \r\n to indicate the
 * end of a line. It even does its best to handle clients which mix them (even
 * though this is a really stupid idea (tm).) It also handles Line White Space
 * (LWS), where a line end followed by whitespace is considered LWS. This is
 * the method used in RTSP (and HTTP) to break long lines.
 */
static GstRTSPResult
read_line (GstRTSPContext * rtsp_ctx, guint8 * buffer, guint * idx, guint size)
{
  while (TRUE) {
    guint8 c;
    gint r;

    if (rtsp_ctx->read_ahead == READ_AHEAD_EOH) {
      /* the last call to read_line() already determined that we have reached
       * the end of the headers, so convey that information now */
      rtsp_ctx->read_ahead = 0;
      break;
    } else if (rtsp_ctx->read_ahead == READ_AHEAD_CRLF) {
      /* the last call to read_line() left off after having read \r\n */
      c = '\n';
    } else if (rtsp_ctx->read_ahead == READ_AHEAD_CRLFCR) {
      /* the last call to read_line() left off after having read \r\n\r */
      c = '\r';
    } else if (rtsp_ctx->read_ahead != 0) {
      /* the last call to read_line() left us with a character to start with */
      c = (guint8) rtsp_ctx->read_ahead;
      rtsp_ctx->read_ahead = 0;
    } else {
      /* read the next character */
      r = fill_bytes (rtsp_ctx, &c, 1);
      if (G_UNLIKELY (r == 0)) {
        return GST_RTSP_EEOF;
      } else if (G_UNLIKELY (r < 0)) {
        if (ERRNO_IS_EAGAIN)
          return GST_RTSP_EINTR;
        if (!ERRNO_IS_EINTR)
          return GST_RTSP_ESYS;
        continue;
      }
    }

    /* special treatment of line endings */
    if (c == '\r' || c == '\n') {
      guint8 read_ahead;

    retry:
      /* need to read ahead one more character to know what to do... */
      r = fill_bytes (rtsp_ctx, &read_ahead, 1);
      if (G_UNLIKELY (r == 0)) {
        return GST_RTSP_EEOF;
      } else if (G_UNLIKELY (r < 0)) {
        if (ERRNO_IS_EAGAIN) {
          /* remember the original character we read and try again next time */
          if (rtsp_ctx->read_ahead == 0)
            rtsp_ctx->read_ahead = c;
          return GST_RTSP_EINTR;
        }
        if (!ERRNO_IS_EINTR)
          return GST_RTSP_ESYS;
        goto retry;
      }

      if (read_ahead == ' ' || read_ahead == '\t') {
        if (rtsp_ctx->read_ahead == READ_AHEAD_CRLFCR) {
          /* got \r\n\r followed by whitespace, treat it as a normal line
           * followed by one starting with LWS */
          rtsp_ctx->read_ahead = read_ahead;
          break;
        } else {
          /* got LWS, change the line ending to a space and continue */
          c = ' ';
          rtsp_ctx->read_ahead = read_ahead;
        }
      } else if (rtsp_ctx->read_ahead == READ_AHEAD_CRLFCR) {
        if (read_ahead == '\r' || read_ahead == '\n') {
          /* got \r\n\r\r or \r\n\r\n, treat it as the end of the headers */
          rtsp_ctx->read_ahead = READ_AHEAD_EOH;
          break;
        } else {
          /* got \r\n\r followed by something else, this is not really
           * supported since we have probably just eaten the first character
           * of the body or the next message, so just ignore the second \r
           * and live with it... */
          rtsp_ctx->read_ahead = read_ahead;
          break;
        }
      } else if (rtsp_ctx->read_ahead == READ_AHEAD_CRLF) {
        if (read_ahead == '\r') {
          /* got \r\n\r so far, need one more character... */
          rtsp_ctx->read_ahead = READ_AHEAD_CRLFCR;
          goto retry;
        } else if (read_ahead == '\n') {
          /* got \r\n\n, treat it as the end of the headers */
          rtsp_ctx->read_ahead = READ_AHEAD_EOH;
          break;
        } else {
          /* found the end of a line, keep read_ahead for the next line */
          rtsp_ctx->read_ahead = read_ahead;
          break;
        }
      } else if (c == read_ahead) {
        /* got double \r or \n, treat it as the end of the headers */
        rtsp_ctx->read_ahead = READ_AHEAD_EOH;
        break;
      } else if (c == '\r' && read_ahead == '\n') {
        /* got \r\n so far, still need more to know what to do... */
        rtsp_ctx->read_ahead = READ_AHEAD_CRLF;
        goto retry;
      } else {
        /* found the end of a line, keep read_ahead for the next line */
        rtsp_ctx->read_ahead = read_ahead;
        break;
      }
    }

    if (G_LIKELY (*idx < size - 1))
      buffer[(*idx)++] = c;
  }
  buffer[*idx] = '\0';

  return GST_RTSP_OK;
}

static GstRTSPResult
parse_string (gchar * dest, gint size, gchar ** src)
{
  GstRTSPResult res = GST_RTSP_OK;
  gint idx;

  idx = 0;
  /* skip spaces */
  while (g_ascii_isspace (**src))
    (*src)++;

  while (!g_ascii_isspace (**src) && **src != '\0') {
    if (idx < size - 1)
      dest[idx++] = **src;
    else
      res = GST_RTSP_EPARSE;
    (*src)++;
  }
  if (size > 0)
    dest[idx] = '\0';

  return res;
}

static GstRTSPResult
parse_protocol_version (gchar * protocol, GstRTSPMsgType * type,
    GstRTSPVersion * version)
{
  GstRTSPResult res = GST_RTSP_OK;
  gchar *ver;

  if (G_LIKELY ((ver = strchr (protocol, '/')) != NULL)) {
    guint major;
    guint minor;
    gchar dummychar;

    *ver++ = '\0';

    /* the version number must be formatted as X.Y with nothing following */
    if (sscanf (ver, "%u.%u%c", &major, &minor, &dummychar) != 2)
      res = GST_RTSP_EPARSE;

    if (g_ascii_strcasecmp (protocol, "RTSP") == 0) {
      if (major != 1 || minor != 0) {
        *version = GST_RTSP_VERSION_INVALID;
        res = GST_RTSP_ERROR;
      }
    } else if (g_ascii_strcasecmp (protocol, "HTTP") == 0) {
      if (*type == GST_RTSP_MESSAGE_REQUEST)
        *type = GST_RTSP_MESSAGE_HTTP_REQUEST;
      else if (*type == GST_RTSP_MESSAGE_RESPONSE)
        *type = GST_RTSP_MESSAGE_HTTP_RESPONSE;

      if (major == 1 && minor == 1) {
        *version = GST_RTSP_VERSION_1_1;
      } else if (major != 1 || minor != 0) {
        *version = GST_RTSP_VERSION_INVALID;
        res = GST_RTSP_ERROR;
      }
    } else
      res = GST_RTSP_EPARSE;
  } else
    res = GST_RTSP_EPARSE;

  return res;
}

static GstRTSPResult
parse_response_status (guint8 * buffer, GstRTSPMessage * msg)
{
  GstRTSPResult res = GST_RTSP_OK;
  GstRTSPResult res2;
  gchar versionstr[20];
  gchar codestr[4];
  gint code;
  gchar *bptr;

  bptr = (gchar *) buffer;

  if (parse_string (versionstr, sizeof (versionstr), &bptr) != GST_RTSP_OK)
    res = GST_RTSP_EPARSE;

  if (parse_string (codestr, sizeof (codestr), &bptr) != GST_RTSP_OK)
    res = GST_RTSP_EPARSE;
  code = atoi (codestr);
  if (G_UNLIKELY (*codestr == '\0' || code < 0 || code >= 600))
    res = GST_RTSP_EPARSE;

  while (g_ascii_isspace (*bptr))
    bptr++;

  if (G_UNLIKELY (gst_rtsp_message_init_response (msg, code, bptr,
              NULL) != GST_RTSP_OK))
    res = GST_RTSP_EPARSE;

  res2 = parse_protocol_version (versionstr, &msg->type,
      &msg->type_data.response.version);
  if (G_LIKELY (res == GST_RTSP_OK))
    res = res2;

  return res;
}

static GstRTSPResult
parse_request_line (guint8 * buffer, GstRTSPMessage * msg)
{
  GstRTSPResult res = GST_RTSP_OK;
  GstRTSPResult res2;
  gchar versionstr[20];
  gchar methodstr[20];
  gchar urlstr[4096];
  gchar *bptr;
  GstRTSPMethod method;

  bptr = (gchar *) buffer;

  if (parse_string (methodstr, sizeof (methodstr), &bptr) != GST_RTSP_OK)
    res = GST_RTSP_EPARSE;
  method = gst_rtsp_find_method (methodstr);

  if (parse_string (urlstr, sizeof (urlstr), &bptr) != GST_RTSP_OK)
    res = GST_RTSP_EPARSE;
  if (G_UNLIKELY (*urlstr == '\0'))
    res = GST_RTSP_EPARSE;

  if (parse_string (versionstr, sizeof (versionstr), &bptr) != GST_RTSP_OK)
    res = GST_RTSP_EPARSE;

  if (G_UNLIKELY (*bptr != '\0'))
    res = GST_RTSP_EPARSE;

  if (G_UNLIKELY (gst_rtsp_message_init_request (msg, method,
              urlstr) != GST_RTSP_OK))
    res = GST_RTSP_EPARSE;

  res2 = parse_protocol_version (versionstr, &msg->type,
      &msg->type_data.request.version);
  if (G_LIKELY (res == GST_RTSP_OK))
    res = res2;

  if (G_LIKELY (msg->type == GST_RTSP_MESSAGE_REQUEST)) {
    /* GET and POST are not allowed as RTSP methods */
    if (msg->type_data.request.method == GST_RTSP_GET ||
        msg->type_data.request.method == GST_RTSP_POST) {
      msg->type_data.request.method = GST_RTSP_INVALID;
      if (res == GST_RTSP_OK)
        res = GST_RTSP_ERROR;
    }
  } else if (msg->type == GST_RTSP_MESSAGE_HTTP_REQUEST) {
    /* only GET and POST are allowed as HTTP methods */
    if (msg->type_data.request.method != GST_RTSP_GET &&
        msg->type_data.request.method != GST_RTSP_POST) {
      msg->type_data.request.method = GST_RTSP_INVALID;
      if (res == GST_RTSP_OK)
        res = GST_RTSP_ERROR;
    }
  }

  return res;
}

/* parsing lines means reading a Key: Value pair */
static GstRTSPResult
parse_line (guint8 * buffer, GstRTSPMessage * msg)
{
  GstRTSPHeaderField field;
  gchar *line = (gchar *) buffer;
  gchar *value;

  if ((value = strchr (line, ':')) == NULL || value == line)
    goto parse_error;

  /* trim space before the colon */
  if (value[-1] == ' ')
    value[-1] = '\0';

  /* replace the colon with a NUL */
  *value++ = '\0';

  /* find the header */
  field = gst_rtsp_find_header_field (line);
  if (field == GST_RTSP_HDR_INVALID)
    goto done;

  /* split up the value in multiple key:value pairs if it contains comma(s) */
  while (*value != '\0') {
    gchar *next_value;
    gchar *comma = NULL;
    gboolean quoted = FALSE;
    guint comment = 0;

    /* trim leading space */
    if (*value == ' ')
      value++;

    /* for headers which may not appear multiple times, and thus may not
     * contain multiple values on the same line, we can short-circuit the loop
     * below and the entire value results in just one key:value pair*/
    if (!gst_rtsp_header_allow_multiple (field))
      next_value = value + strlen (value);
    else
      next_value = value;

    /* find the next value, taking special care of quotes and comments */
    while (*next_value != '\0') {
      if ((quoted || comment != 0) && *next_value == '\\' &&
          next_value[1] != '\0')
        next_value++;
      else if (comment == 0 && *next_value == '"')
        quoted = !quoted;
      else if (!quoted && *next_value == '(')
        comment++;
      else if (comment != 0 && *next_value == ')')
        comment--;
      else if (!quoted && comment == 0) {
        /* To quote RFC 2068: "User agents MUST take special care in parsing
         * the WWW-Authenticate field value if it contains more than one
         * challenge, or if more than one WWW-Authenticate header field is
         * provided, since the contents of a challenge may itself contain a
         * comma-separated list of authentication parameters."
         *
         * What this means is that we cannot just look for an unquoted comma
         * when looking for multiple values in Proxy-Authenticate and
         * WWW-Authenticate headers. Instead we need to look for the sequence
         * "comma [space] token space token" before we can split after the
         * comma...
         */
        if (field == GST_RTSP_HDR_PROXY_AUTHENTICATE ||
            field == GST_RTSP_HDR_WWW_AUTHENTICATE) {
          if (*next_value == ',') {
            if (next_value[1] == ' ') {
              /* skip any space following the comma so we do not mistake it for
               * separating between two tokens */
              next_value++;
            }
            comma = next_value;
          } else if (*next_value == ' ' && next_value[1] != ',' &&
              next_value[1] != '=' && comma != NULL) {
            next_value = comma;
            comma = NULL;
            break;
          }
        } else if (*next_value == ',')
          break;
      }

      next_value++;
    }

    /* trim space */
    if (value != next_value && next_value[-1] == ' ')
      next_value[-1] = '\0';

    if (*next_value != '\0')
      *next_value++ = '\0';

    /* add the key:value pair */
    if (*value != '\0')
      gst_rtsp_message_add_header (msg, field, value);

    value = next_value;
  }

done:
  return GST_RTSP_OK;

  /* ERRORS */
parse_error:
  {
    return GST_RTSP_EPARSE;
  }
}

/* convert all consecutive whitespace to a single space */
static void
normalize_line (guint8 * buffer)
{
  while (*buffer) {
    if (g_ascii_isspace (*buffer)) {
      guint8 *tmp;

      *buffer++ = ' ';
      for (tmp = buffer; g_ascii_isspace (*tmp); tmp++) {
      }
      if (buffer != tmp)
        memmove (buffer, tmp, strlen ((gchar *) tmp) + 1);
    } else {
      buffer++;
    }
  }
}

/* returns:
 *  GST_RTSP_OK when a complete message was read.
 *  GST_RTSP_EEOF: when the read socket is closed
 *  GST_RTSP_EINTR: when more data is needed.
 *  GST_RTSP_..: some other error occured.
 */
static GstRTSPResult
build_next(GstRTSPContext *rtsp_ctx, GstRTSPBuilder* builder,
	GstRTSPMessage *message)
{
  GstRTSPResult res;

  while (TRUE) {
    switch (builder->state) {
      case STATE_START:
      {
        guint8 c;

        builder->offset = 0;
        res = read_bytes(rtsp_ctx, (guint8 *)builder->buffer,
        	&builder->offset, 1);
        if (res != GST_RTSP_OK)
          goto done;

        c = builder->buffer[0];

        /* we have 1 bytes now and we can see if this is a data message or
         * not */
        if (c == '$') {
          /* data message, prepare for the header */
          builder->state = STATE_DATA_HEADER;
        } else if (c == '\n' || c == '\r') {
          /* skip \n and \r */
          builder->offset = 0;
        } else {
          builder->line = 0;
          builder->state = STATE_READ_LINES;
        }
        break;
      }
      case STATE_DATA_HEADER:
      {
        res = read_bytes(rtsp_ctx, (guint8 *)builder->buffer,
            &builder->offset, 4);
        if (res != GST_RTSP_OK)
          goto done;

        gst_rtsp_message_init_data (message, builder->buffer[1]);

        builder->body_len = (builder->buffer[2] << 8) | builder->buffer[3];
        builder->body_data = g_malloc (builder->body_len + 1);
        builder->body_data[builder->body_len] = '\0';
        builder->offset = 0;
        builder->state = STATE_DATA_BODY;
        break;
      }
      case STATE_DATA_BODY:
      {
        res = read_bytes(rtsp_ctx, builder->body_data, &builder->offset,
            builder->body_len);
        if (res != GST_RTSP_OK)
          goto done;

        /* we have the complete body now, store in the message adjusting the
         * length to include the traling '\0' */
        gst_rtsp_message_take_body (message,
            (guint8 *) builder->body_data, builder->body_len + 1);
        builder->body_data = NULL;
        builder->body_len = 0;

        builder->state = STATE_END;
        break;
      }
      case STATE_READ_LINES:
      {
        res = read_line (rtsp_ctx, builder->buffer, &builder->offset,
            sizeof (builder->buffer));
        if (res != GST_RTSP_OK)
          goto done;

        /* we have a regular response */
        if (builder->buffer[0] == '\0') {
          gchar *hdrval;

          /* empty line, end of message header */
          /* see if there is a Content-Length header, but ignore it if this
           * is a POST request with an x-sessioncookie header */
          if (gst_rtsp_message_get_header (message,
                  GST_RTSP_HDR_CONTENT_LENGTH, &hdrval, 0) == GST_RTSP_OK &&
              (message->type != GST_RTSP_MESSAGE_HTTP_REQUEST ||
                  message->type_data.request.method != GST_RTSP_POST ||
                  gst_rtsp_message_get_header (message,
                      GST_RTSP_HDR_X_SESSIONCOOKIE, NULL, 0) != GST_RTSP_OK)) {
            /* there is, prepare to read the body */
            builder->body_len = atol (hdrval);
            builder->body_data = g_try_malloc (builder->body_len + 1);
            /* we can't do much here, we need the length to know how many bytes
             * we need to read next and when allocation fails, something is
             * probably wrong with the length. */
            if (builder->body_data == NULL)
              goto invalid_body_len;

            builder->body_data[builder->body_len] = '\0';
            builder->offset = 0;
            builder->state = STATE_DATA_BODY;
          } else {
            builder->state = STATE_END;
          }
          break;
        }

        /* we have a line */
        normalize_line (builder->buffer);
        if (builder->line == 0) {
          /* first line, check for response status */
          if (memcmp (builder->buffer, "RTSP", 4) == 0 ||
              memcmp (builder->buffer, "HTTP", 4) == 0) {
            builder->status = parse_response_status (builder->buffer, message);
          } else {
            builder->status = parse_request_line (builder->buffer, message);
          }
        } else {
          /* else just parse the line */
          res = parse_line (builder->buffer, message);
          if (res != GST_RTSP_OK)
            builder->status = res;
        }
        builder->line++;
        builder->offset = 0;
        break;
      }
      case STATE_END:
      {
        gchar *session_cookie;
        gchar *session_id;

        if (message->type == GST_RTSP_MESSAGE_DATA) {
          /* data messages don't have headers */
          res = GST_RTSP_OK;
          goto done;
        }

        /* save the tunnel session in the rtsp_ctx */
        if (message->type == GST_RTSP_MESSAGE_HTTP_REQUEST &&
            !rtsp_ctx->manual_http &&
            rtsp_ctx->tstate == TUNNEL_STATE_NONE &&
            gst_rtsp_message_get_header (message, GST_RTSP_HDR_X_SESSIONCOOKIE,
                &session_cookie, 0) == GST_RTSP_OK) {
          strncpy (rtsp_ctx->tunnelid, session_cookie, TUNNELID_LEN);
          rtsp_ctx->tunnelid[TUNNELID_LEN - 1] = '\0';
          rtsp_ctx->tunneled = TRUE;
        }

        /* save session id in the rtsp_ctx for further use */
        if (message->type == GST_RTSP_MESSAGE_RESPONSE &&
            gst_rtsp_message_get_header (message, GST_RTSP_HDR_SESSION,
                &session_id, 0) == GST_RTSP_OK) {
          gint maxlen, i;

          maxlen = sizeof (rtsp_ctx->session_id) - 1;
          /* the sessionid can have attributes marked with ;
           * Make sure we strip them */
          for (i = 0; session_id[i] != '\0'; i++) {
            if (session_id[i] == ';') {
              maxlen = i;
              /* parse timeout */
              do {
                i++;
              } while (g_ascii_isspace (session_id[i]));
              if (g_str_has_prefix (&session_id[i], "timeout=")) {
                gint to;

                /* if we parsed something valid, configure */
                if ((to = atoi (&session_id[i + 8])) > 0)
                  rtsp_ctx->timeout = to;
              }
              break;
            }
          }

          /* make sure to not overflow */
          strncpy (rtsp_ctx->session_id, session_id, maxlen);
          rtsp_ctx->session_id[maxlen] = '\0';
        }
        res = builder->status;
        goto done;
      }
      default:
        res = GST_RTSP_ERROR;
        break;
    }
  }
done:
  return res;

  /* ERRORS */
invalid_body_len:
  {
    return GST_RTSP_ERROR;
  }
}


/**
 * gst_rtsp_ctx_set_http_mode:
 * @rtsp_ctx: a #GstRTSPContext
 * @enable: %TRUE to enable manual HTTP mode
 *
 * By setting the HTTP mode to %TRUE the message parsing will support HTTP
 * messages in addition to the RTSP messages. It will also disable the
 * automatic handling of setting up an HTTP tunnel.
 *
 * Since: 0.10.25
 */
void
gst_rtsp_ctx_set_http_mode (GstRTSPContext * rtsp_ctx, gboolean enable)
{
  g_return_if_fail (rtsp_ctx != NULL);

  rtsp_ctx->manual_http = enable;
}

/**
 * gst_rtsp_ctx_set_tunneled:
 * @rtsp_ctx: a #GstRTSPContext
 * @tunneled: the new state
 *
 * Set the HTTP tunneling state of the rtsp_ctx. This must be configured before
 * the @rtsp_ctx is connected.
 *
 * Since: 0.10.23
 */
void
gst_rtsp_ctx_set_tunneled (GstRTSPContext * rtsp_ctx, gboolean tunneled)
{
  g_return_if_fail (rtsp_ctx != NULL);

  rtsp_ctx->tunneled = tunneled;
}

/**
 * gst_rtsp_ctx_is_tunneled:
 * @rtsp_ctx: a #GstRTSPContext
 *
 * Get the tunneling state of the rtsp_ctx. 
 *
 * Returns: if @rtsp_ctx is using HTTP tunneling.
 *
 * Since: 0.10.23
 */
gboolean
gst_rtsp_ctx_is_tunneled (const GstRTSPContext * rtsp_ctx)
{
  g_return_val_if_fail (rtsp_ctx != NULL, FALSE);

  return rtsp_ctx->tunneled;
}

/**
 * gst_rtsp_ctx_get_tunnelid:
 * @rtsp_ctx: a #GstRTSPContext
 *
 * Get the tunnel session id the rtsp_ctx. 
 *
 * Returns: returns a non-empty string if @rtsp_ctx is being tunneled over HTTP.
 *
 * Since: 0.10.23
 */
const gchar *
gst_rtsp_ctx_get_tunnelid (const GstRTSPContext *rtsp_ctx)
{
  g_return_val_if_fail (rtsp_ctx != NULL, NULL);

  if (!rtsp_ctx->tunneled)
    return NULL;

  return rtsp_ctx->tunnelid;
}

GstRTSPResult
gst_rtsp_ctx_recv(GstRTSPContext *rtsp_ctx, gint rfd, GstRTSPBuilder *builder,
	GstRTSPMessage *message, gint *err)
{
	GstRTSPResult res;
	gint r, ec;
	g_assert(rtsp_ctx && builder && message);

	while (TRUE)
	{
		res = GST_RTSP_EINTR;
		if (rtsp_ctx->rcv_buffer_bytes)
			res = build_next(rtsp_ctx, builder, message);

		if (res != GST_RTSP_EINTR)
			return res;

		if (rtsp_ctx->rcv_buffer_bytes >= rtsp_ctx->rcv_buffer_size)
		{
			return GST_RTSP_EPARSE;
		}

		if (rtsp_ctx->rcv_buffer_offset)
		{
			memcpy(rtsp_ctx->rcv_buffer, &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_offset],
				rtsp_ctx->rcv_buffer_bytes);
			rtsp_ctx->rcv_buffer_offset = 0;
		}

		r = READ_SOCKET(rfd, &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_bytes],
			rtsp_ctx->rcv_buffer_size - rtsp_ctx->rcv_buffer_bytes);
		if (r < 0)
		{
			ec = errno;
			if (err)
			{
				*err = ec;
			}

			if (ec == EAGAIN)
			{
				return GST_RTSP_EINTR;
			}
			if (ec != EINTR)
			{			
        		return GST_RTSP_ESYS;
        	}
        	else
        		continue;
		}
		else if (r == 0)
		{
			if (err)
				*err = ECONNRESET;
			return GST_RTSP_EEOF;
		}
		else
		{
			rtsp_ctx->rcv_buffer_bytes += r;
		}
	}
}


GstRTSPContext *
gst_rtsp_ctx_new(gsize rb_size, const gchar *initial_buffer)
{
	GstRTSPContext *rtsp_ctx;
	g_assert(rb_size > 0);

	rtsp_ctx = g_new0(GstRTSPContext, 1);
    rtsp_ctx->tunneled = FALSE;
    rtsp_ctx->tstate = TUNNEL_STATE_NONE;

	rtsp_ctx->initial_buffer = g_strdup(initial_buffer);

	rtsp_ctx->rcv_buffer = g_malloc(rb_size);
	rtsp_ctx->rcv_buffer_size = rb_size;

	rtsp_ctx->timeout = 60;

	return rtsp_ctx;
}

GstRTSPResult
gst_rtsp_ctx_close (GstRTSPContext * rtsp_ctx)
{
  g_return_val_if_fail (rtsp_ctx != NULL, GST_RTSP_EINVAL);

  rtsp_ctx->read_ahead = 0;

  g_free (rtsp_ctx->initial_buffer);
  rtsp_ctx->initial_buffer = NULL;
  rtsp_ctx->initial_buffer_offset = 0;

  g_free(rtsp_ctx->rcv_buffer);
  rtsp_ctx->rcv_buffer = NULL;
  rtsp_ctx->rcv_buffer_size = 0;
  rtsp_ctx->rcv_buffer_size = 0;
  rtsp_ctx->rcv_buffer_offset = 0;
  rtsp_ctx->rcv_buffer_bytes = 0;
  
  rtsp_ctx->tunneled = FALSE;
  rtsp_ctx->tstate = TUNNEL_STATE_NONE;
  rtsp_ctx->ctxp = NULL;
 
  rtsp_ctx->timeout = 60;
  rtsp_ctx->session_id[0] = '\0';

  return GST_RTSP_OK;
}

void gst_rtsp_ctx_free(GstRTSPContext *rtsp_ctx)
{
	g_assert(rtsp_ctx != NULL);

	gst_rtsp_ctx_close(rtsp_ctx);
	g_free(rtsp_ctx);	
}

//:~ End
