#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"


time_t
jpf_get_seconds_of_time(gint hour, gint min, gint sec)
{
	return sec + min * 60 + hour * 60 * 60;
}


static __inline__ gint
jpf_check_format(gint hour, gint min, gint sec)
{
	if (hour > 23 || hour < 0)
		return -EINVAL;

	if (min > 59 || min < 0)
		return -EINVAL;

	if (sec > 59 || sec < 0)
		return -EINVAL;

	return 0;
}


gint
jpf_get_string_time_range(const gchar *str, time_t *start,
	time_t *end)
{
	gint s_hour, s_min, s_sec, e_hour, e_min, e_sec;

	if (!str || !start || !end)
		return -EINVAL;

	if (sscanf(str, "%d:%d:%d-%d:%d:%d", &s_hour, &s_min,
		&s_sec, &e_hour, &e_min, &e_sec) != 6)
	{
		return -EINVAL;
	}

	if (jpf_check_format(s_hour, s_min, s_sec))
		return -EINVAL;

	if (jpf_check_format(e_hour, e_min, e_sec))
		return -EINVAL;

	*start = jpf_get_seconds_of_time(s_hour, s_min, s_sec);
	*end = jpf_get_seconds_of_time(e_hour, e_min, e_sec);

	return 0;
}

