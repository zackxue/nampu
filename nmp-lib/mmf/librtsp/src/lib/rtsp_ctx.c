
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#include "rtsp_ctx.h"
#include "rtsp_mem.h"
#include "rtsp_log.h"


#ifdef WIN32
#define READ_SOCKET(fd, buf, len) recv (fd, (char *)buf, len, MSG_DONTWAIT)
#define GET_CUR_ERROR (WSAGetLastError ())
#define ERRNO_IS_EAGAIN (cur_error == WSAEWOULDBLOCK)
#define ERRNO_IS_EINTR (cur_error == WSAEINTR)
#else
#define READ_SOCKET(fd, buf, len) read (fd, buf, len)
#define GET_CUR_ERROR (errno)
#define ERRNO_IS_EAGAIN (cur_error == EAGAIN)
#define ERRNO_IS_EINTR (cur_error == EINTR)
#endif


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


void build_init(rtsp_builder *builder)
{
	memset(builder, 0, sizeof(rtsp_builder));
	builder->state = STATE_START;
}

void build_reset(rtsp_builder *builder)
{
	my_free(builder->body_data, builder->body_len + 1);
	memset(builder, 0, sizeof(rtsp_builder));
}

rtsp_context *rtsp_ctx_new(size_t rb_size)
{
	rtsp_context *rtsp_ctx;
	assert(rb_size > 0);

	rtsp_ctx = (rtsp_context *)my_alloc(sizeof(rtsp_context));
	memset(rtsp_ctx, 0, sizeof(rtsp_context));

	rtsp_ctx->rcv_buffer = my_alloc(rb_size);
	memset(rtsp_ctx->rcv_buffer, 0, rb_size);
	rtsp_ctx->rcv_buffer_size = rb_size;

	return rtsp_ctx;
}

RTSP_RESULT rtsp_ctx_close(rtsp_context *rtsp_ctx)
{
	return_val_if_fail(rtsp_ctx != NULL, RTSP_EINVAL);

	rtsp_ctx->read_ahead = 0;
	my_free(rtsp_ctx->rcv_buffer, rtsp_ctx->rcv_buffer_size);
	rtsp_ctx->rcv_buffer = NULL;
	rtsp_ctx->rcv_buffer_size = 0;
	rtsp_ctx->rcv_buffer_offset = 0;
	rtsp_ctx->rcv_buffer_bytes = 0;

	return RTSP_OK;
}

void rtsp_ctx_free(rtsp_context *rtsp_ctx)
{
	assert(rtsp_ctx != NULL);

	rtsp_ctx_close(rtsp_ctx);
	my_free(rtsp_ctx, sizeof(rtsp_context));
}


static int fill_bytes(rtsp_context *rtsp_ctx, uint8 *buffer, uint32 size)
{
	uint32 to_r;
	//func_begin("\n");
	
	if (rtsp_ctx->rcv_buffer_bytes) {
		to_r = MIN(size, rtsp_ctx->rcv_buffer_bytes);
		memcpy(buffer, &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_offset], to_r);
		rtsp_ctx->rcv_buffer_offset += to_r;
		rtsp_ctx->rcv_buffer_bytes -= to_r;
		return to_r;
	}
	else {
		errno = EAGAIN;
		return -1;
	}
}

static RTSP_RESULT read_bytes(rtsp_context *rtsp_ctx, uint8 *buffer, 
	uint32 *idx, uint32 size)
{
	uint32 left;
	//func_begin("\n");

	if (unlikely(*idx > size))
		return RTSP_ERROR;

	left = size - *idx;
	while (left) {
		int r;
		
		r = fill_bytes(rtsp_ctx, &buffer[*idx], left);
		if (unlikely (r == 0)) {
			return RTSP_EEOF;
		} else if (unlikely (r < 0)) {
		int cur_error = GET_CUR_ERROR;
		if (ERRNO_IS_EAGAIN)
			return RTSP_EINTR;
		if (!ERRNO_IS_EINTR)
			return RTSP_ESYS;
		} else {
			left -= r;
			*idx += r;
		}
	}
	return RTSP_OK;
}


static RTSP_RESULT read_line (rtsp_context *rtsp_ctx, uint8 *buffer, 
	uint32 *idx, uint32 size)
{
  func_begin("\n");
  while (1) {
    uint8 c;
    int r;

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
      c = (uint8) rtsp_ctx->read_ahead;
      rtsp_ctx->read_ahead = 0;
    } else {
      /* read the next character */
      r = fill_bytes (rtsp_ctx, &c, 1);
      if (unlikely(r == 0)) {
        return RTSP_EEOF;
      } else if (unlikely (r < 0)) {
        int cur_error = GET_CUR_ERROR;
        if (ERRNO_IS_EAGAIN)
          return RTSP_EINTR;
        if (!ERRNO_IS_EINTR)
          return RTSP_ESYS;
        continue;
      }
    }

    /* special treatment of line endings */
    if (c == '\r' || c == '\n') {
      uint8 read_ahead;

    retry:
      /* need to read ahead one more character to know what to do... */
      r = fill_bytes (rtsp_ctx, &read_ahead, 1);
      if (unlikely (r == 0)) {
        return RTSP_EEOF;
      } else if (unlikely (r < 0)) {
        int cur_error = GET_CUR_ERROR;
        if (ERRNO_IS_EAGAIN) {
          /* remember the original character we read and try again next time */
          if (rtsp_ctx->read_ahead == 0)
            rtsp_ctx->read_ahead = c;
          return RTSP_EINTR;
        }
        if (!ERRNO_IS_EINTR)
          return RTSP_ESYS;
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

    if (likely (*idx < size - 1))
      buffer[(*idx)++] = c;
  }
  buffer[*idx] = '\0';

  return RTSP_OK;
}


/* convert all consecutive whitespace to a single space */
static void
normalize_line (uint8 * buffer)
{
  func_begin("\n");
  while (*buffer) {
    if (isspace (*buffer)) {
      uint8 *tmp;

      *buffer++ = ' ';
      for (tmp = buffer; isspace (*tmp); tmp++) {
      }
      if (buffer != tmp)
        memmove (buffer, tmp, strlen ((char *) tmp) + 1);
    } else {
      buffer++;
    }
  }
}


static RTSP_RESULT
parse_string (char * dest, int size, char ** src)
{
  RTSP_RESULT res = RTSP_OK;
  int idx;
  func_begin("\n");

  idx = 0;
  /* skip spaces */
  while (isspace (**src))
    (*src)++;

  while (!isspace (**src) && **src != '\0') {
    if (idx < size - 1)
      dest[idx++] = **src;
    else
      res = RTSP_EPARSE;
    (*src)++;
  }
  if (size > 0)
    dest[idx] = '\0';

  return res;
}


/*
 *	zyt,type暂时不使用，后期确定后可去除
 */
static RTSP_RESULT parse_protocol_version (char *protocol, 
	RTSP_MSG_TYPE * type, RTSP_VERSION *version)
{
	RTSP_RESULT res = RTSP_OK;
	char *ver;
	func_begin("\n");

	if (likely((ver = strchr (protocol, '/')) != NULL)) {
		uint32 major;
		uint32 minor;
		char dummychar;

		*ver++ = '\0';

		/* the version number must be formatted as X.Y with nothing following */
		if (sscanf (ver, "%u.%u%c", &major, &minor, &dummychar) != 2)
			res = RTSP_EPARSE;

		if (strcasecmp(protocol, "RTSP") == 0) {
			  if (major != 1 || minor != 0) {
				*version = RTSP_VERSION_INVALID;
				res = RTSP_ERROR;
			}
		} else
			res = RTSP_EPARSE;
	} else
		res = RTSP_EPARSE;

	return res;
}


static RTSP_RESULT parse_response_status(uint8 *buffer, rtsp_message *msg)
{
	RTSP_RESULT res = RTSP_OK;
	RTSP_RESULT res2;
	char versionstr[20];
	char codestr[4];
	int code;
	char *bptr;
	func_begin("\n");

	bptr = (char *)buffer;

	if (parse_string(versionstr, sizeof(versionstr), &bptr) != RTSP_OK)
		res = RTSP_EPARSE;

	if (parse_string(codestr, sizeof(codestr), &bptr) != RTSP_OK)
		res = RTSP_EPARSE;
	code = atoi(codestr);
	if (unlikely(*codestr == '\0' || code < 0 || code >= 600))
		res = RTSP_EPARSE;

	while (isspace(*bptr))
		bptr++;

	if (unlikely(rtsp_message_init_response(msg, code, bptr, NULL) != 
		RTSP_OK))
		res = RTSP_EPARSE;

	res2 = parse_protocol_version (versionstr, &msg->type,
		&msg->type_data.response.version);
	if (likely(res == RTSP_OK))
		res = res2;

	return res;
}


static RTSP_RESULT parse_request_line (uint8 *buffer, rtsp_message *msg)
{
	RTSP_RESULT res = RTSP_OK;
	RTSP_RESULT res2;
	char versionstr[20];
	char methodstr[20];
	char urlstr[4096];
	char *bptr;
	RTSP_METHOD method;
	func_begin("\n");

	bptr = (char *)buffer;

	if (parse_string (methodstr, sizeof (methodstr), &bptr) != RTSP_OK)
		res = RTSP_EPARSE;
	method = rtsp_find_method (methodstr);

	if (parse_string (urlstr, sizeof (urlstr), &bptr) != RTSP_OK)
		res = RTSP_EPARSE;
	if (unlikely(*urlstr == '\0'))
		res = RTSP_EPARSE;

	if (parse_string (versionstr, sizeof (versionstr), &bptr) != RTSP_OK)
		res = RTSP_EPARSE;

	if (unlikely (*bptr != '\0'))
		res = RTSP_EPARSE;

	if (unlikely (rtsp_message_init_request (msg, method, urlstr) != 
		RTSP_OK))
		res = RTSP_EPARSE;

	res2 = parse_protocol_version(versionstr, &msg->type, 
		&msg->type_data.request.version);
	if (likely (res == RTSP_OK))
		res = res2;

	/* GET and POST are not allowed as RTSP methods */
	if (msg->type_data.request.method == RTSP_GET ||
		msg->type_data.request.method == RTSP_POST)
	{
		msg->type_data.request.method = RTSP_INVALID;
		if (res == RTSP_OK)
			res = RTSP_ERROR;
	}

	return res;
}


/* zyt一行多键值对处理待看 */
/* parsing lines means reading a Key: Value pair */
static RTSP_RESULT parse_line(uint8 *buffer, rtsp_message *msg)
{
	RTSP_HEADER_FIELD field;
	char *line = (char *)buffer;
	char *value;
	func_begin("\n");

	if ((value = strchr (line, ':')) == NULL || value == line)
		goto parse_error;

	/* trim space before the colon */
	if (value[-1] == ' ')
		value[-1] = '\0';

	/* replace the colon with a NUL */
	*value++ = '\0';

	/* find the header */
	field = rtsp_find_header_field (line);
	if (field == RTSP_HDR_INVALID)
		goto done;

	/* split up the value in multiple key:value pairs if it contains comma(s) */
	while (*value != '\0') {
	char *next_value;
	char *comma = NULL;
	mbool quoted = 0;
	uint32 comment = 0;

	/* trim leading space */
	if (*value == ' ')
		value++;

	/* for headers which may not appear multiple times, and thus may not
	 * contain multiple values on the same line, we can short-circuit the loop
	 * below and the entire value results in just one key:value pair*/
	if (!rtsp_header_allow_multiple (field))
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
	        if (field == RTSP_HDR_PROXY_AUTHENTICATE ||
	            field == RTSP_HDR_WWW_AUTHENTICATE) {
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
	      rtsp_message_add_header (msg, field, value);

	    value = next_value;
	}
done:
  return RTSP_OK;  

	/* ERRORS */
parse_error:
	return RTSP_EPARSE;
}


static RTSP_RESULT build_next(rtsp_context *rtsp_ctx, rtsp_builder *builder, 
	rtsp_message *message)
{
	RTSP_RESULT res;
	func_begin("\n");

	while (1)
	{
		switch (builder->state) {
		case STATE_START:
		{
			log_2("case STATE_START:\n");
			uint8 c;

			builder->offset = 0;
			res = read_bytes(rtsp_ctx, (uint8 *)builder->buffer, 
				&builder->offset, 1);
			if (res != RTSP_OK)
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
			log_2("case STATE_DATA_HEADER:\n");
			res = read_bytes(rtsp_ctx, (uint8 *)builder->buffer, 
				&builder->offset, 4);
			if (res != RTSP_OK)
				goto done;

			rtsp_message_init_data(message, builder->buffer[1]);

			builder->body_len = (builder->buffer[2] << 8) | builder->buffer[3];
			builder->body_data = my_alloc(builder->body_len + 1);
			builder->body_data[builder->body_len] = '\0';
			builder->offset = 0;
			builder->state = STATE_DATA_BODY;
			break;
		}
		case STATE_DATA_BODY:
		{
			log_2("case STATE_DATA_BODY:\n");
			//log_2("builder->offset = %d, builder->body_len = %d\n", 
			//	builder->offset, builder->body_len);
			
			res = read_bytes(rtsp_ctx, builder->body_data, &builder->offset,
				builder->body_len);
			if (res != RTSP_OK)
				goto done;

			/* we have the complete body now, store in the message adjusting the
			* length to include the traling '\0' */
			rtsp_message_take_body (message,
				(uint8 *) builder->body_data, builder->body_len + 1);
			builder->body_data = NULL;
			builder->body_len = 0;

			builder->state = STATE_END;
			break;
		}
		case STATE_READ_LINES:
		{
			log_2("case STATE_READ_LINES:\n");
			//log_2("rtsp_ctx->rcv_buffer_offset = %d, rtsp_ctx->rcv_buffer_bytes = %d\n", 
			//	rtsp_ctx->rcv_buffer_offset, rtsp_ctx->rcv_buffer_bytes);
			res = read_line(rtsp_ctx, builder->buffer, &builder->offset, 
				sizeof(builder->buffer));
			//log_2("after read_line, rtsp_ctx->rcv_buffer_offset = %d, rtsp_ctx->rcv_buffer_bytes = %d\n", 
			//	rtsp_ctx->rcv_buffer_offset, rtsp_ctx->rcv_buffer_bytes);
			if (res != RTSP_OK)
				goto done;
			log_2("builder->buffer:%s\n", builder->buffer);

			/* we have a regular response */
			if (builder->buffer[0] == '\0') {
				char *hdrval;

				if (rtsp_message_get_header(message, RTSP_HDR_CONTENT_LENGTH, 
					&hdrval, 0) == RTSP_OK) {
					 /* there is, prepare to read the body */
					 builder->body_len = atol(hdrval);
					 if (builder->body_len + 1 < 0)
					 	goto invalid_body_len;
					 builder->body_data = my_try_alloc(builder->body_len + 1);
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
			normalize_line(builder->buffer);
			if (builder->line == 0) {
				/* first line, check for response status */
				if (memcmp (builder->buffer, "RTSP", 4) == 0) {
					builder->status = parse_response_status(builder->buffer, 
						message);
				} else {
					builder->status = parse_request_line(builder->buffer, message);
				}
			} else {
				/* else just parse the line */
				res = parse_line (builder->buffer, message);
				if (res != RTSP_OK)
					builder->status = res;
			}
			builder->line++;
			builder->offset = 0;
			break;
		}
		case STATE_END:
		{
			log_2("case STATE_END:\n");

			if (message->type == RTSP_MESSAGE_DATA) {
				/* data messages don't have headers */
				res = RTSP_OK;
				goto done;
			}
			res = builder->status;
			goto done;
		}
		default:
			res = RTSP_ERROR;
			break;
		}
	}
	
done:
	log_2("goto done-> res = %d\n", res);
	return res;

	/* ERRORS */
invalid_body_len:
	log_2("goto invalid_body_len->\n");
	return RTSP_ERROR;
}


RTSP_RESULT rtsp_ctx_recv(rtsp_context *rtsp_ctx, int rfd, rtsp_builder *builder, 
	rtsp_message *message, int *err)
{
	RTSP_RESULT res;
	int n, merr;

	assert(rtsp_ctx && builder && message);
	func_begin("\n");

	while (1)
	{
		res = RTSP_EINTR;
		if (rtsp_ctx->rcv_buffer_bytes)
			res = build_next(rtsp_ctx, builder, message);

		if (res != RTSP_EINTR)
			return res;

		if (rtsp_ctx->rcv_buffer_bytes >= rtsp_ctx->rcv_buffer_size)
			return RTSP_EPARSE;

		if (rtsp_ctx->rcv_buffer_offset)
		{
			/* zyt待处理 */
			memcpy(rtsp_ctx->rcv_buffer, &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_offset],	//内存重叠，但src地址大于dst地址，可以用memcpy(否则需改为memmove)
				rtsp_ctx->rcv_buffer_bytes);
			rtsp_ctx->rcv_buffer_offset = 0;
		}

		n = READ_SOCKET(rfd, &rtsp_ctx->rcv_buffer[rtsp_ctx->rcv_buffer_bytes], 
			rtsp_ctx->rcv_buffer_size - rtsp_ctx->rcv_buffer_bytes);
		if (n < 0)
		{
			merr = errno;
			log_2("READ_SOCKET n(%d) < 0, errno:%s\n", n, strerror(merr));
			if (err)
				*err = merr;
			
			if (merr == EAGAIN)
				return RTSP_EINTR;
			else if (merr != EINTR)
				return RTSP_ESYS;
			else
				continue;
		}
		else if (n == 0)
		{
			log_2("READ_SOCKET n = 0\n");
			if (err)
				*err = ECONNRESET;
			return RTSP_EEOF;
		}
		else
		{
			rtsp_ctx->rcv_buffer_bytes += n;
		}
	}
}

