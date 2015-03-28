#ifndef __NMP_UTILITY_H__
#define __NMP_UTILITY_H__

#include <glib.h>

typedef struct _NmpTime NmpTime;
struct _NmpTime
{
	gint		year;
	gint		month;
	gint		date;
	gint		hour;
	gint		minute;
	gint		second;
	gint		day_of_week;
};


gint nmp_get_real_time(NmpTime *pt);
time_t nmp_get_seconds_of_day( void );
time_t nmp_get_seconds_of_time(gint hour, gint min, gint sec);

void nmp_make_str_from_seconds(time_t secs, gchar buf[], gsize size);

time_t nmp_make_time_t(gchar *str);

gint nmp_get_string_time_range(const gchar *str, time_t *start,
	time_t *end);

gint nmp_get_string_date(const gchar *str, gint *year, gint *mon,
	gint *date);

gint nmp_make_time_str(gchar buf[], gsize size, time_t time);

gulong nmp_make_utc_time(guint year, guint mon, guint day, guint hour,
    guint min, guint sec);

gint nmp_set_system_utc_time(guint year, guint mon, guint day, guint hour,
    guint min, guint sec);

guint nmp_str_hash_pjw(const gchar *start, gsize len);

gchar *nmp_regex_string(const gchar *str, gchar *reg);

gint nmp_make_abs_path(const gchar* path);


#endif	/* __NMP_UTILITY_H__ */
