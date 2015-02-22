#include "nmp_mod_ams.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_msg_ams.h"
#include "nmp_internal_msg.h"
#include "nmp_shared.h"
#include "nmp_ams_struct.h"
#include "nmp_ams_policy.h"


USING_MSG_ID_MAP(cms);
//#define AMS_MH_TEST


gint
jpf_mod_ams_action_handler(gint dst, gint msg_type, void *parm, guint size)
{
	jpf_warning("<JpfAmsMh> jpf_mod_ams_action_handler begin...");
	gint ret;
	JpfAppObj *self = (JpfAppObj *)jpf_get_mod_ams();

	ret = jpf_cms_mod_deliver_msg_2(self, dst, msg_type,
		parm, size);

	return ret;
}


static __inline__ gint
jpf_mod_ams_register(JpfModAms *self, JpfNetIO *io,  JpfMsgID msg_id,
    JpfAmsRegister *req, JpfAmsRegisterRes *res)
{
    gint ret;
    JpfID conflict;

    G_ASSERT(self != NULL && io != NULL && req != NULL && res != NULL);

    ret = jpf_mod_ams_new_ams(self, io,  req->ams_id, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = jpf_mod_ams_sync_req(self, msg_id, req,
         sizeof(JpfAmsRegister), res, sizeof(JpfAmsRegisterRes));

    return ret;
}


JpfMsgFunRet
jpf_mod_ams_register_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModAms *self;
    JpfNetIO *io;
    JpfAmsRegister *req_info;
    JpfAmsRegisterRes res_info;
    JpfMsgID msg_id;
    JpfGuestBase *ams_base;
    JpfAms *ams;
    JpfMsgAmsOnlineChange notify_info;
    gint ret;

    self = (JpfModAms*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&notify_info, 0, sizeof(notify_info));
    notify_info.ams_id[AMS_ID_LEN - 1] = 0;
    strncpy(notify_info.ams_id, req_info->ams_id, AMS_ID_LEN - 1);
    memset(&res_info, 0, sizeof(res_info));
    ret = jpf_mod_ams_register(self, io, msg_id, req_info, &res_info);
    if (ret)
    {
        jpf_print(
            "<JpfModAms> ams:%s register failed, err:%d",
            req_info->ams_id, -ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }

    ams_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!ams_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModAms> Can't find ams:'%s' in container, io timeout?", notify_info.ams_id);
    }
    else
   {
        ams = (JpfAms *)ams_base;
        ams->ams_state = STAT_AMS_ONLINE;
        jpf_mods_container_put_guest(self->container, ams_base);

         jpf_print(
             "<JpfModAms> ams:%s register ok",
             req_info->ams_id
         );

        strncpy(res_info.domain_id, jpf_get_local_domain_id(), DOMAIN_ID_LEN - 1);

        jpf_check_keepalive_time(&res_info.keep_alive_time);
        jpf_net_io_set_ttd(io, res_info.keep_alive_time*1000*TIMEOUT_N_PERIODS);

        notify_info.new_status = 1;
        jpf_mod_ams_change_ams_online_status(app_obj, notify_info);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;

}


JpfMsgFunRet
jpf_mod_ams_heart_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModAms *self;
    JpfNetIO *io;
    JpfGuestBase *ams_base;
    JpfAmsHeart *req_info;
    JpfAmsHeartRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;

    self = (JpfModAms*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    ams_base = jpf_mods_container_get_guest_2(self->container, req_info->ams_id);
    if (G_UNLIKELY(!ams_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModAms> amsId:%s No such guest.", req_info->ams_id);
    }
    else
    {
        jpf_get_utc_time(res_info.server_time);
        jpf_mods_container_put_guest(self->container, ams_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_ams_backward(JpfModAms *self, JpfSysMsg *msg, const gchar *id_str)
{
    JpfGuestBase *ams_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    ams_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!ams_base))
    {
        jpf_warning("<JpfModAms> deliver msg '%s' failed, AmsId:%s no such ams.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(ams_base);
    BUG_ON(!io);

    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, ams_base);

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_ams_backward_2(JpfModAms *self, JpfSysMsg *msg, const gchar *id_str, const gchar *session_id)
{
    JpfGuestBase *ams_base;
    JpfNetIO *io;
    gint msg_id;
    JpfErrRes      code;

    memset(&code, 0, sizeof(code));
    msg_id = MSG_GETID(msg);
    ams_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!ams_base))
    {
        jpf_warning("<JpfModAms> deliver msg '%s' failed, AmsId:%s no such ams.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        //jpf_sysmsg_destroy(msg);


        SET_CODE(&code.code, E_NOSUCHGUEST);
	if (session_id)
            memcpy(code.session, session_id, USER_NAME_LEN - 1);

        MSG_SET_DSTPOS(msg, BUSSLOT_POS_BSS);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));
        MSG_SET_RESPONSE(msg);
        return MFR_DELIVER_BACK;
    }

    io = IO_OF_GUEST(ams_base);
    BUG_ON(!io);

    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, ams_base);

    return MFR_DELIVER_AHEAD;
}


static int jpf_get_week_time(JpfTime *jpf_time, char *time)
{
	struct tm cur_tm;
	time_t cur_time;
	sscanf(time, "%d-%d-%d %d:%d:%d",
		&cur_tm.tm_year, &cur_tm.tm_mon, &cur_tm.tm_mday,
		&cur_tm.tm_hour, &cur_tm.tm_min, &cur_tm.tm_sec
	);
	cur_tm.tm_mon -= 1;
	cur_tm.tm_year -= 1900;

	cur_time = mktime(&cur_tm);
	localtime_r(&cur_time, &cur_tm);

	jpf_time->day_of_week = cur_tm.tm_wday;
	jpf_time->cur_time = cur_tm.tm_hour * 60 * 60 + cur_tm.tm_min * 60 + cur_tm.tm_sec;

	return 0;
}

static int jpf_mod_ams_get_alarm_info(JpfAlarmInfo *alarm_info,
	JpfSubmitAlarm *req_info)
{
	memset(alarm_info, 0, sizeof(JpfAlarmInfo));

	strncpy(alarm_info->alarm_guid.domain_id, req_info->domain_id,
		DOMAIN_ID_LEN - 1);
	strncpy(alarm_info->alarm_guid.guid, req_info->guid, MAX_ID_LEN - 1);
	alarm_info->alarm_type = 1 << (guint)(req_info->alarm_type);

	if (strlen(req_info->alarm_time) == 0)
	{
		jpf_warning("<JpfAmsMh>error, alarm_time=%s.", req_info->alarm_time);
		return -1;
	}
	strncpy(alarm_info->alarm_time, req_info->alarm_time, TIME_INFO_LEN - 1);
	return jpf_get_week_time(&alarm_info->time, req_info->alarm_time);
}


/*
 *	if return 0, destroy_msg need to be destroyed
 */
gint jpf_ams_get_action_info_from_dbs(JpfMsgAmsGetActionInfoRes **res,
	JpfAlarmInfo *alarm_info, JpfSysMsg **destroy_msg)
{
	JpfMsgAmsGetActionInfo req;
	JpfSysMsg *msg;
	gint ret;

	req.alarm_guid = alarm_info->alarm_guid;
	msg = jpf_sysmsg_new_2(MSG_AMS_GET_ACTION_INFO, &req,
		sizeof(JpfMsgAmsGetActionInfo), ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	ret = jpf_app_mod_sync_request((JpfAppMod*)jpf_get_mod_ams(), &msg);
	if (G_UNLIKELY(ret))
	{
		jpf_warning("<JpfModAms> request action info failed!");
		goto end;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning("<JpfModAms> request action info timeout!");
		return -E_TIMEOUT;
	}

	*res = MSG_GET_DATA(msg);
	BUG_ON(!(*res));
	if ((*res)->result < 0)
	{
		ret = -1;
		jpf_warning("<JpfModAms> request action info, result < 0!");
		goto end;
	}

	*destroy_msg = msg;
	return 0;

end:
	jpf_sysmsg_destroy(msg);
	return ret;
}


void jpf_mod_ams_do_alarm_action(JpfAlarmInfo *alarm_info)
{
	g_assert(alarm_info);
	JpfMsgAmsGetActionInfoRes *res = NULL;
	JpfSysMsg *msg = NULL;
	gint ret;

	ret = jpf_ams_find_gu_and_action(alarm_info);
	if (ret == 0)	//success
		return ;

	ret = jpf_ams_get_action_info_from_dbs(&res, alarm_info, &msg);
	if (ret != 0)	//failed
		return ;

	ret = jpf_ams_add_gu_action_info(&alarm_info->alarm_guid, res);
	if (ret != 0)	//failed
	{
		jpf_warning("<JpfModAms>jpf_ams_add_gu_action_info failed!");
	}

	if (msg)		//注意，res指向的为msg中的内存
		jpf_sysmsg_destroy(msg);

	ret = jpf_ams_find_gu_and_action(alarm_info);
	if (ret != 0)	//failed
	{
		jpf_warning("<JpfModAms>error:second find action failed!");
	}
}


void ams_alarm_info_print(JpfAlarmInfo *alarm_info)
{
	jpf_print("<JpfAmsMh>alarm happen!");
	jpf_print("<JpfAmsMh>alarm domain_id:%s.", alarm_info->alarm_guid.domain_id);
	jpf_print("<JpfAmsMh>alarm guid:%s.", alarm_info->alarm_guid.guid);
	jpf_print("<JpfAmsMh>alarm type:%d.", alarm_info->alarm_type);
	jpf_print("<JpfAmsMh>alarm week:%d, time:%ld.", alarm_info->time.day_of_week,
		alarm_info->time.cur_time);
}


JpfMsgFunRet
jpf_mod_ams_alarm_link_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModAms *self;
	JpfSubmitAlarm *req_info;
	JpfAlarmInfo alarm_info;
	gint ret = 0;

	self = (JpfModAms*)app_obj;
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_mod_ams_get_alarm_info(&alarm_info, req_info);
	if (ret < 0)
	{
		jpf_warning("<JpfModAms> get alarm info error.");
	}
	else
	{
#ifdef AMS_MH_TEST
		ams_alarm_info_print(&alarm_info);
#endif
		jpf_mod_ams_do_alarm_action(&alarm_info);
	}

	jpf_sysmsg_destroy(msg);

	return MFR_ACCEPTED;
}


JpfMsgFunRet
jpf_mod_ams_change_link_time_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModAms *self;
	JpfShareGuid *req_info;
	gint ret = 0;

	self = (JpfModAms*)app_obj;
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_ams_remove_alarm_node(req_info);
	if (ret == 0)
	{
		jpf_print("<JpfAmsMh> jpf_ams_remove_alarm_node success");
	}
	else
	{
		jpf_print("<JpfAmsMh> jpf_ams_remove_alarm_node failed");
	}

	jpf_sysmsg_destroy(msg);

	return MFR_ACCEPTED;
}


JpfMsgFunRet
jpf_mod_ams_del_alarm_link_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModAms *self;
	JpfShareGuid *req_info;
	gint ret = 0;

	self = (JpfModAms*)app_obj;
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_ams_remove_alarm_node(req_info);
	if (ret == 0)
	{
		jpf_print("<JpfAmsMh> jpf_ams_remove_alarm_node success");
	}
	else
	{
		jpf_print("<JpfAmsMh> jpf_ams_remove_alarm_node failed");
	}

	jpf_sysmsg_destroy(msg);

	return MFR_ACCEPTED;
}


JpfMsgFunRet
jpf_mod_ams_device_info_change_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModAms *self;
	JpfGuestBase *ams_base;
	gint msg_id;
	JpfAmsId *req_info = NULL;

	self = JPF_MODAMS(app_obj);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	msg_id = MSG_GETID(msg);
	ams_base = jpf_mods_container_get_guest_2(self->container, req_info->ams_id);
	if (G_UNLIKELY(!ams_base))
	{
		jpf_warning("<JpfModAms> deliver msg '%s' failed, AmsId:%s no such ams.",
			MESSAGE_ID_TO_STR(cms, msg_id), req_info->ams_id);
		jpf_sysmsg_destroy(msg);

		return MFR_ACCEPTED;
	}

	jpf_sysmsg_attach_io(msg, IO_OF_GUEST(ams_base));
	jpf_mods_container_put_guest(self->container, ams_base);

	return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_ams_get_device_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);
	JpfModAms *self;
	JpfNetIO *io;
	JpfGuestBase *ams_base;
	JpfAmsGetDeviceInfo *req_info;
	JpfAmsGetDeviceInfoRes res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModAms*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ams_base = jpf_mods_container_get_guest_2(self->container, req_info->ams_id);
	if (G_UNLIKELY(!ams_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModAms> amsId:%s No such guest.", req_info->ams_id);
	}
	else
	{
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
		jpf_mods_container_put_guest(self->container, ams_base);
		return MFR_DELIVER_AHEAD;
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_ams_get_device_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModAms *self;
    JpfAmsGetDeviceInfoRes *res_info;

    self = (JpfModAms*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    return jpf_mod_ams_backward(self, msg, res_info->ams_id);
}



void
jpf_mod_ams_register_msg_handler(JpfModAms *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_AMS_REGISTER,
        jpf_mod_ams_register_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_AMS_HEART,
        jpf_mod_ams_heart_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SUBMIT_ALARM,
        NULL,
        jpf_mod_ams_alarm_link_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MSG_CHANGE_LINK_TIME_POLICY,
        NULL,
        jpf_mod_ams_change_link_time_policy_b,
        0
    );

	jpf_app_mod_register_msg(
		super_self,
		MSG_DEL_ALARM_LINK,
		NULL,
		jpf_mod_ams_del_alarm_link_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_AMS_DEVICE_INFO_CHANGE,
		NULL,
		jpf_mod_ams_device_info_change_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_AMS_GET_DEVICE_INFO,
		jpf_mod_ams_get_device_info_f,
		jpf_mod_ams_get_device_info_b,
		0
	);
}

