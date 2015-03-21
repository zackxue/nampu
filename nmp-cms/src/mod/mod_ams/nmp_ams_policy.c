#include <string.h>
#include <time.h>
#include "nmp_mods.h"
#include "nmp_ams_policy.h"
#include "nmp_msg_ams.h"
#include "nmp_debug.h"
#include "nmp_utility.h"
#include "nmp_res_ctl.h"
#include "nmp_shared.h"
#include "nmp_memory.h"


#define CUR_TIME	(time(NULL))
#define AMS_NEW_NODE_TIME			(60)			//60s
#define AMS_POLICY_CLEAR_TIME		(1000 * 10)	//10s扫描一次
#define LEVEL_POS					(20)
#define nmp_ams_print nmp_print
//#define AMS_POLICY_TEST

static NmpGuPool g_gu_pool;
static NmpAction *g_action[AMS_ACTION_MAX];
static nmp_ams_action_handler g_ams_action_hook = NULL;

static NmpActionOps g_action_record_ops;
static NmpActionOps g_action_io_ops;
static NmpActionOps g_action_step_ops;
static NmpActionOps g_action_preset_ops;
static NmpActionOps g_action_snapshot_ops;
static NmpActionOps g_action_map_ops;

#define AMS_ACTIONS_CLOSED	(-1)
#define AMS_ACTION_BIT_CLOSED	(-2)


static guint
nmp_str_hash(const gchar *start, gsize len)
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


static gint
nmp_check_alarm_action_enabled(guint action_bit)
{
	NmpResourcesCap res_cap;
	memset(&res_cap, 0, sizeof(res_cap));

	nmp_mod_get_resource_cap(&res_cap);
	if (!(res_cap.module_bits & MODULE_ALM_BIT))
	{
		return AMS_ACTIONS_CLOSED;
	}

	if (!(res_cap.modules_data[SYS_MODULE_ALM] & action_bit))
	{
		return AMS_ACTION_BIT_CLOSED;
	}

	return 0;
}


NmpPolicy *nmp_ams_new_policy()
{
	NmpPolicy *p;

	p = g_malloc0(sizeof(NmpPolicy));
	p->time_segs = NULL;
	p->mutex = g_mutex_new();

	return p;
}


static void
nmp_ams_free_seg(gpointer data, gpointer null)
{
	g_free(data);
}

static __inline__ void
__nmp_policy_clear(NmpPolicy *p)
{
	g_list_foreach(p->time_segs, nmp_ams_free_seg, NULL);
	g_list_free(p->time_segs);
	p->time_segs = NULL;
}

static void
nmp_policy_clear(NmpPolicy *p)
{
	g_assert(p != NULL);

	g_mutex_lock(p->mutex);
	__nmp_policy_clear(p);
	g_mutex_unlock(p->mutex);
}

static void
nmp_ams_delete_policy(NmpPolicy *p)
{
	nmp_policy_clear(p);
	g_mutex_free(p->mutex);
	g_free(p);
}


static void
nmp_ams_gu_free(gpointer data)
{
	g_assert(data != NULL);
	gint i;

	NmpAmsGu *gu = (NmpAmsGu *)data;
	if (gu->time_policy)
		nmp_ams_delete_policy(gu->time_policy);

	if (gu->act)
	{
		for (i = 0; i < AMS_ACTION_MAX; i++)
		{
			NmpAction *action = gu->act->actions[i];
			if (!action)
				break;

			(*action->ops->free)(action);
			g_free(action);
			gu->act->actions[i] = NULL;
		}

		g_free(gu->act);
	}

	g_free(gu);
}


static guint
nmp_gu_hash_fn(gconstpointer key)
{
	NmpShareGuid *_key;
	g_assert(key != NULL);

	_key = (NmpShareGuid*)key;

	return nmp_str_hash(_key->guid, strlen(_key->guid));
}

static gint
nmp_guid_equal(NmpShareGuid *guid_1, NmpShareGuid *guid_2)
{
	g_assert(guid_1 && guid_2);

	return !strcmp(guid_1->domain_id, guid_2->domain_id)
		&& !strcmp(guid_1->guid, guid_2->guid);
}

static gboolean
nmp_gu_key_equal(gconstpointer a, gconstpointer b)
{
	NmpShareGuid *gu_a, *gu_b;

	gu_a = (NmpShareGuid*)a;
	gu_b = (NmpShareGuid*)b;

	return nmp_guid_equal(gu_a, gu_b);
}


void nmp_ams_set_action_handler(nmp_ams_action_handler hook)
{
	g_ams_action_hook = hook;
}

gint nmp_ams_action_handle(gint dst, gint msg_type, void *parm, guint size)
{
	if (!g_ams_action_hook)
	{
		return -1;
	}

	return (*g_ams_action_hook)(dst, msg_type, parm, size);
}


static gboolean
nmp_ams_gu_equal(gpointer key, gpointer value, gpointer user_data)
{
	g_assert(value);
	NmpShareGuid *alarm_guid = (NmpShareGuid *)user_data;
	NmpShareGuid *_key = (NmpShareGuid *)key;

	return (!strcmp(alarm_guid->guid, _key->guid) &&
		!strcmp(alarm_guid->domain_id, _key->domain_id));
}


static gint
nmp_ams_policy_match(gconstpointer a, gconstpointer b)
{
	NmpTimeSegment *seg = (NmpTimeSegment *)a;
	NmpTime *time = (NmpTime *)b;
/*
	nmp_warning("zyt, seg->day_of_week:%d", seg->day_of_week);
	nmp_warning("zyt, seg->start:%ld", seg->start);
	nmp_warning("zyt, seg->end:%ld", seg->end);
	nmp_warning("-------zyt, time->day_of_week:%d", time->day_of_week);
	nmp_warning("-------zyt, time->cur_time:%ld", time->cur_time);
*/
	return (time->day_of_week == seg->day_of_week &&
		time->cur_time >= seg->start &&
		time->cur_time <= seg->end) ? 0 : 1;
}


static gint
__nmp_ams_policy_check(NmpPolicy *policy, NmpTime *time)
{
	if (g_list_find_custom(policy->time_segs, time, nmp_ams_policy_match))
		return 0;
	return -1;
}

static gint
nmp_ams_policy_check(NmpPolicy *policy, NmpTime *time)
{
	g_assert(policy && time);
	gint ret;

	g_mutex_lock(policy->mutex);
	ret = __nmp_ams_policy_check(policy, time);
	g_mutex_unlock(policy->mutex);

	return ret;
}


gint
__nmp_ams_find_gu_and_action(NmpAlarmInfo *alarm_info)
{
	//nmp_ams_print("func begin------");
	NmpAmsGu *ams_gu;
	gint ret;
	gint i;

	ams_gu = g_hash_table_find(g_gu_pool.gu, nmp_ams_gu_equal,
		&alarm_info->alarm_guid);
	if (!ams_gu)
	{
		nmp_ams_print("<NmpAmsPolicy>no find gu hash node.");
		return -1;
	}

	ret = nmp_ams_policy_check(ams_gu->time_policy, &alarm_info->time);
	if (ret != 0)
	{
		nmp_ams_print("<NmpAmsPolicy>ams policy check, no action.");
		return 0;
	}

	if (ams_gu->act)
	{
		for (i = 0; i < AMS_ACTION_MAX; i++)
		{
			NmpAction *action = ams_gu->act->actions[i];
			if (!action)
				break;

			(*action->ops->action)(action, alarm_info);
		}
	}

	return 0;
}

/*
 *	return:	0:	find success
 *			-1:	find failed
 */
gint
nmp_ams_find_gu_and_action(NmpAlarmInfo *alarm_info)
{
	g_assert(alarm_info);
	gint ret;

	g_mutex_lock(g_gu_pool.mutex);
	ret = __nmp_ams_find_gu_and_action(alarm_info);
	g_mutex_unlock(g_gu_pool.mutex);

	return ret;
}


NmpActions *nmp_get_actions(NmpMsgAmsGetActionInfoRes *info)
{
	//nmp_ams_print("func begin------");
	gint i = 0, action_count = 0;
	NmpActions *actions = (NmpActions *)g_malloc0(sizeof(NmpActions));
	if (!actions)
	{
		nmp_warning("<NmpAmsPolicy>no memory!");
		return NULL;
	}

	for (i = 0; i < AMS_ACTION_MAX; i++)
	{
		if (!g_action[i])
			break;

		NmpAction *p = NULL;
		p = (*g_action[i]->ops->get_data)(info);
		if (!p)
			continue;

		nmp_warning("<NmpAmsPolicy> action type = %d", p->type);
		actions->actions[action_count++] = p;
	}
	if (action_count == 0)
	{
		nmp_warning("<NmpAmsPolicy>get action error, data is NULL.");
		g_free(actions);
		return NULL;
	}

	return actions;
}


static void nmp_register_action_type(int size, NmpActionOps *ops)
{
	g_assert(ops);
	static gint g_action_i = 0;
	NmpAction *p_action = (NmpAction *)g_malloc0(size);
	if (!p_action)
		return ;

	(*ops->init)(p_action);
	p_action->ops = ops;

	g_action[g_action_i++] = p_action;
}


static NmpAction *nmp_new_action_type(int size, NmpActionOps *ops,
	NmpAmsActionType type)
{
	NmpAction *p_action = (NmpAction *)g_malloc0(size);
	if (!p_action)
		return NULL;

	(*ops->init)(p_action);
	p_action->ops = ops;
	p_action->type = type;

	return p_action;
}


static __inline__ gint
__nmp_gu_pool_add_gu(NmpGuPool *p, NmpAmsGu *gu)
{
	g_hash_table_insert(p->gu, &gu->guid, gu);

	return 0;
}

gint
nmp_gu_pool_add_gu(NmpGuPool *p, NmpAmsGu *gu)
{
	g_assert(p && gu);
	gint ret;

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_add_gu(p, gu);
	g_mutex_unlock(p->mutex);

	return ret;
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
nmp_merge_seg(NmpTimeSegment *seg1, NmpTimeSegment *seg2)
{
	if (seg1->day_of_week != seg2->day_of_week)
		return -1;

	return nmp_merge_time_seg(seg1, seg2);
}


static void
nmp_add_seg(gpointer data, gpointer user_data)
{
	NmpTimeSegment **p_seg = (NmpTimeSegment**)user_data;
	NmpTimeSegment *seg = (NmpTimeSegment*)data;

	if (!*p_seg)
		return;

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
		p->time_segs = g_list_append(p->time_segs, seg_dup);
	}
}


static void
nmp_policy_add_time_seg_day(NmpPolicy *p,
	gint day, time_t start, time_t end)
{
	NmpTimeSegment seg;
	g_assert(p != NULL);

	memset(&seg, 0, sizeof(seg));
	seg.day_of_week = day;
	seg.start = start;
	seg.end = end;

	nmp_policy_add_seg_internal(p, &seg);
}


static __inline__ void
nmp_mod_policy_adjust_seg(time_t *start, time_t *end)
{
	*end += 1;
}


static __inline__ void
nmp_ams_policy_parse(NmpPolicy *policy, NmpActionPolicy *p)
{
	gint day, day_upper, seg, seg_upper;
	NmpWeekday *w;
	NmpTimeSeg *s;
	time_t start, end;

	//nmp_ams_print("p->weekday_num:%d\n", p->weekday_num);
	day_upper = p->weekday_num > WEEKDAYS ? WEEKDAYS : p->weekday_num;
	for (day = 0; day < day_upper; ++day)
	{
		w = &p->weekdays[day];
		//nmp_ams_print("w->time_seg_num:%d\n", w->time_seg_num);

		seg_upper = w->time_seg_num > TIME_SEG_NUM ? TIME_SEG_NUM
			: w->time_seg_num;
		for (seg = 0; seg < seg_upper; ++seg)
		{
			s = &w->time_segs[seg];
			//nmp_ams_print("-------weekday:%d------> time_seg:%s\n",
			//	w->weekday, s->time_seg);
			if (!nmp_get_string_time_range(s->time_seg, &start, &end))
			{
				nmp_mod_policy_adjust_seg(&start, &end);
				nmp_policy_add_time_seg_day(policy,
					(w->weekday < 7 ? w->weekday : 0), start, end);
			}
		}
	}
}


#ifdef AMS_POLICY_TEST
static void ams_src_policy_print(NmpActionPolicy *action_policy)
{
	gint i, j;
	nmp_ams_print("<NmpAmsPolicy>weekday_num=%d.", action_policy->weekday_num);

	for (i = 0; i < action_policy->weekday_num; i++)
	{
		NmpWeekday *weekday = &action_policy->weekdays[i];
		nmp_ams_print("<NmpAmsPolicy>----weekday=%d", weekday->weekday);
		nmp_ams_print("<NmpAmsPolicy>    time_seg_num=%d", weekday->time_seg_num);
		for (j = 0; j < weekday->time_seg_num; j++)
		{
			nmp_ams_print("<NmpAmsPolicy>    enable=%d, time_seg:%s",
				weekday->time_segs[j].seg_enable, weekday->time_segs[j].time_seg);
		}
	}
}

static void policy_print(gpointer data, gpointer user_data)
{
	NmpTimeSegment *seg = (NmpTimeSegment *)data;
	nmp_ams_print("<NmpAmsPolicy>****policy print");
	nmp_ams_print("<NmpAmsPolicy>    day_of_week=%d", seg->day_of_week);
	nmp_ams_print("<NmpAmsPolicy>    start=%ld", seg->start);
	nmp_ams_print("<NmpAmsPolicy>    end=%ld", seg->end);
}

static void ams_alarm_policy_print(NmpPolicy *policy)
{
	g_mutex_lock(policy->mutex);
	nmp_ams_print("<NmpAmsPolicy> ams_alarm_policy_print begin...");
	g_list_foreach(policy->time_segs, policy_print, NULL);
	g_mutex_unlock(policy->mutex);
}
#endif


gint nmp_ams_get_gu_info(NmpAmsGu *ams_gu, NmpShareGuid *alarm_guid,
	NmpMsgAmsGetActionInfoRes *action_info)
{
	ams_gu->guid = *alarm_guid;
	NmpPolicy *policy;
	NmpActions *actions;

	policy = nmp_ams_new_policy();
	if (!policy)
	{
		nmp_warning("<NmpAmsPolicy>no memory!");
		return -1;
	}

	nmp_ams_policy_parse(policy, &action_info->action_policy);

#ifdef AMS_POLICY_TEST
	ams_src_policy_print(&action_info->action_policy);
	ams_alarm_policy_print(policy);
#endif

	actions = nmp_get_actions(action_info);
	if (!actions)
	{
		nmp_ams_delete_policy(policy);
		return -1;
	}

	ams_gu->time_policy = policy;
	ams_gu->act = actions;

	return 0;
}


gint nmp_ams_add_gu_action_info(NmpShareGuid *alarm_guid,
	NmpMsgAmsGetActionInfoRes *action_info)
{
	g_assert(alarm_guid && action_info);
	NmpAmsGu *ams_gu;
	gint ret;

	ams_gu = (NmpAmsGu *)g_malloc0(sizeof(NmpAmsGu));
	if (!ams_gu)
	{
		nmp_warning("<NmpAmsPolicy>no memory!");
		return -1;
	}

	ret = nmp_ams_get_gu_info(ams_gu, alarm_guid, action_info);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy>nmp_ams_get_gu_info failed!");
		nmp_ams_gu_free(ams_gu);
		return -1;
	}

	ams_gu->time = CUR_TIME;
	nmp_gu_pool_add_gu(&g_gu_pool, ams_gu);

	return ret;
}


gint nmp_ams_remove_alarm_node(NmpShareGuid *key)
{
	g_assert(key);
	gint ret;

	g_mutex_lock(g_gu_pool.mutex);
	ret = g_hash_table_remove(g_gu_pool.gu, key) ? 0 : -1;
	g_mutex_unlock(g_gu_pool.mutex);

	return ret;
}


static int nmp_action_record_init(NmpAction *p)
{
	g_assert(p);
	NmpActionRecord *ar = (NmpActionRecord *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int nmp_action_record_free(NmpAction *p)
{
	g_assert(p);
	//NmpActionRecord *ar = (NmpActionRecord *)p;

	return 0;
}


static NmpActionP nmp_action_record_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_record_get_data begin------");
	g_assert(info);
	NmpMsgActionRecord *ar_src;
	NmpActionRecord *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_RECORD])
		return NULL;

	ar_src = (NmpMsgActionRecord *)info->actions[AMS_ACTION_RECORD];
	count = ar_src->action_gu_count;
	size = sizeof(NmpActionRecord) + sizeof(NmpActionRecordGu) * count;

	ar = (NmpActionRecord *)nmp_new_action_type(size, &g_action_record_ops,
		AMS_ACTION_RECORD);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (NmpAction *)ar;
}


static int nmp_action_record_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_record_do begin------");
	g_assert(p && alarm_info);
	NmpActionRecord *ar = (NmpActionRecord *)p;
	NmpAmsActionRecord action_info;
	gint ret = 0;
	gint i, j;

	ret = nmp_check_alarm_action_enabled(ACTION_RECORD_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action record not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionRecordGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.level = act_gu->level;
		action_info.time_len = act_gu->time_len;
		action_info.alarm_type = alarm_info->alarm_type;

		if (act_gu->mss_count == 0)
			nmp_warning("<NmpAmsPolicy> act_gu->mss_count = 0.");
		for (j = 0; j < act_gu->mss_count; j++)
		{
			action_info.mss_id = act_gu->mss_id[j];
			if (nmp_ams_action_handle(BUSSLOT_POS_MSS, MESSAGE_ALARM_LINK_RECORD,
				&action_info, sizeof(action_info)) != 0)
			{
				ret = -1;
				nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
				break;
			}
		}
	}

	return ret;
}


static NmpActionOps g_action_record_ops =
{
	.init = nmp_action_record_init,
	.free = nmp_action_record_free,
	.get_data = nmp_action_record_get_data,
	.action = nmp_action_record_do
};


static int nmp_action_io_init(NmpAction *p)
{
	g_assert(p);
	NmpActionIO *ar = (NmpActionIO *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int nmp_action_io_free(NmpAction *p)
{
	g_assert(p);

	return 0;
}


static NmpActionP nmp_action_io_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_io_get_data begin------");
	g_assert(info);
	NmpMsgActionIO *ar_src;
	NmpActionIO *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_IO])
		return NULL;

	ar_src = (NmpMsgActionIO *)info->actions[AMS_ACTION_IO];
	count = ar_src->action_gu_count;
	size = sizeof(NmpActionIO) + sizeof(NmpActionIOGu) * count;

	ar = (NmpActionIO *)nmp_new_action_type(size, &g_action_io_ops,
		AMS_ACTION_IO);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (NmpAction *)ar;
}


static int nmp_action_io_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_io_do begin------");
	g_assert(p && alarm_info);
	NmpActionIO *ar = (NmpActionIO *)p;
	NmpAmsActionIO action_info;
	gint ret = 0;
	gint i;

	ret = nmp_check_alarm_action_enabled(ACTION_IO_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action io not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionIOGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.time_len = act_gu->time_len;
		strncpy(action_info.io_value, act_gu->io_value, IO_VALUE_LEN - 1);
		strncpy(action_info.session, nmp_get_local_domain_id(), SESSION_ID_LEN - 1);

		if (nmp_ams_action_handle(BUSSLOT_POS_PU, MESSAGE_ALARM_LINK_IO,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static NmpActionOps g_action_io_ops =
{
	.init = nmp_action_io_init,
	.free = nmp_action_io_free,
	.get_data = nmp_action_io_get_data,
	.action = nmp_action_io_do
};


static int nmp_action_step_init(NmpAction *p)
{
	g_assert(p);
	NmpActionStep *ar = (NmpActionStep *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int nmp_action_step_free(NmpAction *p)
{
	g_assert(p);

	return 0;
}


static NmpActionP nmp_action_step_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_step_get_data begin------");
	g_assert(info);
	NmpMsgActionStep *ar_src;
	NmpActionStep *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_STEP])
		return NULL;

	ar_src = (NmpMsgActionStep *)info->actions[AMS_ACTION_STEP];
	count = ar_src->action_gu_count;
	size = sizeof(NmpActionStep) + sizeof(NmpActionStepGu) * count;

	ar = (NmpActionStep *)nmp_new_action_type(size, &g_action_step_ops,
		AMS_ACTION_STEP);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
		ar->action_gu[i].ec_guid[LEVEL_POS] = (gchar)(ar->action_gu[i].level % 10 + '0');	//设置码流
	}

	return (NmpAction *)ar;
}


static int nmp_action_step_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_step_do begin------");
	g_assert(p && alarm_info);
	NmpActionStep *ar = (NmpActionStep *)p;
	tw_run_step_request action_info;
	gint ret = 0;
	gint i;

	ret = nmp_check_alarm_action_enabled(ACTION_TW_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action tw(step) not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionStepGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));

		strcpy(action_info.session_id, "");
		action_info.tw_id = act_gu->tw_id;
		action_info.screen_id = act_gu->screen_id;
		action_info.division_id = act_gu->division_id;
		action_info.division_num = act_gu->division_num;
		strncpy(action_info.ec_name, act_gu->ec_name, TW_MAX_VALUE_LEN - 1);
		strncpy(action_info.ec_domain_id, act_gu->ec_domain_id, TW_ID_LEN - 1);
		strncpy(action_info.ec_guid, act_gu->ec_guid, TW_ID_LEN - 1);

		if (nmp_ams_action_handle(BUSSLOT_POS_TW, MSG_LINK_RUN_STEP,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static NmpActionOps g_action_step_ops =
{
	.init = nmp_action_step_init,
	.free = nmp_action_step_free,
	.get_data = nmp_action_step_get_data,
	.action = nmp_action_step_do
};


static int nmp_action_preset_init(NmpAction *p)
{
	g_assert(p);
	NmpActionPreset *ar = (NmpActionPreset *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int nmp_action_preset_free(NmpAction *p)
{
	g_assert(p);

	return 0;
}


static NmpActionP nmp_action_preset_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_preset_get_data begin------");
	g_assert(info);
	NmpMsgActionPreset *ar_src;
	NmpActionPreset *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_PRESET])
		return NULL;

	ar_src = (NmpMsgActionPreset *)info->actions[AMS_ACTION_PRESET];
	count = ar_src->action_gu_count;
	size = sizeof(NmpActionPreset) + sizeof(NmpActionPresetGu) * count;

	ar = (NmpActionPreset *)nmp_new_action_type(size, &g_action_preset_ops,
		AMS_ACTION_PRESET);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (NmpAction *)ar;
}

#if 0
static void nmp_action_preset_print(NmpAmsActionPreset *action_info)
{
	nmp_ams_print("_____________________nmp_action_preset_print begin...");
	nmp_ams_print("action_info->action_guid.guid:%s", action_info->action_guid.guid);
	nmp_ams_print("action_info->action_guid.domain_id:%s", action_info->action_guid.domain_id);
	nmp_ams_print("action_info->preset_num = %d", action_info->preset_num);
	nmp_ams_print("action_info->session:%s", action_info->session);
	nmp_ams_print("______end...");
}
#endif

static int nmp_action_preset_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_preset_do begin------");
	g_assert(p && alarm_info);
	NmpActionPreset *ar = (NmpActionPreset *)p;
	NmpAmsActionPreset action_info;
	gint ret = 0;
	gint i;

	ret = nmp_check_alarm_action_enabled(ACTION_PRESET_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action preset not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionPresetGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.preset_num = act_gu->preset_num;
		strncpy(action_info.session, nmp_get_local_domain_id(), SESSION_ID_LEN - 1);
		//nmp_action_preset_print(&action_info);

		if (nmp_ams_action_handle(BUSSLOT_POS_PU, MESSAGE_ALARM_LINK_PRESET,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static NmpActionOps g_action_preset_ops =
{
	.init = nmp_action_preset_init,
	.free = nmp_action_preset_free,
	.get_data = nmp_action_preset_get_data,
	.action = nmp_action_preset_do
};


static int nmp_action_snapshot_init(NmpAction *p)
{
	g_assert(p);
	NmpActionSnapshot *ar = (NmpActionSnapshot *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int nmp_action_snapshot_free(NmpAction *p)
{
	g_assert(p);

	return 0;
}


static NmpActionP nmp_action_snapshot_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_snapshot_get_data begin------");
	g_assert(info);
	NmpMsgActionSnapshot *ar_src;
	NmpActionSnapshot *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_SNAPSHOT])
		return NULL;

	ar_src = (NmpMsgActionSnapshot *)info->actions[AMS_ACTION_SNAPSHOT];
	count = ar_src->action_gu_count;
	size = sizeof(NmpActionSnapshot) + sizeof(NmpActionSnapshotGu) * count;

	ar = (NmpActionSnapshot *)nmp_new_action_type(size, &g_action_snapshot_ops,
		AMS_ACTION_SNAPSHOT);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (NmpAction *)ar;
}

#if 0
static void nmp_action_snapshop_print(NmpAmsActionSnapshot *action_info)
{
	nmp_ams_print("_____________________nmp_action_snapshop_print begin...");
	nmp_ams_print("action_info->action_guid.guid:%s", action_info->action_guid.guid);
	nmp_ams_print("action_info->action_guid.domain_id:%s", action_info->action_guid.domain_id);
	nmp_ams_print("action_info->level:%d", action_info->level);
	nmp_ams_print("action_info->mss_id.mss_id:%s", action_info->mss_id.mss_id);
	nmp_ams_print("action_info->picture_count:%u", action_info->picture_count);
	nmp_ams_print("action_info->alarm_type:%u", action_info->alarm_type);
	nmp_ams_print("______end...");
}
#endif

static int nmp_action_snapshot_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_snapshot_do begin------");
	g_assert(p && alarm_info);
	NmpActionSnapshot *ar = (NmpActionSnapshot *)p;
	NmpAmsActionSnapshot action_info;
	gint ret = 0;
	gint i, j;

	ret = nmp_check_alarm_action_enabled(ACTION_CAP_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action snapshot not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionSnapshotGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			nmp_ams_print("<NmpAmsPolicy> action_snapshot alarm_type(s):%d, " \
				"alarm_info->alarm_type:%d, not action",
				act_gu->alarm_type, alarm_info->alarm_type);
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.level = act_gu->level;
		action_info.picture_count = act_gu->picture_count;
		action_info.alarm_type = alarm_info->alarm_type;

		if (act_gu->mss_count == 0)
			nmp_warning("<NmpAmsPolicy> act_gu->mss_count = 0.");
		for (j = 0; j < act_gu->mss_count; j++)
		{
			action_info.mss_id = act_gu->mss_id[j];
			//nmp_action_snapshop_print(&action_info);

			if (nmp_ams_action_handle(BUSSLOT_POS_MSS, MESSAGE_ALARM_LINK_SNAPSHOT,
				&action_info, sizeof(action_info)) != 0)
			{
				ret = -1;
				nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
				break;
			}
		}
	}

	return ret;
}


static NmpActionOps g_action_snapshot_ops =
{
	.init = nmp_action_snapshot_init,
	.free = nmp_action_snapshot_free,
	.get_data = nmp_action_snapshot_get_data,
	.action = nmp_action_snapshot_do
};


static int nmp_action_map_init(NmpAction *p)
{
	g_assert(p);
	NmpActionMap *ar = (NmpActionMap *)p;

	ar->action_gu_count = 0;
	ar->cu_count= 0;

	return 0;
}


static int nmp_action_map_free(NmpAction *p)
{
	g_assert(p);

	return 0;
}


static NmpActionP nmp_action_map_get_data(NmpMsgAmsGetActionInfoRes *info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_map_get_data begin------");
	g_assert(info);
	NmpMsgActionMap *ar_src;
	NmpActionMap *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_MAP])
		return NULL;

	ar_src = (NmpMsgActionMap *)info->actions[AMS_ACTION_MAP];
	count = ar_src->cu_count;
	size = sizeof(NmpActionMap) + sizeof(NmpAllCuOwnPu) * count;

	ar = (NmpActionMap *)nmp_new_action_type(size, &g_action_map_ops,
		AMS_ACTION_MAP);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	ar->cu_count = ar_src->cu_count;
	for (i = 0; i < ar_src->cu_count; i++)
	{
		ar->cu_list[i] = ar_src->cu_list[i];
	}

	ar->defence_id = ar_src->defence_id;
	strncpy(ar->defence_name, ar_src->defence_name, AREA_NAME_LEN - 1);
	ar->map_id = ar_src->map_id;
	strncpy(ar->map_name, ar_src->map_name, MAP_NAME_LEN - 1);
	strncpy(ar->gu_name, ar_src->gu_name, GU_NAME_LEN - 1);
	return (NmpAction *)ar;
}


static int nmp_action_map_do(NmpAction *p, NmpAlarmInfo *alarm_info)
{
	nmp_ams_print("<NmpAmsPolicy> nmp_action_map_do begin------");
	g_assert(p && alarm_info);
	NmpActionMap *ar = (NmpActionMap *)p;
	NmpAmsActionMap *action_info;
	gint ret = 0;
	gint i, size, gu_count = 0;

	ret = nmp_check_alarm_action_enabled(ACTION_EMAP_BIT);
	if (ret != 0)
	{
		nmp_warning("<NmpAmsPolicy> action io not enabled, ret = %d.", ret);
		return 0;
	}

	size = sizeof(NmpAmsActionMap) + sizeof(NmpAllCuOwnPu)*ar->cu_count;
	action_info = nmp_mem_kalloc(size);
	memset(action_info, 0, size);
	for (i = 0; i < ar->action_gu_count; i++)
	{
		NmpActionMapGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		action_info->action_gu[gu_count].action_guid = act_gu->action_guid;
		strcpy(action_info->action_gu[gu_count].gu_name, act_gu->gu_name);
		action_info->action_gu[gu_count].map_id = act_gu->map_id;
		strcpy(action_info->action_gu[gu_count].map_name, act_gu->map_name);
		action_info->action_gu[gu_count].level = act_gu->level;
		gu_count++;

	}

	if (gu_count > 0)
	{
		action_info->action_guid = alarm_info->alarm_guid;
		action_info->alarm_type = alarm_info->alarm_type;
		strncpy(action_info->alarm_time, alarm_info->alarm_time, TIME_INFO_LEN - 1);
		strcpy(action_info->gu_name, ar->gu_name);
		action_info->defence_id = ar->defence_id;
		strcpy(action_info->defence_name, ar->defence_name);
		action_info->map_id = ar->map_id;
		strcpy(action_info->map_name, ar->map_name);
		action_info->cu_count = ar->cu_count;
		memcpy(action_info->cu_list, ar->cu_list, ar->cu_count*sizeof(NmpAllCuOwnPu));
		action_info->action_gu_count = gu_count;
		if (nmp_ams_action_handle(BUSSLOT_POS_CU, MESSAGE_ALARM_LINK_MAP,
			action_info, size) != 0)
		{
			ret = -1;
			nmp_warning("<NmpAmsPolicy>send msg failed, may be no memory!");
		}
	}

	nmp_mem_kfree(action_info, size);

	return ret;
}


static NmpActionOps g_action_map_ops =
{
	.init = nmp_action_map_init,
	.free = nmp_action_map_free,
	.get_data = nmp_action_map_get_data,
	.action = nmp_action_map_do
};




static gboolean
nmp_ams_if_old_node(gpointer key, gpointer value, gpointer user_data)
{
	g_assert(key && value);

	NmpAmsGu *gu = (NmpAmsGu *)value;
	GTime cur_time = CUR_TIME;

	if (cur_time >= gu->time && cur_time - gu->time <= AMS_NEW_NODE_TIME)
	{
		return FALSE;
	}

	return TRUE;
}


static gboolean
nmp_ams_pool_timer(gpointer user_data)
{
	NmpGuPool *p = (NmpGuPool *)user_data;
	gint n_del;

	g_mutex_lock(p->mutex);

	n_del = g_hash_table_foreach_remove(p->gu, nmp_ams_if_old_node, NULL);

	g_mutex_unlock(p->mutex);
	if (n_del != 0)
		nmp_print("********************* n_del = %d.", n_del);
	return TRUE;
}


void nmp_ams_init_gu_pool()
{
	memset(&g_gu_pool, 0, sizeof(NmpGuPool));

	g_gu_pool.mutex = g_mutex_new();

	g_gu_pool.gu = g_hash_table_new_full(nmp_gu_hash_fn, nmp_gu_key_equal,
		NULL, nmp_ams_gu_free);

	g_gu_pool.timer = nmp_set_timer(AMS_POLICY_CLEAR_TIME,
		nmp_ams_pool_timer, &g_gu_pool);
}


void nmp_ams_init_actions()
{
	nmp_register_action_type(sizeof(NmpActionRecord), &g_action_record_ops);
	nmp_register_action_type(sizeof(NmpActionIO), &g_action_io_ops);
	nmp_register_action_type(sizeof(NmpActionStep), &g_action_step_ops);
	nmp_register_action_type(sizeof(NmpActionPreset), &g_action_preset_ops);
	nmp_register_action_type(sizeof(NmpActionSnapshot), &g_action_snapshot_ops);
	nmp_register_action_type(sizeof(NmpActionMap), &g_action_map_ops);
}


void nmp_ams_policy_init()
{
	nmp_ams_init_gu_pool();
	nmp_ams_init_actions();
}

