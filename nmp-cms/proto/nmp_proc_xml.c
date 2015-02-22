
#include "nmp_xml.h"
#include "nmp_sysmsg.h"
#include "nmp_message.h"
#include "nmp_proc_xml.h"
#include "nmp_errno.h"

USING_MSG_ID_MAP(cms);

JpfSysMsg *
jpf_get_sysmsg_from_xml(char *xml_buff, gint xml_len, guint seq)
{
	JpfMsgInfo *msg_info;
	JpfSysMsg  *sys_msg;
	gchar     cmd[MAX_CMD_ID_LEN] = {0};
	gint        msg_id;
	gint        cmd_len;

    printf("================receive xml_buff=%s\n",xml_buff);
    msg_info = jpf_parse_str_xml(xml_buff, xml_len, seq);
    if (G_UNLIKELY(!msg_info))
    {
        jpf_warning("jpf_parse_str_xml() failed!");
        return NULL;
    }

     printf("--------------msg_info->msg_id=%s\n",msg_info->msg_id);
     cmd_len = strlen(msg_info->msg_id);
     printf("--------------cmd_len=%d\n",cmd_len);

    if ((cmd_len > 8)&&(strcmp(&msg_info->msg_id[cmd_len - 8],"Response") == 0))
    	strncpy(cmd, msg_info->msg_id, cmd_len - 8);
    else
    	strncpy(cmd, msg_info->msg_id, cmd_len);

    msg_id = MESSAGE_STR_TO_ID(cms,cmd);
    printf("--------------msg_id=%d,cmd=%s\n",msg_id,cmd);
    if (G_UNLIKELY(MESSAGE_INVALID_ID(msg_id)))
    {
        jpf_warning("translate message \"%s\" to id failed!", msg_info->msg_id);
        jpf_free_msginfo(msg_info);
        return NULL;
    }

    sys_msg = jpf_sysmsg_new(msg_id,
    	 msg_info->private_data,
    	 msg_info->private_size,
    	 seq,
    	 (JpfMsgPrivDes)msg_info->priv_destroy);

    jpf_free_msginfo_head(msg_info);

    return sys_msg;
}


gint
jpf_proto_create_xml_str(char *xml_buff, int *buff_size, JpfSysMsg *sys_msg)
{
	JpfMsgInfo  msg_info;
	gint        msg_id;
	gint        ret;

	G_ASSERT(xml_buff != NULL && buff_size != NULL && sys_msg != NULL);

	msg_id = MSG_GETID(sys_msg);
	//printf("------------_create_xml_str--msg_id=%d\n",msg_id);
	if (G_LIKELY(MESSAGE_ID_TO_STR(cms,msg_id)))
	{
		msg_info.msg_id[MAX_CMD_ID_LEN - 1] = 0;
		strncpy(msg_info.msg_id, MESSAGE_ID_TO_STR(cms,msg_id), MAX_CMD_ID_LEN -1);
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
		printf("id to str msg_id=%s\n", msg_info.msg_id);
	}

	msg_info.private_data = MSG_GET_DATA(sys_msg);
	msg_info.private_size = MSG_DATA_SIZE(sys_msg);

    ret = jpf_create_str_xml(xml_buff, buff_size, &msg_info);

    return ret;
	//return jpf_create_str_xml(xml_buff, buff_size, &msg_info);
}

