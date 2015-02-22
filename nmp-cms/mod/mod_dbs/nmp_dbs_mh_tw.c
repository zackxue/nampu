#include <sys/unistd.h>
#include <assert.h>
#include "nmp_mod_dbs.h"
#include "nmp_dbs_struct.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_tw_interface.h"
#include "nmp_dbs_fun.h"

#define ASSERT_CHECK

typedef struct
{
	int	scr_id;
	int	div_id;
	char	dis_guid[TW_ID_LEN];
	char	dis_domain_id[TW_ID_LEN];
	tw_group_division_response division;
} group_read_one_line;


static int get_ec_url(JpfAppObj *app_obj, char *dis_guid, char *dis_domain_id,
	char *ec_guid, char *ec_domain_id, char *ec_url, char *ec_dec_plug)
{
	G_ASSERT(app_obj != NULL && ec_url != NULL&& dis_guid != NULL
		&& dis_domain_id != NULL&& ec_guid != NULL&& ec_domain_id != NULL);

	JpfMsgGetMdsIpRes mds_ip;
	gchar dis_puid[MAX_ID_LEN] = {0};
	gchar enc_puid[MAX_ID_LEN] = {0};
	gchar cms_ip[MAX_IP_LEN] = {0};
	//gchar tmp[MAX_ID_LEN];
	gint channel = 0, level = 0;
	gint code = 0;

	jpf_get_puid_from_guid(dis_guid, dis_puid);
	jpf_get_pu_cms_ip(app_obj, dis_puid, dis_domain_id, cms_ip);
	jpf_get_puid_from_guid(ec_guid, enc_puid);
	jpf_get_channel_from_guid(ec_guid, &channel);
	jpf_get_level_from_guid(ec_guid, &level);
	if (ec_dec_plug)
	{
		jpf_get_mf_from_guid(ec_guid, ec_dec_plug);
		if (!strcmp(ec_dec_plug, MANUFACTUR_JXJ_2))
			strcpy(ec_dec_plug, MANUFACTUR_JXJ);
	}
	memset(&mds_ip, 0, sizeof(mds_ip));
	code = jpf_dbs_get_mds_info(app_obj, cms_ip, enc_puid, ec_domain_id, &mds_ip);
	if (G_LIKELY(!code))
	{
		snprintf(ec_url, MAX_URL_LEN,
			"rtsp://%s:%d/dev=@%s/media=%d/channel=%d&level=%d",
			mds_ip.mds_ip, mds_ip.rtsp_port, enc_puid, MEDIA_VIDEO_BROWSE, channel, level
		);
	}
	else
		jpf_warning("<TwGetRoute> No route to Mds,error:%d", code);

	return code;
}


JpfMsgFunRet
jpf_mod_dbs_get_dis_guid_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	tw_dis_guid_request *req_info;
	tw_dis_guid_response res_info;
	int code = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	memset(&res_info, 0, sizeof(res_info));
	code = jpf_get_scr_display_guid(app_obj, &res_info.dis_guid,
		req_info->screen_id, req_info->tw_id);
	res_info.result = code;
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_get_ec_url_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	tw_ec_url_request *req_info;
	tw_ec_url_response res_info;

	memset(&res_info, 0, sizeof(res_info));
	req_info = (tw_ec_url_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	/* 构造返回数据 */
	res_info.result = get_ec_url(app_obj, req_info->dis_guid, req_info->dis_domain_id,
		req_info->ec_guid, req_info->ec_domain_id, res_info.ec_url, res_info.ec_dec_plug);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	printf("<JpfModDbs> res_info.ec_url:%s\n", res_info.ec_url);

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);

	return MFR_DELIVER_BACK;
}


static __inline__ void
jpf_mod_dbs_get_tour_info(JpfMysqlRes *mysql_res,
	tw_tour_msg_response *tour_res)
{
	G_ASSERT(mysql_res != NULL);

	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	unsigned int row_num, field_num;
	int step_num = 0;
	gchar *name, *value;
	gint field_i = 0, step_i = 0;

	row_num = jpf_sql_get_num_rows(mysql_res);
	field_num =jpf_sql_get_num_fields(mysql_res);

	while ((mysql_row = jpf_sql_fetch_row(mysql_res)))
	{
		jpf_sql_field_seek(mysql_res, 0);
		mysql_fields = jpf_sql_fetch_fields(mysql_res);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			tw_tour_step_response *tour_step = &tour_res->steps[step_i];
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "step_no"))
			{
				tour_step->step_num = ++step_num;
			}
			else if (!strcmp(name, "interval"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tour_step->step_ttl = atoi(value);
			}
			else if (!strcmp(name, "gu_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					tour_step->ec_name[TW_MAX_VALUE_LEN - 1] = '\0';
					strncpy(tour_step->ec_name, value, TW_MAX_VALUE_LEN - 1);
				}
			}
			else if (!strcmp(name, "encoder_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					tour_step->ec_domain_id[TW_ID_LEN - 1] = '\0';
					strncpy(tour_step->ec_domain_id, value, TW_ID_LEN - 1);
				}
			}
			else if (!strcmp(name, "encoder_guid"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					tour_step->ec_guid[TW_ID_LEN - 1] = '\0';
					strncpy(tour_step->ec_guid, value, TW_ID_LEN - 1);
				}
			}
			else if (!strcmp(name, "level"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tour_step->level = atoi(value);
			}

			if (step_i != 0)
				continue;
			if (!strcmp(name, "tour_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					tour_res->tour_name[TW_MAX_VALUE_LEN - 1] = '\0';
					strncpy(tour_res->tour_name, value, TW_MAX_VALUE_LEN - 1);
				}
			}
			else if (!strcmp(name, "auto_jump"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tour_res->auto_jump = atoi(value);
			}

		}

		step_i++;
	}
}


static __inline__ tw_tour_msg_response *
jpf_mod_dbs_get_tour(JpfMysqlRes *mysql_res, gint *size)
{
	tw_tour_msg_response *tour_res;
	gint row_num, len;

	row_num = jpf_sql_get_num_rows(mysql_res);
	if (row_num == 0)
	{
		len = sizeof(tw_tour_msg_response);
		tour_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!tour_res))
			return NULL;

		memset(tour_res, 0, len);
		tour_res->result = 0;
		tour_res->step_count = 0;
		jpf_warning("tour has no step");
	}
	else
	{
		len = sizeof(tw_tour_msg_response) + row_num *
			sizeof(tw_tour_step_response);

		tour_res = jpf_mem_kalloc(len);
		if (G_UNLIKELY(!tour_res))
			return NULL;

		memset(tour_res, 0, len);
		tour_res->steps = (tw_tour_step_response *)((int)tour_res +
			sizeof(tw_tour_msg_response));
		tour_res->step_count = row_num;
		tour_res->result = 0;

		jpf_mod_dbs_get_tour_info(mysql_res, tour_res);
	}

	*size = len;
	return tour_res;
}


JpfMsgFunRet
jpf_mod_dbs_get_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	tw_tour_msg_request *req_info;
	tw_tour_msg_response *res_info;
	JpfMysqlRes *result;
	char query_buf[QUERY_STR_LEN] = {0};
	gint size, ret;

	req_info = (tw_tour_msg_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.tour_name,t1.auto_jump,t2.*,t3.gu_name from %s as t1," \
		"%s as t2, %s as t3 where t1.tour_id=%d and t1.tour_id=t2.tour_id " \
		"and t2.encoder_domain=t3.gu_domain and t2.encoder_guid=t3.gu_id " \
		"order by t2.step_no",
		TOUR_TABLE, TOUR_STEP_TABLE, GU_TABLE, req_info->tour_id
	);

	result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!result);

	if (G_LIKELY(!MYSQL_RESULT_CODE(result)))	//success:0 fail:!0
	{
		res_info = jpf_mod_dbs_get_tour(result, &size);
		if (G_UNLIKELY(!res_info))
		{
			jpf_warning("<dbs_mh_tw> alloc error");
			jpf_sql_put_res(result, sizeof(JpfMysqlRes));
			return MFR_ACCEPTED;
		}
	}
	else
	{
		ret = MYSQL_RESULT_CODE(result);
		goto sql_operate_failed;
	}

	goto end_get_tour;

sql_operate_failed:
	size = sizeof(tw_tour_msg_response);
	res_info = jpf_mem_kalloc(size);
	if (!res_info)
	{
		jpf_warning("<dbs_mh_tw> alloc error");
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		return MFR_ACCEPTED;
	}
	memset(res_info, 0, size);
	res_info->result = ret;

	/* 构造返回数据 */
end_get_tour:
	jpf_sysmsg_set_private(msg, res_info, size, (JpfMsgPrivDes)jpf_mem_kfree);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);
	if (result)
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return MFR_DELIVER_BACK;
}


static void jpf_mod_get_group_info(tw_group_msg_response *res_info,
	JpfMysqlRes *mysql_res)
{
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	unsigned int row_num, field_num;
	gchar *name, *value;
	gint field_i = 0;

	row_num = jpf_sql_get_num_rows(mysql_res);
	field_num =jpf_sql_get_num_fields(mysql_res);

	while ((mysql_row = jpf_sql_fetch_row(mysql_res)))
	{
		jpf_sql_field_seek(mysql_res, 0);
		mysql_fields = jpf_sql_fetch_fields(mysql_res);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "tw_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					res_info->tw_id = atoi(value);
			}
			else if (!strcmp(name, "group_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					res_info->group_name[TW_MAX_VALUE_LEN - 1] = '\0';
					strncpy(res_info->group_name, value, TW_MAX_VALUE_LEN - 1);
				}
			}
		}
	}
}

static __inline__ void
insert_step_num_sort(tw_group_msg_response *res_info, unsigned int step_num)
{
	if (res_info->step_count >= TW_MAX_STEPS_TOP)
		return ;

	int i;
	unsigned int *nums = res_info->step_num;

	for (i = res_info->step_count - 1; i >= 0; i--)
	{
		if (step_num >= nums[i])
		{
			nums[i + 1] = step_num;
			break;
		}
		else
		{
			nums[i + 1] = nums[i];
		}
	}
	if (i < 0)
		nums[0] = step_num;

	res_info->step_count++;
}

static void jpf_mod_get_group_info_2(tw_group_msg_response *res_info,
	JpfMysqlRes *mysql_res)
{
	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	unsigned int row_num, field_num;
	unsigned int step_num;
	gchar *name, *value;
	gint field_i = 0;

	row_num = jpf_sql_get_num_rows(mysql_res);
	field_num =jpf_sql_get_num_fields(mysql_res);

	while ((mysql_row = jpf_sql_fetch_row(mysql_res)))
	{
		jpf_sql_field_seek(mysql_res, 0);
		mysql_fields = jpf_sql_fetch_fields(mysql_res);

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "step_no"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
				{
					step_num = (unsigned int)atoi(value);
					insert_step_num_sort(res_info, step_num);
				}
			}
		}
	}
#ifdef ASSERT_CHECK
	int i;
	for (i = 0; i < res_info->step_count - 1; i++)
	{
		assert(res_info->step_num[i] < res_info->step_num[i + 1]);
	}
#endif
}


JpfMsgFunRet
jpf_mod_dbs_get_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	tw_group_msg_request *req_info;
	tw_group_msg_response res_info;
	JpfMysqlRes *result;
	char query_buf[QUERY_STR_LEN] = {0};
	gint ret;
	memset(&res_info, 0, sizeof(tw_group_msg_response));

	req_info = (tw_group_msg_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select * from %s where group_id=%d",
		GROUP_TABLE, req_info->group_id
	);

	result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(result)))	//success:0 fail:!0
	{
		ret = MYSQL_RESULT_CODE(result);
		goto sql_operate_failed;
	}

	jpf_mod_get_group_info(&res_info, result);
	jpf_sql_put_res(result, sizeof(JpfMysqlRes));

	snprintf(query_buf, QUERY_STR_LEN,
		"select step_no from %s where group_id=%d",
		GROUP_STEP_TABLE, req_info->group_id
	);

	result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(result)))	//success:0 fail:!0
	{
		ret = MYSQL_RESULT_CODE(result);
		goto sql_operate_failed;
	}

	jpf_mod_get_group_info_2(&res_info, result);
	res_info.result = 0;
	goto end;

sql_operate_failed:
	res_info.result = ret;

end:
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);
	if (result)
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return MFR_DELIVER_BACK;
}


static int
stor_group_info(tw_group_step_n_response *res, group_read_one_line *info)
{
	int screen_i, screen_sum;
	tw_group_screen_response *screen = NULL;

	screen_sum = res->screen_sum;
	for (screen_i = 0; screen_i < screen_sum; screen_i++)
	{
		if (res->screens[screen_i].screen_id == 0)
		{
			screen = &res->screens[screen_i];
			screen->screen_id = info->scr_id;
			strncpy(screen->dis_guid, info->dis_guid, TW_ID_LEN);
			strncpy(screen->dis_domain_id, info->dis_domain_id, TW_ID_LEN);
			screen->division_id = info->div_id;
			break;
		}
		else if (res->screens[screen_i].screen_id == info->scr_id)
		{
			screen = &res->screens[screen_i];
			break;
		}
	}

	if (!screen)
	{
		jpf_warning("<dbs_mh_tw> get screen error, maybe group has been changed, " \
			"screen_sum = %d!", screen_sum);
		return -1;
	}

	assert(screen->div_sum < TW_MAX_DIVISIONS);
	tw_group_division_response *division = &screen->divisions[screen->div_sum];
	memcpy(division, &info->division, sizeof(tw_group_division_response));
	screen->div_sum++;

	return 0;
}

static int
jpf_mod_dbs_get_group_step(JpfMysqlRes *mysql_res,
	tw_group_step_n_response *res)
{
	G_ASSERT(mysql_res != NULL && res != NULL);

	JpfMysqlRow mysql_row;
	JpfMysqlField* mysql_fields;
	unsigned int row_num, field_num;
	group_read_one_line tmp;
	gchar *name, *value;
	gint field_i = 0;

	row_num = jpf_sql_get_num_rows(mysql_res);
	field_num =jpf_sql_get_num_fields(mysql_res);

	while ((mysql_row = jpf_sql_fetch_row(mysql_res)))
	{
		jpf_sql_field_seek(mysql_res, 0);
		mysql_fields = jpf_sql_fetch_fields(mysql_res);
		memset(&tmp, 0, sizeof(group_read_one_line));

		for (field_i = 0; field_i < field_num; field_i++)
		{
			name = jpf_sql_get_field_name(mysql_fields, field_i);

			if (!strcmp(name, "scr_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tmp.scr_id = atoi(value);
			}
			else if (!strcmp(name, "div_id"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tmp.div_id= atoi(value);
			}
			else if (!strcmp(name, "level"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tmp.division.level= atoi(value);
			}
			else if (!strcmp(name, "dis_guid"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					strncpy(tmp.dis_guid, value, TW_ID_LEN - 1);
			}
			else if (!strcmp(name, "dis_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					strncpy(tmp.dis_domain_id, value, TW_ID_LEN - 1);
			}
			else if (!strcmp(name, "div_no"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					tmp.division.division_num = atoi(value);
			}
			else if (!strcmp(name, "encoder_domain"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					strncpy(tmp.division.ec_domain_id, value, TW_ID_LEN - 1);
			}
			else if (!strcmp(name, "encoder_guid"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					strncpy(tmp.division.ec_guid, value, TW_ID_LEN - 1);
			}
			else if (!strcmp(name, "gu_name"))
			{
				value = jpf_sql_get_field_value(mysql_row, field_i);
				if (value)
					strncpy(tmp.division.ec_name, value, TW_MAX_VALUE_LEN - 1);
			}
		}

		if (stor_group_info(res, &tmp) != 0)
			return -1;
	}

	return 0;
}


JpfMsgFunRet
jpf_mod_dbs_get_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	tw_group_step_n_request *req_info;
	tw_group_step_n_response *res_info;
	JpfMysqlRes *result = NULL;
	char query_buf[QUERY_STR_LEN] = {0};
	gint interval, screen_sum, size, ret;

	req_info = (tw_group_step_n_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	snprintf(query_buf, QUERY_STR_LEN,
		"select `interval` as count from %s where group_id=%d and step_no=%d",
		GROUP_STEP_TABLE, req_info->group_id, req_info->step_num
	);
	interval = jpf_get_record_count(app_obj, query_buf);
	if (interval <= 0)
	{
		ret = -1;
		jpf_warning("<dbs_mh_tw> group step interval = %d", interval);
		goto sql_operate_failed;
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select count(distinct scr_id) as count from %s where step_no=%d",
		GROUP_STEP_INFO_TABLE, req_info->step_num
	);
	screen_sum = jpf_get_record_count(app_obj, query_buf);
	if (screen_sum < 0)
	{
		ret = -1;
		jpf_warning("<dbs_mh_tw> screen_sum = %d", screen_sum);
		goto sql_operate_failed;
	}
	if (screen_sum == 0)
	{
		ret = 0;
		jpf_warning("<dbs_mh_tw> screen_sum = 0");
	}

	snprintf(query_buf, QUERY_STR_LEN,
		"select t1.*,t2.gu_name,t4.dis_guid,t4.dis_domain from %s as t1,%s as t2,%s as t3," \
		"%s as t4 where t1.step_no=%d and t1.encoder_domain=t2.gu_domain and " \
		"t1.encoder_guid=t2.gu_id and t3.group_id=%d and t3.tw_id=t4.tw_id " \
		"and t1.scr_id=t4.scr_id",
		GROUP_STEP_INFO_TABLE, GU_TABLE, GROUP_TABLE, SCREEN_TABLE,
		req_info->step_num, req_info->group_id
	);

	result = jpf_dbs_do_query_res(app_obj, query_buf);
	BUG_ON(!result);
	if (G_UNLIKELY(MYSQL_RESULT_CODE(result)))	//success:0 fail:!0
	{
		ret = MYSQL_RESULT_CODE(result);
		goto sql_operate_failed;
	}

	size = sizeof(tw_group_step_n_response) + screen_sum *
		sizeof(tw_group_screen_response);
	res_info = jpf_mem_kalloc(size);
	if (G_UNLIKELY(!res_info))
	{
		jpf_warning("<dbs_mh_tw> alloc error");
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		return MFR_ACCEPTED;
	}

	memset(res_info, 0, size);
	res_info->screen_sum = screen_sum;
	res_info->step_ttl = interval;
	if (screen_sum == 0)
	{
		res_info->result = 0;
		goto end;
	}
	res_info->screens = (tw_group_screen_response *)((int)res_info +
		sizeof(tw_group_step_n_response));

	res_info->result = jpf_mod_dbs_get_group_step(result, res_info);
	goto end;


sql_operate_failed:
	size = sizeof(tw_group_step_n_response);
	res_info = jpf_mem_kalloc(size);
	if (!res_info)
	{
		jpf_warning("<dbs_mh_tw> alloc error");
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
		return MFR_ACCEPTED;
	}
	memset(res_info, 0, size);
	res_info->result = ret;

end:
	jpf_sysmsg_set_private(msg, res_info, size, (JpfMsgPrivDes)jpf_mem_kfree);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_TW);
	if (result)
		jpf_sql_put_res(result, sizeof(JpfMysqlRes));
	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_dbs_check_ec_url_update_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	tw_update_url *req_info, res_info;
	char new_url[MAX_URL_LEN] = {0};
	JpfSysMsg *res_msg;
	int result;

	req_info = (tw_update_url *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	result = get_ec_url(app_obj, req_info->dis_guid, req_info->dis_domain_id,
		req_info->ec_guid, req_info->ec_domain_id, new_url, NULL);

	if (result)
	{
		jpf_warning("get_ec_url failed");
		goto end;
	}

	if (strlen(new_url) && strcmp(req_info->ec_url, new_url))	//url已经更新
	{
		memcpy(&res_info, req_info, sizeof(tw_update_url));
		res_info.ec_url[MAX_URL_LEN - 1] = '\0';
		strncpy(res_info.ec_url, new_url, MAX_URL_LEN - 1);

		jpf_sysmsg_destroy(msg);

		res_msg = jpf_sysmsg_new_2(MSG_TW_INFO_NOTIFY_UPDATE_EC_URL, &res_info,
			sizeof(tw_update_url), ++msg_seq_generator);
		if (G_UNLIKELY(!res_msg))
		{
			jpf_warning("jpf_sysmsg_new_2 failed");
			return MFR_ACCEPTED;
		}
		MSG_SET_DSTPOS(res_msg, BUSSLOT_POS_TW);
		jpf_app_obj_deliver_out(app_obj, res_msg);
		return MFR_ACCEPTED;
	}

end:
	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


void
jpf_mod_dbs_register_tw_msg_handler(JpfModDbs *self)
{
	JpfAppMod *super_self = (JpfAppMod*)self;

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_GET_DIS_GUID,
		NULL,
		jpf_mod_dbs_get_dis_guid_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_GET_EC_URL,
		NULL,
		jpf_mod_dbs_get_ec_url_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_GET_TOUR,
		NULL,
		jpf_mod_dbs_get_tour_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_GET_GROUP,
		NULL,
		jpf_mod_dbs_get_group_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_GET_GROUP_STEP_N,
		NULL,
		jpf_mod_dbs_get_group_step_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_CHECK_EC_URL_UPDATE,
		NULL,
		jpf_mod_dbs_check_ec_url_update_b,
		0
	);
}

