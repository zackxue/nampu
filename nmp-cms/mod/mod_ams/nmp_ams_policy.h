/*
 *	@author:	zyt
 *	@time:	2013/1/7
 */
#ifndef __NMP_AMS_POLICY_H__
#define __NMP_AMS_POLICY_H__

#include <glib.h>
#include "nmp_msg_share.h"
#include "nmp_share_struct.h"
#include "nmp_internal_msg.h"



typedef struct _JpfTime JpfTime;
struct _JpfTime
{
	gint		day_of_week;
	time_t		cur_time;
};

typedef struct _JpfAlarmInfo JpfAlarmInfo;
struct _JpfAlarmInfo
{
	JpfShareGuid	alarm_guid;
	JpfTime			time;
	guint			alarm_type;		//1,2,4,8,...
	gchar	      alarm_time[TIME_INFO_LEN];
};


typedef struct _JpfTimeSegment JpfTimeSegment;
struct _JpfTimeSegment
{
	gint			day_of_week;
	time_t		start;
	time_t		end;
};

typedef struct _JpfPolicy JpfPolicy;
struct _JpfPolicy
{
	GList		*time_segs;
	GMutex		*mutex;
};


typedef struct _JpfAction JpfAction;
typedef JpfAction *JpfActionP;
typedef struct _JpfActionOps JpfActionOps;

struct _JpfActionOps
{
	gint		(*init)(JpfAction *p);
	gint		(*free)(JpfAction *p);
	JpfActionP	(*get_data)(JpfMsgAmsGetActionInfoRes *info);
	gint		(*action)(JpfAction *p, JpfAlarmInfo *alarm_info);
};

struct _JpfAction
{
	JpfActionOps	*ops;
	JpfAmsActionType	type;
};


typedef struct _JpfActions JpfActions;
struct _JpfActions
{
	JpfAction	*actions[AMS_ACTION_MAX];
};

typedef struct _JpfAmsGu JpfAmsGu;
struct _JpfAmsGu
{
	JpfShareGuid	guid;
	GTime		time;		//节点加入时间
	JpfPolicy		*time_policy;
	JpfActions	*act;
};

typedef struct _JpfGuPool JpfGuPool;
struct _JpfGuPool
{
	GHashTable	*gu;
	GMutex		*mutex;
	guint		timer;
};


/********************** action_record **********************/

typedef JpfMsgActionRecordGu JpfActionRecordGu;

typedef struct _JpfActionRecord JpfActionRecord;
struct _JpfActionRecord
{
	JpfAction		super;

	guint		action_gu_count;
	JpfActionRecordGu	action_gu[0];
};

/********************** action_io **********************/

typedef JpfMsgActionIOGu JpfActionIOGu;

typedef struct _JpfActionIO JpfActionIO;
struct _JpfActionIO
{
	JpfAction		super;

	guint		action_gu_count;
	JpfActionIOGu	action_gu[0];
};

/********************** action_step **********************/

typedef JpfMsgActionStepGu JpfActionStepGu;

typedef struct _JpfActionStep JpfActionStep;
struct _JpfActionStep
{
	JpfAction		super;

	guint		action_gu_count;
	JpfActionStepGu	action_gu[0];
};

/********************** action_preset **********************/

typedef JpfMsgActionPresetGu JpfActionPresetGu;

typedef struct _JpfActionPreset JpfActionPreset;
struct _JpfActionPreset
{
	JpfAction		super;

	guint		action_gu_count;
	JpfActionPresetGu	action_gu[0];
};

/********************** action_snapshot **********************/

typedef JpfMsgActionSnapshotGu JpfActionSnapshotGu;

typedef struct _JpfActionSnapshot JpfActionSnapshot;
struct _JpfActionSnapshot
{
	JpfAction		super;

	guint		action_gu_count;
	JpfActionSnapshotGu	action_gu[0];
};

/********************** action_map **********************/

typedef JpfMsgActionMapGu JpfActionMapGu;

typedef struct _JpfActionMap JpfActionMap;
struct _JpfActionMap
{
	JpfAction		super;

	JpfShareGuid	action_guid;
	gchar			gu_name[GU_NAME_LEN];
	gint			defence_id;
	gchar			defence_name[AREA_NAME_LEN];
	gint			map_id;
	gchar			map_name[MAP_NAME_LEN];
	guint			action_gu_count;
	JpfActionMapGu	action_gu[AMS_MAX_LINK_MAP_GU];
	gint				cu_count;
	JpfAllCuOwnPu		cu_list[0];
};


typedef gint (*jpf_ams_action_handler)(gint dst, gint msg_type, void *parm, guint size);

void jpf_ams_set_action_handler(jpf_ams_action_handler hook);

void jpf_ams_policy_init();

gint jpf_ams_find_gu_and_action(JpfAlarmInfo *alarm_info);

gint jpf_ams_add_gu_action_info(JpfShareGuid *alarm_guid,
	JpfMsgAmsGetActionInfoRes *action_info);

gint jpf_ams_remove_alarm_node(JpfShareGuid *key);

#endif	/* __NMP_AMS_POLICY_H__ */
