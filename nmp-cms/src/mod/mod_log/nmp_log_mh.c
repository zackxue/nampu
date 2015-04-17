#include "nmp_mod_log.h"
#include "nmp_message.h"
#include "nmp_share_errno.h"
#include "nmp_memory.h"
#include "nmp_internal_msg.h"
#include "nmp_share_debug.h"
#include "nmp_log_defs.h"
#include "nmp_msg_bss.h"
#include "nmp_mysql_defs.h"
#include "nmp_shared.h"
#include "nmp_log_mysql_fun.h"


#define NMP_DEAL_LOG_RESULT(ret) do {	\
	if (ret) {	\
		nmp_warning("<NmpLogMh> nmp_log failed, ret = %d.", ret);	\
	}	\
} while (0)

#define NMP_CHECK_MSS_MSG_RES(msg) do { \
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_BSS) { \
		nmp_sysmsg_destroy(msg); \
		return MFR_ACCEPTED; \
	} \
} while (0)

#define LOG_INVERTED_ORDER		1
#define LOG_TMP_STR_LEN		32
#define NMP_LOG_TIME_LEN		32


static void nmp_get_time_str(time_t time_uint, gchar *time_str)
{
	struct tm t_tm;
	localtime_r(&time_uint, &t_tm);

	snprintf(time_str, NMP_LOG_TIME_LEN, "%d-%d-%d %d:%d:%d",
		t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday,
		t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
}

#if 0
static time_t nmp_get_time_uint(gint year, gint mon, gint day,
	gint hour, gint min, gint sec)
{
	time_t time_uint;
	struct tm t_tm;

	t_tm.tm_year = year - 1900;
	t_tm.tm_mon = mon - 1;
	t_tm.tm_mday = day;
	t_tm.tm_hour = hour;
	t_tm.tm_min = min;
	t_tm.tm_sec = sec;

	time_uint = mktime(&t_tm);
	return time_uint;
}
#endif

static __inline__ void
nmp_mod_log_del_redundant_log(NmpAppObj *self)
{
	char query_buf[QUERY_STR_LEN] = {0};
	NmpModLog *mod_log;
	NmpMsgErrCode result;
	glong affect_num = 0;
	gint total_num;

	mod_log = (NmpModLog *)self;
	if (!mod_log->del_log_flag)
		return;
	mod_log->del_log_flag = 0;

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s", LOG_TABLE
	);

	total_num =  nmp_log_get_record_count(self, query_buf);
	if (total_num < LOG_MAX_LOG_NUM)
		return;

	snprintf(query_buf, QUERY_STR_LEN,
		"delete from %s order by order_num limit %d ",
		LOG_TABLE, LOG_DEL_LOG_NUM_PERTIME
	);

	nmp_log_dbs_do_del_code(self, NULL, query_buf, &result, &affect_num);
}


static gint nmp_log(NmpAppObj *app_obj, NmpSysMsg *msg, guint operate_id,
	LOG_LEVEL log_level, gchar *user_name, gint result_code,
	gchar *child_type_1, gchar *child_type_2, gchar *child_type_3)
{
	G_ASSERT(app_obj != NULL && msg != NULL && user_name != NULL);
	gchar query_buf[QUERY_STR_LEN];
	NmpMsgErrCode result;
	glong affect_num = 0;
	time_t cur_time;
	gchar cur_time_str[NMP_LOG_TIME_LEN] = {0};
	gchar child_str[QUERY_STR_LEN];
	memset(&result, 0, sizeof(result));

	if (result_code < 0)
		result_code = -result_code;

	cur_time = time(NULL);
	nmp_get_time_str(cur_time, cur_time_str);

	if (child_type_1 == NULL)
	{
		snprintf(child_str, QUERY_STR_LEN, "NULL,NULL,NULL");
	}
	else if (child_type_2 == NULL)
	{
		snprintf(child_str, QUERY_STR_LEN, "'%s',NULL,NULL", child_type_1);
	}
	else if (child_type_3 == NULL)
	{
		snprintf(child_str, QUERY_STR_LEN, "'%s','%s',NULL", child_type_1,
			child_type_2);
	}
	else
	{
		snprintf(child_str, QUERY_STR_LEN, "'%s','%s','%s'", child_type_1,
			child_type_2, child_type_3);
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"insert into %s (log_time,log_time_str,log_level,user_name,operate_id," \
		"result_code,child_type1,child_type2,child_type3) values('%ld','%s'," \
		"'%d','%s','%u','%d',%s)",
		LOG_TABLE, cur_time, cur_time_str, log_level, user_name, operate_id,
		result_code, child_str
	);

	nmp_log_dbs_do_query_code(app_obj, msg, query_buf, &result, &affect_num);
	nmp_mod_log_del_redundant_log(app_obj);

	return result.err_no;
}


static void
nmp_log_modify_sysmsg(NmpSysMsg *sys_msg, gpointer priv_data, gsize size,
	NmpBusSlotPos src_pos, NmpBusSlotPos dst_pos, NmpMsgPrivDes msg_priv_destroy)
{
	nmp_sysmsg_set_private(sys_msg, priv_data, size, msg_priv_destroy);
	MSG_SET_DSTPOS(sys_msg, dst_pos);
	MSG_SET_RESPONSE(sys_msg);
}


static __inline__ void
nmp_query_log_info(NmpMysqlRes *result, NmpQueryLogRes *query_res)
{
	G_ASSERT(query_res != NULL);

	gint info_no = 0, field_no = 0;
	NmpMysqlRow mysql_row;
	NmpMysqlField* mysql_fields;
	gchar *name;
	gchar *value;
	unsigned int row_num;
	unsigned int field_num;

	row_num = nmp_log_sql_get_num_rows(result);
	field_num = nmp_log_sql_get_num_fields(result);

	while ((mysql_row = nmp_log_sql_fetch_row(result)))
	{
		nmp_log_sql_field_seek(result, 0);
		mysql_fields = nmp_log_sql_fetch_fields(result);
		NmpBssLog *log = &query_res->log_list[info_no];

		for(field_no = 0; field_no < field_num; field_no++)
		{
			name = nmp_log_sql_get_field_name(mysql_fields, field_no);

			if (!strcmp(name, "order_num"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
					log->order_num = atoi(value);
			}
			else if (!strcmp(name, "log_level"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
					log->log_level = atoi(value);
			}
			else if (!strcmp(name, "operate_id"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
					log->log_id = atoi(value);
			}
			else if (!strcmp(name, "result_code"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
					log->result_code = atoi(value);
			}
			else if (!strcmp(name, "log_time_str"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
				{
					log->log_time[TIME_INFO_LEN - 1] = '\0';
					strncpy(log->log_time, value, TIME_INFO_LEN - 1);
				}
			}
			else if (!strcmp(name, "user_name"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
				{
					log->user_name[USER_NAME_LEN - 1] = '\0';
					strncpy(log->user_name, value, USER_NAME_LEN - 1);
				}
			}
			else if (!strcmp(name, "child_type1"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
				{
					log->child_data1[LOG_CHILD_DATA_LEN - 1] = '\0';
					strncpy(log->child_data1, value, LOG_CHILD_DATA_LEN - 1);
				}
			}
			else if (!strcmp(name, "child_type2"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
				{
					log->child_data2[LOG_CHILD_DATA_LEN - 1] = '\0';
					strncpy(log->child_data2, value, LOG_CHILD_DATA_LEN - 1);
				}
			}
			else if (!strcmp(name, "child_type3"))
			{
				value = nmp_log_sql_get_field_value(mysql_row, field_no);
				if(value)
				{
					log->child_data3[LOG_CHILD_DATA_LEN - 1] = '\0';
					strncpy(log->child_data3, value, LOG_CHILD_DATA_LEN - 1);
				}
			}
			else
				cms_debug("no need mysql name %s \n", name);
		}

		info_no++;
		if (info_no >= query_res->req_total)
			break;
	}
}


static __inline__ NmpQueryLogRes *
nmp_log_query_log(NmpMysqlRes *mysql_res, gint *size)
{
	gint row_num, len;
	NmpQueryLogRes *query_res;

	row_num = nmp_log_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(NmpQueryLogRes);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		SET_CODE(query_res, E_NODBENT);
		query_res->total_num = 0;
		query_res->req_total = 0;
		nmp_warning("<NmpLogMh> log info has been delete?");
	}
	else
	{
		len = sizeof(NmpQueryLogRes) + row_num * sizeof(NmpBssLog);
		query_res = nmp_mem_kalloc(len);
		if (G_UNLIKELY(!query_res))
			return NULL;

		memset(query_res, 0, len);
		query_res->req_total = row_num;
		SET_CODE(query_res, 0);
		nmp_query_log_info(mysql_res, query_res);
	}
	*size = len;

	return query_res;
}

#if 0
void query_log_print(NmpQueryLogRes *res)
{
	gint i, count;
	count = res->req_total;

	nmp_warning("<log> res->code=%d", res->code);
	nmp_warning("<log> res->bss_usr:%s", res->bss_usr);
	nmp_warning("<log> res->total_num=%d", res->total_num);
	nmp_warning("<log> res->req_total=%d-----------------------", res->req_total);

	for (i = 0; i < count; i++)
	{
		nmp_warning("<log> log_level=%d", res->log_list[i].log_level);
		nmp_warning("<log> log_id=%d", res->log_list[i].log_id);
		nmp_warning("<log> result_code=%d", res->log_list[i].result_code);
		nmp_warning("<log> log_time:%s", res->log_list[i].log_time);
		nmp_warning("<log> user_name:%s", res->log_list[i].user_name);
		nmp_warning("<log> child_data1:%s", res->log_list[i].child_data1);
		nmp_warning("<log> child_data2:%s", res->log_list[i].child_data2);
		nmp_warning("<log> child_data3:%s", res->log_list[i].child_data3);
	}
}
#endif

NmpMsgFunRet
nmp_mod_log_query_log_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	NmpQueryLog *req_info;
	NmpQueryLogRes *query_res = NULL;
	NmpMysqlRes *result = NULL;
	gchar query_buf[QUERY_STR_LEN] = {0};
	gchar order_tmp[LOG_TMP_STR_LEN] = {0};
	gint size, ret = 0;
	gint total_num;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(*) as count from %s where log_level&%d and " \
		"log_time_str>='%s' and log_time_str<='%s'",
		LOG_TABLE, req_info->log_level, req_info->start_time, req_info->end_time
	);

	total_num =  nmp_log_get_record_count(app_obj, query_buf);
	if (total_num == 0)
	{
		ret = 0;
		goto query_log_end;
	}

	if (req_info->order_by == LOG_INVERTED_ORDER)
		strncpy(order_tmp, "log_time_str desc", LOG_TMP_STR_LEN - 1);
	else
		strncpy(order_tmp, "log_time_str", LOG_TMP_STR_LEN - 1);
	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s where log_level&%d and log_time_str>='%s' " \
		"and log_time_str<='%s' order by %s limit %d,%d",
		LOG_TABLE, req_info->log_level, req_info->start_time, req_info->end_time,
		order_tmp, req_info->start_num, req_info->req_count
	);

	result = nmp_log_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!result);

	if (G_LIKELY(!MYSQL_RESULT_CODE(result)))  //success:0 fail:!0
	{
		query_res = nmp_log_query_log(result, &size);
		if (G_UNLIKELY(!query_res))
		{
			nmp_warning("<NmpLogMh> alloc error");
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
		if (total_num <= query_res->req_total)
			query_res->total_num = query_res->req_total;
		else
			query_res->total_num = total_num;
	}
	else
	{
		ret = MYSQL_RESULT_CODE(result);
		goto query_log_end;
	}

query_log_end:
	if(result)
		nmp_log_sql_put_res(result, sizeof(NmpMysqlRes));

	if (query_res == NULL)
	{
		size = sizeof(NmpQueryLogRes);
		query_res = nmp_mem_kalloc(size);
		if (!query_res)
		{
			nmp_warning("<NmpLogMh> alloc error");
			nmp_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}

		memset(query_res, 0, size);
		SET_CODE(query_res, ret);
		query_res->total_num = 0;
		query_res->req_total = 0;
	}

	query_res->bss_usr[USER_NAME_LEN - 1] = '\0';
	strncpy(query_res->bss_usr, req_info->bss_usr, USER_NAME_LEN - 1);
	//query_log_print(query_res);
	nmp_log_modify_sysmsg(msg, query_res, size, BUSSLOT_POS_DBS,
		BUSSLOT_POS_BSS, nmp_mem_kfree);

	return MFR_DELIVER_BACK;
}



NmpMsgFunRet
nmp_mod_log_bss_login_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssLoginRes *res_info;
	gint ret;
	//nmp_warning("<NmpLogMh> nmp_mod_log_bss_login_b begin......");

	res_info = (NmpBssLoginRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_BSS_LOGIN, LOG_LEVEL_0,
		res_info->admin_name, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_ADMIN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_ADMIN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_admin_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_ADMIN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_USER_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_USER_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_user_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_USER_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_domain_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_DOMAIN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_modify_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpAddAreaRes *res_info;
	gint ret;

	res_info = (NmpAddAreaRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_MODIFY_AREA, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_AREA, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpAddPuRes *res_info;
	gint ret;

	res_info = (NmpAddPuRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_PU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_PU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_PU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_MDS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_MDS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_mds_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_MDS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_MDS_IP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_MDS_IP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_MSS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_MSS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_MSS, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_record_policy_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_RECORD_POLICY_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_modify_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_MODIFY_MANUFACTURER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_manufacturer_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
/*	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_MANUFACTURER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);
*/
	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_gu_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_GU_TO_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_set_system_time_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpSetServerTimeRes *res_info;
	gint ret;

	res_info = (NmpSetServerTimeRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_SET_TIME, LOG_LEVEL_2,
		res_info->bss_usr, RES_CODE(res_info), res_info->time_zone,
		res_info->system_time, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_database_backup_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DATABASE_BACKUP, LOG_LEVEL_0,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_database_import_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpDbImportRes *res_info;
	gint ret;

	res_info = (NmpDbImportRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DATABASE_IMPORT, LOG_LEVEL_2,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_set_network_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpSetResult *res_info;
	gint ret;

	res_info = (NmpSetResult *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_SET_NETWORK_CONFIG, LOG_LEVEL_2,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpAddHdGroupRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpAddHdGroupRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_HD_GROUP, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpAddHdToGroupRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpAddHdToGroupRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_HD, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_hd_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpDelHdFromGroupRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpDelHdFromGroupRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_HD, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_reboot_mss_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpRebootMssRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpRebootMssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_REBOOT_MSS, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_hd_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpDelHdGroupRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpDelHdGroupRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_HD_GROUP, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_DEFENCE_AREA, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
/*	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_DEFENCE_AREA, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);
*/
	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_defence_area_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_DEFENCE_AREA, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_DEFENCE_MAP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_defence_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_DEFENCE_MAP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_DEFENCE_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_DEFENCE_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_defence_gu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_DEFENCE_GU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_set_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_SET_MAP_HREF, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_MAP_HREF, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_map_href_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_MAP_HREF, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_platform_upgrade_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpPlatformUpgradeResp *res_info;
	gint ret;

	res_info = (NmpPlatformUpgradeResp *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_PLATFORM_UPGRADE, LOG_LEVEL_2,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_alarm_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpMsgCode *res_info;
	gint ret;

	res_info = (NmpMsgCode *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_ALARM, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_set_del_alarm_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_SET_DEL_ALARM_POLICY, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_TW, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODITY_TW, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_tw_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_TW, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_SCREEN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_SCREEN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_SCREEN, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_TOUR, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_TOUR, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_TOUR, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_tour_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_TOUR_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_GROUP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_GROUP_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_GROUP_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_GROUP_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_config_group_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_CONFIG_GROUP_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_GROUP_STEP_INFO, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_group_step_info_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_GROUP_STEP_INFO, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_time_policy_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_TIME_POLICY_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_TIME_POLICY, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_time_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_TIME_POLICY, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_record_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_RECORD_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_RECORD, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_RECORD, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_io_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_IO_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_IO, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_io_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_IO, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_snapshot_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_SNAPSHOT_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_SNAPSHOT, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_snapshot_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_SNAPSHOT, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_preset_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_PRESET_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_PRESET, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_preset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_PRESET, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_step_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_STEP_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_STEP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_auto_add_pu_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_AUTO_ADD_PU, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_set_initiator_name_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpSetInitNameRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpSetInitNameRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_SET_INITIATOR_NAME, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpAddOneIpsanRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpAddOneIpsanRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_ONE_IPSAN, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_one_ipsan_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpDeleteOneIpsanRes *res_info;
	gint ret;

	NMP_CHECK_MSS_MSG_RES(msg);

	res_info = (NmpDeleteOneIpsanRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_ONE_IPSAN, LOG_LEVEL_1,
		res_info->session, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_link_map_config_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_LINK_MAP_CONFIG, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_modify_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_MODIFY_LINK_MAP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_del_link_map_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_DEL_LINK_MAP, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_tw_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_TW_TO_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_log_add_tour_to_user_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	NmpBssRes *res_info;
	gint ret;

	res_info = (NmpBssRes *)MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = nmp_log(app_obj, msg, LOG_ADD_TOUR_TO_USER, LOG_LEVEL_1,
		res_info->bss_usr, RES_CODE(res_info), NULL, NULL, NULL);
	NMP_DEAL_LOG_RESULT(ret);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


#define LOG_REGISTER_MSG_B(_flag, msg_id, b_fun) do { \
	G_ASSERT(msg_id >= 0 && msg_id < LOG_MAX_MESSAGE_NUM); \
	g_log_msg_enable[msg_id].enable = 1; \
	g_log_msg_enable[msg_id].flag = _flag; \
	nmp_app_mod_register_msg(super_self, msg_id, NULL, b_fun, 0); \
} while (0)

/*
 * message
 */
void
nmp_mod_log_register_msg_handler(NmpModLog *self)
{
	NmpAppMod *super_self = (NmpAppMod*)self;

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_LOG,
		NULL,
		nmp_mod_log_query_log_b,
		0
	);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 1
		MESSAGE_BSS_LOGIN, nmp_mod_log_bss_login_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_ADMIN, nmp_mod_log_add_admin_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_ADMIN, nmp_mod_log_modify_admin_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_ADMIN, nmp_mod_log_del_admin_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 5
		MESSAGE_ADD_USER_GROUP, nmp_mod_log_add_user_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_USER_GROUP, nmp_mod_log_modify_user_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_USER_GROUP, nmp_mod_log_del_user_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_USER, nmp_mod_log_add_user_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_USER, nmp_mod_log_modify_user_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 10
		MESSAGE_DEL_USER, nmp_mod_log_del_user_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 11
		MESSAGE_MODIFY_DOMAIN, nmp_mod_log_modify_domain_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 14
		MESSAGE_ADD_MODIFY_AREA, nmp_mod_log_add_modify_area_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 15
		MESSAGE_DEL_AREA, nmp_mod_log_del_area_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_PU, nmp_mod_log_add_pu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_PU, nmp_mod_log_modify_pu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_PU, nmp_mod_log_del_pu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_GU, nmp_mod_log_add_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 20
		MESSAGE_MODIFY_GU, nmp_mod_log_modify_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_GU, nmp_mod_log_del_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_MDS, nmp_mod_log_add_mds_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_MDS, nmp_mod_log_modify_mds_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_MDS, nmp_mod_log_del_mds_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 25
		MESSAGE_ADD_MDS_IP, nmp_mod_log_add_mds_ip_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_MDS_IP, nmp_mod_log_del_mds_ip_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_MSS, nmp_mod_log_add_mss_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_MSS, nmp_mod_log_modify_mss_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_MSS, nmp_mod_log_del_mss_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 30
		MESSAGE_RECORD_POLICY_CONFIG, nmp_mod_log_record_policy_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_MODIFY_MANUFACTURER, nmp_mod_log_add_modify_manufacturer_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_MANUFACTURER, nmp_mod_log_del_manufacturer_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_GU_TO_USER, nmp_mod_log_add_gu_to_user_b);

	nmp_app_mod_register_msg(
		super_self,
		MSG_LOG_SET_SYSTEM_TIME,
		NULL,
		nmp_mod_log_set_system_time_b,
		0
	);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 35
		MESSAGE_DATABASE_BACKUP, nmp_mod_log_database_backup_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DATABASE_IMPORT, nmp_mod_log_database_import_b);

	nmp_app_mod_register_msg(
		super_self,
		MSG_LOG_SET_NETWORK_CONFIG,
		NULL,
		nmp_mod_log_set_network_config_b,
		0
	);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_ADD_HD_GROUP, nmp_mod_log_add_hd_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_ADD_HD, nmp_mod_log_add_hd_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ, // 40
		MESSAGE_DEL_HD, nmp_mod_log_del_hd_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_REBOOT_MSS, nmp_mod_log_reboot_mss_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_DEL_HD_GROUP, nmp_mod_log_del_hd_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_DEFENCE_AREA, nmp_mod_log_add_defence_area_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_DEFENCE_AREA, nmp_mod_log_modify_defence_area_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 45
		MESSAGE_DEL_DEFENCE_AREA, nmp_mod_log_del_defence_area_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_DEFENCE_MAP, nmp_mod_log_add_defence_map_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_DEFENCE_MAP, nmp_mod_log_del_defence_map_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_DEFENCE_GU, nmp_mod_log_add_defence_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_DEFENCE_GU, nmp_mod_log_modify_defence_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 50
		MESSAGE_DEL_DEFENCE_GU, nmp_mod_log_del_defence_gu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_SET_MAP_HREF, nmp_mod_log_set_map_href_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_MAP_HREF, nmp_mod_log_modify_map_href_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_MAP_HREF, nmp_mod_log_del_map_href_b);

	nmp_app_mod_register_msg(
		super_self,
		MSG_LOG_PLATFORM_UPGRADE,
		NULL,
		nmp_mod_log_platform_upgrade_b,
		0
	);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 55
		MESSAGE_DEL_ALARM, nmp_mod_log_del_alarm_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_SET_DEL_ALARM_POLICY, nmp_mod_log_set_del_alarm_policy_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_TW, nmp_mod_log_add_tw_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_TW, nmp_mod_log_modify_tw_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_TW, nmp_mod_log_del_tw_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 60
		MESSAGE_ADD_SCREEN, nmp_mod_log_add_screen_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_SCREEN, nmp_mod_log_modify_screen_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_SCREEN, nmp_mod_log_del_screen_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_TOUR, nmp_mod_log_add_tour_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_TOUR, nmp_mod_log_modify_tour_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 65
		MESSAGE_DEL_TOUR, nmp_mod_log_del_tour_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_TOUR_STEP, nmp_mod_log_add_tour_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_GROUP, nmp_mod_log_add_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_GROUP, nmp_mod_log_modify_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_GROUP, nmp_mod_log_del_group_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 70
		MESSAGE_ADD_GROUP_STEP, nmp_mod_log_add_group_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_GROUP_STEP, nmp_mod_log_modify_group_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_GROUP_STEP, nmp_mod_log_del_group_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_CONFIG_GROUP_STEP, nmp_mod_log_config_group_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_GROUP_STEP_INFO, nmp_mod_log_modify_group_step_info_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 75
		MESSAGE_DEL_GROUP_STEP_INFO, nmp_mod_log_del_group_step_info_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_TIME_POLICY_CONFIG, nmp_mod_log_link_time_policy_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_TIME_POLICY, nmp_mod_log_modify_link_time_policy_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_TIME_POLICY, nmp_mod_log_del_link_time_policy_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_RECORD_CONFIG, nmp_mod_log_link_record_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 80
		MESSAGE_MODIFY_LINK_RECORD, nmp_mod_log_modify_link_record_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_RECORD, nmp_mod_log_del_link_record_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_IO_CONFIG, nmp_mod_log_link_io_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_IO, nmp_mod_log_modify_link_io_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_IO, nmp_mod_log_del_link_io_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 85
		MESSAGE_LINK_SNAPSHOT_CONFIG, nmp_mod_log_link_snapshot_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_SNAPSHOT, nmp_mod_log_modify_link_snapshot_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_SNAPSHOT, nmp_mod_log_del_link_snapshot_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_PRESET_CONFIG, nmp_mod_log_link_preset_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_PRESET, nmp_mod_log_modify_link_preset_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES, // 90
		MESSAGE_DEL_LINK_PRESET, nmp_mod_log_del_link_preset_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_STEP_CONFIG, nmp_mod_log_link_step_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_STEP, nmp_mod_log_modify_link_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_STEP, nmp_mod_log_del_link_step_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_AUTO_ADD_PU, nmp_mod_log_auto_add_pu_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ, // 95
		MESSAGE_SET_INIT_NAME, nmp_mod_log_set_initiator_name_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_ADD_ONE_IPSAN, nmp_mod_log_add_one_ipsan_b);

	LOG_REGISTER_MSG_B(LOG_MSG_REQ,
		MESSAGE_DELETE_ONE_IPSAN, nmp_mod_log_del_one_ipsan_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_LINK_MAP_CONFIG, nmp_mod_log_link_map_config_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_MODIFY_LINK_MAP, nmp_mod_log_modify_link_map_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_DEL_LINK_MAP, nmp_mod_log_del_link_map_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_TW_TO_USER, nmp_mod_log_add_tw_to_user_b);

	LOG_REGISTER_MSG_B(LOG_MSG_RES,
		MESSAGE_ADD_TOUR_TO_USER, nmp_mod_log_add_tour_to_user_b);
}

