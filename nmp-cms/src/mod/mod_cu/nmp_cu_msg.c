#include "nmp_cu_struct.h"
#include "nmp_internal_msg.h"
#include "nmp_sysmsg.h"
#include "nmp_mod_cu.h"
#include "nmp_errno.h"

//static guint msg_seq_generator = 0;

gint
jpf_mod_cu_get_user_info(JpfModCu *self, JpfUsr *user)
{
    JpfMsgUserInfo req_info;
    JpfMsgUserInfoRes *res_info;
    JpfSysMsg *msg;
    gint ret;

    memset(&req_info, 0, sizeof(req_info));
    strncpy(req_info.name, user->user_name, USER_NAME_LEN - 1);

    msg = jpf_sysmsg_new_2(MSG_GET_USER_INFO, &req_info,
    	sizeof(req_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
        jpf_warning(
        	"<JpfModCu> request user info failed!"
        );
        jpf_sysmsg_destroy(msg);
        return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
    	jpf_warning(
    		"<JpfModCu> request user info timeout!"
    	);
    	return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
    	jpf_warning(
    		"<JpfModCu> request user info err:%d!", ret
    	);
    	jpf_sysmsg_destroy(msg);
    	return ret;
    }

    jpf_mod_cu_add_user_info(user, res_info->passwd,
    	res_info->group_id);
    jpf_sysmsg_destroy(msg);
    return 0;
}


gint
jpf_mod_cu_get_group_info(JpfModCu *self, JpfUsrGroup *grp)
{
    JpfMsgUserGroupInfo req_info;
    JpfMsgUserGroupInfoRes *res_info;
    JpfSysMsg *msg;
    gint ret;

    memset(&req_info, 0, sizeof(req_info));
    req_info.group_id = grp->id;

    msg = jpf_sysmsg_new_2(MSG_GET_USER_GROUP_INFO, &req_info,
    	sizeof(req_info), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
        jpf_warning(
    		"<JpfModCu> request group info failed!"
    	 );
    	 jpf_sysmsg_destroy(msg);

    	 return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
        jpf_warning(
        	"<JpfModCu> request group info timeout!"
        );

        return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
        jpf_warning(
        	"<JpfModCu> request group info err:%d!", ret
        );
        jpf_sysmsg_destroy(msg);

        return ret;
    }

    jpf_mod_cu_add_group_info(grp, res_info->rank,
    	res_info->permission);

    jpf_sysmsg_destroy(msg);

    return 0;
}

gint
jpf_mod_cu_get_user_login_info(JpfModCu *self, JpfMsgUserLoginInfoRes *res)
{
	JpfMsgUserLoginInfoRes *res_info;
	JpfSysMsg *msg;
	gint ret;

	msg = jpf_sysmsg_new_2(MSG_GET_USER_LOGIN_INFO, NULL,
		0, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	ret = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
	if (G_UNLIKELY(ret))	/* send failed */
	{
		jpf_warning(
			"<JpfModCu> request domain and root area info failed!"
		);
		jpf_sysmsg_destroy(msg);
		return ret;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModCu> request domain and root area info timeout!"
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	ret = RES_CODE(res_info);
	if (ret)
	{
		jpf_warning(
			"<JpfModCu> request domain and root area info err:%d!", ret
		);
		jpf_sysmsg_destroy(msg);
		return ret;
	}

    memcpy(res, res_info,sizeof(JpfMsgUserLoginInfoRes));
    jpf_sysmsg_destroy(msg);

	return 0;
}


gint
jpf_mod_cu_get_mdu_ip(
    JpfModCu *self, JpfMsgGetMdsIp *req_info,
    JpfMsgGetMdsIpRes *getMduIpRes
)
{
    JpfMsgGetMdsIpRes *res_info;
    JpfSysMsg *msg;
    gint ret;

    msg = jpf_sysmsg_new_2(MSG_GET_MDS_IP, req_info,
    	sizeof(JpfMsgGetMdsIp), ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return -E_NOMEM;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    ret = jpf_app_mod_sync_request((JpfAppMod*)self, &msg);
    if (G_UNLIKELY(ret))	/* send failed */
    {
    	jpf_warning(
    		"<JpfModCu> request mdu ip failed!"
    	);
    	jpf_sysmsg_destroy(msg);
    	return ret;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
    	jpf_warning(
    		"<JpfModCu> request mdu ip timeout!"
    	);
    	return -E_TIMEOUT;
    }

    res_info = MSG_GET_DATA(msg);
    BUG_ON(!res_info);

    ret = RES_CODE(res_info);
    if (ret)
    {
    	jpf_warning(
    		"<JpfModCu> request mdu ip err:%d!", ret
    	);
    	jpf_sysmsg_destroy(msg);
    	return ret;
    }

    memcpy(getMduIpRes, res_info,sizeof(JpfMsgGetMdsIpRes));
    jpf_sysmsg_destroy(msg);

    return 0;
}

//:~ End
