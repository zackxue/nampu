#ifndef __NMP_POLICY_H__
#define __NMP_POLICY_H__

#include <stdio.h>
#include <time.h>
#include <glib.h>

enum		/* time seg type */
{
	TS_TYPE_NULL,
	TS_TYPE_DAY,
	TS_TYPE_DATE
};


enum		/* time seg rec type */
{
	TS_REC_TYPE_NULL,
	TS_REC_TYPE_AUTO = 1 << 0,
	TS_REC_TYPE_ALARM = 1 << 1
};


typedef struct __NmpTimeSegment NmpTimeSegment;
struct __NmpTimeSegment
{
	gint	seg_type;		/* time seg type, date? day? */
	gint	rec_type;
	
	union {
		struct {
			gint	day_of_week;
		}day;

		struct {
			gint	year;
			gint	month;
			gint	date;
		}date;
	}time;

	time_t	start;	/* [start, end) */
	time_t	end;
};


typedef struct __NmpPolicy NmpPolicy;
struct __NmpPolicy
{
	GList	*time_segs;
	GMutex	*mutex;

	gint	seg_now;
	gint	rec_now;
};


NmpPolicy *nmp_new_policy( void );
void nmp_delete_policy(NmpPolicy *p);

void nmp_policy_add_time_seg_date(NmpPolicy *p, gint rec_type, gint year,
	gint month, gint date, time_t start, time_t end);
void nmp_policy_add_time_seg_day(NmpPolicy *p, gint rec_type, gint day,
	time_t start, time_t end);

gint nmp_policy_check(NmpPolicy *p);
void nmp_policy_clear(NmpPolicy *p);


#endif	/* __NMP_POLICY_H__ */
