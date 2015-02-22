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
}JpfDevGuOverFlag;

void
jpf_dbs_modify_sysmsg(JpfSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 JpfBusSlotPos src_pos,
					 JpfBusSlotPos dst_pos,
					 JpfMsgPrivDes msg_priv_destroy
				     );

void
jpf_dbs_modify_sysmsg_2(JpfSysMsg *sys_msg,
					 gpointer   priv_data,
					 gsize size,
					 JpfBusSlotPos src_pos,
					 JpfBusSlotPos dst_pos
				     );

gint jpf_get_mds_ip(JpfMysqlRes *mysql_result, JpfMsgGetMdsIpRes *res_info);

gint jpf_dbs_get_mds_info(JpfAppObj *app_obj, gchar *cms_ip,
    gchar *puid, gchar *domain_id, JpfMsgGetMdsIpRes *mds_ip);

gint
jpf_get_scr_display_guid(JpfAppObj *app_obj,
                        tw_general_guid *guid, gint screen_id, gint tw_id);

JpfGetDivModeRes *
jpf_dbs_get_div_mode(JpfMysqlRes *mysql_res, gint *size);

gint
jpf_get_pu_cms_ip(JpfAppObj *app_obj, gchar *puid,
	char *domain_id, gchar *cms_ip);

gint
jpf_dbs_check_gu_type_count(JpfAppObj *app_obj,
	gint add_count, gint total_num, gchar *gu_type);

gint
jpf_dbs_get_dev_type_count(JpfAppObj *app_obj, gchar *dev_type);

gint
jpf_dbs_get_online_dev_type_count(JpfAppObj *app_obj, gchar *dev_type);

gint
jpf_dbs_get_dev_total_count(JpfAppObj *app_obj);

gint
jpf_dbs_get_online_dev_total_count(JpfAppObj *app_obj);

gint
jpf_dbs_get_gu_type_count(JpfAppObj *app_obj,  gchar *gu_type);

void
jpf_mod_dbs_deliver_out_msg(JpfAppObj *self, JpfSysMsg *msg);

#endif
