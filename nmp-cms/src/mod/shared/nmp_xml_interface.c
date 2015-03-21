#include "nmp_xml.h"
#include "nmp_msg_ams.h"


void
nmp_parse_ams_time_policy(gchar *time_policy, NmpActionPolicy *res_info)
{
	nmp_xml_parse_time_policy(time_policy, res_info);
}


