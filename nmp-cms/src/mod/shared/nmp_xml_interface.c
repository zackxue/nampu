#include "nmp_xml.h"
#include "nmp_msg_ams.h"


void
jpf_parse_ams_time_policy(gchar *time_policy, JpfActionPolicy *res_info)
{
	jpf_xml_parse_time_policy(time_policy, res_info);
}


