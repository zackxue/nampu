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
#define jpf_ams_print jpf_print
//#define AMS_POLICY_TEST

static JpfGuPool g_gu_pool;
static JpfAction *g_action[AMS_ACTION_MAX];
static jpf_ams_action_handler g_ams_action_hook = NULL;

static JpfActionOps g_action_record_ops;
static JpfActionOps g_action_io_ops;
static JpfActionOps g_action_step_ops;
static JpfActionOps g_action_preset_ops;
static JpfActionOps g_action_snapshot_ops;
static JpfActionOps g_action_map_ops;

#define AMS_ACTIONS_CLOSED	(-1)
#define AMS_ACTION_BIT_CLOSED	(-2)


static guint
jpf_str_hash(const gchar *start, gsize len)
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
jpf_check_alarm_action_enabled(guint action_bit)
{
	JpfResourcesCap res_cap;
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


JpfPolicy *jpf_ams_new_policy()
{
	JpfPolicy *p;

	p = g_malloc0(sizeof(JpfPolicy));
	p->time_segs = NULL;
	p->mutex = g_mutex_new();

	return p;
}


static void
jpf_ams_free_seg(gpointer data, gpointer null)
{
	g_free(data);
}

static __inline__ void
__jpf_policy_clear(JpfPolicy *p)
{
	g_list_foreach(p->time_segs, jpf_ams_free_seg, NULL);
	g_list_free(p->time_segs);
	p->time_segs = NULL;
}

static void
jpf_policy_clear(JpfPolicy *p)
{
	g_assert(p != NULL);

	g_mutex_lock(p->mutex);
	__jpf_policy_clear(p);
	g_mutex_unlock(p->mutex);
}

static void
jpf_ams_delete_policy(JpfPolicy *p)
{
	jpf_policy_clear(p);
	g_mutex_free(p->mutex);
	g_free(p);
}


static void
jpf_ams_gu_free(gpointer data)
{
	g_assert(data != NULL);
	gint i;

	JpfAmsGu *gu = (JpfAmsGu *)data;
	if (gu->time_policy)
		jpf_ams_delete_policy(gu->time_policy);

	if (gu->act)
	{
		for (i = 0; i < AMS_ACTION_MAX; i++)
		{
			JpfAction *action = gu->act->actions[i];
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
jpf_gu_hash_fn(gconstpointer key)
{
	JpfShareGuid *_key;
	g_assert(key != NULL);

	_key = (JpfShareGuid*)key;

	return jpf_str_hash(_key->guid, strlen(_key->guid));
}

static gint
jpf_guid_equal(JpfShareGuid *guid_1, JpfShareGuid *guid_2)
{
	g_assert(guid_1 && guid_2);

	return !strcmp(guid_1->domain_id, guid_2->domain_id)
		&& !strcmp(guid_1->guid, guid_2->guid);
}

static gboolean
jpf_gu_key_equal(gconstpointer a, gconstpointer b)
{
	JpfShareGuid *gu_a, *gu_b;

	gu_a = (JpfShareGuid*)a;
	gu_b = (JpfShareGuid*)b;

	return jpf_guid_equal(gu_a, gu_b);
}


void jpf_ams_set_action_handler(jpf_ams_action_handler hook)
{
	g_ams_action_hook = hook;
}

gint jpf_ams_action_handle(gint dst, gint msg_type, void *parm, guint size)
{
	if (!g_ams_action_hook)
	{
		return -1;
	}

	return (*g_ams_action_hook)(dst, msg_type, parm, size);
}


static gboolean
jpf_ams_gu_equal(gpointer key, gpointer value, gpointer user_data)
{
	g_assert(value);
	JpfShareGuid *alarm_guid = (JpfShareGuid *)user_data;
	JpfShareGuid *_key = (JpfShareGuid *)key;

	return (!strcmp(alarm_guid->guid, _key->guid) &&
		!strcmp(alarm_guid->domain_id, _key->domain_id));
}


static gint
jpf_ams_policy_match(gconstpointer a, gconstpointer b)
{
	JpfTimeSegment *seg = (JpfTimeSegment *)a;
	JpfTime *time = (JpfTime *)b;
/*
	jpf_warning("zyt, seg->day_of_week:%d", seg->day_of_week);
	jpf_warning("zyt, seg->start:%ld", seg->start);
	jpf_warning("zyt, seg->end:%ld", seg->end);
	jpf_warning("-------zyt, time->day_of_week:%d", time->day_of_week);
	jpf_warning("-------zyt, time->cur_time:%ld", time->cur_time);
*/
	return (time->day_of_week == seg->day_of_week &&
		time->cur_time >= seg->start &&
		time->cur_time <= seg->end) ? 0 : 1;
}


static gint
__jpf_ams_policy_check(JpfPolicy *policy, JpfTime *time)
{
	if (g_list_find_custom(policy->time_segs, time, jpf_ams_policy_match))
		return 0;
	return -1;
}

static gint
jpf_ams_policy_check(JpfPolicy *policy, JpfTime *time)
{
	g_assert(policy && time);
	gint ret;

	g_mutex_lock(policy->mutex);
	ret = __jpf_ams_policy_check(policy, time);
	g_mutex_unlock(policy->mutex);

	return ret;
}


gint
__jpf_ams_find_gu_and_action(JpfAlarmInfo *alarm_info)
{
	//jpf_ams_print("func begin------");
	JpfAmsGu *ams_gu;
	gint ret;
	gint i;

	ams_gu = g_hash_table_find(g_gu_pool.gu, jpf_ams_gu_equal,
		&alarm_info->alarm_guid);
	if (!ams_gu)
	{
		jpf_ams_print("<JpfAmsPolicy>no find gu hash node.");
		return -1;
	}

	ret = jpf_ams_policy_check(ams_gu->time_policy, &alarm_info->time);
	if (ret != 0)
	{
		jpf_ams_print("<JpfAmsPolicy>ams policy check, no action.");
		return 0;
	}

	if (ams_gu->act)
	{
		for (i = 0; i < AMS_ACTION_MAX; i++)
		{
			JpfAction *action = ams_gu->act->actions[i];
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
jpf_ams_find_gu_and_action(JpfAlarmInfo *alarm_info)
{
	g_assert(alarm_info);
	gint ret;

	g_mutex_lock(g_gu_pool.mutex);
	ret = __jpf_ams_find_gu_and_action(alarm_info);
	g_mutex_unlock(g_gu_pool.mutex);

	return ret;
}


JpfActions *jpf_get_actions(JpfMsgAmsGetActionInfoRes *info)
{
	//jpf_ams_print("func begin------");
	gint i = 0, action_count = 0;
	JpfActions *actions = (JpfActions *)g_malloc0(sizeof(JpfActions));
	if (!actions)
	{
		jpf_warning("<JpfAmsPolicy>no memory!");
		return NULL;
	}

	for (i = 0; i < AMS_ACTION_MAX; i++)
	{
		if (!g_action[i])
			break;

		JpfAction *p = NULL;
		p = (*g_action[i]->ops->get_data)(info);
		if (!p)
			continue;

		jpf_warning("<JpfAmsPolicy> action type = %d", p->type);
		actions->actions[action_count++] = p;
	}
	if (action_count == 0)
	{
		jpf_warning("<JpfAmsPolicy>get action error, data is NULL.");
		g_free(actions);
		return NULL;
	}

	return actions;
}


static void jpf_register_action_type(int size, JpfActionOps *ops)
{
	g_assert(ops);
	static gint g_action_i = 0;
	JpfAction *p_action = (JpfAction *)g_malloc0(size);
	if (!p_action)
		return ;

	(*ops->init)(p_action);
	p_action->ops = ops;

	g_action[g_action_i++] = p_action;
}


static JpfAction *jpf_new_action_type(int size, JpfActionOps *ops,
	JpfAmsActionType type)
{
	JpfAction *p_action = (JpfAction *)g_malloc0(size);
	if (!p_action)
		return NULL;

	(*ops->init)(p_action);
	p_action->ops = ops;
	p_action->type = type;

	return p_action;
}


static __inline__ gint
__jpf_gu_pool_add_gu(JpfGuPool *p, JpfAmsGu *gu)
{
	g_hash_table_insert(p->gu, &gu->guid, gu);

	return 0;
}

gint
jpf_gu_pool_add_gu(JpfGuPool *p, JpfAmsGu *gu)
{
	g_assert(p && gu);
	gint ret;

	g_mutex_lock(p->mutex);
	ret = __jpf_gu_pool_add_gu(p, gu);
	g_mutex_unlock(p->mutex);

	return ret;
}


static __inline__ gint
jpf_merge_time_seg(JpfTimeSegment *seg1, JpfTimeSegment *seg2)
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
jpf_merge_seg(JpfTimeSegment *seg1, JpfTimeSegment *seg2)
{
	if (seg1->day_of_week != seg2->day_of_week)
		return -1;

	return jpf_merge_time_seg(seg1, seg2);
}


static void
jpf_add_seg(gpointer data, gpointer user_data)
{
	JpfTimeSegment **p_seg = (JpfTimeSegment**)user_data;
	JpfTimeSegment *seg = (JpfTimeSegment*)data;

	if (!*p_seg)
		return;

	if (!jpf_merge_seg(seg, *p_seg))
	{
		*p_seg = NULL;
	}
}


static __inline__ void
jpf_policy_add_seg_internal(JpfPolicy *p, JpfTimeSegment *seg)
{
	JpfTimeSegment *seg_dup;

	g_list_foreach(p->time_segs, jpf_add_seg, &seg);
	if (seg)
	{
		seg_dup = g_memdup(seg, sizeof(JpfTimeSegment));
		p->time_segs = g_list_append(p->time_segs, seg_dup);
	}
}


static void
jpf_policy_add_time_seg_day(JpfPolicy *p,
	gint day, time_t start, time_t end)
{
	JpfTimeSegment seg;
	g_assert(p != NULL);

	memset(&seg, 0, sizeof(seg));
	seg.day_of_week = day;
	seg.start = start;
	seg.end = end;

	jpf_policy_add_seg_internal(p, &seg);
}


static __inline__ void
nmp_mod_policy_adjust_seg(time_t *start, time_t *end)
{
	*end += 1;
}


static __inline__ void
jpf_ams_policy_parse(JpfPolicy *policy, JpfActionPolicy *p)
{
	gint day, day_upper, seg, seg_upper;
	JpfWeekday *w;
	JpfTimeSeg *s;
	time_t start, end;

	//jpf_ams_print("p->weekday_num:%d\n", p->weekday_num);
	day_upper = p->weekday_num > WEEKDAYS ? WEEKDAYS : p->weekday_num;
	for (day = 0; day < day_upper; ++day)
	{
		w = &p->weekdays[day];
		//jpf_ams_print("w->time_seg_num:%d\n", w->time_seg_num);

		seg_upper = w->time_seg_num > TIME_SEG_NUM ? TIME_SEG_NUM
			: w->time_seg_num;
		for (seg = 0; seg < seg_upper; ++seg)
		{
			s = &w->time_segs[seg];
			//jpf_ams_print("-------weekday:%d------> time_seg:%s\n",
			//	w->weekday, s->time_seg);
			if (!jpf_get_string_time_range(s->time_seg, &start, &end))
			{
				nmp_mod_policy_adjust_seg(&start, &end);
				jpf_policy_add_time_seg_day(policy,
					(w->weekday < 7 ? w->weekday : 0), start, end);
			}
		}
	}
}


#ifdef AMS_POLICY_TEST
static void ams_src_policy_print(JpfActionPolicy *action_policy)
{
	gint i, j;
	jpf_ams_print("<JpfAmsPolicy>weekday_num=%d.", action_policy->weekday_num);

	for (i = 0; i < action_policy->weekday_num; i++)
	{
		JpfWeekday *weekday = &action_policy->weekdays[i];
		jpf_ams_print("<JpfAmsPolicy>----weekday=%d", weekday->weekday);
		jpf_ams_print("<JpfAmsPolicy>    time_seg_num=%d", weekday->time_seg_num);
		for (j = 0; j < weekday->time_seg_num; j++)
		{
			jpf_ams_print("<JpfAmsPolicy>    enable=%d, time_seg:%s",
				weekday->time_segs[j].seg_enable, weekday->time_segs[j].time_seg);
		}
	}
}

static void policy_print(gpointer data, gpointer user_data)
{
	JpfTimeSegment *seg = (JpfTimeSegment *)data;
	jpf_ams_print("<JpfAmsPolicy>****policy print");
	jpf_ams_print("<JpfAmsPolicy>    day_of_week=%d", seg->day_of_week);
	jpf_ams_print("<JpfAmsPolicy>    start=%ld", seg->start);
	jpf_ams_print("<JpfAmsPolicy>    end=%ld", seg->end);
}

static void ams_alarm_policy_print(JpfPolicy *policy)
{
	g_mutex_lock(policy->mutex);
	jpf_ams_print("<JpfAmsPolicy> ams_alarm_policy_print begin...");
	g_list_foreach(policy->time_segs, policy_print, NULL);
	g_mutex_unlock(policy->mutex);
}
#endif


gint jpf_ams_get_gu_info(JpfAmsGu *ams_gu, JpfShareGuid *alarm_guid,
	JpfMsgAmsGetActionInfoRes *action_info)
{
	ams_gu->guid = *alarm_guid;
	JpfPolicy *policy;
	JpfActions *actions;

	policy = jpf_ams_new_policy();
	if (!policy)
	{
		jpf_warning("<JpfAmsPolicy>no memory!");
		return -1;
	}

	jpf_ams_policy_parse(policy, &action_info->action_policy);

#ifdef AMS_POLICY_TEST
	ams_src_policy_print(&action_info->action_policy);
	ams_alarm_policy_print(policy);
#endif

	actions = jpf_get_actions(action_info);
	if (!actions)
	{
		jpf_ams_delete_policy(policy);
		return -1;
	}

	ams_gu->time_policy = policy;
	ams_gu->act = actions;

	return 0;
}


gint jpf_ams_add_gu_action_info(JpfShareGuid *alarm_guid,
	JpfMsgAmsGetActionInfoRes *action_info)
{
	g_assert(alarm_guid && action_info);
	JpfAmsGu *ams_gu;
	gint ret;

	ams_gu = (JpfAmsGu *)g_malloc0(sizeof(JpfAmsGu));
	if (!ams_gu)
	{
		jpf_warning("<JpfAmsPolicy>no memory!");
		return -1;
	}

	ret = jpf_ams_get_gu_info(ams_gu, alarm_guid, action_info);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy>jpf_ams_get_gu_info failed!");
		jpf_ams_gu_free(ams_gu);
		return -1;
	}

	ams_gu->time = CUR_TIME;
	jpf_gu_pool_add_gu(&g_gu_pool, ams_gu);

	return ret;
}


gint jpf_ams_remove_alarm_node(JpfShareGuid *key)
{
	g_assert(key);
	gint ret;

	g_mutex_lock(g_gu_pool.mutex);
	ret = g_hash_table_remove(g_gu_pool.gu, key) ? 0 : -1;
	g_mutex_unlock(g_gu_pool.mutex);

	return ret;
}


static int jpf_action_record_init(JpfAction *p)
{
	g_assert(p);
	JpfActionRecord *ar = (JpfActionRecord *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int jpf_action_record_free(JpfAction *p)
{
	g_assert(p);
	//JpfActionRecord *ar = (JpfActionRecord *)p;

	return 0;
}


static JpfActionP jpf_action_record_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_record_get_data begin------");
	g_assert(info);
	JpfMsgActionRecord *ar_src;
	JpfActionRecord *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_RECORD])
		return NULL;

	ar_src = (JpfMsgActionRecord *)info->actions[AMS_ACTION_RECORD];
	count = ar_src->action_gu_count;
	size = sizeof(JpfActionRecord) + sizeof(JpfActionRecordGu) * count;

	ar = (JpfActionRecord *)jpf_new_action_type(size, &g_action_record_ops,
		AMS_ACTION_RECORD);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (JpfAction *)ar;
}


static int jpf_action_record_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_record_do begin------");
	g_assert(p && alarm_info);
	JpfActionRecord *ar = (JpfActionRecord *)p;
	JpfAmsActionRecord action_info;
	gint ret = 0;
	gint i, j;

	ret = jpf_check_alarm_action_enabled(ACTION_RECORD_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action record not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionRecordGu *act_gu = &ar->action_gu[i];
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
			jpf_warning("<JpfAmsPolicy> act_gu->mss_count = 0.");
		for (j = 0; j < act_gu->mss_count; j++)
		{
			action_info.mss_id = act_gu->mss_id[j];
			if (jpf_ams_action_handle(BUSSLOT_POS_MSS, MESSAGE_ALARM_LINK_RECORD,
				&action_info, sizeof(action_info)) != 0)
			{
				ret = -1;
				jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
				break;
			}
		}
	}

	return ret;
}


static JpfActionOps g_action_record_ops =
{
	.init = jpf_action_record_init,
	.free = jpf_action_record_free,
	.get_data = jpf_action_record_get_data,
	.action = jpf_action_record_do
};


static int jpf_action_io_init(JpfAction *p)
{
	g_assert(p);
	JpfActionIO *ar = (JpfActionIO *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int jpf_action_io_free(JpfAction *p)
{
	g_assert(p);

	return 0;
}


static JpfActionP jpf_action_io_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_io_get_data begin------");
	g_assert(info);
	JpfMsgActionIO *ar_src;
	JpfActionIO *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_IO])
		return NULL;

	ar_src = (JpfMsgActionIO *)info->actions[AMS_ACTION_IO];
	count = ar_src->action_gu_count;
	size = sizeof(JpfActionIO) + sizeof(JpfActionIOGu) * count;

	ar = (JpfActionIO *)jpf_new_action_type(size, &g_action_io_ops,
		AMS_ACTION_IO);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (JpfAction *)ar;
}


static int jpf_action_io_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_io_do begin------");
	g_assert(p && alarm_info);
	JpfActionIO *ar = (JpfActionIO *)p;
	JpfAmsActionIO action_info;
	gint ret = 0;
	gint i;

	ret = jpf_check_alarm_action_enabled(ACTION_IO_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action io not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionIOGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.time_len = act_gu->time_len;
		strncpy(action_info.io_value, act_gu->io_value, IO_VALUE_LEN - 1);
		strncpy(action_info.session, jpf_get_local_domain_id(), SESSION_ID_LEN - 1);

		if (jpf_ams_action_handle(BUSSLOT_POS_PU, MESSAGE_ALARM_LINK_IO,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static JpfActionOps g_action_io_ops =
{
	.init = jpf_action_io_init,
	.free = jpf_action_io_free,
	.get_data = jpf_action_io_get_data,
	.action = jpf_action_io_do
};


static int jpf_action_step_init(JpfAction *p)
{
	g_assert(p);
	JpfActionStep *ar = (JpfActionStep *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int jpf_action_step_free(JpfAction *p)
{
	g_assert(p);

	return 0;
}


static JpfActionP jpf_action_step_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_step_get_data begin------");
	g_assert(info);
	JpfMsgActionStep *ar_src;
	JpfActionStep *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_STEP])
		return NULL;

	ar_src = (JpfMsgActionStep *)info->actions[AMS_ACTION_STEP];
	count = ar_src->action_gu_count;
	size = sizeof(JpfActionStep) + sizeof(JpfActionStepGu) * count;

	ar = (JpfActionStep *)jpf_new_action_type(size, &g_action_step_ops,
		AMS_ACTION_STEP);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
		ar->action_gu[i].ec_guid[LEVEL_POS] = (gchar)(ar->action_gu[i].level % 10 + '0');	//设置码流
	}

	return (JpfAction *)ar;
}


static int jpf_action_step_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_step_do begin------");
	g_assert(p && alarm_info);
	JpfActionStep *ar = (JpfActionStep *)p;
	tw_run_step_request action_info;
	gint ret = 0;
	gint i;

	ret = jpf_check_alarm_action_enabled(ACTION_TW_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action tw(step) not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionStepGu *act_gu = &ar->action_gu[i];
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

		if (jpf_ams_action_handle(BUSSLOT_POS_TW, MSG_LINK_RUN_STEP,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static JpfActionOps g_action_step_ops =
{
	.init = jpf_action_step_init,
	.free = jpf_action_step_free,
	.get_data = jpf_action_step_get_data,
	.action = jpf_action_step_do
};


static int jpf_action_preset_init(JpfAction *p)
{
	g_assert(p);
	JpfActionPreset *ar = (JpfActionPreset *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int jpf_action_preset_free(JpfAction *p)
{
	g_assert(p);

	return 0;
}


static JpfActionP jpf_action_preset_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_preset_get_data begin------");
	g_assert(info);
	JpfMsgActionPreset *ar_src;
	JpfActionPreset *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_PRESET])
		return NULL;

	ar_src = (JpfMsgActionPreset *)info->actions[AMS_ACTION_PRESET];
	count = ar_src->action_gu_count;
	size = sizeof(JpfActionPreset) + sizeof(JpfActionPresetGu) * count;

	ar = (JpfActionPreset *)jpf_new_action_type(size, &g_action_preset_ops,
		AMS_ACTION_PRESET);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (JpfAction *)ar;
}

#if 0
static void jpf_action_preset_print(JpfAmsActionPreset *action_info)
{
	jpf_ams_print("_____________________jpf_action_preset_print begin...");
	jpf_ams_print("action_info->action_guid.guid:%s", action_info->action_guid.guid);
	jpf_ams_print("action_info->action_guid.domain_id:%s", action_info->action_guid.domain_id);
	jpf_ams_print("action_info->preset_num = %d", action_info->preset_num);
	jpf_ams_print("action_info->session:%s", action_info->session);
	jpf_ams_print("______end...");
}
#endif

static int jpf_action_preset_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_preset_do begin------");
	g_assert(p && alarm_info);
	JpfActionPreset *ar = (JpfActionPreset *)p;
	JpfAmsActionPreset action_info;
	gint ret = 0;
	gint i;

	ret = jpf_check_alarm_action_enabled(ACTION_PRESET_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action preset not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionPresetGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			continue;
		}

		memset(&action_info, 0, sizeof(action_info));
		action_info.action_guid = act_gu->action_guid;
		action_info.preset_num = act_gu->preset_num;
		strncpy(action_info.session, jpf_get_local_domain_id(), SESSION_ID_LEN - 1);
		//jpf_action_preset_print(&action_info);

		if (jpf_ams_action_handle(BUSSLOT_POS_PU, MESSAGE_ALARM_LINK_PRESET,
			&action_info, sizeof(action_info)) != 0)
		{
			ret = -1;
			jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
		}
	}

	return ret;
}


static JpfActionOps g_action_preset_ops =
{
	.init = jpf_action_preset_init,
	.free = jpf_action_preset_free,
	.get_data = jpf_action_preset_get_data,
	.action = jpf_action_preset_do
};


static int jpf_action_snapshot_init(JpfAction *p)
{
	g_assert(p);
	JpfActionSnapshot *ar = (JpfActionSnapshot *)p;

	ar->action_gu_count = 0;

	return 0;
}


static int jpf_action_snapshot_free(JpfAction *p)
{
	g_assert(p);

	return 0;
}


static JpfActionP jpf_action_snapshot_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_snapshot_get_data begin------");
	g_assert(info);
	JpfMsgActionSnapshot *ar_src;
	JpfActionSnapshot *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_SNAPSHOT])
		return NULL;

	ar_src = (JpfMsgActionSnapshot *)info->actions[AMS_ACTION_SNAPSHOT];
	count = ar_src->action_gu_count;
	size = sizeof(JpfActionSnapshot) + sizeof(JpfActionSnapshotGu) * count;

	ar = (JpfActionSnapshot *)jpf_new_action_type(size, &g_action_snapshot_ops,
		AMS_ACTION_SNAPSHOT);
	if (!ar)
		return NULL;

	ar->action_gu_count = ar_src->action_gu_count;
	for (i = 0; i < ar_src->action_gu_count; i++)
	{
		ar->action_gu[i] = ar_src->action_gu[i];
	}

	return (JpfAction *)ar;
}

#if 0
static void jpf_action_snapshop_print(JpfAmsActionSnapshot *action_info)
{
	jpf_ams_print("_____________________jpf_action_snapshop_print begin...");
	jpf_ams_print("action_info->action_guid.guid:%s", action_info->action_guid.guid);
	jpf_ams_print("action_info->action_guid.domain_id:%s", action_info->action_guid.domain_id);
	jpf_ams_print("action_info->level:%d", action_info->level);
	jpf_ams_print("action_info->mss_id.mss_id:%s", action_info->mss_id.mss_id);
	jpf_ams_print("action_info->picture_count:%u", action_info->picture_count);
	jpf_ams_print("action_info->alarm_type:%u", action_info->alarm_type);
	jpf_ams_print("______end...");
}
#endif

static int jpf_action_snapshot_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_snapshot_do begin------");
	g_assert(p && alarm_info);
	JpfActionSnapshot *ar = (JpfActionSnapshot *)p;
	JpfAmsActionSnapshot action_info;
	gint ret = 0;
	gint i, j;

	ret = jpf_check_alarm_action_enabled(ACTION_CAP_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action snapshot not enabled, ret = %d.", ret);
		return 0;
	}

	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionSnapshotGu *act_gu = &ar->action_gu[i];
		if (!(act_gu->alarm_type & alarm_info->alarm_type))
		{
			jpf_ams_print("<JpfAmsPolicy> action_snapshot alarm_type(s):%d, " \
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
			jpf_warning("<JpfAmsPolicy> act_gu->mss_count = 0.");
		for (j = 0; j < act_gu->mss_count; j++)
		{
			action_info.mss_id = act_gu->mss_id[j];
			//jpf_action_snapshop_print(&action_info);

			if (jpf_ams_action_handle(BUSSLOT_POS_MSS, MESSAGE_ALARM_LINK_SNAPSHOT,
				&action_info, sizeof(action_info)) != 0)
			{
				ret = -1;
				jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
				break;
			}
		}
	}

	return ret;
}


static JpfActionOps g_action_snapshot_ops =
{
	.init = jpf_action_snapshot_init,
	.free = jpf_action_snapshot_free,
	.get_data = jpf_action_snapshot_get_data,
	.action = jpf_action_snapshot_do
};


static int jpf_action_map_init(JpfAction *p)
{
	g_assert(p);
	JpfActionMap *ar = (JpfActionMap *)p;

	ar->action_gu_count = 0;
	ar->cu_count= 0;

	return 0;
}


static int jpf_action_map_free(JpfAction *p)
{
	g_assert(p);

	return 0;
}


static JpfActionP jpf_action_map_get_data(JpfMsgAmsGetActionInfoRes *info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_map_get_data begin------");
	g_assert(info);
	JpfMsgActionMap *ar_src;
	JpfActionMap *ar;
	gint count, size;
	gint i;

	if (!info->actions[AMS_ACTION_MAP])
		return NULL;

	ar_src = (JpfMsgActionMap *)info->actions[AMS_ACTION_MAP];
	count = ar_src->cu_count;
	size = sizeof(JpfActionMap) + sizeof(JpfAllCuOwnPu) * count;

	ar = (JpfActionMap *)jpf_new_action_type(size, &g_action_map_ops,
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
	return (JpfAction *)ar;
}


static int jpf_action_map_do(JpfAction *p, JpfAlarmInfo *alarm_info)
{
	jpf_ams_print("<JpfAmsPolicy> jpf_action_map_do begin------");
	g_assert(p && alarm_info);
	JpfActionMap *ar = (JpfActionMap *)p;
	JpfAmsActionMap *action_info;
	gint ret = 0;
	gint i, size, gu_count = 0;

	ret = jpf_check_alarm_action_enabled(ACTION_EMAP_BIT);
	if (ret != 0)
	{
		jpf_warning("<JpfAmsPolicy> action io not enabled, ret = %d.", ret);
		return 0;
	}

	size = sizeof(JpfAmsActionMap) + sizeof(JpfAllCuOwnPu)*ar->cu_count;
	action_info = jpf_mem_kalloc(size);
	memset(action_info, 0, size);
	for (i = 0; i < ar->action_gu_count; i++)
	{
		JpfActionMapGu *act_gu = &ar->action_gu[i];
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
		memcpy(action_info->cu_list, ar->cu_list, ar->cu_count*sizeof(JpfAllCuOwnPu));
		action_info->action_gu_count = gu_count;
		if (jpf_ams_action_handle(BUSSLOT_POS_CU, MESSAGE_ALARM_LINK_MAP,
			action_info, size) != 0)
		{
			ret = -1;
			jpf_warning("<JpfAmsPolicy>send msg failed, may be no memory!");
		}
	}

	jpf_mem_kfree(action_info, size);

	return ret;
}


static JpfActionOps g_action_map_ops =
{
	.init = jpf_action_map_init,
	.free = jpf_action_map_free,
	.get_data = jpf_action_map_get_data,
	.action = jpf_action_map_do
};




static gboolean
jpf_ams_if_old_node(gpointer key, gpointer value, gpointer user_data)
{
	g_assert(key && value);

	JpfAmsGu *gu = (JpfAmsGu *)value;
	GTime cur_time = CUR_TIME;

	if (cur_time >= gu->time && cur_time - gu->time <= AMS_NEW_NODE_TIME)
	{
		return FALSE;
	}

	return TRUE;
}


static gboolean
jpf_ams_pool_timer(gpointer user_data)
{
	JpfGuPool *p = (JpfGuPool *)user_data;
	gint n_del;

	g_mutex_lock(p->mutex);

	n_del = g_hash_table_foreach_remove(p->gu, jpf_ams_if_old_node, NULL);

	g_mutex_unlock(p->mutex);
	if (n_del != 0)
		jpf_print("********************* n_del = %d.", n_del);
	return TRUE;
}


void jpf_ams_init_gu_pool()
{
	memset(&g_gu_pool, 0, sizeof(JpfGuPool));

	g_gu_pool.mutex = g_mutex_new();

	g_gu_pool.gu = g_hash_table_new_full(jpf_gu_hash_fn, jpf_gu_key_equal,
		NULL, jpf_ams_gu_free);

	g_gu_pool.timer = jpf_set_timer(AMS_POLICY_CLEAR_TIME,
		jpf_ams_pool_timer, &g_gu_pool);
}


void jpf_ams_init_actions()
{
	jpf_register_action_type(sizeof(JpfActionRecord), &g_action_record_ops);
	jpf_register_action_type(sizeof(JpfActionIO), &g_action_io_ops);
	jpf_register_action_type(sizeof(JpfActionStep), &g_action_step_ops);
	jpf_register_action_type(sizeof(JpfActionPreset), &g_action_preset_ops);
	jpf_register_action_type(sizeof(JpfActionSnapshot), &g_action_snapshot_ops);
	jpf_register_action_type(sizeof(JpfActionMap), &g_action_map_ops);
}


void jpf_ams_policy_init()
{
	jpf_ams_init_gu_pool();
	jpf_ams_init_actions();
}

