#include <string.h>
#include "nmp_policy.h"
#include "nmp_utility.h"


NmpPolicy *
nmp_new_policy( void )
{
	NmpPolicy *p;

	p = g_new0(NmpPolicy, 1);
	p->time_segs = NULL;
	p->mutex = g_mutex_new();
	p->rec_now = TS_REC_TYPE_NULL;

	return p;
}


static void
nmp_free_seg(gpointer data)
{
	g_free(data);
}


static __inline__ void
__nmp_policy_clear(NmpPolicy *p)
{
	g_list_foreach(p->time_segs, (GFunc)nmp_free_seg, NULL);
  	g_list_free(p->time_segs);
	p->time_segs = NULL;
}


void
nmp_policy_clear(NmpPolicy *p)
{
	g_assert(p != NULL);

	g_mutex_lock(p->mutex);
	__nmp_policy_clear(p);
	g_mutex_unlock(p->mutex);
}


void
nmp_delete_policy(NmpPolicy *p)
{
	nmp_policy_clear(p);
	g_mutex_free(p->mutex);
	g_free(p);
}


static __inline__ gint
nmp_merge_time_seg(NmpTimeSegment *seg1, NmpTimeSegment *seg2)
{
	if (seg1->start <= seg2->end && seg2->start <= seg1->end)
	{
		seg1->start = MIN(seg1->start, seg2->start);
		seg1->end = MAX(seg1->end, seg2->end);
		return 0;
	}

	return -1;
}


static __inline__ gint
nmp_merge_date_seg(NmpTimeSegment *seg1, NmpTimeSegment *seg2)
{
#define SEG_DATE_EQU(s1, s2, name) \
	((s1)->time.date.name == (s2)->time.date.name)
	if (!SEG_DATE_EQU(seg1, seg2, year) ||
		!SEG_DATE_EQU(seg1, seg2, month) ||
		!SEG_DATE_EQU(seg1, seg2, date))
	{
		return -1;
	}
#undef SEG_DATE_EQU

	return nmp_merge_time_seg(seg1, seg2);
}


static __inline__ gint
nmp_merge_day_seg(NmpTimeSegment *seg1, NmpTimeSegment *seg2)
{
	if (seg1->time.day.day_of_week != seg2->time.day.day_of_week)
		return -1;

	return nmp_merge_time_seg(seg1, seg2);
}


static __inline__ gint
nmp_merge_seg(NmpTimeSegment *seg1, NmpTimeSegment *seg2)
{
	if (seg1->seg_type == TS_TYPE_DATE)
		return nmp_merge_date_seg(seg1, seg2);
	return nmp_merge_day_seg(seg1, seg2);
}


static void
nmp_add_seg(gpointer data, gpointer user_data)
{
	NmpTimeSegment **p_seg = (NmpTimeSegment**)user_data;
	NmpTimeSegment *seg = (NmpTimeSegment*)data;

	if (!*p_seg)
		return;

	if (seg->seg_type != (*p_seg)->seg_type ||
		seg->rec_type != (*p_seg)->rec_type)
	{
		return;
	}

	if (!nmp_merge_seg(seg, *p_seg))
	{
		*p_seg = NULL;
	}
}


static __inline__ void
nmp_policy_add_seg_internal(NmpPolicy *p, NmpTimeSegment *seg)
{
	NmpTimeSegment *seg_dup;

	g_list_foreach(p->time_segs, nmp_add_seg, &seg);
	if (seg)
	{
		seg_dup = g_memdup(seg, sizeof(NmpTimeSegment));
		if (seg->seg_type == TS_TYPE_DAY)
		{
			p->time_segs = g_list_append(p->time_segs, seg_dup);
		}
		else
		{
			p->time_segs = g_list_prepend(p->time_segs, seg_dup);
		}
	}
}


void
nmp_policy_add_time_seg_day(NmpPolicy *p, gint rec_type,
	gint day, time_t start, time_t end)
{
	NmpTimeSegment seg;
	g_assert(p != NULL);

	memset(&seg, 0, sizeof(seg));
	seg.seg_type = TS_TYPE_DAY;
	seg.rec_type = rec_type;
	seg.time.day.day_of_week = day;
	seg.start = start;
	seg.end = end;

	nmp_policy_add_seg_internal(p, &seg);
}


void
nmp_policy_add_time_seg_date(NmpPolicy *p, gint rec_type, gint year,
	gint month, gint date, time_t start, time_t end)
{
	NmpTimeSegment seg;

	memset(&seg, 0, sizeof(seg));
	seg.seg_type = TS_TYPE_DATE;
	seg.rec_type = rec_type;
	seg.time.date.year = year;
	seg.time.date.month = month;
	seg.time.date.date = date;
	seg.start = start;
	seg.end = end;

	nmp_policy_add_seg_internal(p, &seg);
}


static __inline__ gint
nmp_time_in_seg(NmpTimeSegment *seg, time_t secs)
{
	if (secs >= seg->start && secs <  seg->end)
		return 1;
	return 0;
}


static __inline__ void
nmp_check_date_seg(NmpPolicy *p, NmpTimeSegment *seg)
{
	NmpTime tm;
	time_t secs;

	nmp_get_real_time(&tm);

#define __SEG_P(seg, name) ((seg)->time.date.name)
	if (tm.year != __SEG_P(seg, year) ||
		tm.month != __SEG_P(seg, month) ||
		tm.date != __SEG_P(seg, date))
	{
		return;
	}
#undef __SEG_P

	secs = nmp_get_seconds_of_day();
	if (nmp_time_in_seg(seg, secs))
	{
		p->rec_now += seg->rec_type;
	}
}


static __inline__ void
nmp_check_day_seg(NmpPolicy *p, NmpTimeSegment *seg)
{
	NmpTime tm;
	time_t secs;

	nmp_get_real_time(&tm);

	if (tm.day_of_week != seg->time.day.day_of_week)
		return;

	secs = nmp_get_seconds_of_day();
	if (nmp_time_in_seg(seg, secs))
	{
		p->rec_now += seg->rec_type;
	}
}


static void
nmp_check_seg(gpointer data, gpointer user_data)
{
	NmpTimeSegment *seg = (NmpTimeSegment*)data;
	NmpPolicy *p = (NmpPolicy*)user_data;

	if (seg->seg_type != p->seg_now)
		return;

	if (seg->seg_type == TS_TYPE_DATE)
		nmp_check_date_seg(p, seg);
	else
		nmp_check_day_seg(p, seg);
}


static __inline__ void
nmp_policy_find_day_seg(NmpPolicy *p)
{
	p->seg_now = TS_TYPE_DAY;
	p->rec_now = TS_REC_TYPE_NULL;
	g_list_foreach(p->time_segs, nmp_check_seg, p);
}


static __inline__ void
nmp_policy_find_date_seg(NmpPolicy *p)
{
	p->seg_now = TS_TYPE_DATE;
	p->rec_now = TS_REC_TYPE_NULL;
	g_list_foreach(p->time_segs, nmp_check_seg, p);
}


static __inline__ gint
__nmp_policy_check(NmpPolicy *p)
{
	nmp_policy_find_date_seg(p);
	if (p->rec_now == TS_REC_TYPE_NULL)
		nmp_policy_find_day_seg(p);

	return p->rec_now;
}


gint
nmp_policy_check(NmpPolicy *p)
{
	gint ret;
	g_assert(p != NULL);

	g_mutex_lock(p->mutex);
	ret = __nmp_policy_check(p);
	g_mutex_unlock(p->mutex);

	return ret;	
}


//:~ End
