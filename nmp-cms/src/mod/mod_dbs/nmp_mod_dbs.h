#ifndef __NMP_MOD_DBS_H__
#define __NMP_MOD_DBS_H__

#include "nmp_mods.h"
#include "nmp_appmod.h"
#include "db_connect_pool.h"


//E_USRINFO 287

#define NMP_DBS_CHECK_MYSQL_RESULT(mysql_result, ret, row_num, end) do {	\
	BUG_ON(!mysql_result);	\
	if (G_UNLIKELY(MYSQL_RESULT_CODE(mysql_result))) {	\
		ret = MYSQL_RESULT_CODE(mysql_result);	\
		goto end;	\
	}	\
	row_num = jpf_sql_get_num_rows(mysql_result);	\
	if (G_UNLIKELY(row_num == 0)) {	\
		ret = E_USRINFO;	\
		goto end;	\
	}	\
} while (0)



G_BEGIN_DECLS

#define MYSQL_HOST			"127.0.0.1"
#define DB_NAME				"jxj_platform_db"
#define DB_ADMIN_NAME		"root"
#define DB_ADMIN_PASSWORD	""

#define PU_TABLE			"dev_configure_table"
#define PU_RUNNING_TABLE	"dev_running_info_table"
#define ADMIN_TABLE			"super_user_table"
#define DOMAIN_TABLE		"domain_table"
#define AREA_TABLE			"domain_area_table"
#define GU_TABLE			"dev_guconf_table"
#define USER_GROUP_TABLE	"user_group_table"
#define USER_TABLE			"user_manage_table"
#define MANUFACTURER_TABLE	"dev_manufacturer_table"
#define MDS_TABLE			"dispatch_unit_table"
#define MDS_IP_TABLE		"dispatch_unit_ip_table"
#define MSS_TABLE			"store_unit_table"
#define RECORD_POLICY_TABLE		"record_policy_table"
#define HD_GROUP_TABLE			"hd_group_table"
#define USER_OWN_GU_TABLE		"user_guconf_table"
#define ALARM_INFO_TABLE		"alarm_info_table"
#define MSS_UPDATE_TABLE		"mss_update_table"
#define DEFENCE_AREA_TABLE		"defence_area_table"
#define MAP_TABLE				"map_table"
#define DEFENCE_MAP_TABLE		"defence_map_table"
#define MAP_GU_TABLE			"map_gu_table"
#define MAP_HREF_TABLE			"map_href_table"
#define PARAM_CONFIG_TABLE		"param_config_table"
#define TW_TABLE				"tw_table"
#define SCREEN_TABLE			"tw_screens_table"
#define SCREEN_DIVISION_TABLE	"tw_scr_division_table"
#define TOUR_TABLE				"tour_table"
#define TOUR_STEP_TABLE			"tour_step_table"
#define GROUP_TABLE				"group_table"
#define GROUP_STEP_TABLE		"group_step_table"
#define GROUP_STEP_INFO_TABLE	"group_step_info_table"
#define AMS_TABLE				"alarm_unit_table"
#define AMS_CONFIGURE_TABLE			"alarm_dev_configure_table"
#define LINK_TIME_POLICY_TABLE		"link_time_policy_table"
#define ALARM_LINK_RECORD_TABLE		"link_record_table"
#define ALARM_LINK_IO_TABLE			"link_io_table"
#define ALARM_LINK_SNAPSHOT_TABLE	"link_snapshot_table"
#define ALARM_LINK_PRESET_TABLE		"link_preset_table"
#define ALARM_LINK_STEP_TABLE		"link_step_table"
#define ALARM_LINK_TOUR_TABLE		"link_tour_table"
#define ALARM_LINK_GROUP_TABLE		"link_group_table"
#define ALARM_LINK_MAP_TABLE		"link_map_table"
#define USER_OWN_TW_TABLE			"user_twconf_table"
#define USER_OWN_TOUR_TABLE			"user_tourconf_table"
#define AREA_DEV_ONLINE_STATUS_TABLE	"area_dev_online_status_table"
#define IVS_TABLE						"intelligence_analyse_table"
#define USER_GU_NUMBER_TABLE		"user_guconf_number_table"
#define USER_SCREEN_NUMBER_TABLE	"user_screen_number_table"
#define USER_TOUR_NUMBER_TABLE		"user_tour_number_table"
#define USER_GROUP_NUMBER_TABLE		"user_group_number_table"


#define DB_MAX_CONN_NUM         100
#define DB_MIN_CONN_NUM         5

#define DEL_ALARM_FLAG           (1 << 0)
#define ENABLE_DEL_ALARM        (1 << 1)


#define NMP_TYPE_MODDBS	(nmp_mod_dbs_get_type())
#define NMP_IS_MODDBS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODDBS))
#define NMP_IS_MODDBS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODDBS))
#define NMP_MODDBS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODDBS, JpfModDbs))
#define JpfModDBSClass(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODDBS, JpfModDbsClass))
#define NMP_MODDBS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODDBS, JpfModDbsClass))

typedef struct _JpfModDbs JpfModDbs;
typedef struct _JpfModDbsClass JpfModDbsClass;
struct _JpfModDbs
{
	NmpAppMod	  parent_object;
	db_conn_pool_conf *pool_conf;
	db_conn_pool_info *pool_info;
	gint  del_alarm_flag;
	gint wdd_status;  //0:加密狗正常，1:加密狗未接或者接触不良
	gint authorization_expired;  //授权到期标志
	gint res_over_flag;  //业务点数超过授权数，按位表示,bit0:av,bit1:ds,bit2:ai,bit3:ao
	gint time_status;  //时间状态，0:正常，1:系统时间错误
};

struct _JpfModDbsClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_dbs_get_type( void );

void
nmp_mod_dbs_register_bss_msg_handler(JpfModDbs *self);

void
nmp_mod_dbs_register_cu_msg_handler(JpfModDbs *self);

void
jpf_mods_dbs_broadcast_msg(JpfModDbs * self, gpointer priv, gsize size);
G_END_DECLS


#endif	//__NMP_MOD_BSS_H__

