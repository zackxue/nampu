#include <sys/unistd.h>
#include "nmp_mod_tw.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_msg_tw.h"
#include "nmp_tw_interface.h"
#include "nmp_internal_msg.h"


#define jpf_tw_mh_log(fmt, args ...) do {	\
	if (1)	\
		printf("<jpf_tw_mh_log> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)


NmpMsgFunRet
nmp_mod_tw_run_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_run_step_request *req_info;
	tw_run_step_request_with_seq req_with_seq;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_run_step_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	req_with_seq.seq = MSG_SEQ(msg);
	memcpy(&req_with_seq.req, req_info, sizeof(tw_run_step_request));

	ret = jpf_tw_run_step(&req_with_seq);
	if (ret)
	{
		jpf_warning("jpf_tw_run_step failed, ec_guid:%s",
			req_info->ec_guid);
	}
	else
	{
		jpf_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_run_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_run_tour_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_run_tour_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_run_tour(req_info);
	if (ret)
	{
		jpf_warning("jpf_tw_run_tour failed, tour_id:%d",
			req_info->tour_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_run_tour success, tour_id:%d\n",
			req_info->tour_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_run_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_run_group_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_run_group_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_run_group(req_info);
	if (ret)
	{
		jpf_warning("jpf_tw_run_group failed, group_id:%d",
			req_info->group_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_run_group success, group_id:%d\n",
			req_info->group_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_stop_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_stop_tour_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_stop_tour_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_stop_tour(req_info);
	if (ret)
	{
		jpf_tw_mh_log("jpf_tw_stop_tour failed, tour_id:%d\n",
			req_info->tour_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_stop_tour success, tour_id:%d\n",
			req_info->tour_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_stop_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_stop_group_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_stop_group_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_stop_group(req_info);
	if (ret)
	{
		jpf_tw_mh_log("jpf_tw_stop_group failed, group_id:%d\n",
			req_info->group_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_stop_group success, group_id:%d\n",
			req_info->group_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_if_run_tour_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_if_run_tour_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_if_run_tour_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_if_run_tour(req_info);
	if (ret)
	{
		jpf_tw_mh_log("jpf_tw_if_run_tour -> no, tour_id:%d\n",
			req_info->tour_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_stop_tour -> yes, tour_id:%d\n",
			req_info->tour_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_if_run_group_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_if_run_group_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_if_run_group_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_if_run_group(req_info);
	if (ret)
	{
		jpf_tw_mh_log("jpf_tw_if_run_group -> no, group_id:%d\n",
			req_info->group_id);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_if_run_group -> yes, group_id:%d\n",
			req_info->group_id);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_stop_gpt_by_division_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_division_position *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_division_position *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_tw_stop_gpt_by_division(req_info);
	if (ret)
	{
		jpf_tw_mh_log("jpf_tw_stop_gpt_by_division failed, tw_id:%d, " \
			"screen_id:%d, division_num:%d\n",
			req_info->tw_id, req_info->screen_id, req_info->division_num);
	}
	else
	{
		jpf_tw_mh_log("jpf_tw_stop_gpt_by_division success, tw_id:%d, " \
			"screen_id:%d, division_num:%d\n",
			req_info->tw_id, req_info->screen_id, req_info->division_num);
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_play_resp_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_decoder_rsp *rsp_info;
	tw_decoder_rsp_with_seq rsp_with_seq;
	gint ret;

	self = (JpfModTw *)app_obj;
	rsp_info = (tw_decoder_rsp *)MSG_GET_DATA(msg);
	BUG_ON(!rsp_info);

	rsp_with_seq.seq = MSG_SEQ(msg);
	memcpy(&rsp_with_seq.dec_rsp, rsp_info, sizeof(tw_decoder_rsp));

	ret = jpf_tw_deal_decoder_res(&rsp_with_seq);
	if (ret)
	{
		jpf_warning("jpf_tw_deal_decoder_res failed");
	}

	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_tw_change_direction_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_PU)
	{
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);
	}
	else
	{
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_PU);
	}

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_operate_b(NmpAppObj *app_obj, NmpSysMsg *msg, TW_INFO_TYPE type)
{
	JpfModTw *self;
	tw_operate *req_info;
	tw_operate_with_seq req_with_seq;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModTw *)app_obj;
	req_info = (tw_operate *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	req_with_seq.seq = MSG_SEQ(msg);
	memcpy(&req_with_seq.operate, req_info, sizeof(tw_operate));

	ret = jpf_tw_screen_operate(&req_with_seq, type);
	if (ret)
	{
		jpf_warning("jpf_tw_screen_operate failed");
	}
	else
	{
		jpf_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session_id, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


NmpMsgFunRet
nmp_mod_tw_operate_resp_b(NmpAppObj *app_obj, NmpSysMsg *msg, TW_INFO_TYPE type)
{

	JpfModTw *self;
	tw_operate_decoder_rsp *rsp_info;
	tw_operate_decoder_rsp_with_seq rsp_with_seq;
	gint ret;

	self = (JpfModTw *)app_obj;
	rsp_info = (tw_operate_decoder_rsp *)MSG_GET_DATA(msg);
	BUG_ON(!rsp_info);

	rsp_with_seq.seq = MSG_SEQ(msg);
	memcpy(&rsp_with_seq.operate_dec_rsp, rsp_info, sizeof(tw_operate_decoder_rsp));

	ret = jpf_tw_deal_decoder_operate_res(&rsp_with_seq, type);
	if (ret)
	{
		jpf_warning("jpf_tw_deal_decoder_operate_res failed");
	}

	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
nmp_mod_tw_clear_division_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_CU)
		return nmp_mod_tw_operate_b(app_obj, msg, TW_CLEAR_TO_DEC);
	else
		return nmp_mod_tw_operate_resp_b(app_obj, msg, TW_CLEAR_TO_DEC);
}

NmpMsgFunRet
nmp_mod_tw_change_div_mode_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_CU)
		return nmp_mod_tw_operate_b(app_obj, msg, TW_CHANGE_DIVISION_MODE_TO_DEC);
	else
		return nmp_mod_tw_operate_resp_b(app_obj, msg, TW_CHANGE_DIVISION_MODE_TO_DEC);
}

NmpMsgFunRet
nmp_mod_tw_full_screen_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_CU)
		return nmp_mod_tw_operate_b(app_obj, msg, TW_FULL_SCREEN_TO_DEC);
	else
		return nmp_mod_tw_operate_resp_b(app_obj, msg, TW_FULL_SCREEN_TO_DEC);
}

NmpMsgFunRet
nmp_mod_tw_exit_full_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	if (MSG_GET_SRCPOS(msg) == BUSSLOT_POS_CU)
		return nmp_mod_tw_operate_b(app_obj, msg, TW_EXIT_FULL_SCREEN_TO_DEC);
	else
		return nmp_mod_tw_operate_resp_b(app_obj, msg, TW_EXIT_FULL_SCREEN_TO_DEC);
}


NmpMsgFunRet
nmp_mod_tw_update_ec_url_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_update_url *update_url;
	gint ret;

	self = (JpfModTw *)app_obj;
	update_url = (tw_update_url *)MSG_GET_DATA(msg);
	BUG_ON(!update_url);

	ret = jpf_tw_update_url(update_url);
	if (ret)
	{
		jpf_warning("jpf_tw_update_url failed\n");
	}

	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}

/*
void jpf_step_print_test(tw_run_step_request *req_info)
{
	jpf_print("<JpfTwMh> req_info->tw_id:%d", req_info->tw_id);
	jpf_print("<JpfTwMh> req_info->screen_id:%d", req_info->screen_id);
	jpf_print("<JpfTwMh> req_info->division_id:%d", req_info->division_id);
	jpf_print("<JpfTwMh> req_info->division_num:%d", req_info->division_num);
	jpf_print("<JpfTwMh> req_info->ec_name:%s", req_info->ec_name);
	jpf_print("<JpfTwMh> req_info->ec_domain_id:%s", req_info->ec_domain_id);
	jpf_print("<JpfTwMh> req_info->ec_guid:%s", req_info->ec_guid);
}
*/
NmpMsgFunRet
nmp_mod_tw_link_run_step_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	JpfModTw *self;
	tw_run_step_request *req_info;
	JpfCuExecuteRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));
	jpf_print("<JpfTwMh> nmp_mod_tw_link_run_step_b begin...");

	self = (JpfModTw *)app_obj;
	req_info = (tw_run_step_request *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	//jpf_step_print_test(req_info);

	ret = jpf_tw_run_action_step(req_info);
	if (ret)
	{
		jpf_warning("<JpfTwMh> jpf_tw_run_action_step failed, ec_guid:%s",
			req_info->ec_guid);
	}

	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


void
nmp_mod_tw_register_msg_handler(JpfModTw *self)
{
	NmpAppMod *super_self = (NmpAppMod*)self;

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_RUN_STEP,
		NULL,
		nmp_mod_tw_run_step_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_RUN_TOUR,
		NULL,
		nmp_mod_tw_run_tour_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_RUN_GROUP,
		NULL,
		nmp_mod_tw_run_group_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_STOP_TOUR,
		NULL,
		nmp_mod_tw_stop_tour_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_STOP_GROUP,
		NULL,
		nmp_mod_tw_stop_group_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_TOUR_RUN_STATE,
		NULL,
		nmp_mod_tw_if_run_tour_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_GROUP_RUN_STATE,
		NULL,
		nmp_mod_tw_if_run_group_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_STOP_DIVISION,
		NULL,
		nmp_mod_tw_stop_gpt_by_division_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_PLAY,
		NULL,
		nmp_mod_tw_play_resp_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_GET_SCR_STATE,
		NULL,
		nmp_mod_tw_change_direction_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_TW_CLEAR_DIVISION,
		NULL,
		nmp_mod_tw_clear_division_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_CHANGE_DIV_MODE,
		NULL,
		nmp_mod_tw_change_div_mode_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_FULL_SCREEN,
		NULL,
		nmp_mod_tw_full_screen_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MESSAGE_EXIT_FULL_SCREEN,
		NULL,
		nmp_mod_tw_exit_full_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MSG_TW_INFO_NOTIFY_UPDATE_EC_URL,
		NULL,
		nmp_mod_tw_update_ec_url_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MSG_LINK_RUN_STEP,
		NULL,
		nmp_mod_tw_link_run_step_b,
		0
	);
}

