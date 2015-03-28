#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"


gint
nmp_get_real_time(NmpTime *pt)
{
	struct tm tmp;
	time_t t = time(NULL);
	g_assert(pt != NULL);

	memset(pt, 0, sizeof(*pt));
	if(localtime_r(&t, &tmp) == NULL)
	{
		return -1;
	}

	pt->year = tmp.tm_year + 1900;
	pt->month = tmp.tm_mon + 1;
	pt->date = tmp.tm_mday;
	pt->hour = tmp.tm_hour;
	pt->minute = tmp.tm_min;
	pt->second = tmp.tm_sec;
	pt->day_of_week = tmp.tm_wday;

	return 0;  
}


time_t
nmp_get_seconds_of_day( void )
{
	time_t sec;
	NmpTime st;

	if(nmp_get_real_time(&st))
	{
		return -1;
	}

	sec = st.second + st.minute * 60 + st.hour * 60 * 60;
	
	return sec;
}


time_t
nmp_get_seconds_of_time(gint hour, gint min, gint sec)
{
	return sec + min * 60 + hour * 60 * 60;
}


void
nmp_make_str_from_seconds(time_t secs, gchar buf[], gsize size)
{
	snprintf(buf, size, "%02ld:%02ld:%02ld", 
		secs / 3600, (secs % 3600) / 60, (secs % 3600) % 60
	);

	buf[size - 1] = 0;	//windows fix.
}


time_t
nmp_make_time_t(gchar *str)
{
	gint ret;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));
	ret = sscanf(str, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon,
		&tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
	if (ret != 6)
		return (time_t)0;

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;

	return mktime(&tm);
}


static __inline__ gint
nmp_check_format(gint hour, gint min, gint sec)
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
nmp_get_string_time_range(const gchar *str, time_t *start,
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

	if (nmp_check_format(s_hour, s_min, s_sec))
		return -EINVAL;

	if (nmp_check_format(e_hour, e_min, e_sec))
		return -EINVAL;	

	*start = nmp_get_seconds_of_time(s_hour, s_min, s_sec);
	*end = nmp_get_seconds_of_time(e_hour, e_min, e_sec);

	return 0;
}


gint
nmp_get_string_date(const gchar *str, gint *year, gint *mon,
	gint *date)
{
	if (!str || !year || !mon || !date)
		return -EINVAL;

	if (sscanf(str, "%d-%d-%d", year, mon, date) != 3)
		return -EINVAL;

	return 0;
}


gint
nmp_make_time_str(gchar buf[], gsize size, time_t time)
{
	gint year, mon, date, hour, min, sec;
	struct tm tmp;

	if(localtime_r(&time, &tmp) == NULL)
	{
		return -1;
	}

	year = tmp.tm_year + 1900;
	mon = tmp.tm_mon + 1;
	date = tmp.tm_mday;
	hour = tmp.tm_hour;
	min = tmp.tm_min;
	sec = tmp.tm_sec;

	snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, date, hour, min, sec);	//fixme.
	return 0;
}


gulong
nmp_make_utc_time(guint year, guint mon, guint day, guint hour,
    guint min, guint sec)	/* from utc time string to utc time_t */
{
    if (0 >= (gint)(mon -= 2))
    {
         mon += 12;
         year -= 1;
    }

    return ((((gulong)(year/4 - year/100 + year/400 + 367*mon/12 + day) +
             year*365 - 719499
          )*24 + hour
       )*60 + min
    )*60 + sec;
}


gint
nmp_set_system_utc_time(guint year, guint mon, guint day, guint hour,
    guint min, guint sec)	/* utc time format, 8 hours smaller than GMT-8 time */
{
	struct timeval tv;

	tv.tv_sec = (time_t)nmp_make_utc_time(year, mon, day, hour, min, sec);
	tv.tv_usec = 0;

	if (settimeofday(&tv, NULL) < 0)
		return -errno;
	system("hwclock -w > /dev/null 2>&1 &");
	return 0;
}


guint
nmp_str_hash_pjw(const gchar *start, gsize len)
{
	guint g, h = 0;
	const gchar *end;

	end = start + len;
	while (start < end)
	{
		h = (h << 4) + *start;
		++start;

		if ((g = (h & 0xF0000000)))
		{
			h ^= (g >> 24);
			h ^= g;
		}
	}

	return h;
}


gchar *
nmp_regex_string(const gchar *str, gchar *reg)
{
	GRegex *regex;
	GMatchInfo *match_info = NULL;
	gchar *value;

	regex = g_regex_new(reg, 0, 0, NULL);
	if (!regex)
	{
		nmp_warning(
			"g_regex_new() failed!"
		);
		return NULL;
	}

	value = NULL;

	if (g_regex_match(regex, str, 0, &match_info))
	{
		value = g_match_info_fetch(match_info, 0);	
	}

	g_match_info_free(match_info);
	g_regex_unref(regex);

	return value;
}


gint
nmp_get_substring(gchar result[], gint buf_size,
	const gchar *p_begin, const gchar *p_end)
{
	gchar* temp;
	G_ASSERT(result != NULL && p_begin != NULL && 
		p_end != NULL);

	if(p_end - p_begin >= buf_size)
		return -1;

	temp = result;

	while(p_begin != p_end)
		*temp++ = *p_begin++;

	*temp = '\0';
	return 0;
}


gint
nmp_make_abs_path(const gchar* path)
{
	gchar _path[PATH_MAX];
	gint len;
	const gchar *p_begin, *p_end;

	G_ASSERT(path != NULL);

	len = strlen(path);
	if(len < 1 || path[0] != '/')
		return -1;

	if(len == 1)
		return 0;

	p_end = &path[len - 1];
	while(p_end != path)
	{
		if(*p_end == ' ' || *p_end == '\t' || *p_end == '/')
			--p_end;
		else
			break;
	}
	++p_end;

	p_begin = path + 1;

	if(p_begin == p_end)
		return 0;

	while(p_begin != p_end)
	{
		if(*p_begin == '/')
		{
			if(nmp_get_substring(_path, PATH_MAX, path, p_begin) < 0)
				return -1;
			
			if(mkdir(_path, 0700) < 0 && errno != EEXIST)
				return -1;
		}
		++p_begin;
	}

	if(nmp_get_substring(_path, PATH_MAX, path, p_end) < 0)
		return -1;

	if(mkdir(_path, 0700) < 0 && errno != EEXIST)
		return -1;

	return 0;
}

//:~ End
