#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nmp_mod_bss.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_sysctl.h"
#include "nmp_internal_msg.h"
#include "nmp_bss_struct.h"
#include "search_device.h"
#include "nmp_search_pu.h"

USING_MSG_ID_MAP(cms);

#define BSS_TIMEOUT    (45 * 1000)
#define POPEN_OPERATE_BASE	"system_network_config"
#define POPEN_OPERATE_BASE2	"system_generic_config"

#define POPEN_GET_NET_INTERFACE	POPEN_OPERATE_BASE" get_net_interface"
#define POPEN_GET_NETWORK_CONFIG	POPEN_OPERATE_BASE" get_network_config"
#define POPEN_SET_NETWORK_CONFIG	POPEN_OPERATE_BASE" set_network_config"
#define POPEN_SET_NETWORK_CONFIG_END	POPEN_OPERATE_BASE" set_network_config_end"
#define POPEN_GET_IPS_BESIDES		POPEN_OPERATE_BASE" get_ips_besides"

#define POPEN_GET_MODULES_FLAG	POPEN_OPERATE_BASE2" get_modules_flag"
#define POPEN_GET_MDS_STATE		POPEN_OPERATE_BASE2" get_mds_state"
#define POPEN_GET_MSS_STATE		POPEN_OPERATE_BASE2" get_mss_state"
#define POPEN_GET_IVS_STATE		POPEN_OPERATE_BASE2" get_ivs_state"
#define MAX_STATE_NUM		(0xffff)

#define QUERY_INFO(str, type)	(strstr(str, type))
#define GET_INFO_ADDR(str)		(strchr(str, ':') + 1)

/*
 *	operate res must be JpfMsgErrCode!
 */
#define JPF_DEAL_GET_BSS_USR_NAME(app_obj, msg, name, ret) do {	\
	ret = jpf_mod_bss_get_admin_name(app_obj, msg, name);	\
	if (ret) {	\
		return jpf_mod_deal_get_admin_name_failed(app_obj, msg, ret);	\
	}	\
} while (0)

#define JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, name) do {	\
	name[USER_NAME_LEN - 1] = '\0';	\
	strncpy(name, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);	\
} while (0)

#define JPF_CREATE_MSG_TO_LOG(msg_id, priv_p, size) do { \
	JpfSysMsg *_msg_to_log; \
	_msg_to_log = jpf_sysmsg_new_2(msg_id, priv_p, size, 0); \
	MSG_SET_DSTPOS(_msg_to_log, BUSSLOT_POS_LOG); \
	jpf_app_obj_deliver_out((JpfAppObj *)self, _msg_to_log); \
} while (0)


gint
jpf_mod_bss_get_admin_name(JpfAppObj *app_obj, JpfSysMsg *msg, gchar *name)
{
    JpfModBss *self;
    JpfGuestBase *bss_base;
    JpfNetIO *io;
    JpfMsgID msg_id;
    gint ret = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);
    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        name[USER_NAME_LEN - 1] = '\0';
        strncpy(name, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    return ret;
}


JpfMsgFunRet
jpf_mod_bss_forward(JpfModBss *self, JpfSysMsg *msg, const gchar *id_str)
{
    JpfErrRes	code;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    io = MSG_IO(msg);
    BUG_ON(!io);
    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        MSG_SET_RESPONSE(msg);
        memset(&code, 0, sizeof(code));
        SET_CODE(&code, ret);
	 if (id_str)
	 	strncpy(code.session, id_str, USER_NAME_LEN - 1);
        jpf_sysmsg_set_private_2(msg, &code, sizeof(code));

        return MFR_DELIVER_BACK;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);
    jpf_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_bss_backward(JpfModBss *self, JpfSysMsg *msg, const gchar *id_str)
{
    JpfGuestBase *bss_base;
    JpfNetIO *io;
    gint msg_id;

    msg_id = MSG_GETID(msg);
    bss_base = jpf_mods_container_get_guest_2(self->container, id_str);
    if (G_UNLIKELY(!bss_base))
    {
        jpf_warning("<JpfModBss> deliver msg '%s' failed, session:%s no such name.",
            MESSAGE_ID_TO_STR(cms, msg_id), id_str);
        jpf_sysmsg_destroy(msg);

        return MFR_ACCEPTED;
    }

    io = IO_OF_GUEST(bss_base);
    BUG_ON(!io);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_attach_io(msg, io);
    jpf_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_bss_forward_2(JpfAppObj *app_obj, JpfSysMsg *msg, gchar *name)
{
    gint ret;
    JpfMsgErrCode res_info;

    ret = jpf_mod_bss_get_admin_name(app_obj, msg, name);
    if (!ret)
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    memset(&res_info, 0, sizeof(res_info));
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_forward_to_log(JpfAppObj *app_obj, JpfSysMsg *msg, gchar *name)
{
    gint ret;
    JpfMsgErrCode res_info;

    ret = jpf_mod_bss_get_admin_name(app_obj, msg, name);
    if (!ret)
    {
        MSG_SET_DSTPOS(msg, BUSSLOT_POS_LOG);
        return MFR_DELIVER_AHEAD;
    }

    memset(&res_info, 0, sizeof(res_info));
    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_deal_get_admin_name_failed(JpfAppObj *app_obj, JpfSysMsg *msg, gint ret)
{
	JpfMsgErrCode res_info;

	memset(&res_info, 0, sizeof(res_info));
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static __inline__ gint
jpf_mod_bss_admin_login(JpfModBss *self, JpfNetIO *io,  JpfMsgID msg_id,
    JpfBssLoginInfo *admin, JpfBssLoginRes *res)
{
    gint ret;
    JpfID conflict;

    G_ASSERT(self != NULL && io != NULL && admin != NULL && res != NULL);

    ret = jpf_mod_bss_new_admin(self, io,  admin->admin_name, &conflict);
    if (G_UNLIKELY(ret))
        return ret;

    ret = jpf_mod_bss_sync_req(self, msg_id, admin,
         sizeof(JpfBssLoginInfo), res, sizeof(JpfBssLoginRes));

    return ret;
}


JpfMsgFunRet
jpf_mod_bss_login_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfBssLoginInfo *req_info;
    JpfBssLoginRes  res_info;
    JpfMsgID msg_id;
    gint ret;
    JpfResourcesCap res_cap;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    req_info = (JpfBssLoginInfo*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    ret = jpf_mod_bss_admin_login(self, io, msg_id, req_info, &res_info);
    if (ret)
    {
        jpf_warning(
            "<JpfModBss> admin:%s login failed, err:%d",
            req_info->admin_name, ret
        );

        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        jpf_app_obj_deliver_in((JpfAppObj*)self, msg);
        jpf_mod_acc_release_io((JpfModAccess*)self, io);
        jpf_mod_container_del_io(self->container, io);

        return MFR_ACCEPTED;
    }
    else
    {
        jpf_print(
            "<JpfModBss> admin:%s login ok",
            req_info->admin_name
        );
        memset(&res_cap, 0, sizeof(res_cap));
        jpf_mod_get_resource_cap(&res_cap);
        res_info.module_sets = res_cap.module_bits;
        jpf_net_set_io_ttd(io, BSS_TIMEOUT);
        SET_CODE(&res_info, -ret);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }
}


JpfMsgFunRet
jpf_mod_bss_heart_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfBssHeart *req_info;
    JpfBssHeartResp res_info;
    JpfMsgID msg_id;
    gint ret = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest_2(self->container, req_info->admin_name);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> admin name:%s No such guest.", req_info->admin_name);
    }
    else
    {
        //snprintf(res_info.server_time, TIME_INFO_LEN, "%d", time(NULL));
        jpf_get_utc_time(res_info.server_time);
        printf("res_info.server_time=%s\n",res_info.server_time);
        jpf_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_validata_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAdminInfo *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAdminInfo), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> validata admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> validata admin:%s ok",
                req_info->admin_name
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_validata_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfValidateUserGroup *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfValidateUserGroup), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> validata user group:%s exist, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> validata user group:%s inexist",
                req_info->group_name
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_validata_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfValidateUser *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfValidateUser), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> validata user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> validata user:%s ok",
                req_info->username
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_validata_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfValidateArea *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfValidateArea), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> validata area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> validata area:%s ok",
                req_info->area_name
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_validata_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfValidatePu *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfValidatePu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> validata pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> validata pu:%s ok",
                req_info->puid
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_general_cmd_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;
    JpfMsgID msg_id;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
    {
        jpf_warning("<JpfModBss> msg:%s failed, err:%d.",
            MESSAGE_ID_TO_STR(cms, msg_id), -RES_CODE(res_info));
    }
    else
    {
        jpf_print("<JpfModBss> msg:%s operator ok.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_general_modify_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    msg_id = MSG_GETID(msg);
    ret = RES_CODE(res_info);
    if (ret < 0)
        SET_CODE(res_info, -ret);

    ret = RES_CODE(res_info);
    if (ret && (ret != E_NODBENT))
    {
        jpf_warning("<JpfModBss> msg:%s failed, err:%d.",
            MESSAGE_ID_TO_STR(cms, msg_id), -RES_CODE(res_info));
    }
    else
    {
         SET_CODE(res_info, 0);
        jpf_print("<JpfModBss> msg:%s operator ok.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddAdmin *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = (JpfAddAdmin*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->admin_name, exclude_reg))
            || (regex_mached(req_info->password, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_admin_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddAdmin), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add admin:%s ok",
                req_info->admin_name
            );
        }
    add_admin_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddUserGroup *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->group_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_user_group_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddUserGroup), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add user group:%s failed, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add user group:%s ok",
                req_info->group_name
            );
        }
       add_user_group_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}



JpfMsgFunRet
jpf_mod_bss_add_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddUser *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    printf("=======================bss mh group_id:%d\n",req_info->group_id);
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->username, exclude_reg))
            || (regex_mached(req_info->password, exclude_reg))
            || (regex_mached(req_info->user_phone, exclude_reg))
            || (regex_mached(req_info->user_description, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_user_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddUser), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add user:%s ok",
                req_info->username
            );
        }

        add_user_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddArea *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        if ((regex_mached(req_info->area_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_area_string_format_wrong;
        }
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddArea), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add arear:%s ok",
                req_info->area_name
            );
        }

    add_area_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}

/*
JpfMsgFunRet
jpf_mod_bss_add_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddPu *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        if ((!regex_mached(req_info->puid, puid_reg))
			 || (regex_mached(req_info->pu_info, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)) )
        {
            ret = -E_STRINGFORMAT;
            goto add_pu_string_format_wrong;
        }
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddPu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add pu:%s ok",
                req_info->puid
            );
        }
   add_pu_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/


JpfMsgFunRet
jpf_mod_bss_add_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddPu *req_info;
    JpfMsgErrCode res_info;
    JpfResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->puid, puid_reg))
			 || (regex_mached(req_info->pu_info, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)) )
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        goto err_add_pu;
    }

    jpf_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    jpf_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = jpf_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
        {
             SET_CODE(&res_info, -ret);
             goto err_add_pu;
        }
    }
    else
        goto err_add_pu;

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);

err_add_pu:
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    jpf_warning(
        "<JpfModBss> add pu:%s failed, err:%d",
        req_info->puid, RES_CODE(&res_info)
    );
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfAddPuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
    {
	    jpf_warning("<JpfModBss> admin:%s add pu failed, err:%d",
	    	res_info->bss_usr, -RES_CODE(res_info));
    }
    else
	    jpf_warning("<JpfModBss> admin:%s add pu ok", res_info->bss_usr);

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_add_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddGu *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
       if ((!regex_mached(req_info->guid, guid_reg))
			 || (regex_mached(req_info->gu_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_gu_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddGu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add gu:%s ok",
                req_info->guid
            );
        }
      add_gu_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_add_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddGu *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (regex_mached(req_info->gu_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> add gu:%s-* failed, err:%d",
                req_info->puid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    if (RES_CODE(res_info))
	    jpf_warning("<JpfModBss> add gu failed, err:%d", -RES_CODE(res_info));
    else
	    jpf_print("<JpfModBss> add gu ok");

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_mds_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddMds *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mds_id, mds_reg))
            || (regex_mached(req_info->mds_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_mds_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddMds), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add mds:%s failed, err:%d",
                req_info->mds_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add mds:%s ok",
                req_info->mds_name
            );
        }
    add_mds_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_mds_ip_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddMdsIp *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddMdsIp), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add mds ip:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add mds ip:%s ok",
                req_info->mds_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddMss *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mss_id, mss_reg))
            || (regex_mached(req_info->mss_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto add_mss_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddMss), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add mss:%s failed, err:%d",
                req_info->mss_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add mss:%s ok",
                req_info->mss_name
            );
        }
    add_mss_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_gu_to_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddGuToUser *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret,size;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    size =  MSG_DATA_SIZE(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, size, NULL, 0);
        if (ret)
            jpf_warning( "<JpfModBss> add gu to user: failed, err:%d", ret);
        else
            jpf_print( "<JpfModBss> add gu to user: ok" );

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_tw_to_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddTwToUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tw_to_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tour_to_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddTourToUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tour_to_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_defence_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddDefenceArea *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddDefenceArea), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add defence area:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add defence area:%d ok",
                req_info->defence_area_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_defence_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddDefenceMap *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddDefenceMap), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add defence map failed,defence id:%d, map id:%d,err:%d",
                req_info->defence_area_id, req_info->map_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add defence map ok,defence id:%d, map id:%d",
                req_info->defence_area_id, req_info->map_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_defence_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddDefenceGu *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddDefenceGu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add defence gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add defence gu:%s ok",
                req_info->guid
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_map_href_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfSetMapHref *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfSetMapHref), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add map Href:%d failed, err:%d",
                req_info->dst_map_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add map Href:%d ok",
                req_info->dst_map_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_del_alarm_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelAlarmPolicy *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfDelAlarmPolicy), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> set auto del alarm policy failed, err:%d",
                 ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> set auto del alarm policy ok"
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddTw *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddTw), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add tw:%s failed, err:%d",
                req_info->tw_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add tw %s ok",
                req_info->tw_name
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_screen_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddScreen *req_info;
    JpfMsgErrCode res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms
            , msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfAddScreen), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> add tw %d screen failed, err:%d",
                req_info->tw_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> add tw %d screen ok",
                req_info->tw_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tour_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddTourStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_tour_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_group_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_ivs_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfAddIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_validate_gu_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfValidateGuMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_validate_gu_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

JpfMsgFunRet
jpf_mod_bss_config_group_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfConfigGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_time_policy_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkTimePolicyConfig *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link time policy config:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_record_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkRecord *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link record:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_IO_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkIO *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link IO:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_snapshot_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkSnapshot *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link snapshot:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_preset_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkPreset *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link preset:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkStepConfig *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link step:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkTourConfig *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link tour:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkGroupConfig *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link group:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_link_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfLinkMap *req_info;
    JpfMsgErrCode res_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->guid, guid_reg))
			 || (!regex_mached(req_info->domain, domain_reg))
			 || (!regex_mached(req_info->link_guid, guid_reg))
			 || (!regex_mached(req_info->link_domain, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        MSG_SET_RESPONSE(msg);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        jpf_warning(
                "<JpfModBss> link map:%s-* failed, err:%d",
                req_info->guid, E_STRINGFORMAT
        );
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddAdmin *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s failed,No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->password, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_admin_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfAddAdmin), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> modify admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> modify admin:%s ok",
                req_info->admin_name
            );
        }

     modify_admin_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfUserGroupInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
	JpfSysMsg *msg_del_user;
	JpfMsgDelUserGroup del_group;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

	memset(&del_group, 0, sizeof(del_group));
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->group_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_user_group_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfUserGroupInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify user group:%s failed, err:%d",
                req_info->group_name, ret
            );
        }
        else
        {

            jpf_print(
                      "<JpfModBss> modify user group:%s ok",
                      req_info->group_name
                  );

	   		sprintf(del_group.group_ids,"'%d",req_info->group_id);
	    	del_group.group_permissions = req_info->group_permissions;
	    	del_group.group_rank = req_info->group_rank;
			msg_del_user = jpf_sysmsg_new_2(MSG_DEL_USER_GROUP, &del_group, sizeof(del_group), ++msg_seq_generator);
			if (G_UNLIKELY(!msg_del_user))
			{
				jpf_mods_container_put_guest(self->container, bss_base);
				return MFR_DELIVER_BACK;
			}

			MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
			jpf_mod_bss_deliver_out_msg(app_obj, msg_del_user);

	    	ret = 0;
        }
        modify_user_group_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfUserInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfSysMsg *msg_del_user;
    JpfMsgDelUser user;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&user, 0, sizeof(user));
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
	 jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->password, exclude_reg))
            || (regex_mached(req_info->user_phone, exclude_reg))
            || (regex_mached(req_info->user_description, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_user_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self, msg_id, req_info,
              sizeof(JpfUserInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {

            jpf_print(
                "<JpfModBss> modify user:%s ok",
                req_info->username
            );

            if (ret != -E_NODBENT)
            {
                strncpy(user.name, req_info->username, USER_NAME_LEN -1);
                msg_del_user = jpf_sysmsg_new_2(MSG_DEL_USER, &user, sizeof(user), ++msg_seq_generator);
                if (G_UNLIKELY(!msg_del_user))
                {
                    jpf_mods_container_put_guest(self->container, bss_base);
                    return MFR_DELIVER_BACK;
                }

                MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
                jpf_mod_bss_deliver_out_msg(app_obj, msg_del_user);
            }

            ret = 0;
        }
      modify_user_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_domain_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDomainInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    JpfSysMsg *msg_modify_domain;
    JpfModifyDomain domain;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->domain_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_domain_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfDomainInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify domain:%s failed, err:%d",
                req_info->domain_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> modify domain:%s ok",
                req_info->domain_id
            );

            if (ret != -E_NODBENT)
            {
                strncpy(domain.dm_name, req_info->domain_name, DOMAIN_NAME_LEN -1);
                msg_modify_domain = jpf_sysmsg_new_2(MESSAGE_NOTIFY_MODIFY_DOMAIN, &domain, sizeof(domain), ++msg_seq_generator);
                if (G_UNLIKELY(!msg_modify_domain))
                {
                    jpf_mods_container_put_guest(self->container, bss_base);
                    return MFR_DELIVER_BACK;
                }

                MSG_SET_DSTPOS(msg_modify_domain, BUSSLOT_POS_CU);
                jpf_mod_bss_deliver_out_msg(app_obj, msg_modify_domain);
            }

	     ret = 0;
        }

        modify_domain_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_modify_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAreaInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->area_name, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_area_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfAreaInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify area:%s failed, err:%d",
                req_info->area_name, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> add or modify area:%s ok",
                req_info->area_name
            );
        }
   modify_area_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_modify_manufacturer_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddModifyManufacturer *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((!regex_mached(req_info->mf_id, manufactur_reg))
		|| (regex_mached(req_info->mf_name, mf_exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_manufacturer_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info,
              sizeof(JpfAddModifyManufacturer), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> modify manufacturer:%s failed, err:%d",
                req_info->mf_id, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify manufacturer:%s ok",
                req_info->mf_id
            );
        }
        modify_manufacturer_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfPuInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if ((regex_mached(req_info->pu_info, exclude_reg)))
        {
            ret = -E_STRINGFORMAT;
            goto modify_pu_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfPuInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify pu:%s failed, err:%d",
                req_info->puid, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify pu:%s ok",
                req_info->puid
            );
        }
      modify_pu_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->gu_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_gu_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfGuInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> modify gu:%s ok",
                req_info->guid
            );
            ret = 0;
        }
        modify_gu_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_mds_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfMdsInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->mds_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_mds_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfMdsInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify mds:%s failed, err:%d",
                req_info->mds_name, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify mds:%s ok",
                req_info->mds_name
            );
        }

   modify_mds_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfMssInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        if (regex_mached(req_info->mss_name, exclude_reg))
        {
            ret = -E_STRINGFORMAT;
            goto modify_mss_string_format_wrong;
        }

        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfMssInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify mss:%s failed, err:%d",
                req_info->mss_name, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify mss:%s ok",
                req_info->mss_name
            );
        }

   modify_mss_string_format_wrong:
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_defence_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDefenceAreaInfo *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfDefenceAreaInfo), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify defence area id:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify defence area id:%d ok",
                req_info->defence_area_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_defence_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfModifyDefenceGu *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfModifyDefenceGu), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify defence gu:%d failed, err:%d",
                req_info->map_id, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify defence gu:%d ok",
                req_info->map_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_map_href_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfModifyMapHref *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfModifyMapHref), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify map href:%d failed, err:%d",
                req_info->src_map_id, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify map href:%d ok",
                req_info->src_map_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfModifyTw *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfModifyTw), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify tw:%s failed, err:%d",
                req_info->tw_name, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify tw:%s ok",
                req_info->tw_name
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_screen_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfModifyScreen *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, sizeof(JpfModifyScreen), NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify screen:%d failed, err:%d",
                req_info->screen_id, ret
            );
        }
        else
        {
            ret = 0;
            jpf_print(
                "<JpfModBss> modify screen:%d ok",
                req_info->screen_id
            );
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_modify_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModifyTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModifyGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_group_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModifyGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_group_step_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModifyGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_time_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkTimePolicy *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_record_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_IO_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkIO *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_snapshot_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_preset_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkPreset *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkStep *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkTour *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkGroup *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_link_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyLinkMap *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ivs_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfIvsInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelAdmin *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfSysMsg *msg_notify;
    JpfForceUsrOffline notify_info;
    JpfMsgID msg_id;
    gint ret;
	gchar *username;
	gchar *name;
	gchar  offline_name[USER_NAME_LEN];

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = (JpfDelAdmin*)MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfDelAdmin), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del admin:%s failed, err:%d",
                req_info->admin_name, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> del admin:%s ok",
                req_info->admin_name
            );

            memset(&notify_info, 0, sizeof(notify_info));
            //strncpy(notify_info.reason, "user has been delete", OFFLINE_REASON_LEN - 1);
            notify_info.reason = 0;
            msg_notify = jpf_sysmsg_new_2(MESSAGE_FORCE_USR_OFFLINE,
                 &notify_info, sizeof(notify_info), ++msg_seq_generator);
            if (G_UNLIKELY(!msg_notify))
            {
                jpf_sysmsg_destroy(msg);
                jpf_warning("create sysmsg:%d err", MESSAGE_FORCE_USR_OFFLINE);
            }

            name = req_info->admin_name;
            printf("##################name : %s\n",name);
            while((username = strsep(&name, ",")) != NULL)
            {
                memset(offline_name, 0, USER_NAME_LEN);
                strncpy(offline_name,&username[1],strlen(username) - 2);
                offline_name[USER_NAME_LEN - 1] = 0;
                printf("&&&&&&&&&&&&&&&username : %s,offline_name:%s\n",username,offline_name);
                jpf_mod_bss_force_usr_offline(self, offline_name, msg_notify);
            }

	        jpf_sysmsg_destroy(msg_notify);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelUserGroup *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
	JpfSysMsg *msg_del_user;
	JpfMsgDelUserGroup del_group;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&del_group, 0, sizeof(del_group));
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelUserGroup), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del user group:%s failed, err:%d",
                req_info->group_id, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> del user group:%s ok",
                req_info->group_id
            );

			strncpy(del_group.group_ids, req_info->group_id, MULTI_NAME_LEN -1);
			msg_del_user = jpf_sysmsg_new_2(MSG_DEL_USER_GROUP, &del_group, sizeof(del_group), ++msg_seq_generator);
			if (G_UNLIKELY(!msg_del_user))
			{
				jpf_mods_container_put_guest(self->container, bss_base);
				return MFR_DELIVER_BACK;
			}

			MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
			jpf_mod_bss_deliver_out_msg(app_obj, msg_del_user);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelUser *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    JpfSysMsg *msg_del_user;
    JpfMsgDelUser user;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&user, 0, sizeof(user));
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelUser), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del user:%s failed, err:%d",
                req_info->username, ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> del user:%s ok",
                req_info->username
            );

	    strncpy(user.name, req_info->username, USER_NAME_LEN -1);
           msg_del_user = jpf_sysmsg_new_2(MSG_DEL_USER, &user, sizeof(user), ++msg_seq_generator);
           if (G_UNLIKELY(!msg_del_user))
           {
              jpf_mods_container_put_guest(self->container, bss_base);
              return MFR_DELIVER_BACK;
           }

           MSG_SET_DSTPOS(msg_del_user, BUSSLOT_POS_CU);
           jpf_mod_bss_deliver_out_msg(app_obj, msg_del_user);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelArea *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfDelArea), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del area:%d failed, err:%d",
                req_info->area_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del area:%d ok", req_info->area_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelPu *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret, size;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        size = sizeof(JpfDelPu) + req_info->count*sizeof(JpfPuPoint);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info,size, NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del pu failed, err:%d",
                 ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del pu ok");
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelGu *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelGu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del gu:%s ok", req_info->guid);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_manufacturer_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelManufacturer *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(
	     self, msg_id,  req_info,
            sizeof(JpfDelManufacturer), NULL, 0
        );
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del manufacturer:%s failed, err:%d",
                req_info->mf_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del manufacturer:%s ok", req_info->mf_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_mds_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelMds *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelMds), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del mds:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del mds:%s ok", req_info->mds_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_mds_ip_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelMdsIp *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfDelMdsIp), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del mds ip:%s failed, err:%d",
                req_info->mds_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del mds ip:%s ok", req_info->mds_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelMss *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelMss), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del mss:%s failed, err:%d",
                req_info->mss_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del mss:%s ok", req_info->mss_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_defence_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelDefenceArea *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelDefenceArea), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del defence area:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del defence area:%d ok", req_info->defence_area_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_defence_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelDefenceMap *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelDefenceMap), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del defence map:%d failed, err:%d",
                req_info->defence_area_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del defence map:%d ok", req_info->defence_area_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_defence_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelDefenceGu *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelDefenceGu), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del defence gu:%s failed, err:%d",
                req_info->guid, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del defence gu:%s ok", req_info->guid);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_map_href_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelMapHref *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelMapHref), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del map Href:%d failed, err:%d",
                req_info->dst_map_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del map Href:%d ok", req_info->dst_map_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelTw *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelTw), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del tw:%d failed, err:%d",
                req_info->tw_id, ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del tw:%d ok", req_info->tw_id);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_screen_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelScreen *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id,  req_info, sizeof(JpfDelScreen), NULL, 0);
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del screen failed, err:%d",
                 ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del screen ok");
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_del_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_group_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_group_step_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_time_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkTimePolicy *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_record_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkRecord *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_IO_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkIO *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_snapshot_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkSnapshot *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_preset_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkPreset *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_link_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelLinkMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_ivs_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfDelIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


/*
JpfMsgFunRet
jpf_mod_bss_query_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryAdmin *req_info;
    JpfQueryAdminRes  *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_warning("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryAdmin), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query admin list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("query admin failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_admin_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryAdmin *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_admin_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAdminRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


/*
JpfMsgFunRet
jpf_mod_bss_query_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryUserGroup *req_info;
    JpfQueryUserGroupRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryUserGroup), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query user group list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query user group failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }
    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_user_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUserGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryUserGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryUser *req_info;
    JpfQueryUserRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_warning("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryUser), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query user list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query user failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_user_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUser *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryUserRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_domain_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryDomain *req_info;
    JpfQueryDomainRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_warning("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }

    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryDomain), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query domain ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query domain failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
          jpf_sysmsg_destroy(msg);
	   return MFR_ACCEPTED;
    }
    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_domain_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryDomain *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_domain_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryDomainRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryArea *req_info;
    JpfQueryAreaRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryArea), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query area list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query area failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryArea *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAreaRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryPu *req_info;
    JpfQueryPuRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryPu), &size);
        if (res_info)
       {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query pu list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query pu failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/


JpfMsgFunRet
jpf_mod_bss_query_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryPu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryPuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryGu *req_info;
    JpfQueryGuRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
        BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryGu), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query pu list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query pu failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_manufacturer_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryManufacturer *req_info;
    JpfQueryManufacturerRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_warning("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryManufacturer), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query manufacturer list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query manufacturer failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_manufacturer_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryManufacturer *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_manufacturer_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryManufacturerRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_mds_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryMds *req_info;
    JpfQueryMdsRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryMds), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_mds_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUserGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_mds_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryMdsRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_mds_ip_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryMdsIp *req_info;
    JpfQueryMdsIpRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryMdsIp), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_mds_ip_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryMdsIp *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_mds_ip_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryMdsIpRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryMss *req_info;
    JpfQueryMssRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryMss), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryMss *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryMssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_defence_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryDefenceArea *req_info;
    JpfQueryDefenceAreaRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryDefenceArea), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
           // jpf_print("<JpfModBss> query defence area list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query defence area failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_defence_area_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryDefenceArea *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_defence_area_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryDefenceAreaRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_defence_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryDefenceMap*req_info;
    JpfQueryDefenceMapRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryDefenceMap), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query defence map list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query defence map failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_defence_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryDefenceMap *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_defence_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryDefenceMapRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_defence_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryDefenceGu *req_info;
    JpfQueryDefenceGuRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryDefenceGu), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query defence gu list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_defence_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryDefenceGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_defence_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryDefenceGuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_map_href_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryMapHref *req_info;
    JpfQueryMapHrefRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryMapHref), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query defence gu list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_map_href_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryMapHref *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_map_href_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryMapHrefRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}
/*
JpfMsgFunRet
jpf_mod_bss_query_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryTw *req_info;
    JpfQueryTwRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryTw), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query tw list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query tw failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryTw *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryTwRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_screen_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryScreen *req_info;
    JpfQueryScreenRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryScreen), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query screen list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query screen failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_screen_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryScreen *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_screen_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryScreenRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}

/*
JpfMsgFunRet
jpf_mod_bss_query_screen_division_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryScrDiv *req_info;
    JpfQueryScrDivRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryScrDiv), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            //jpf_print("<JpfModBss> query screen division list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query screen division failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_screen_division_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryScrDiv *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_screen_division_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryScrDivRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryTourRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_tour_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryTourStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_tour_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryTourStepRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryGroup *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryGroupStep *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGroupStepRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryGroupStepInfo *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGroupStepInfoRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_div_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryGroupStepDiv *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_group_step_div_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGroupStepDivRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}
/*
JpfMsgFunRet
jpf_mod_bss_query_record_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryRecordPolicy *req_info;
    JpfQueryRecordPolicyRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryRecordPolicy), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            res_info->type = req_info->type;
            //jpf_print("<JpfModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("<JpfModBss> query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/


JpfMsgFunRet
jpf_mod_bss_query_record_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryRecordPolicy *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_record_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryRecordPolicyRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_record_policy_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfRecordPolicyConfig *req_info;
    JpfMsgErrCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret, i;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);
    size = MSG_DATA_SIZE(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self,  msg_id, req_info, size, NULL, 0);
        if (ret && (ret != -E_NODBENT))
        {
            jpf_warning(
                "<JpfModBss> modify policy failed, err:%d",
                 ret
            );
        }
        else
        {
            jpf_print(
                "<JpfModBss> modify policy ok"
            );

            if (ret != -E_NODBENT)
            {
                JpfNotifyPolicyChange policy_change;
                memset(&policy_change, 0, sizeof(policy_change));
                switch (req_info->type ){
                case 0:
                    for (i = 0; i < req_info->gu_count; i++)
                    {
                        strncpy(policy_change.guid, req_info->record_policy[i].guid, MAX_ID_LEN -1);
                        strncpy(policy_change.domain_id, req_info->record_policy[i].domain_id, DOMAIN_ID_LEN -1);
                        strncpy(policy_change.mss_id, req_info->record_policy[i].mss_id, MSS_ID_LEN -1);
                        jpf_mod_bss_notify_policy_change(app_obj, &policy_change, sizeof(policy_change));
                    }
                    break;
                case 1:
                    strncpy(policy_change.mss_id, req_info->mss_id, MSS_ID_LEN -1);
                    policy_change.all_changed = 1;
                    jpf_mod_bss_notify_policy_change(app_obj, &policy_change, sizeof(policy_change));
                    break;
                }
            }

	     ret = 0;
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


/*
JpfMsgFunRet
jpf_mod_bss_query_user_own_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryUserOwnGu *req_info;
    JpfQueryUserOwnGuRes *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_error("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);

        res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryUserOwnGu), &size);
        if (res_info)
        {
             ret = RES_CODE(res_info);
             //jpf_print("<JpfModBss> query mds list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("query mds failed, err:%d", ret);
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if(res_info)
    {
        MSG_SET_RESPONSE(msg);
        SET_CODE(res_info, -ret);
        jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
        return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}
*/

JpfMsgFunRet
jpf_mod_bss_query_user_own_gu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUserOwnGu *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_own_gu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryUserOwnGuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_own_tw_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUserOwnTw *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_own_tw_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryUserOwnTwRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_own_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryUserOwnTour *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_user_own_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryUserOwnTourRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_del_alarm_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryDelAlarmPolicyRes res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = jpf_mod_bss_sync_req(self, msg_id, NULL,
            0, &res_info, sizeof(res_info));
        if (ret)
        {
            jpf_warning("<JpfModBss> query del_alarm_policy failed, err:%d", ret);
        }
        else
        {
	     //jpf_print("<JpfModBss> query del_alarm_policy ok");
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}

JpfMsgFunRet
jpf_mod_bss_query_system_time_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryServerTimeRes res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        ret = snprintf( res_info.system_time, TIME_LEN - 1, "%ld", time(NULL));
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, 0);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_system_time_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfSetServerTime *req_info;
    JpfSetServerTimeRes res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    memset(&res_info, 0, sizeof(res_info));

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
        req_info = MSG_GET_DATA(msg);
    	 BUG_ON(!req_info);
        ret = jpf_set_system_time(req_info->system_time, req_info->time_zone);
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);

    /*** sent msg to mod_log ***/
    strncpy(res_info.time_zone, req_info->time_zone, TIME_ZONE_LEN - 1);	//must before jpf_sysmsg_set_private_2
    strncpy(res_info.system_time, req_info->system_time, TIME_LEN - 1);
    JPF_CREATE_MSG_TO_LOG(MSG_LOG_SET_SYSTEM_TIME, &res_info, sizeof(res_info));

    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_link_time_policy_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkTimePolicy *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_time_policy_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkTimePolicyRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
	    SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_record_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkRecord *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_record_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkRecordRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_IO_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkIO *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_IO_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkIORes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_snapshot_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkSnapshot *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_snapshot_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkSnapshotRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_preset_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkPreset *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_preset_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkPresetRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_step_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkStep *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_step_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkStepRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_tour_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkTour *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_tour_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkTourRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkGroup *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkGroupRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_map_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLinkMap *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_link_map_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLinkMapRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ivs_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryIvs *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ivs_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryIvsRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_database_backup_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModBss *self;
    JpfNetIO *io;
    JpfDbBackup *req_info;
    JpfMsgErrCode res_info;
    JpfMsgID msg_id;
    struct stat file_state;
    gint ret = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    if (stat(jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),&file_state) ||
    		!S_ISDIR(file_state.st_mode))
    {
        ret = -ENOENT;
        jpf_warning("<JpfModBss> backup directory:%s does not exist.",
        			jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH));
        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
        return MFR_DELIVER_BACK;
    }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_database_backup_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

	JpfModBss *self;
	JpfBssRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_database_import_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    G_ASSERT(app_obj != NULL && msg != NULL);

    JpfModBss *self;
    JpfNetIO *io;
    JpfDbImport *req_info;
    JpfDbImportRes res_info;
    JpfMsgID msg_id;
    struct stat file_state;
    gint ret = 0;
    char filename[STRING_LEN] = {0};

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));

    sprintf(filename, "%s/%s", jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH),
			req_info->filename);
     if (stat(filename, &file_state) ||
 				!S_ISREG(file_state.st_mode))
     {
         ret = -ENOENT;
         jpf_warning("<JpfModBss> backup directory:%s does not exist.",
						jpf_get_sys_parm_str(SYS_PARM_DBBACKUPPATH));

        MSG_SET_RESPONSE(msg);
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
     }

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_database_import_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfDbImportRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	if (RES_CODE(res_info) == 0)
      {
        sleep(2);
        exit(-1);
      }

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


static void comma_to_space(char *str, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (str[i] == ',')
			str[i] = ' ';
	}
}

static int check_interface_valid(char *eth_str)
{
	if (strlen(eth_str) == strlen("eth0") &&
		strstr(eth_str, "eth") &&
		eth_str[3] >= '0' && eth_str[3] <= '9')
		return 0;

	return -1;
}


JpfMsgFunRet
jpf_mod_bss_get_net_interface_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetNetInterfaceConfigRes res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	char res[STRING_LEN] = {0};

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
		goto end;
	}
	jpf_mods_container_put_guest(self->container, bss_base);

	snprintf(query_buf, STRING_LEN - 1, POPEN_GET_NET_INTERFACE);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		jpf_warning("<JpfModBss> get net interface, popen failed");
		ret = -errno;
		goto end;
	}

	int first = 1;
	int str_len = 0;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';
		if (first)
		{
			str_len = snprintf(res, STRING_LEN, "%s", buffer);
			first = 0;
		}
		else
			str_len += snprintf(&res[str_len], STRING_LEN - str_len, ",%s", buffer);

		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);
	res_info.network_interface[NET_INTERFACE_LEN - 1] = '\0';
	strncpy(res_info.network_interface, res, NET_INTERFACE_LEN - 1);

end:
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static void get_net_info(JpfGetNetworkConfigRes *res_info, int ip_i, char *buffer)
{
	IpInfo *cur_ip_info = &res_info->ip_list[ip_i];

	if (QUERY_INFO(buffer, "ip"))
	{
		cur_ip_info->ip[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->ip, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "mask"))
	{
		cur_ip_info->netmask[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->netmask, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "gateway"))
	{
		cur_ip_info->gateway[MAX_IP_LEN - 1] = '\0';
		strncpy(cur_ip_info->gateway, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
	}
	else if (QUERY_INFO(buffer, "dns"))
	{
		res_info->dns[MAX_IP_LEN - 1] = '\0';
		if (res_info->dns[0] == '\0')
			strncpy(res_info->dns, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
		else
		{
			res_info->dns[strlen(res_info->dns)] = ',';
			strncpy(&res_info->dns[strlen(res_info->dns)], GET_INFO_ADDR(buffer),
				MAX_IP_LEN - 1 - strlen(res_info->dns));
			res_info->dns[MAX_IP_LEN - 1] = '\0';
		}
	}
}

JpfMsgFunRet
jpf_mod_bss_get_network_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetNetworkConfig *req_info;
	JpfGetNetworkConfigRes *res_info = NULL;
	JpfMsgID msg_id;
	gint ret = 0;
	gint size = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	req_info = (JpfGetNetworkConfig *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	msg_id = MSG_GETID(msg);
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
		goto end;
	}
	jpf_mods_container_put_guest(self->container, bss_base);

	jpf_warning("<JpfModBss> get network configure");
	if (check_interface_valid(req_info->network_interface) != 0)
	{
		ret = -E_STRINGFORMAT;
		jpf_warning("<JpfModBss> interface msg error, network_interface:%s",
			req_info->network_interface);
		goto end;
	}

	snprintf(query_buf, STRING_LEN - 1, "%s %s", POPEN_GET_NETWORK_CONFIG,
		req_info->network_interface);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		ret = -errno;
		jpf_warning("<JpfModBss> get network configure, popen failed\n");
		goto end;
	}

	int ip_i = -1;
	int first_line = 1;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (first_line)
		{
			int count = atoi(buffer);
			if (count < 0 || count > 255)
			{
				ret = -E_STRINGFORMAT;
				jpf_warning("<JpfModBss> error: count = %d", count);
				pclose(fp);
				goto end;
			}

			size = sizeof(JpfGetNetworkConfigRes) + sizeof(IpInfo) * count;
			res_info = (JpfGetNetworkConfigRes *)malloc(size);
			if (!res_info)
			{
				ret = -ENOMEM;
				jpf_warning("<JpfModBss> no memory!");
				size = 0;
				pclose(fp);
				goto end;
			}
			memset(res_info, 0, size);
			res_info->count = count;
			first_line = 0;
			continue;
		}

		if (QUERY_INFO(buffer, "IPINFO"))	//一组ip信息的开始
		{
			ip_i++;
			if (ip_i >= res_info->count)
				break;
			continue;
		}
		get_net_info(res_info, ip_i, buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);

end:
	MSG_SET_RESPONSE(msg);
	if (!res_info)
	{
		res_info = (JpfGetNetworkConfigRes *)malloc(sizeof(JpfGetNetworkConfigRes));
		if (!res_info)
		{
			jpf_sysmsg_destroy(msg);
			return MFR_ACCEPTED;
		}
	}
	SET_CODE(res_info, -ret);
	jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);

	return MFR_DELIVER_BACK;
}


static void get_network(char *network, char *ip, char *mask)
{
	unsigned int ip_uint = inet_addr(ip);
	unsigned int mask_uint = inet_addr(mask);
	unsigned int network_uint = ip_uint & mask_uint;
	char *tmp = NULL;

	struct in_addr inaddr;
	inaddr.s_addr = network_uint;
	tmp = inet_ntoa(inaddr);

	strncpy(network, tmp, MAX_IP_LEN);
}

static void get_broadcast(char *broadcast, char *ip, char *mask)
{
	unsigned int ip_uint = inet_addr(ip);
	unsigned int mask_uint = inet_addr(mask);
	unsigned int broadcast_uint = ip_uint | (~mask_uint);
	char *tmp = NULL;

	struct in_addr inaddr;
	inaddr.s_addr = broadcast_uint;
	tmp = inet_ntoa(inaddr);

	strncpy(broadcast, tmp, MAX_IP_LEN);
}


static int if_ip_conflict(char *ip1, char *mask1, char *ip2, char *mask2)
{
	//jpf_print("ip1:%s, mask1:%s, ip2:%s, mask2:%s", ip1, mask1, ip2, mask2);
	unsigned int ip1_uint = inet_addr(ip1), ip2_uint = inet_addr(ip2);
	unsigned int mask1_uint = inet_addr(mask1), mask2_uint = inet_addr(mask2);
	if (mask1_uint != mask2_uint)
		return 0;
	if ((ip1_uint & mask1_uint) == (ip2_uint & mask2_uint))
		return 1;
	return 0;
}


static int 
__check_netmask_valid(char *mask)
{
	unsigned int mask_host = ntohl(inet_addr(mask));
	unsigned int temp = ~mask_host + 1;

	if (mask_host != 0 && (temp & (temp - 1)) == 0)
		return 0;
	return 1;
}


static int check_netmask_valid(JpfSetNetworkConfig *req_info)
{
	int ip_i;

	for (ip_i = 0; ip_i < req_info->count; ip_i++)
	{
		IpInfo *cur_ip_info = &req_info->ip_list[ip_i];
		if (__check_netmask_valid(cur_ip_info->netmask) != 0)
		{
			jpf_warning("<JpfModBss> netmask error, netmask:%s", 
				cur_ip_info->netmask);
			return -E_NETMASKERR;
		}
	}

	return 0;
}


static int check_ip_conflict(JpfSetNetworkConfig *req_info)
{
	char *eth_str = req_info->network_interface;
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	char tmp_ip[MAX_IP_LEN] = {0}, tmp_mask[MAX_IP_LEN] = {0};
	int start_check = 0;
	FILE *fp;

	snprintf(query_buf, STRING_LEN - 1, "%s %s", POPEN_GET_IPS_BESIDES,
		eth_str);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		jpf_warning("<JpfModBss> get_ips_besides, popen failed\n");
		return (-errno);
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		if (QUERY_INFO(buffer, "ip"))
		{
			tmp_ip[MAX_IP_LEN - 1] = '\0';
			strncpy(tmp_ip, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
			start_check = 0;
		}
		else if (QUERY_INFO(buffer, "mask"))
		{
			tmp_mask[MAX_IP_LEN - 1] = '\0';
			strncpy(tmp_mask, GET_INFO_ADDR(buffer), MAX_IP_LEN - 1);
			start_check = 1;
		}
		else
		{
			jpf_warning("<JpfModBss> error, popen read buffer:%s", buffer);
			pclose(fp);
			return -E_GUESTIOCFLT;
		}

		if (!start_check)
			continue;

		int ip_i;
		for (ip_i = 0; ip_i < req_info->count; ip_i++)
		{
			IpInfo *cur_ip_info = &req_info->ip_list[ip_i];

			if (if_ip_conflict(tmp_ip, tmp_mask, cur_ip_info->ip,
				cur_ip_info->netmask))
			{
				jpf_warning("<JpfModBss> ip conflict, old ip:%s, mask:%s, " \
					"new ip:%s, mask:%s", tmp_ip, tmp_mask, cur_ip_info->ip,
					cur_ip_info->netmask);
				pclose(fp);
				return -E_GUESTIOCFLT;
			}
		}
	}
	pclose(fp);

	return 0;
}


JpfMsgFunRet
jpf_mod_bss_set_network_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfSetNetworkConfig *req_info;
	JpfSetResult	res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	FILE *fp;
	char query_buf[STRING_LEN] = {0};
	gchar network[MAX_IP_LEN] = {0};
	gchar broadcast[MAX_IP_LEN] = {0};
	gchar dns[MAX_IP_LEN] = {0};

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	strncpy(dns, req_info->dns, MAX_IP_LEN - 1);
	comma_to_space(dns, MAX_IP_LEN - 1);

	memset(&res_info, 0, sizeof(res_info));
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
		goto end;
	}
	JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
	jpf_mods_container_put_guest(self->container, bss_base);

	jpf_warning("<JpfModBss> set network configure");
	if (check_interface_valid(req_info->network_interface) != 0)
	{
		ret = -E_STRINGFORMAT;
		jpf_warning("<JpfModBss> interface msg error, network_interface:%s",
			req_info->network_interface);
		goto end;
	}

	if ((ret = check_netmask_valid(req_info)) != 0)
	{
		goto end;
	}

	if ((ret = check_ip_conflict(req_info)) != 0)
	{
		jpf_warning("<JpfModBss> set %s, ip conflict", req_info->network_interface);
		goto end;
	}

	int ip_i = 0;
	for (ip_i = 0; ip_i < req_info->count; ip_i++)
	{
		IpInfo *cur_ip_info = &req_info->ip_list[ip_i];
		get_network(network, cur_ip_info->ip, cur_ip_info->netmask);
		get_broadcast(broadcast, cur_ip_info->ip, cur_ip_info->netmask);
		snprintf(query_buf, STRING_LEN - 1, "%s %s %d %s %s %s %s %s %s",
			POPEN_SET_NETWORK_CONFIG, req_info->network_interface,
			ip_i, cur_ip_info->ip, cur_ip_info->netmask, network, broadcast,
			cur_ip_info->gateway, dns);

		fp = popen(query_buf, "r");
		if (!fp)
		{
			jpf_warning("<JpfModBss> set network configure, popen failed\n");
			ret = -errno;
			goto end;
		}
		pclose(fp);
		memset(query_buf, 0, sizeof(query_buf));
	}
	snprintf(query_buf, STRING_LEN - 1, "%s %s %d &",
		POPEN_SET_NETWORK_CONFIG_END, req_info->network_interface, req_info->count);
	system(query_buf);
	sleep(3);

end:
	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);

	JPF_CREATE_MSG_TO_LOG(MSG_LOG_SET_NETWORK_CONFIG, &res_info, sizeof(res_info));

	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	return MFR_DELIVER_BACK;
}

JpfMsgFunRet
jpf_mod_bss_add_hd_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddHdGroup *req_info;
    JpfAddHdGroupRes res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        MSG_SET_RESPONSE(msg);
        memset(&res_info, 0, sizeof(res_info));
        SET_CODE(&res_info, -ret);
        jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

        return MFR_DELIVER_BACK;
    }

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_MSS);
    jpf_mods_container_put_guest(self->container, bss_base);

    return MFR_DELIVER_AHEAD;
}


JpfMsgFunRet
jpf_mod_bss_add_hd_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfAddHdGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_add_hd_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfAddHdToGroup *req_info;

    self = (JpfModBss*)app_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_add_hd_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfAddHdToGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}

JpfMsgFunRet
jpf_mod_bss_del_hd_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfDelHdFromGroup *req_info;

    self = (JpfModBss*)app_obj;
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_del_hd_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfDelHdFromGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_reboot_mss_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfRebootMss *req_info;

    self = (JpfModBss*)app_obj;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_reboot_mss_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfRebootMssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_all_hd_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryAllHdGroup *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_all_hd_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAllHdGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));
    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_hd_group_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryHdGroupInfo *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_hd_group_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryHdGroupInfoRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_all_hd_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryAllHd *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_all_hd_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAllHdRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_del_hd_group_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelHdGroup *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_del_hd_group_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfDelHdGroupRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_hd_format_progress_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGetHdFormatProgress *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_hd_format_progress_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfGetHdFormatProgressRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_gu_record_status_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryGuRecordStatus *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_gu_record_status_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryGuRecordStatusRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_platform_upgrade_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfPlatformUpgrade *req_info;
    JpfPlatformUpgradeResp res_info;
    JpfMsgID msg_id;
    gchar script_path[MAX_FILE_PATH + 2] = {0};
    gint ret = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, res_info.bss_usr);
        jpf_warning("system begin to upgrade");
	 ret = jpf_get_platform_upgrade_script(script_path);
	 if (ret)
	 {
	     jpf_warning("get platform upgrade script error:%d", ret);
            system("upgrade-platform-system &");
	 }
	 else
	 {
	     strcat(script_path, " &");
	     system(script_path);
	 }

        jpf_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);

    JPF_CREATE_MSG_TO_LOG(MSG_LOG_PLATFORM_UPGRADE, &res_info, sizeof(res_info));

    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_alarm_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfQueryAlarm *req_info;
    JpfQueryAlarmRes  *res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;
    gint size = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
        res_info = jpf_mem_kalloc(sizeof(JpfMsgErrCode));
        if (G_UNLIKELY(!res_info))
        {
            jpf_warning("<JpfModBss> alloc memory error");
            jpf_sysmsg_destroy(msg);
            return MFR_ACCEPTED;
        }
    }
    else
    {
    	req_info = MSG_GET_DATA(msg);
    	BUG_ON(!req_info);
       //JpfBss *bss;
     //  bss = (JpfBss*)bss_base;
       strncpy(req_info->bss_usr, ID_OF_GUEST(bss_base), USER_NAME_LEN - 1);

       /* res_info = jpf_mod_bss_sync_req_2(self, msg_id, req_info,
            sizeof(JpfQueryAlarm), &size);
        if (res_info)
        {
            ret = RES_CODE(res_info);
            printf("query alarm list code %d\n",ret);
            jpf_print("<JpfModBss> query alarm list ok");
        }
        else
        {
            ret = -ENOMEM;
            jpf_error("query alarm failed, err:%d", ret);
        }*/

        jpf_mods_container_put_guest(self->container, bss_base);
	 MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
        return MFR_DELIVER_AHEAD;
    }

    MSG_SET_RESPONSE(msg);
    if(res_info)
    {
        SET_CODE(res_info, -ret);
    	 jpf_sysmsg_set_private(msg, res_info, size, jpf_mem_kfree);
    }
    else
    {
        jpf_sysmsg_destroy(msg);
	 return MFR_ACCEPTED;
    }

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_alarm_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAlarmRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_alarm_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDelAlarm *req_info;
    JpfMsgCode  res_info;
    JpfGuestBase *bss_base;
    JpfMsgID msg_id;
    gint ret;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    memset(&res_info, 0, sizeof(res_info));
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.", MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        JPF_GET_BSS_USR_NAME_FROM_GUESTBASE(bss_base, req_info->bss_usr);
        ret = jpf_mod_bss_sync_req(self, msg_id, req_info, sizeof(JpfDelAlarm), &res_info, sizeof(res_info));
        if (ret)
        {
            jpf_warning(
                "<JpfModBss> del alarm failed, err:%d",
                 ret
            );
        }
        else
        {
            jpf_print( "<JpfModBss> del alarm ok");
        }

        jpf_mods_container_put_guest(self->container, bss_base);
    }

    SET_CODE(&res_info, -ret);
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_server_resource_info_f(JpfAppObj *app_obj,
	JpfSysMsg *msg)
{
	JpfQueryServerResourceInfo *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_server_resource_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryServerResourceInfoRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_search_device_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfSearchPuRes res_info;
    search_array_t  *pu_list;
    JpfMsgID msg_id;
    gint ret = 0;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    memset(&res_info, 0, sizeof(JpfSearchPuRes));
    msg_id = MSG_GETID(msg);
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        pu_list = search_device();
        jpf_search_pu_lock();
        jpf_set_search_pu_list(pu_list);
        jpf_search_pu_unlock();
        res_info.pu_count = pu_list->count;
        // res_info.pu_count = 10;
        jpf_warning("---------pu_list->count=%d",pu_list->count);
        destory_search_result(pu_list);
        ret = 0;
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfGetSearchedPuRes *
jpf_get_searched_pus_list(JpfGetSearchedPu *req, search_pu_list *pu_list)
{
	gint size, start_num, res_count;
	JpfGetSearchedPuRes *get_search_pu = NULL;

	if ((req->start_num + req->req_num) > pu_list->count)
	{
		if (req->req_num > pu_list->count)
		{
			size = sizeof(JpfGetSearchedPuRes) + pu_list->count*sizeof(search_result_t);
			res_count = pu_list->count;
			start_num = 0;
		}
		else
		{
  			res_count = pu_list->count - req->start_num;

			size = sizeof(JpfGetSearchedPuRes) + res_count*sizeof(search_result_t);
			start_num = req->start_num;
		}

		get_search_pu = jpf_mem_kalloc(size);
		if (!get_search_pu)
			return NULL;

		memset(get_search_pu, 0, size);
		get_search_pu->pu_count = pu_list->count;
		get_search_pu->res_count = res_count;
		memcpy(get_search_pu->search_pu, &pu_list->result[start_num],
			res_count*sizeof(search_result_t));
		return get_search_pu;
	}

	size = sizeof(JpfGetSearchedPuRes) + req->req_num*sizeof(search_result_t);
	get_search_pu = jpf_mem_kalloc(size);
	if (get_search_pu)
	{
		memset(get_search_pu, 0, size);
		start_num = req->start_num;
		res_count = req->req_num;
		get_search_pu->pu_count = pu_list->count;
		get_search_pu->res_count = res_count;
		memcpy(get_search_pu->search_pu, &pu_list->result[start_num],
			res_count*sizeof(search_result_t));
	}
	return get_search_pu;
}


JpfMsgFunRet
jpf_mod_bss_get_searched_device_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfGetSearchedPu *req_info;
    JpfGetSearchedPuRes *res_info = NULL;
    search_pu_list  *pu_list = NULL;
    JpfMsgID msg_id;
    gint ret = 0, size;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    msg_id = MSG_GETID(msg);
    size = sizeof(JpfGetSearchedPuRes);
    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        jpf_search_pu_lock();
        pu_list = jpf_get_search_pu_list(pu_list);
        res_info = jpf_get_searched_pus_list(req_info, pu_list);
        jpf_search_pu_unlock();
        if (res_info)
        {
            size = sizeof(JpfGetSearchedPuRes) + res_info->res_count*sizeof(search_result_t);
            int i;
            for (i = 0; i < res_info->res_count; i++)
            {
                jpf_covert_pu_type(&res_info->search_pu[i].jpf_srch.dev_info.pu_type,
                    &res_info->search_pu[i].jpf_srch.dev_info.pu_type);
             }
         }
        ret = 0;
        jpf_mods_container_put_guest(self->container, bss_base);
    }

    if (!res_info)
    {
        res_info = jpf_mem_kalloc(size);
        memset(res_info, 0, size);
    }
    MSG_SET_RESPONSE(msg);
    SET_CODE(res_info, -ret);
    jpf_sysmsg_set_private_2(msg, res_info, size);
    jpf_mem_kfree(res_info, size);
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_cms_all_ips_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfQueryCmsAllIpRes *res_info = NULL;
    JpfHostIps ips;
    JpfMsgID msg_id;
    gint ret = 0, size;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s No such guest.",
        	MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&ips, 0, sizeof(ips));
        jpf_get_host_ips(&ips);
        size = sizeof(JpfQueryCmsAllIpRes) + ips.count*sizeof(JpfCmsIp);
        res_info = jpf_mem_kalloc(size);
        memset(res_info, 0, size);
        res_info->count = ips.count;
        memcpy(res_info->ips, ips.ips, ips.count*sizeof(JpfCmsIp));
        jpf_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    if (!res_info)
    {
        size = sizeof(JpfQueryCmsAllIpRes);
        res_info = jpf_mem_kalloc(size);
        memset(res_info, 0, size);
    }

    SET_CODE(res_info, -ret);
    jpf_sysmsg_set_private_2(msg, res_info, size);
    jpf_mem_kfree(res_info, size);
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_tw_auth_info_f(JpfAppObj *app_obj,
	JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfQueryTwAuthInfo *req_info;
    JpfQueryTwAuthInfoRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s error,admin no login.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&res_cap, 0, sizeof(res_cap));
        jpf_mod_get_resource_cap(&res_cap);
        res_info.tw_auth_type = res_cap.modules_data[SYS_MODULE_TW];
        jpf_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_query_alarm_link_auth_info_f(JpfAppObj *app_obj,
	JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGuestBase *bss_base;
    JpfQueryAlarmLinkAuthInfo *req_info;
    JpfQueryAlarmLinkAuthInfoRes res_info;
    JpfMsgID msg_id;
    gint ret = 0;
    JpfResourcesCap res_cap;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    msg_id = MSG_GETID(msg);
    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    bss_base = jpf_mods_container_get_guest(self->container, io);
    if (G_UNLIKELY(!bss_base))
    {
        ret = -E_NOSUCHGUEST;
        jpf_warning("<JpfModBss> msg:%s error,admin no login.",
            MESSAGE_ID_TO_STR(cms, msg_id));
    }
    else
    {
        memset(&res_cap, 0, sizeof(res_cap));
        jpf_mod_get_resource_cap(&res_cap);
        res_info.alarm_link_auth_type = res_cap.modules_data[SYS_MODULE_ALM];
        jpf_mods_container_put_guest(self->container, bss_base);
     }

    MSG_SET_RESPONSE(msg);
    SET_CODE(&res_info, -ret);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_auto_add_pu_f(JpfAppObj *app_obj,
	JpfSysMsg *msg)
{
    JpfAutoAddPu *req_info;
    JpfMsgErrCode res_info;
    JpfResourcesCap res_cap;
    gchar mf[MF_ID_LEN] = {0};
    gint ret;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    if ((!regex_mached(req_info->puid, puid_reg))
                    ||(regex_mached(req_info->dev_name, exclude_reg))
			 || (!regex_mached(req_info->domain_id, domain_reg)))
    {
        SET_CODE(&res_info, E_STRINGFORMAT);
        goto err_auto_add_pu;
    }

    jpf_get_mf_from_guid(req_info->puid, mf);
    memset(&res_cap, 0, sizeof(res_cap));
    jpf_mod_get_resource_cap(&res_cap);
    if (res_cap.module_bits&MODULE_CMS_BIT)
    {
        ret = jpf_compare_manufacturer(res_cap.modules_data[SYS_MODULE_CMS], mf);
        if (ret)
        {
             SET_CODE(&res_info, -ret);
             goto err_auto_add_pu;
        }
    }
    else
        goto err_auto_add_pu;

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
err_auto_add_pu:
    MSG_SET_RESPONSE(msg);
    jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
    jpf_warning(
            "<JpfModBss> auto add pu:%s failed, err:%d",
            req_info->puid, E_STRINGFORMAT
        );
    return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_auto_add_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfBssRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_get_next_puno_f(JpfAppObj *app_obj,
	JpfSysMsg *msg)
{
	JpfGetNextPuNo *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_get_next_puno_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfGetNextPuNoRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_get_initiator_name_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGetInitName *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_initiator_name_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfGetInitNameRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_set_initiator_name_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfSetInitName *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_set_initiator_name_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfSetInitNameRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_ipsan_info_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGetIpsanInfo *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_ipsan_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfGetIpsanInfoRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_add_one_ipsan_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfAddOneIpsan *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_add_one_ipsan_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfAddOneIpsanRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_delete_one_ipsan_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfDeleteOneIpsan *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_delete_one_ipsan_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfDeleteOneIpsanRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_one_ipsan_detail_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfNetIO *io;
    JpfGetOneIpsanDetail *req_info;

    self = (JpfModBss*)app_obj;
    io = MSG_IO(msg);
    BUG_ON(!io);

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward(self, msg, req_info->session);
}


JpfMsgFunRet
jpf_mod_bss_get_one_ipsan_detail_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfGetOneIpsanDetailRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->session);
}


JpfMsgFunRet
jpf_mod_bss_query_log_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryLog *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_to_log(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_log_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfQueryLogRes *res_info;

	self = (JpfModBss*)app_obj;
	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (RES_CODE(res_info) < 0)
		SET_CODE(res_info, -RES_CODE(res_info));

	return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


#if ONLINE_RATE_FLAG
JpfMsgFunRet
jpf_mod_bss_query_area_dev_rate_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfQueryAreaDevRate *req_info;

    req_info = MSG_GET_DATA(msg);
    BUG_ON(!req_info);

    return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_area_dev_rate_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAreaDevRateRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}
#endif


static int jpf_mod_get_state_num(char *operate)
{
	char query_buf[STRING_LEN] = {0};
	char buffer[STRING_LEN] = {0};
	FILE *fp;
	int res = -1;

	snprintf(query_buf, STRING_LEN - 1, operate);
	fp = popen(query_buf, "r");
	if (!fp)
	{
		jpf_warning("<JpfModBss> get_state_num, popen failed\n");
		return (-errno);
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
			buffer[strlen(buffer) - 1] = '\0';

		res = atoi(buffer);
		if (res < 0 || res > MAX_STATE_NUM)
		{
			jpf_warning("<JpfModBss> get_state_num error, state_num=%d\n", res);
			res = -E_STRINGFORMAT;
		}
		break;
	}
	pclose(fp);

	return res;
}


JpfMsgFunRet
jpf_mod_bss_get_server_flag_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfGetServerFlagRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;

	res_info.server_flag = jpf_mod_get_state_num(POPEN_GET_MODULES_FLAG);
	if (res_info.server_flag < 0)
	{
		jpf_warning("<JpfModBss> get server_flag failed, server_flag = %d!",
			res_info.server_flag);
		ret = res_info.server_flag;
		res_info.server_flag = 0;
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_mds_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetMdsConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfMdsCtl gc;
		jpf_get_mds_parm(&gc);
		strncpy(res_info.mds_id, gc.mds_id, MDS_ID_LEN - 1);
		res_info.start_port = gc.start_port;
		res_info.end_port = gc.end_port;
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_mds_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfSetMdsConfig *req_info;
	JpfSetResult res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfMdsCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		jpf_get_mds_parm(&gc_old);
		strncpy(gc.mds_id, req_info->mds_id, MDS_ID_LEN - 1);
		gc.start_port = req_info->start_port;
		gc.end_port = req_info->end_port;

		jpf_set_mds_parm(gc);
		if (strcmp(gc.mds_id, gc_old.mds_id))
			system("killall -9 mds");
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_mds_state_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetMdsStateRes res_info;
	JpfMsgID msg_id;
	gint ret = 0;

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = jpf_mod_get_state_num(POPEN_GET_MDS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_mss_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetMssConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfMssCtl gc;
		jpf_get_mss_parm(&gc);
		strncpy(res_info.mss_id, gc.mss_id, MSS_ID_LEN - 1);
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_mss_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfSetMssConfig *req_info;
	JpfSetResult res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfMssCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		jpf_get_mss_parm(&gc_old);
		strncpy(gc.mss_id, req_info->mss_id, MSS_ID_LEN - 1);

		jpf_set_mss_parm(gc);
		if (strcmp(gc.mss_id, gc_old.mss_id))
			system("killall -9 mss");
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_mss_state_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetMssStateRes res_info;
	JpfMsgID msg_id;
	gint ret = 0;

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = jpf_mod_get_state_num(POPEN_GET_MSS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_ivs_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetIvsConfigRes res_info;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfIvsCtl gc;
		jpf_get_ivs_parm(&gc);
		strncpy(res_info.ivs_id, gc.ivs_id, IVS_ID_LEN - 1);
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_set_ivs_config_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfSetIvsConfig *req_info;
	JpfSetResult res_info;
	JpfMsgID msg_id;
	gint ret = 0;
	memset(&res_info, 0, sizeof(res_info));

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		JpfIvsCtl gc, gc_old;
		memset(&gc, 0, sizeof(gc));
		memset(&gc_old, 0, sizeof(gc_old));
		jpf_get_ivs_parm(&gc_old);
		strncpy(gc.ivs_id, req_info->ivs_id, IVS_ID_LEN - 1);

		jpf_set_ivs_parm(gc);
		if (strcmp(gc.ivs_id, gc_old.ivs_id))
			system("killall -9 ivs");
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_get_ivs_state_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModBss *self;
	JpfNetIO *io;
	JpfGuestBase *bss_base;
	JpfGetIvsStateRes res_info;
	JpfMsgID msg_id;
	gint ret = 0;

	self = (JpfModBss*)app_obj;
	io = MSG_IO(msg);
	BUG_ON(!io);

	msg_id = MSG_GETID(msg);
	memset(&res_info, 0, sizeof(res_info));
	bss_base = jpf_mods_container_get_guest(self->container, io);
	if (G_UNLIKELY(!bss_base))
	{
		ret = -E_NOSUCHGUEST;
		jpf_warning("<JpfModBss> user has not login.");
	}
	else
	{
		jpf_mods_container_put_guest(self->container, bss_base);
	}

	MSG_SET_RESPONSE(msg);
	res_info.state = jpf_mod_get_state_num(POPEN_GET_IVS_STATE);
	if (res_info.state < 0)
	{
		ret = res_info.state;
		res_info.state = 0;
	}
	SET_CODE(&res_info, -ret);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	return MFR_DELIVER_BACK;
}


static void jpf_will_shutdown()
{
	system("sleep 1 && shutdown -h now &");
};

static JpfMsgFunRet
jpf_mod_bss_cms_shutdown_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfSetResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	SET_CODE(&res_info, 0);

	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_RESPONSE(msg);

	jpf_will_shutdown();

	return MFR_DELIVER_BACK;
}


static void jpf_will_reboot()
{
	system("sleep 1 && reboot &");
};

static JpfMsgFunRet
jpf_mod_bss_cms_reboot_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfSetResult res_info;
	memset(&res_info, 0, sizeof(res_info));

	SET_CODE(&res_info, 0);

	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_RESPONSE(msg);

	jpf_will_reboot();

	return MFR_DELIVER_BACK;
}


JpfMsgFunRet
jpf_mod_bss_add_ams_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfAddAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_add_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ams_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_ams_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfDelAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_del_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ams_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryAms *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ams_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAmsRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ams_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfQueryAmsPu *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_query_ams_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfQueryAmsPuRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ams_pu_f(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfModifyAmsPu *req_info;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	return jpf_mod_bss_forward_2(app_obj, msg, req_info->bss_usr);
}


JpfMsgFunRet
jpf_mod_bss_modify_ams_pu_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
    JpfModBss *self;
    JpfBssRes *res_info;

    self = (JpfModBss*)app_obj;
    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    if (RES_CODE(res_info) < 0)
        SET_CODE(res_info, -RES_CODE(res_info));

    return jpf_mod_bss_backward(self, msg, res_info->bss_usr);
}



void
jpf_mod_bss_register_msg_handler(JpfModBss *self)
{
    JpfAppMod *super_self = (JpfAppMod*)self;

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_LOGIN,
        jpf_mod_bss_login_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_BSS_HEART,
        jpf_mod_bss_heart_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ADMIN,
        jpf_mod_bss_add_admin_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_ADMIN,
        jpf_mod_bss_modify_admin_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ADMIN,
        jpf_mod_bss_del_admin_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ADMIN,
        jpf_mod_bss_query_admin_f,
        jpf_mod_bss_query_admin_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_ADMIN,
        jpf_mod_bss_validata_admin_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER_GROUP,
        jpf_mod_bss_add_user_group_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER_GROUP,
        jpf_mod_bss_validata_user_group_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_GROUP,
        jpf_mod_bss_query_user_group_f,
        jpf_mod_bss_query_user_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER_GROUP,
        jpf_mod_bss_modify_user_group_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER_GROUP,
        jpf_mod_bss_del_user_group_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_USER,
        jpf_mod_bss_add_user_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_USER,
        jpf_mod_bss_validata_user_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER,
        jpf_mod_bss_query_user_f,
        jpf_mod_bss_query_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_USER,
        jpf_mod_bss_modify_user_f,
        NULL,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_USER,
        jpf_mod_bss_del_user_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DOMAIN,
        jpf_mod_bss_query_domain_f,
        jpf_mod_bss_query_domain_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DOMAIN,
        jpf_mod_bss_modify_domain_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DOMAIN,
        NULL,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DOMAIN,
        NULL,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA,
        jpf_mod_bss_query_area_f,
        jpf_mod_bss_query_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_AREA,
        jpf_mod_bss_add_modify_area_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_AREA,
        jpf_mod_bss_del_area_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_PU,
        jpf_mod_bss_add_pu_f,
        jpf_mod_bss_add_pu_b,
        0
    );

    /*jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_PU,
        jpf_mod_bss_validata_pu_f,
        NULL,
        0
    );*/

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_PU,
        jpf_mod_bss_query_pu_f,
        jpf_mod_bss_query_pu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_PU,
        jpf_mod_bss_modify_pu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_PU,
        jpf_mod_bss_del_pu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU,
        jpf_mod_bss_add_gu_f,
        jpf_mod_bss_add_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU,
        jpf_mod_bss_query_gu_f,
        jpf_mod_bss_query_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GU,
        jpf_mod_bss_modify_gu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GU,
        jpf_mod_bss_del_gu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS,
        jpf_mod_bss_add_mds_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS,
        jpf_mod_bss_query_mds_f,
        jpf_mod_bss_query_mds_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MDS,
        jpf_mod_bss_modify_mds_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS,
        jpf_mod_bss_del_mds_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MDS_IP,
        jpf_mod_bss_add_mds_ip_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MDS_IP,
        jpf_mod_bss_query_mds_ip_f,
        jpf_mod_bss_query_mds_ip_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MDS_IP,
        jpf_mod_bss_del_mds_ip_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MSS,
        jpf_mod_bss_add_mss_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MSS,
        jpf_mod_bss_query_mss_f,
        jpf_mod_bss_query_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MSS,
        jpf_mod_bss_modify_mss_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MSS,
        jpf_mod_bss_del_mss_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_POLICY,
        jpf_mod_bss_query_record_policy_f,
        jpf_mod_bss_query_record_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_RECORD_POLICY_CONFIG,
        jpf_mod_bss_record_policy_config_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MANUFACTURER,
        jpf_mod_bss_query_manufacturer_f,
        jpf_mod_bss_query_manufacturer_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_MODIFY_MANUFACTURER,
        jpf_mod_bss_add_modify_manufacturer_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MANUFACTURER,
        jpf_mod_bss_del_manufacturer_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GU_TO_USER,
        jpf_mod_bss_add_gu_to_user_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_GU,
        jpf_mod_bss_query_user_own_gu_f,
        jpf_mod_bss_query_user_own_gu_b,
        0
    );

	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW_TO_USER,
        jpf_mod_bss_add_tw_to_user_f,
        jpf_mod_bss_add_tw_to_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TW,
        jpf_mod_bss_query_user_own_tw_f,
        jpf_mod_bss_query_user_own_tw_b,
        0
    );

	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_TO_USER,
        jpf_mod_bss_add_tour_to_user_f,
        jpf_mod_bss_add_tour_to_user_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_USER_OWN_TOUR,
        jpf_mod_bss_query_user_own_tour_f,
        jpf_mod_bss_query_user_own_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TIME,
        jpf_mod_bss_query_system_time_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_TIME,
        jpf_mod_bss_set_system_time_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_BACKUP,
        jpf_mod_bss_database_backup_f,
        jpf_mod_bss_database_backup_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DATABASE_IMPORT,
        jpf_mod_bss_database_import_f,
        jpf_mod_bss_database_import_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETINTERFACE_CONFIG,
        jpf_mod_bss_get_net_interface_config_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NETWORK_CONFIG,
        jpf_mod_bss_get_network_config_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_NETWORK_CONFIG,
        jpf_mod_bss_set_network_config_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD_GROUP,
        jpf_mod_bss_add_hd_group_f,
        jpf_mod_bss_add_hd_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD,
        jpf_mod_bss_add_hd_f,
        jpf_mod_bss_add_hd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD,
        jpf_mod_bss_del_hd_f,
        jpf_mod_bss_del_hd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_REBOOT_MSS,
        jpf_mod_bss_reboot_mss_f,
        jpf_mod_bss_reboot_mss_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD_GROUP,
        jpf_mod_bss_query_all_hd_group_f,
        jpf_mod_bss_query_all_hd_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUP_INFO,
        jpf_mod_bss_query_hd_group_info_f,
        jpf_mod_bss_query_hd_group_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALL_HD,
        jpf_mod_bss_query_all_hd_f,
        jpf_mod_bss_query_all_hd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD_GROUP,
        jpf_mod_bss_del_hd_group_f,
        jpf_mod_bss_del_hd_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_HD_FORMAT_PROGRESS,
        jpf_mod_bss_get_hd_format_progress_f,
        jpf_mod_bss_get_hd_format_progress_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GU_RECORD_STATUS,
        jpf_mod_bss_query_gu_record_status_f,
        jpf_mod_bss_query_gu_record_status_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_AREA,
        jpf_mod_bss_add_defence_area_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_AREA,
        jpf_mod_bss_modify_defence_area_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_AREA,
        jpf_mod_bss_del_defence_area_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_AREA,
        jpf_mod_bss_query_defence_area_f,
        jpf_mod_bss_query_defence_area_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_MAP,
        jpf_mod_bss_add_defence_map_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_MAP,
        jpf_mod_bss_del_defence_map_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_MAP,
        jpf_mod_bss_query_defence_map_f,
        jpf_mod_bss_query_defence_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_DEFENCE_GU,
        jpf_mod_bss_add_defence_gu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_DEFENCE_GU,
        jpf_mod_bss_modify_defence_gu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_DEFENCE_GU,
        jpf_mod_bss_del_defence_gu_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEFENCE_GU,
        jpf_mod_bss_query_defence_gu_f,
        jpf_mod_bss_query_defence_gu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_MAP_HREF,
        jpf_mod_bss_set_map_href_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_MAP_HREF,
        jpf_mod_bss_modify_map_href_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_MAP_HREF,
        jpf_mod_bss_del_map_href_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_MAP_HREF,
        jpf_mod_bss_query_map_href_f,
        jpf_mod_bss_query_map_href_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_PLATFORM_UPGRADE,
        jpf_mod_bss_platform_upgrade_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM,
        jpf_mod_bss_query_alarm_f,
        jpf_mod_bss_query_alarm_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ALARM,
        jpf_mod_bss_del_alarm_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_DEL_ALARM_POLICY,
        jpf_mod_bss_query_del_alarm_policy_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_DEL_ALARM_POLICY,
        jpf_mod_bss_set_del_alarm_policy_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TW,
        jpf_mod_bss_add_tw_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TW,
        jpf_mod_bss_modify_tw_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TW,
        jpf_mod_bss_del_tw_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW,
        jpf_mod_bss_query_tw_f,
        jpf_mod_bss_query_tw_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_SCREEN,
        jpf_mod_bss_add_screen_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_SCREEN,
        jpf_mod_bss_modify_screen_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_SCREEN,
        jpf_mod_bss_del_screen_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCREEN,
        jpf_mod_bss_query_screen_f,
        jpf_mod_bss_query_screen_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SCR_DIV,
        jpf_mod_bss_query_screen_division_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR,
        jpf_mod_bss_add_tour_f,
        jpf_mod_bss_add_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_TOUR,
        jpf_mod_bss_modify_tour_f,
        jpf_mod_bss_modify_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_TOUR,
        jpf_mod_bss_del_tour_f,
        jpf_mod_bss_del_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR,
        jpf_mod_bss_query_tour_f,
        jpf_mod_bss_query_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_TOUR_STEP,
        jpf_mod_bss_add_tour_step_f,
        jpf_mod_bss_add_tour_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TOUR_STEP,
        jpf_mod_bss_query_tour_step_f,
        jpf_mod_bss_query_tour_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP,
        jpf_mod_bss_add_group_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP,
        jpf_mod_bss_modify_group_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP,
        jpf_mod_bss_del_group_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP,
        jpf_mod_bss_query_group_f,
        jpf_mod_bss_query_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_STEP,
        jpf_mod_bss_add_group_step_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP,
        jpf_mod_bss_modify_group_step_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP,
        jpf_mod_bss_del_group_step_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP,
        jpf_mod_bss_query_group_step_f,
        jpf_mod_bss_query_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_CONFIG_GROUP_STEP,
        jpf_mod_bss_config_group_step_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_GROUP_STEP_INFO,
        jpf_mod_bss_modify_group_step_info_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_STEP_INFO,
        jpf_mod_bss_del_group_step_info_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_INFO,
        jpf_mod_bss_query_group_step_info_f,
        jpf_mod_bss_query_group_step_info_b,
        0
    );

  	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GROUP_STEP_DIV,
        jpf_mod_bss_query_group_step_div_f,
        jpf_mod_bss_query_group_step_div_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TIME_POLICY_CONFIG,
        jpf_mod_bss_link_time_policy_config_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TIME_POLICY,
        jpf_mod_bss_modify_link_time_policy_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TIME_POLICY,
        jpf_mod_bss_query_link_time_policy_f,
        jpf_mod_bss_query_link_time_policy_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TIME_POLICY,
        jpf_mod_bss_del_link_time_policy_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_RECORD_CONFIG,
        jpf_mod_bss_link_record_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_RECORD,
        jpf_mod_bss_modify_link_record_f,
        jpf_mod_bss_general_modify_b,
        0
    );


    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_RECORD,
        jpf_mod_bss_query_link_record_f,
        jpf_mod_bss_query_link_record_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_RECORD,
        jpf_mod_bss_del_link_record_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_IO_CONFIG,
        jpf_mod_bss_link_IO_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_IO,
        jpf_mod_bss_modify_link_IO_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_IO,
        jpf_mod_bss_query_link_IO_f,
        jpf_mod_bss_query_link_IO_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_IO,
        jpf_mod_bss_del_link_IO_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_SNAPSHOT_CONFIG,
        jpf_mod_bss_link_snapshot_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_SNAPSHOT,
        jpf_mod_bss_modify_link_snapshot_f,
        jpf_mod_bss_general_modify_b,
        0
    );


    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_SNAPSHOT,
        jpf_mod_bss_query_link_snapshot_f,
        jpf_mod_bss_query_link_snapshot_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_SNAPSHOT,
        jpf_mod_bss_del_link_snapshot_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_PRESET_CONFIG,
        jpf_mod_bss_link_preset_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_PRESET,
        jpf_mod_bss_modify_link_preset_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_PRESET,
        jpf_mod_bss_query_link_preset_f,
        jpf_mod_bss_query_link_preset_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_PRESET,
        jpf_mod_bss_del_link_preset_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_STEP_CONFIG,
        jpf_mod_bss_link_step_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_STEP,
        jpf_mod_bss_modify_link_step_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_STEP,
        jpf_mod_bss_query_link_step_f,
        jpf_mod_bss_query_link_step_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_STEP,
        jpf_mod_bss_del_link_step_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_TOUR_CONFIG,
        jpf_mod_bss_link_tour_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_TOUR,
        jpf_mod_bss_modify_link_tour_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_TOUR,
        jpf_mod_bss_query_link_tour_f,
        jpf_mod_bss_query_link_tour_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_TOUR,
        jpf_mod_bss_del_link_tour_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_GROUP_CONFIG,
        jpf_mod_bss_link_group_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_GROUP,
        jpf_mod_bss_modify_link_group_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_GROUP,
        jpf_mod_bss_query_link_group_f,
        jpf_mod_bss_query_link_group_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_GROUP,
        jpf_mod_bss_del_link_group_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

   jpf_app_mod_register_msg(
        super_self,
        MESSAGE_LINK_MAP_CONFIG,
        jpf_mod_bss_link_map_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_LINK_MAP,
        jpf_mod_bss_modify_link_map_f,
        jpf_mod_bss_general_modify_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LINK_MAP,
        jpf_mod_bss_query_link_map_f,
        jpf_mod_bss_query_link_map_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_LINK_MAP,
        jpf_mod_bss_del_link_map_f,
        jpf_mod_bss_general_cmd_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_SERVER_RESOURCE,
        jpf_mod_bss_query_server_resource_info_f,
        jpf_mod_bss_query_server_resource_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_TW_AUTH_INFO,
        jpf_mod_bss_query_tw_auth_info_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_ALARM_LINK_AUTH_INFO,
        jpf_mod_bss_query_alarm_link_auth_info_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_CMS_ALL_IP,
        jpf_mod_bss_query_cms_all_ips_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SEARCH_PU,
        jpf_mod_bss_search_device_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_SEARCH_PUS,
        jpf_mod_bss_get_searched_device_f,
        NULL,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_AUTO_ADD_PU,
        jpf_mod_bss_auto_add_pu_f,
        jpf_mod_bss_auto_add_pu_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_NEXT_PUNO,
        jpf_mod_bss_get_next_puno_f,
        jpf_mod_bss_get_next_puno_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_INIT_NAME,
        jpf_mod_bss_get_initiator_name_f,
        jpf_mod_bss_get_initiator_name_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_SET_INIT_NAME,
        jpf_mod_bss_set_initiator_name_f,
        jpf_mod_bss_set_initiator_name_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IPSAN_INFO,
        jpf_mod_bss_get_ipsan_info_f,
        jpf_mod_bss_get_ipsan_info_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ONE_IPSAN,
        jpf_mod_bss_add_one_ipsan_f,
        jpf_mod_bss_add_one_ipsan_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DELETE_ONE_IPSAN,
        jpf_mod_bss_delete_one_ipsan_f,
        jpf_mod_bss_delete_one_ipsan_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ONE_IPSAN,
        jpf_mod_bss_get_one_ipsan_detail_f,
        jpf_mod_bss_get_one_ipsan_detail_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_LOG,
        jpf_mod_bss_query_log_f,
        jpf_mod_bss_query_log_b,
        0
    );

#if ONLINE_RATE_FLAG
    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_AREA_DEV_ONLINE_RATE,
        jpf_mod_bss_query_area_dev_rate_f,
        jpf_mod_bss_query_area_dev_rate_b,
        0
    );
#endif

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_VALIDATA_GU_MAP,
        jpf_mod_bss_validate_gu_map_f,
        jpf_mod_bss_validate_gu_map_b,
        0
    );

	jpf_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_IVS,
        jpf_mod_bss_add_ivs_f,
        jpf_mod_bss_add_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_MODIFY_IVS,
        jpf_mod_bss_modify_ivs_f,
        jpf_mod_bss_modify_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_IVS,
        jpf_mod_bss_del_ivs_f,
        jpf_mod_bss_del_ivs_b,
        0
    );

    jpf_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IVS,
        jpf_mod_bss_query_ivs_f,
        jpf_mod_bss_query_ivs_b,
        0
    );

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_SERVER_FLAG,
		jpf_mod_bss_get_server_flag_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MDS_CONFIG,
		jpf_mod_bss_get_mds_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_MDS_CONFIG,
		jpf_mod_bss_set_mds_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MDS_STATE,
		jpf_mod_bss_get_mds_state_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MSS_CONFIG,
		jpf_mod_bss_get_mss_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_MSS_CONFIG,
		jpf_mod_bss_set_mss_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_MSS_STATE,
		jpf_mod_bss_get_mss_state_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_IVS_CONFIG,
		jpf_mod_bss_get_ivs_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_SET_IVS_CONFIG,
		jpf_mod_bss_set_ivs_config_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_IVS_STATE,
		jpf_mod_bss_get_ivs_state_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_CMS_SHUTDOWN,
		jpf_mod_bss_cms_shutdown_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_CMS_REBOOT,
		jpf_mod_bss_cms_reboot_f,
		NULL,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_ADD_AMS,
		jpf_mod_bss_add_ams_f,
		jpf_mod_bss_add_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS,
		jpf_mod_bss_modify_ams_f,
		jpf_mod_bss_modify_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_DEL_AMS,
		jpf_mod_bss_del_ams_f,
		jpf_mod_bss_del_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS,
		jpf_mod_bss_query_ams_f,
		jpf_mod_bss_query_ams_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_QUERY_AMS_PU,
		jpf_mod_bss_query_ams_pu_f,
		jpf_mod_bss_query_ams_pu_b,
		0
	);

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_MODIFY_AMS_PU,
		jpf_mod_bss_modify_ams_pu_f,
		jpf_mod_bss_modify_ams_pu_b,
		0
	);
}

