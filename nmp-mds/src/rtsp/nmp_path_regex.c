#include "nmp_path_regex.h"
#include "nmp_debug.h"

gchar *
nmp_rtsp_param_string(const gchar *uri, gchar *reg)
{
	GRegex *regex;
	GMatchInfo *match_info = NULL;
	gchar *value;

	regex = g_regex_new(reg, 0, 0, NULL);
	if (!regex)
	{
		nmp_warning(
			"[device] g_regex_new() failed!"
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


