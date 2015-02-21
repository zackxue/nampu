
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtsp_url.h"
#include "rtsp_log.h"


static void rtsp_string_init(rtsp_string *string)
{
	if (string->str != NULL && string->str != string->init)
	{
		my_free(string->str, string->str_len);
	}
	
	memset(string, 0, sizeof(rtsp_string));
}


static void rtsp_read_string(rtsp_string *string, const char *val)
{
	rtsp_string_init(string);
	
	if (val)
	{
		string->str_len = STRLEN1(val);
		
		if (string->str_len > DEF_RTSP_STRING_LEN)
		{
			string->str = my_strdup(val);
		}
		else
		{
			string->str = string->init;
			memcpy(string->str, val, string->str_len);
		}
	}
	else
	{
		log_0("val = NULL\n");
	}
}


static void rtsp_read_string_n(rtsp_string *string, const char *val, size_t n)
{
	rtsp_string_init(string);
	
	if (val)
	{
		string->str_len = n + 1;
		
		if (string->str_len > DEF_RTSP_STRING_LEN)
		{
			string->str = my_strndup(val, n);
		}
		else
		{
			string->str = string->init;
			strncpy(string->str, val, string->str_len);
			string->str[n] = '\0';
		}
	}
	else
	{
		log_0("val = NULL\n");
	}
}


/* format is rtsp[u]://[user:passwd@]host[:port]/abspath[?query] where host
 * is a host name, an IPv4 dotted decimal address ("aaa.bbb.ccc.ddd") or an
 * [IPv6] address ("[aabb:ccdd:eeff:gghh::sstt]" note the brackets around the
 * address to allow the distinction between ':' as an IPv6 hexgroup separator
 * and as a host/port separator) 
 *
 * eg:rtsp://user:passwd@host:12233/abspath?happy_query
 */
RTSP_RESULT rtsp_url_parse(const char *url_str, rtsp_url **url)
{
	rtsp_url *res;
	char *p, *delim, *at, *col;
	char *host_end = NULL;

	return_val_if_fail(url_str != NULL, RTSP_EINVAL);
	return_val_if_fail(url != NULL, RTSP_EINVAL);

	res = (rtsp_url *)my_alloc(sizeof(rtsp_url));
	memset(res, 0, sizeof(rtsp_url));

	p = (char *)url_str;

	col = strstr(p, "://");
	if (col == NULL)
		goto invalid;
	p = col + 3;

	delim = strpbrk(p, "/?");
	at = strchr(p, '@');

	if (at && delim && at > delim)
		at = NULL;

	if (at) {
		col = strchr(p, ':');

		/* must have a ':' and it must be before the '@' */
		if (col == NULL || col > at)
			goto invalid;

		rtsp_read_string_n(&res->user, p, col - p);
		col++;
		rtsp_read_string_n(&res->passwd, col, at - col);

		/* move to host */
		p = at + 1;
	}

	if (*p == '[') {
		log_0("not support ip v6\n");
		goto invalid;
	}

	/* zyt Ö»´¦Àí ip v4 */
	col = strchr (p, ':');
	/* we have a ':' and a delimiter but the ':' is after the delimiter, it's
	* not really part of the hostname */
	if (col && delim && col >= delim)
		col = NULL;
	host_end = col ? col : delim;


	if (!host_end)
		rtsp_read_string(&res->host, p);
	else {
		rtsp_read_string_n(&res->host, p, host_end - p);

		if (col) {
			res->port = strtoul(col + 1, NULL, 10);
		} else {
			res->port = 0;
		}
	}
	p = delim;

	if (p && *p == '/') {
		delim = strchr (p, '?');
		if (!delim)
			rtsp_read_string(&res->abspath, p);
		else
			rtsp_read_string_n(&res->abspath, p, delim - p);
		p = delim;
	} else {
		rtsp_read_string(&res->abspath, "/");
	}

	if (p && *p == '?')
		rtsp_read_string(&res->query, p + 1);

	*url = res;

	return RTSP_OK;

  /* ERRORS */
invalid:
	{
		rtsp_url_free(res);
		return RTSP_EINVAL;
	}
}


rtsp_url *rtsp_url_copy(const rtsp_url *url)
{
	rtsp_url *res;

	return_val_if_fail(url != NULL, NULL);

	res = (rtsp_url *)my_alloc(sizeof(rtsp_url));
	memset(res, 0, sizeof(rtsp_url));

	rtsp_read_string(&res->user, url->user.str);
	rtsp_read_string(&res->passwd, url->passwd.str);
	rtsp_read_string(&res->host, url->host.str);
	res->port = url->port;
	rtsp_read_string(&res->abspath, url->abspath.str);
	rtsp_read_string(&res->query, url->query.str);

	return res;
}


void rtsp_url_free(rtsp_url *url)
{
	if (url == NULL)
		return ;

	rtsp_string_init(&url->user);
	rtsp_string_init(&url->passwd);
	rtsp_string_init(&url->host);
	rtsp_string_init(&url->abspath);
	rtsp_string_init(&url->query);
	my_free(url, sizeof(rtsp_url));
}

RTSP_RESULT rtsp_url_set_port(rtsp_url *url, uint16 port)
{
	return_val_if_fail(url != NULL, RTSP_EINVAL);

	url->port = port;

	return RTSP_OK;
}


RTSP_RESULT rtsp_url_get_port(const rtsp_url *url, uint16 *port)
{
	return_val_if_fail(url != NULL, RTSP_EINVAL);
	return_val_if_fail(port != NULL, RTSP_EINVAL);

	if (url->port != 0)
		*port = url->port;
	else
		*port = RTSP_DEFAULT_PORT;

	return RTSP_OK;
}


char *rtsp_url_get_request_uri(const rtsp_url *url)
{
	char *uri;
	const char *pre_query;
	const char *query;

	return_val_if_fail(url != NULL, NULL);

	pre_query = url->query.str ? "?" : "";
	query = url->query.str ? url->query.str : "";

	char url_temp[1024];
	if (url->port != 0) {
		snprintf(url_temp, sizeof(url_temp), "rtsp://%s:%u%s%s%s", 
			url->host.str, url->port, url->abspath.str, pre_query, query);
	} else {
		snprintf(url_temp, sizeof(url_temp), "rtsp://%s%s%s%s", 
			url->host.str, url->abspath.str, pre_query, query);
	}
	uri = my_strdup(url_temp);

	return uri;
}


static int hex_to_int (char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else
    return -1;
}


static void unescape_path_component(char *comp)
{
	uint32 len = strlen(comp);
	uint32 i;

	for (i = 0; i + 2 < len; i++)
		if (comp[i] == '%') {
			int a, b;

			a = hex_to_int (comp[i + 1]);
			b = hex_to_int (comp[i + 2]);

			/* The a||b check is to ensure that the byte is not '\0' */
			if (a >= 0 && b >= 0 && (a || b)) {
				comp[i] = (char) (a * 16 + b);
				memmove (comp + i + 1, comp + i + 3, len - i - 3);
				len -= 2;
				comp[len] = '\0';
			}
		}
}

char **rtsp_url_decode_path_components(const rtsp_url *url)
{
	char **ret;
	uint32 i;

	return_val_if_fail(url != NULL, NULL);
	return_val_if_fail(url->abspath.str != NULL, NULL);

	ret = my_strsplit(url->abspath.str, "/", -1);

	for (i = 0; ret[i]; i++)
		unescape_path_component(ret[i]);

	return ret;
}

