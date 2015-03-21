#ifndef __NMP_DBSFUN_H__
#define __NMP_DBSFUN_H__
#include "nmp_internal_msg.h"
#include "nmp_mysql_fun.h"
#include "nmp_msg_share.h"
#include "nmp_tw_interface.h"

#define ADMIN_TABLE "super_user_table"
#define AV_TYPE "AV"
#define DS_TYPE "DS"
#define AI_TYPE "AI"
#define AO_TYPE "AO"


typedef enum
{
	AV_OVER_FLAG	= 1 << 0,
	DS_OVER_FLAG	= 1 << 1,
	AI_OVER_FLAG	= 1 << 2,
	AO_OVER_FLAG 	= 1 << 3
}NmpDevGuOverFlag;

void
nmp_dbs_modify_sysmsg(NmpSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 NmpBusSlotPos src_pos,
					 NmpBusSlotPos dst_pos,
					 NmpMsgPrivDes msg_priv_destroy
				     );

void
nmp_dbs_modify_sysmsg_2(NmpSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 NmpBusSlotPos src_pos,
					 NmpBusSlotPos dst_pos
				     );

gint nmp_get_mds_ip(NmpMysqlRes *mysql_result, NmpMsgGetMdsIpRes *res_info);

gint nmp_dbs_get_mds_info(NmpAppObj *app_obj, gchar *cms_ip,
    gchar *puid, gchar *domain_id, NmpMsgGetMdsIpRes *mds_ip);

gint
nmp_get_scr_display_guid(NmpAppObj *app_obj,
                        tw_general_guid *guid, gint screen_id, gint tw_id);

NmpGetDivModeRes *
nmp_dbs_get_div_mode(NmpMysqlRes *mysql_res, gint *size);

gint
nmp_get_pu_cms_ip(NmpAppObj *app_obj, gchar *puid,
	char *domain_id, gchar *cms_ip);

gint
nmp_dbs_check_gu_type_count(NmpAppObj *app_obj,
	gint add_count, gint total_num, gchar *gu_type);

gint
nmp_dbs_get_dev_type_count(NmpAppObj *app_obj, gchar *dev_type);

gint
nmp_dbs_get_online_dev_type_count(NmpAppObj *app_obj, gchar *dev_type);

gint
nmp_dbs_get_dev_total_count(NmpAppObj *app_obj);

gint
nmp_dbs_get_online_dev_total_count(NmpAppObj *app_obj);

gint
nmp_dbs_get_gu_type_count(NmpAppObj *app_obj,  gchar *gu_type);

void
nmp_mod_dbs_deliver_out_msg(NmpAppObj *self, NmpSysMsg *msg);

#endif
