#include "nmp_mod_cms.h"
#include "nmp_cms_msg.h"
#include "nmp_message.h"
#include "nmp_sysctl.h"

gint
nmp_mos_cms_make_query_guids_msg(NmpModCms *self, NmpSysMsg **pmsg,
	gint start_row, gint num)
{
	NmpMssGetGuid req_info;
	NmpSysMsg *msg;

	memset(&req_info, 0, sizeof(req_info));

	strncpy(req_info.mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), 
		MSS_ID_LEN - 1);
	req_info.req_num = num;
	req_info.start_row = start_row;

	msg = nmp_sysmsg_new_2(MESSAGE_QUERY_GUIDS, &req_info,
		sizeof(req_info), ++self->cms_conn->seq_generator);

	*pmsg = msg;
	return (msg == NULL);
}


gint
nmp_mos_cms_make_query_mds_msg(NmpModCms *self, NmpSysMsg **pmsg,
	gint start_row, gint num)
{
	NmpMssGetMds req_info;
	NmpSysMsg *msg;

	memset(&req_info, 0, sizeof(req_info));

	strncpy(req_info.mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), 
		MSS_ID_LEN - 1);
	req_info.req_num = num;
	req_info.start_row = start_row;

	msg = nmp_sysmsg_new_2(MESSAGE_GET_ALL_MDS, &req_info,
		sizeof(req_info), ++self->cms_conn->seq_generator);

	*pmsg = msg;
	return (msg == NULL);
}


//:~ End
