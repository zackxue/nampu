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



typedef struct _NmpTime NmpTime;
struct _NmpTime
{
	gint		day_of_week;
	time_t		cur_time;
};

typedef struct _NmpAlarmInfo NmpAlarmInfo;
struct _NmpAlarmInfo
{
	NmpShareGuid	alarm_guid;
	NmpTime			time;
	guint			alarm_type;		//1,2,4,8,...
	gchar	      alarm_time[TIME_INFO_LEN];
};


typedef struct _NmpTimeSegment NmpTimeSegment;
struct _NmpTimeSegment
{
	gint			day_of_week;
	time_t		start;
	time_t		end;
};

typedef struct _NmpPolicy NmpPolicy;
struct _NmpPolicy
{
	GList		*time_segs;
	GMutex		*mutex;
};


typedef struct _NmpAction NmpAction;
typedef NmpAction *NmpActionP;
typedef struct _NmpActionOps NmpActionOps;

struct _NmpActionOps
{
	gint		(*init)(NmpAction *p);
	gint		(*free)(NmpAction *p);
	NmpActionP	(*get_data)(NmpMsgAmsGetActionInfoRes *info);
	gint		(*action)(NmpAction *p, NmpAlarmInfo *alarm_info);
};

struct _NmpAction
{
	NmpActionOps	*ops;
	NmpAmsActionType	type;
};


typedef struct _NmpActions NmpActions;
struct _NmpActions
{
	NmpAction	*actions[AMS_ACTION_MAX];
};

typedef struct _NmpAmsGu NmpAmsGu;
struct _NmpAmsGu
{
	NmpShareGuid	guid;
	GTime		time;		//节点加入时间
	NmpPolicy		*time_policy;
	NmpActions	*act;
};

typedef struct _NmpGuPool NmpGuPool;
struct _NmpGuPool
{
	GHashTable	*gu;
	GMutex		*mutex;
	guint		timer;
};


/********************** action_record **********************/

typedef NmpMsgActionRecordGu NmpActionRecordGu;

typedef struct _NmpActionRecord NmpActionRecord;
struct _NmpActionRecord
{
	NmpAction		super;

	guint		action_gu_count;
	NmpActionRecordGu	action_gu[0];
};

/********************** action_io **********************/

typedef NmpMsgActionIOGu NmpActionIOGu;

typedef struct _NmpActionIO NmpActionIO;
struct _NmpActionIO
{
	NmpAction		super;

	guint		action_gu_count;
	NmpActionIOGu	action_gu[0];
};

/********************** action_step **********************/

typedef NmpMsgActionStepGu NmpActionStepGu;

typedef struct _NmpActionStep NmpActionStep;
struct _NmpActionStep
{
	NmpAction		super;

	guint		action_gu_count;
	NmpActionStepGu	action_gu[0];
};

/********************** action_preset **********************/

typedef NmpMsgActionPresetGu NmpActionPresetGu;

typedef struct _NmpActionPreset NmpActionPreset;
struct _NmpActionPreset
{
	NmpAction		super;

	guint		action_gu_count;
	NmpActionPresetGu	action_gu[0];
};

/********************** action_snapshot **********************/

typedef NmpMsgActionSnapshotGu NmpActionSnapshotGu;

typedef struct _NmpActionSnapshot NmpActionSnapshot;
struct _NmpActionSnapshot
{
	NmpAction		super;

	guint		action_gu_count;
	NmpActionSnapshotGu	action_gu[0];
};

/********************** action_map **********************/

typedef NmpMsgActionMapGu NmpActionMapGu;

typedef struct _NmpActionMap NmpActionMap;
struct _NmpActionMap
{
	NmpAction		super;

	NmpShareGuid	action_guid;
	gchar			gu_name[GU_NAME_LEN];
	gint			defence_id;
	gchar			defence_name[AREA_NAME_LEN];
	gint			map_id;
	gchar			map_name[MAP_NAME_LEN];
	guint			action_gu_count;
	NmpActionMapGu	action_gu[AMS_MAX_LINK_MAP_GU];
	gint				cu_count;
	NmpAllCuOwnPu		cu_list[0];
};


typedef gint (*nmp_ams_action_handler)(gint dst, gint msg_type, void *parm, guint size);

void nmp_ams_set_action_handler(nmp_ams_action_handler hook);

void nmp_ams_policy_init();

gint nmp_ams_find_gu_and_action(NmpAlarmInfo *alarm_info);

gint nmp_ams_add_gu_action_info(NmpShareGuid *alarm_guid,
	NmpMsgAmsGetActionInfoRes *action_info);

gint nmp_ams_remove_alarm_node(NmpShareGuid *key);

#endif	/* __NMP_AMS_POLICY_H__ */
