/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "rc_log.h"
#include "mediaparser.h"


static void digit_to_char(char *dst, uint8_t src)
{
    if (src < 10) {
        *dst = '0' + src;
    } else {
        *dst = 'A' + src - 10;
    }
}

char *extradata2config(MediaProperties *properties)
{
    size_t config_len;
    char *config;
    size_t i;

    if ( properties->extradata_len == 0 )
        return NULL;

    config_len = properties->extradata_len * 2 + 1;
    config = g_malloc(config_len);

    if (config == NULL)
        return NULL;

    for(i = 0; i < properties->extradata_len; i++) {
        digit_to_char(config + 2 * i, properties->extradata[i] >> 4);
        digit_to_char(config + 2 * i + 1, properties->extradata[i] & 0xF);
    }

    config[config_len-1] = '\0';

    return config;
}


gchar *
get_param_string(const gchar *uri, gchar *reg)
{
	GRegex *regex;
	GMatchInfo *match_info = NULL;
	gchar *value;

	regex = g_regex_new(reg, 0, 0, NULL);
	if (!regex)
	{
		rc_log(
			RC_LOG_ERR,
			"[ls] g_regex_new() failed!"
		);
		return NULL;
	}

	value = NULL;

	if (g_regex_match(regex, uri, 0, &match_info))
	{
		value = g_match_info_fetch(match_info, 0);	
	}

	g_match_info_free(match_info);
	g_regex_unref(regex);

	return value;
}


gboolean
regex_string_match(const gchar *uri, const gchar *reg)
{
	gboolean match = FALSE;
	GRegex *regex;
	GMatchInfo *match_info = NULL;

	regex = g_regex_new(reg, 0, 0, NULL);
	if (!regex)
	{
		rc_log(
			RC_LOG_ERR,
			"[ls] g_regex_new() failed!"
		);
		return match;
	}

	if (g_regex_match(regex, uri, 0, &match_info))
	{
		match = TRUE;
	}

	g_match_info_free(match_info);
	g_regex_unref(regex);

    return match;
}

//:~ End