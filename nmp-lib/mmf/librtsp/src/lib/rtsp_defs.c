
#include <stddef.h>
#include <string.h>

#include "rtsp_defs.h"
#include "rtsp_mem.h"

#define MAX_OPTIOINS_BUF		128

struct rtsp_header
{
	char *name;
	mbool multiple;	//0 or 1
};


static const char *rtsp_methods[] = {
  "DESCRIBE",
  "ANNOUNCE",
  "GET_PARAMETER",
  "OPTIONS",
  "PAUSE",
  "PLAY",
  "RECORD",
  "REDIRECT",
  "SETUP",
  "SET_PARAMETER",
  "TEARDOWN",
  "GET",
  "POST",
  NULL
};

static struct rtsp_header rtsp_headers[] = {
  {"Accept", 1},
  {"Accept-Encoding", 1},
  {"Accept-Language", 1},
  {"Allow", 1},
  {"Authorization", 0},
  {"Bandwidth", 0},
  {"Blocksize", 0},
  {"Cache-Control", 1},
  {"Conference", 0},
  {"Connection", 1},
  {"Content-Base", 0},
  {"Content-Encoding", 1},
  {"Content-Language", 1},
  {"Content-Length", 0},
  {"Content-Location", 0},
  {"Content-Type", 0},
  {"CSeq", 0},
  {"Date", 0},
  {"Expires", 0},
  {"From", 0},
  {"If-Modified-Since", 0},
  {"Last-Modified", 0},
  {"Proxy-Authenticate", 1},
  {"Proxy-Require", 1},
  {"Public", 1},
  {"Range", 0},
  {"Referer", 0},
  {"Require", 1},
  {"Retry-After", 0},
  {"RTP-Info", 1},
  {"Scale", 0},
  {"Session", 0},
  {"Server", 0},
  {"Speed", 0},
  {"Transport", 1},
  {"Unsupported", 0},
  {"User-Agent", 0},
  {"Via", 1},
  {"WWW-Authenticate", 1},

  /* Since 0.10.16 */
  {"Location", 0},

  /* Since 0.10.23 */
  {"ETag", 0},
  {"If-Match", 1},

  /* Since 0.10.24 */
  {"Timestamp", 0},

  /* Since 0.10.25 */
  {"Authentication-Info", 0},
  {"Host", 0},
  {"Pragma", 1},
  {"X-Server-IP-Address", 0},
  {"X-Sessioncookie", 0},

  {NULL, 0}
};


char *rtsp_status_as_text(RTSP_STATUS_CODE code)
{
	switch (code)
	{
	case RTSP_STS_CONTINUE:
		return ("Continue");
	case RTSP_STS_OK:
		return ("OK");
	case RTSP_STS_CREATED:
		return ("Created");
	case RTSP_STS_LOW_ON_STORAGE:
		return ("Low on Storage Space");
	case RTSP_STS_MULTIPLE_CHOICES:
		return ("Multiple Choices");
	case RTSP_STS_MOVED_PERMANENTLY:
		return ("Moved Permanently");
	case RTSP_STS_MOVE_TEMPORARILY:
		return ("Move Temporarily");
	case RTSP_STS_SEE_OTHER:
		return ("See Other");
	case RTSP_STS_NOT_MODIFIED:
		return ("Not Modified");
	case RTSP_STS_USE_PROXY:
		return ("Use Proxy");
	case RTSP_STS_BAD_REQUEST:
		return ("Bad Request");
	case RTSP_STS_UNAUTHORIZED:
		return ("Unauthorized");
	case RTSP_STS_PAYMENT_REQUIRED:
		return ("Payment Required");
	case RTSP_STS_FORBIDDEN:
		return ("Forbidden");
	case RTSP_STS_NOT_FOUND:
		return ("Not Found");
	case RTSP_STS_METHOD_NOT_ALLOWED:
		return ("Method Not Allowed");
	case RTSP_STS_NOT_ACCEPTABLE:
		return ("Not Acceptable");
	case RTSP_STS_PROXY_AUTH_REQUIRED:
		return ( "Proxy Authentication Required");
	case RTSP_STS_REQUEST_TIMEOUT:
		return ("Request Time-out");
	case RTSP_STS_GONE:
		return ("Gone");
	case RTSP_STS_LENGTH_REQUIRED:
		return ("Length Required");
	case RTSP_STS_PRECONDITION_FAILED:
		return ("Precondition Failed");
	case RTSP_STS_REQUEST_ENTITY_TOO_LARGE:
		return ( "Request Entity Too Large");
	case RTSP_STS_REQUEST_URI_TOO_LARGE:
		return ("Request-URI Too Large");
	case RTSP_STS_UNSUPPORTED_MEDIA_TYPE:
		return ("Unsupported Media Type");
	case RTSP_STS_PARAMETER_NOT_UNDERSTOOD:
		return ( "Parameter Not Understood");
	case RTSP_STS_CONFERENCE_NOT_FOUND:
		return ("Conference Not Found");
	case RTSP_STS_NOT_ENOUGH_BANDWIDTH:
		return ("Not Enough Bandwidth");
	case RTSP_STS_SESSION_NOT_FOUND:
		return ("Session Not Found");
	case RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE:
		return ( "Method Not Valid in This State");
	case RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE:
		return ( "Header Field Not Valid for Resource");
	case RTSP_STS_INVALID_RANGE:
		return ("Invalid Range");
	case RTSP_STS_PARAMETER_IS_READONLY:
		return ("Parameter Is Read-Only");
	case RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED:
		return ( "Aggregate operation not allowed");
	case RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED:
		return ( "Only aggregate operation allowed");
	case RTSP_STS_UNSUPPORTED_TRANSPORT:
		return ("Unsupported transport");
	case RTSP_STS_DESTINATION_UNREACHABLE:
		return ("Destination unreachable");
	case RTSP_STS_INTERNAL_SERVER_ERROR:
		return ("Internal Server Error");
	case RTSP_STS_NOT_IMPLEMENTED:
		return ("Not Implemented");
	case RTSP_STS_BAD_GATEWAY:
		return ("Bad Gateway");
	case RTSP_STS_SERVICE_UNAVAILABLE:
		return ("Service Unavailable");
	case RTSP_STS_GATEWAY_TIMEOUT:
		return ("Gateway Time-out");
	case RTSP_STS_RTSP_VERSION_NOT_SUPPORTED:
		return ( "RTSP Version not supported");
	case RTSP_STS_OPTION_NOT_SUPPORTED:
		return ("Option not supported");
	default:
		return ("Unknown Method");
	}
}

const char *rtsp_version_as_text(RTSP_VERSION version)
{
	switch (version)
	{
	case RTSP_VERSION_1_0:
		return "1.0";

	case RTSP_VERSION_1_1:
		return "1.1";

	default:
		return "0.0";
	}
}


const char *rtsp_header_as_text (RTSP_HEADER_FIELD field)
{
	if (field == RTSP_HDR_INVALID)
		return NULL;
	else
		return rtsp_headers[field - 1].name;
}


const char *rtsp_method_as_text(RTSP_METHOD method)
{
	int i;

	if (method == RTSP_INVALID)
		return NULL;

	i = 0;
	while ((method & 1) == 0)
	{
		i++;
		method >>= 1;
	}
	return rtsp_methods[i];
}


RTSP_HEADER_FIELD rtsp_find_header_field (const char * header)
{
  int idx;

  for (idx = 0; rtsp_headers[idx].name; idx++) {
    if (strcasecmp (rtsp_headers[idx].name, header) == 0) {
      return idx + 1;
    }
  }
  return RTSP_HDR_INVALID;
}


RTSP_METHOD rtsp_find_method (const char * method)
{
  int idx;

  for (idx = 0; rtsp_methods[idx]; idx++) {
    if (strcasecmp (rtsp_methods[idx], method) == 0) {
      return (1 << idx);
    }
  }
  return RTSP_INVALID;
}


const char *rtsp_options_as_text(RTSP_METHOD options)
{
	mb *mb;
	char *str, *end;

	mb = mb_new(MAX_OPTIOINS_BUF);
	str = mb_content(mb);
	str[0] = '\0';

	if (options & RTSP_OPTIONS)
		strcat(str, "OPTIONS, ");
	if (options & RTSP_DESCRIBE)
		strcat(str, "DESCRIBE, ");
	if (options & RTSP_ANNOUNCE)
	    strcat(str, "ANNOUNCE, ");
	if (options & RTSP_PAUSE)
		strcat(str, "PAUSE, ");
	if (options & RTSP_PLAY)
		strcat(str, "PLAY, ");
	if (options & RTSP_RECORD)
		strcat(str, "RECORD, ");
	if (options & RTSP_REDIRECT)
		strcat(str, "REDIRECT, ");
	if (options & RTSP_SETUP)
		strcat(str, "SETUP, ");
	if (options & RTSP_GET_PARAMETER)
		strcat(str, "GET_PARAMETER, ");
	if (options & RTSP_SET_PARAMETER)
		strcat(str, "SET_PARAMETER, ");
	if (options & RTSP_TEARDOWN)
		strcat(str, "TEARDOWN, ");

	end = str + strlen(str) - 1;
	while (end > str)
	{
		if (*end == ',')
		{
			*end = 0;
			break;
		}
		*end = 0;
		--end;
	}

	return str;
}


mbool rtsp_header_allow_multiple (RTSP_HEADER_FIELD field)
{
  if (field == RTSP_HDR_INVALID)
    return 0;
  else
    return rtsp_headers[field - 1].multiple;
}


void rtsp_free_string(char *str)
{
	mb *mb = ptr_to_mb(str);
	mb_del(mb);
}

