#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_msg_translate.h"


JpfMsgMap g_msg_map =
{
	{
		{"MdsRegister"},	/* 0 */
		{"MdsHeart"}, 		/* 1 */
		{ NULL }
	}
};


JpfMdsMsg * 
nmp_get_sysmsg_from_xml(char *xml_buff, gint xml_len, guint seq)
{
	JpfMsgInfo *msg_info; 
	JpfMdsMsg  *sys_msg;
	gchar       cmd[MAX_CMD_ID_LEN] = {0};
	gint        msg_id;
	gint        cmd_len;	

	msg_info = nmp_parse_str_xml(xml_buff, xml_len, seq);
	if (G_UNLIKELY(!msg_info))
	{
		nmp_warning("nmp_parse_str_xml() failed!");
		return NULL;
	}
	
    cmd_len = strlen(msg_info->msg_id);
	if ((cmd_len > 8) && (!strcmp(&msg_info->msg_id[cmd_len - 8],"Response")))
		strncpy(cmd, msg_info->msg_id, cmd_len - 8);
	else
	  	strncpy(cmd, msg_info->msg_id, cmd_len);

	msg_id = MESSAGE_STR_TO_ID(cmd);
	if (G_UNLIKELY(MESSAGE_INVALID_ID(msg_id)))
	{
		nmp_warning("translate message \"%s\" to id failed!", msg_info->msg_id);
		nmp_free_msginfo(msg_info);
		return NULL;
	}

    sys_msg = nmp_alloc_msg_2(msg_id,
    	 msg_info->private_data,
    	 msg_info->private_size,
    	 (JpfMsgFin)msg_info->priv_destroy,
    	 seq);
    nmp_free_msginfo_head(msg_info);

    return sys_msg;
}


gint 
nmp_proto_create_xml_str(char *xml_buff, int *buff_size, JpfMdsMsg *sys_msg)
{
	JpfMsgInfo  msg_info; 
	gint        msg_id;
	gint        ret;

	G_ASSERT(xml_buff != NULL && buff_size != NULL && sys_msg != NULL);

	msg_id = MSG_ID(sys_msg);

	if (G_LIKELY(MESSAGE_ID_TO_STR(msg_id)))
	{
		msg_info.msg_id[MAX_CMD_ID_LEN - 1] = 0;
		strncpy(msg_info.msg_id, MESSAGE_ID_TO_STR(msg_id), MAX_CMD_ID_LEN -1);
		if (MSG_RESPONSE(sys_msg))
		{
			if ((strlen(msg_info.msg_id) + 8) < (MAX_CMD_ID_LEN -1))
				strcat(msg_info.msg_id,"Response");

			else
				return -E_CMDLEN2LONG;

			ret = RES_CODE(MSG_DATA(sys_msg));
			if (ret < 0)
			{
			    printf("@##$$#@Q$#@$#@#$$@@$R@ Fix me!!!!@@@@@@@@@@@@@@@@@");
				SET_CODE(MSG_DATA(sys_msg), -ret);
			}
		}
		printf("id to str msg_id=%s\n", msg_info.msg_id);
	}

	msg_info.private_data = MSG_DATA(sys_msg);
	msg_info.private_size = MSG_DATA_SIZE(sys_msg);

    ret = nmp_create_str_xml(xml_buff, buff_size, &msg_info);
	printf("-------result code =%d\n",ret);
    return ret;
	//return nmp_create_str_xml(xml_buff, buff_size, &msg_info);	
}

