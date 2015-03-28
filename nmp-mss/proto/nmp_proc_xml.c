#include "nmp_xml.h"
#include "nmp_sysmsg.h"
#include "nmp_message.h"
#include "nmp_proc_xml.h"
#include "nmp_errno.h"


USING_MSG_ID_MAP(mss_tbl);

NmpSysMsg * 
nmp_get_sysmsg_from_xml(char *xml_buff, gint xml_len, guint seq)
{
	NmpMsgInfo *msg_info; 
	NmpSysMsg  *sys_msg;
	gchar     cmd[MAX_CMD_ID_LEN] = {0};
	gint        msg_id;
	gint        cmd_len;	

    msg_info = nmp_parse_str_xml(xml_buff, xml_len, seq);
    if (G_UNLIKELY(!msg_info))
    {
        nmp_warning("nmp_parse_str_xml() failed!");
        return NULL;
    }

    cmd_len = strlen(msg_info->msg_id);

    if ((cmd_len > 8)&&(strcmp(&msg_info->msg_id[cmd_len - 8],"Response") == 0))
    	strncpy(cmd, msg_info->msg_id, cmd_len - 8);
    else
    	strncpy(cmd, msg_info->msg_id, cmd_len);

    msg_id = MESSAGE_STR_TO_ID(mss_tbl, cmd);
    if (G_UNLIKELY(MESSAGE_INVALID_ID(msg_id)))
    {
        nmp_warning("translate message \"%s\" to id failed!", msg_info->msg_id);
        nmp_free_msginfo(msg_info);
        return NULL;
    }

    sys_msg = nmp_sysmsg_new(msg_id,
    	 msg_info->private_data,
    	 msg_info->private_size,
    	 seq,
    	 (NmpMsgPrivDes)msg_info->priv_destroy);

    nmp_free_msginfo_head(msg_info);
    return sys_msg;
}


gint 
nmp_proto_create_xml_str(char *xml_buff, int *buff_size, NmpSysMsg *sys_msg)
{
	NmpMsgInfo  msg_info; 
	gint        msg_id;
	gint        ret;

	G_ASSERT(xml_buff != NULL && buff_size != NULL && sys_msg != NULL);

	msg_id = MSG_GETID(sys_msg);
	if (G_LIKELY(MESSAGE_ID_TO_STR(mss_tbl, msg_id)))
	{
		msg_info.msg_id[MAX_CMD_ID_LEN - 1] = 0;
		strncpy(msg_info.msg_id, MESSAGE_ID_TO_STR(mss_tbl, msg_id), MAX_CMD_ID_LEN -1);
		if (MSG_RESPONSE(sys_msg))
		{
			if ((strlen(msg_info.msg_id) + 8) < (MAX_CMD_ID_LEN -1))
			{
				strcat(msg_info.msg_id,"Response");
			}
			else
			{
				return -E_CMDLEN2LONG;
			}
		}
	}

	msg_info.private_data = MSG_GET_DATA(sys_msg);
	msg_info.private_size = MSG_DATA_SIZE(sys_msg);

    ret = nmp_create_str_xml(xml_buff, buff_size, &msg_info);
    return ret;
	//return nmp_create_str_xml(xml_buff, buff_size, &msg_info);	
}

