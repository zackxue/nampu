
#include "nmp_xmlmem.h"
#include "nmp_packet.h"
#include "nmp_transform.h"


void process_device_info(DeviceInfoPacket *g_dev_info, JDeviceInfo *dev_info, JAction action)
{
	J_ASSERT(g_dev_info && dev_info);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)dev_info->manu_info, (const char*)g_dev_info->manu_info);
			strcpy((char*)dev_info->release_date, (const char*)g_dev_info->release_date);
			strcpy((char*)dev_info->dev_version, (const char*)g_dev_info->dev_version);
			strcpy((char*)dev_info->hw_version, (const char*)g_dev_info->hw_version);
			
			dev_info->pu_type     = g_dev_info->pu_type;
			dev_info->sub_type    = g_dev_info->sub_type;
			dev_info->di_num      = g_dev_info->di_num;
			dev_info->do_num      = g_dev_info->do_num;
			dev_info->channel_num = g_dev_info->channel_num;
			dev_info->rs485_num   = g_dev_info->rs485_num;
			dev_info->rs232_num   = g_dev_info->rs232_num;
			break;
		case ACTION_PACK:
			strcpy((char*)g_dev_info->manu_info, (const char*)dev_info->manu_info);
			strcpy((char*)g_dev_info->release_date, (const char*)dev_info->release_date);
			strcpy((char*)g_dev_info->dev_version, (const char*)dev_info->dev_version);
			strcpy((char*)g_dev_info->hw_version, (const char*)dev_info->hw_version);
			
			g_dev_info->pu_type     = dev_info->pu_type;
			g_dev_info->sub_type    = dev_info->sub_type;
			g_dev_info->di_num      = dev_info->di_num;
			g_dev_info->do_num      = dev_info->do_num;
			g_dev_info->channel_num = dev_info->channel_num;
			g_dev_info->rs485_num   = dev_info->rs485_num;
			g_dev_info->rs232_num   = dev_info->rs232_num;
			break;
		default:
			break;
	}
}
void process_device_ntp_info(DeviceNTPInfoPacket *g_dev_ntp_info, 
										JDeviceNTPInfo *dev_ntp_info, JAction action)
{
	J_ASSERT(g_dev_ntp_info && dev_ntp_info);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)dev_ntp_info->ntp_server_ip, (const char*)g_dev_ntp_info->ntp_server_ip);
			dev_ntp_info->time_zone     = g_dev_ntp_info->time_zone;
			dev_ntp_info->time_interval = g_dev_ntp_info->time_interval;
			dev_ntp_info->ntp_enable    = g_dev_ntp_info->ntp_enable;
			dev_ntp_info->dst_enable    = g_dev_ntp_info->dst_enable;
			dev_ntp_info->reserve       = g_dev_ntp_info->reserve;
			break;
		case ACTION_PACK:
			strcpy((char*)g_dev_ntp_info->ntp_server_ip, (const char*)dev_ntp_info->ntp_server_ip);
			g_dev_ntp_info->time_zone     = dev_ntp_info->time_zone;
			g_dev_ntp_info->time_interval = dev_ntp_info->time_interval;
			g_dev_ntp_info->ntp_enable    = dev_ntp_info->ntp_enable;
			g_dev_ntp_info->dst_enable    = dev_ntp_info->dst_enable;
			g_dev_ntp_info->reserve       = dev_ntp_info->reserve;
			break;
		default:
			break;
	}
}
void process_device_time(DeviceTimePacket *g_dev_time, JDeviceTime *dev_time, JAction action)
{
	J_ASSERT(g_dev_time && dev_time);

	switch (action)
	{
		case ACTION_UNPACK:
			memcpy(&(dev_time->time), &(g_dev_time->time), sizeof(JTime));
			dev_time->zone        = g_dev_time->zone;
			dev_time->sync_enable = g_dev_time->sync_enable;
			break;
		case ACTION_PACK:
			memcpy(&(g_dev_time->time), &(dev_time->time), sizeof(JTime));
			g_dev_time->zone        = dev_time->zone;
			g_dev_time->sync_enable = dev_time->sync_enable;
			break;
		default:
			break;
	}
}
void process_platform_info(PlatformInfoPacket *g_pltf_info, 
									JPlatformInfo *pltf_info, JAction action)
{
	J_ASSERT(g_pltf_info && pltf_info);
	
	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)pltf_info->pu_id, (const char*)g_pltf_info->pu_id);
			strcpy((char*)pltf_info->cms_ip, (const char*)g_pltf_info->cms_ip);
			strcpy((char*)pltf_info->mds_ip, (const char*)g_pltf_info->mds_ip);
			pltf_info->cms_port   = g_pltf_info->cms_port;
			pltf_info->mds_port   = g_pltf_info->mds_port;
			pltf_info->protocol   = g_pltf_info->protocol;
			pltf_info->is_con_cms = g_pltf_info->is_con_cms;
			break;
		case ACTION_PACK:
			strcpy((char*)g_pltf_info->pu_id, (const char*)pltf_info->pu_id);
			strcpy((char*)g_pltf_info->cms_ip, (const char*)pltf_info->cms_ip);
			strcpy((char*)g_pltf_info->mds_ip, (const char*)pltf_info->mds_ip);
			g_pltf_info->cms_port   = pltf_info->cms_port;
			g_pltf_info->mds_port   = pltf_info->mds_port;
			g_pltf_info->protocol   = pltf_info->protocol;
			g_pltf_info->is_con_cms = pltf_info->is_con_cms;
			break;
		default:
			break;
	}
}
void process_network_info(NetworkInfoPacket *g_net_info, JNetworkInfo *net_info, JAction action)
{
	J_ASSERT(g_net_info && net_info);

	switch (action)
	{
		case ACTION_UNPACK:
			memcpy(&(net_info->network), &(g_net_info->network), 
				sizeof(JNetwork)*J_SDK_MAX_NETWORK_TYPE);
			strcpy((char*)net_info->main_dns, (const char*)g_net_info->main_dns);
			strcpy((char*)net_info->backup_dns, (const char*)g_net_info->backup_dns);
			net_info->auto_dns_enable = g_net_info->auto_dns_enable;
			net_info->cur_network     = g_net_info->cur_network;
			net_info->cmd_port        = g_net_info->cmd_port;
			net_info->data_port       = g_net_info->data_port;
			net_info->web_port        = g_net_info->web_port;
            net_info->talk_port       = g_net_info->talk_port;
			break;
		case ACTION_PACK:
			memcpy(&(g_net_info->network), &(net_info->network), 
				sizeof(JNetwork)*J_SDK_MAX_NETWORK_TYPE);
			strcpy((char*)g_net_info->main_dns, (const char*)net_info->main_dns);
			strcpy((char*)g_net_info->backup_dns, (const char*)net_info->backup_dns);
			g_net_info->auto_dns_enable = net_info->auto_dns_enable;
			g_net_info->cur_network     = net_info->cur_network;
			g_net_info->cmd_port        = net_info->cmd_port;
			g_net_info->data_port       = net_info->data_port;
			g_net_info->web_port        = net_info->web_port;
            g_net_info->talk_port       = net_info->talk_port;
			break;
		default:
			break;
	}
}
void process_pppoe_info(PPPOEInfoPacket *g_pppoe_info, JPPPOEInfo *pppoe_info, JAction action)
{
	J_ASSERT(g_pppoe_info && pppoe_info);

	switch (action)
	{
		case ACTION_UNPACK:
			pppoe_info->type   = g_pppoe_info->type;
			pppoe_info->enable = g_pppoe_info->enable;
			strcpy((char*)pppoe_info->ip, (const char*)g_pppoe_info->ip);
			strcpy((char*)pppoe_info->account, (const char*)g_pppoe_info->account);
			strcpy((char*)pppoe_info->passwd, (const char*)g_pppoe_info->passwd);
			break;
		case ACTION_PACK:
			g_pppoe_info->type   = pppoe_info->type;
			g_pppoe_info->enable = pppoe_info->enable;
			strcpy((char*)g_pppoe_info->ip, (const char*)pppoe_info->ip);
			strcpy((char*)g_pppoe_info->account, (const char*)pppoe_info->account);
			strcpy((char*)g_pppoe_info->passwd, (const char*)pppoe_info->passwd);
			break;
		default:
			break;
	}
}
void process_encode_parameter(EncodeParameterPacket *g_encode_para, 
											JEncodeParameter *encode_para, JAction action)
{
	J_ASSERT(g_encode_para && encode_para);

	switch (action)
	{
		case ACTION_UNPACK:
			//encode_para->level            = g_encode_para->level;
			encode_para->frame_rate       = g_encode_para->frame_rate;
			encode_para->i_frame_interval = g_encode_para->i_frame_interval;
			encode_para->video_type       = g_encode_para->video_type;
			encode_para->audio_type       = g_encode_para->audio_type;
			encode_para->au_in_mode       = g_encode_para->au_in_mode;
			encode_para->audio_enble      = g_encode_para->audio_enble;
			encode_para->resolution       = g_encode_para->resolution;
			encode_para->qp_value         = g_encode_para->qp_value;
			encode_para->code_rate        = g_encode_para->code_rate;
			encode_para->frame_priority   = g_encode_para->frame_priority;
			encode_para->format           = g_encode_para->format;
			encode_para->bit_rate         = g_encode_para->bit_rate;
			encode_para->encode_level     = g_encode_para->encode_level;
			break;
		case ACTION_PACK:
			//g_encode_para->level            = encode_para->level;
			g_encode_para->frame_rate       = encode_para->frame_rate;
			g_encode_para->i_frame_interval = encode_para->i_frame_interval;
			g_encode_para->video_type       = encode_para->video_type;
			g_encode_para->audio_type       = encode_para->audio_type;
			g_encode_para->au_in_mode       = encode_para->au_in_mode;
			g_encode_para->audio_enble      = encode_para->audio_enble;
			g_encode_para->resolution       = encode_para->resolution;
			g_encode_para->qp_value         = encode_para->qp_value;
			g_encode_para->code_rate        = encode_para->code_rate;
			g_encode_para->frame_priority   = encode_para->frame_priority;
			g_encode_para->format           = encode_para->format;
			g_encode_para->bit_rate         = encode_para->bit_rate;
			g_encode_para->encode_level     = encode_para->encode_level;
			break;
		default:
			break;
	}
}
void process_display_parameter(DisplayParameterPacket *g_display_para, 
											JDisplayParameter *display_para, JAction action)
{
	J_ASSERT(g_display_para && display_para);

	switch (action)
	{
		case ACTION_UNPACK:
			display_para->contrast   = g_display_para->contrast;
			display_para->bright     = g_display_para->bright;
			display_para->hue        = g_display_para->hue;
			display_para->saturation = g_display_para->saturation;
			display_para->sharpness  = g_display_para->sharpness;
			break;
		case ACTION_PACK:
			g_display_para->contrast   = display_para->contrast;
			g_display_para->bright     = display_para->bright;
			g_display_para->hue        = display_para->hue;
			g_display_para->saturation = display_para->saturation;
			g_display_para->sharpness  = display_para->sharpness;
			break;
		default:
			break;
	}
}
void process_record_parameter(RecordParameterPacket *g_record_para, 
										JRecordParameter *record_para, JAction action)
{
	J_ASSERT(g_record_para && record_para);

	switch (action)
	{
		case ACTION_UNPACK:
			//record_para->level      = g_record_para->level;
			record_para->pre_record = g_record_para->pre_record;
			record_para->auto_cover = g_record_para->auto_cover;
			memcpy(&(record_para->week), &(g_record_para->week), sizeof(JWeek));
			break;
		case ACTION_PACK:
			//g_record_para->level      = record_para->level;
			g_record_para->pre_record = record_para->pre_record;
			g_record_para->auto_cover = record_para->auto_cover;
			memcpy(&(g_record_para->week), &(record_para->week), sizeof(JWeek));
			break;
		default:
			break;
	}
}
void process_hide_parameter(HideParameterPacket *g_hide_para, 
										JHideParameter *hide_para, JAction action)
{
	J_ASSERT(g_hide_para && hide_para);

	switch (action)
	{
		case ACTION_UNPACK:
			hide_para->hide_enable = g_hide_para->hide_enable;
			hide_para->hide_color  = g_hide_para->hide_color;
			memcpy(&(hide_para->hide_area), &(g_hide_para->hide_area), sizeof(JArea));
			hide_para->max_width  = g_hide_para->max_width;
			hide_para->max_height = g_hide_para->max_height;
			break;
		case ACTION_PACK:
			g_hide_para->hide_enable = hide_para->hide_enable;
			g_hide_para->hide_color  = hide_para->hide_color;
			memcpy(&(g_hide_para->hide_area), &(hide_para->hide_area), sizeof(JArea));
			g_hide_para->max_width  = hide_para->max_width;
			g_hide_para->max_height = hide_para->max_height;
			break;
		default:
			break;
	}
}
void process_serial_parameter(SerialParameterPacket *g_serial_para, 
										JSerialParameter *serial_para, JAction action)
{
	J_ASSERT(g_serial_para && serial_para);

	switch (action)
	{
		case ACTION_UNPACK:
			serial_para->serial_no = g_serial_para->serial_no;
			serial_para->baud_rate = g_serial_para->baud_rate;
			serial_para->data_bit  = g_serial_para->data_bit;
			serial_para->stop_bit  = g_serial_para->stop_bit;
			serial_para->verify    = g_serial_para->verify;
			break;
		case ACTION_PACK:
			g_serial_para->serial_no = serial_para->serial_no;
			g_serial_para->baud_rate = serial_para->baud_rate;
			g_serial_para->data_bit  = serial_para->data_bit;
			g_serial_para->stop_bit  = serial_para->stop_bit;
			g_serial_para->verify    = serial_para->verify;
			break;
		default:
			break;
	}
}
void process_osd_parameter(OSDParameterPacket *g_osd_para, 
										JOSDParameter *osd_para, JAction action)
{
	J_ASSERT(g_osd_para && osd_para);

	switch (action)
	{
		case ACTION_UNPACK:
			osd_para->time_enable        = g_osd_para->time_enable;
			osd_para->time_display_x     = g_osd_para->time_display_x;
			osd_para->time_display_y     = g_osd_para->time_display_y;
			osd_para->time_display_color = g_osd_para->time_display_color;
			osd_para->text_enable        = g_osd_para->text_enable;
			osd_para->text_display_x     = g_osd_para->text_display_x;
			osd_para->text_display_y     = g_osd_para->text_display_y;
			osd_para->text_display_color = g_osd_para->text_display_color;
			osd_para->max_width          = g_osd_para->max_width;
			osd_para->max_height         = g_osd_para->max_height;
			strcpy((char*)osd_para->text_data, (const char*)g_osd_para->text_data);
			osd_para->stream_enable      = g_osd_para->stream_enable;
			osd_para->time_display_w     = g_osd_para->time_display_w;
			osd_para->time_display_h     = g_osd_para->time_display_h;
			osd_para->text_display_w     = g_osd_para->text_display_w;
			osd_para->text_display_h     = g_osd_para->text_display_h;
			osd_para->osd_invert_color   = g_osd_para->osd_invert_color;
            memcpy(&osd_para->ext_osd, &g_osd_para->ext_osd, sizeof(JOSDExtend));
			break;
		case ACTION_PACK:
			g_osd_para->time_enable        = osd_para->time_enable;
			g_osd_para->time_display_x     = osd_para->time_display_x;
			g_osd_para->time_display_y     = osd_para->time_display_y;
			g_osd_para->time_display_color = osd_para->time_display_color;
			g_osd_para->text_enable        = osd_para->text_enable;
			g_osd_para->text_display_x     = osd_para->text_display_x;
			g_osd_para->text_display_y     = osd_para->text_display_y;
			g_osd_para->text_display_color = osd_para->text_display_color;
			g_osd_para->max_width          = osd_para->max_width;
			g_osd_para->max_height         = osd_para->max_height;
			strcpy((char*)g_osd_para->text_data, (const char*)osd_para->text_data);
			g_osd_para->stream_enable      = osd_para->stream_enable;
			g_osd_para->time_display_w     = osd_para->time_display_w;
			g_osd_para->time_display_h     = osd_para->time_display_h;
			g_osd_para->text_display_w     = osd_para->text_display_w;
			g_osd_para->text_display_h     = osd_para->text_display_h;
			g_osd_para->osd_invert_color   = osd_para->osd_invert_color;
            memcpy(&g_osd_para->ext_osd, &osd_para->ext_osd, sizeof(JOSDExtend));
			break;
		default:
			break;
	}
}
void process_ptz_parameter(PTZParameterPacket *g_ptz_para, 
									JPTZParameter *ptz_para, JAction action)
{
	J_ASSERT(g_ptz_para && ptz_para);

	switch (action)
	{
		case ACTION_UNPACK:
			ptz_para->serial_no = g_ptz_para->serial_no;
			ptz_para->ptz_addr  = g_ptz_para->ptz_addr;
			ptz_para->protocol  = g_ptz_para->protocol;
			ptz_para->baud_rate = g_ptz_para->baud_rate;
			ptz_para->data_bit  = g_ptz_para->data_bit;
			ptz_para->stop_bit  = g_ptz_para->stop_bit;
			ptz_para->verify    = g_ptz_para->verify;
			break;
		case ACTION_PACK:
			g_ptz_para->serial_no = ptz_para->serial_no;
			g_ptz_para->ptz_addr  = ptz_para->ptz_addr;
			g_ptz_para->protocol  = ptz_para->protocol;
			g_ptz_para->baud_rate = ptz_para->baud_rate;
			g_ptz_para->data_bit  = ptz_para->data_bit;
			g_ptz_para->stop_bit  = ptz_para->stop_bit;
			g_ptz_para->verify    = ptz_para->verify;
			break;
		default:
			break;
	}
}
void process_ftp_parameter(FTPParameterPacket *g_ftp_para, 
									JFTPParameter *ftp_para, JAction action)
{
	J_ASSERT(g_ftp_para && ftp_para);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)ftp_para->ftp_ip, (const char*)g_ftp_para->ftp_ip);
			strcpy((char*)ftp_para->ftp_usr, (const char*)g_ftp_para->ftp_usr);
			strcpy((char*)ftp_para->ftp_pwd, (const char*)g_ftp_para->ftp_pwd);
			strcpy((char*)ftp_para->ftp_path, (const char*)g_ftp_para->ftp_path);
			ftp_para->ftp_port = g_ftp_para->ftp_port;
			break;
		case ACTION_PACK:
			strcpy((char*)g_ftp_para->ftp_ip, (const char*)ftp_para->ftp_ip);
			strcpy((char*)g_ftp_para->ftp_usr, (const char*)ftp_para->ftp_usr);
			strcpy((char*)g_ftp_para->ftp_pwd, (const char*)ftp_para->ftp_pwd);
			strcpy((char*)g_ftp_para->ftp_path, (const char*)ftp_para->ftp_path);
			g_ftp_para->ftp_port = ftp_para->ftp_port;
			break;
		default:
			break;
	}
}
void process_smtp_parameter(SMTPParameterPacket *g_smtp_para, 
										JSMTPParameter *smtp_para, JAction action)
{
	J_ASSERT(g_smtp_para && smtp_para);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)smtp_para->mail_ip, (const char*)g_smtp_para->mail_ip);
			strcpy((char*)smtp_para->mail_addr, (const char*)g_smtp_para->mail_addr);
			strcpy((char*)smtp_para->mail_usr, (const char*)g_smtp_para->mail_usr);
			strcpy((char*)smtp_para->mail_pwd, (const char*)g_smtp_para->mail_pwd);
			strcpy((char*)smtp_para->mail_rctp1, (const char*)g_smtp_para->mail_rctp1);
			strcpy((char*)smtp_para->mail_rctp2, (const char*)g_smtp_para->mail_rctp2);
			strcpy((char*)smtp_para->mail_rctp3, (const char*)g_smtp_para->mail_rctp3);
			smtp_para->mail_port  = g_smtp_para->mail_port;
			smtp_para->ssl_enable = g_smtp_para->ssl_enable;
			break;
		case ACTION_PACK:
			strcpy((char*)g_smtp_para->mail_ip, (const char*)smtp_para->mail_ip);
			strcpy((char*)g_smtp_para->mail_addr, (const char*)smtp_para->mail_addr);
			strcpy((char*)g_smtp_para->mail_usr, (const char*)smtp_para->mail_usr);
			strcpy((char*)g_smtp_para->mail_pwd, (const char*)smtp_para->mail_pwd);
			strcpy((char*)g_smtp_para->mail_rctp1, (const char*)smtp_para->mail_rctp1);
			strcpy((char*)g_smtp_para->mail_rctp2, (const char*)smtp_para->mail_rctp2);
			strcpy((char*)g_smtp_para->mail_rctp3, (const char*)smtp_para->mail_rctp3);
			g_smtp_para->mail_port  = smtp_para->mail_port;
			g_smtp_para->ssl_enable = smtp_para->ssl_enable;
			break;
		default:
			break;
	}
}
void process_upnp_parameter(UPNPParameterPacket *g_upnp_para, 
										JUPNPParameter *upnp_para, JAction action)
{
	J_ASSERT(g_upnp_para && upnp_para);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)upnp_para->upnp_ip, (const char*)g_upnp_para->upnp_ip);
			upnp_para->upnp_enable           = g_upnp_para->upnp_enable;
			upnp_para->upnp_eth_no           = g_upnp_para->upnp_eth_no;
			upnp_para->upnp_model            = g_upnp_para->upnp_model;
			upnp_para->upnp_refresh_time     = g_upnp_para->upnp_refresh_time;
			upnp_para->upnp_data_port        = g_upnp_para->upnp_data_port;
			upnp_para->upnp_web_port         = g_upnp_para->upnp_web_port;
			upnp_para->upnp_data_port_result = g_upnp_para->upnp_data_port_result;
			upnp_para->upnp_web_port_result  = g_upnp_para->upnp_web_port_result;
            upnp_para->upnp_cmd_port         = g_upnp_para->upnp_cmd_port;
			upnp_para->upnp_talk_port        = g_upnp_para->upnp_talk_port;
			upnp_para->upnp_cmd_port_result  = g_upnp_para->upnp_cmd_port_result;
			upnp_para->upnp_talk_port_result = g_upnp_para->upnp_talk_port_result;
			break;
		case ACTION_PACK:
			strcpy((char*)g_upnp_para->upnp_ip, (const char*)upnp_para->upnp_ip);
			g_upnp_para->upnp_enable           = upnp_para->upnp_enable;
			g_upnp_para->upnp_eth_no           = upnp_para->upnp_eth_no;
			g_upnp_para->upnp_model            = upnp_para->upnp_model;
			g_upnp_para->upnp_refresh_time     = upnp_para->upnp_refresh_time;
			g_upnp_para->upnp_data_port        = upnp_para->upnp_data_port;
			g_upnp_para->upnp_web_port         = upnp_para->upnp_web_port;
			g_upnp_para->upnp_data_port_result = upnp_para->upnp_data_port_result;
			g_upnp_para->upnp_web_port_result  = upnp_para->upnp_web_port_result;
            g_upnp_para->upnp_cmd_port         = upnp_para->upnp_cmd_port;
			g_upnp_para->upnp_talk_port        = upnp_para->upnp_talk_port;
			g_upnp_para->upnp_cmd_port_result  = upnp_para->upnp_cmd_port_result;
			g_upnp_para->upnp_talk_port_result = upnp_para->upnp_talk_port_result;
			break;
		default:
			break;
	}
}
void process_device_disk_info(DeviceDiskInfoPacket *g_disk_info, 
									JDeviceDiskInfo *disk_info, JAction action)
{
	J_ASSERT(g_disk_info && disk_info);

	switch (action)
	{
		case ACTION_UNPACK:
			disk_info->disk_num = g_disk_info->disk_num;
			memcpy(&(disk_info->disk), &(g_disk_info->disk), 
				sizeof(JDiskInfo)*J_SDK_MAX_DISK_NUMBER);
			break;
		case ACTION_PACK:
			g_disk_info->disk_num = disk_info->disk_num;
			memcpy(&(g_disk_info->disk), &(disk_info->disk), 
				sizeof(JDiskInfo)*J_SDK_MAX_DISK_NUMBER);
			break;
		default:
			break;
	}
}
void process_format_disk(FormatDiskPacket *g_format_disk, 
									JFormatDisk *format_disk, JAction action)
{
	J_ASSERT(g_format_disk && format_disk);

	switch (action)
	{
		case ACTION_UNPACK:
			format_disk->disk_no  = g_format_disk->disk_no;
			format_disk->progress = g_format_disk->progress;
			break;
		case ACTION_PACK:
			g_format_disk->disk_no  = format_disk->disk_no;
			g_format_disk->progress = format_disk->progress;
			break;
		default:
			break;
	}
}
void process_move_alarm(MoveAlarmPacket *g_move_alarm, 
									JMoveAlarm *move_alarm, JAction action)
{
	J_ASSERT(g_move_alarm && move_alarm);

	switch (action)
	{
		case ACTION_UNPACK:
			move_alarm->move_enable     = g_move_alarm->move_enable;
			move_alarm->sensitive_level = g_move_alarm->sensitive_level;
			move_alarm->detect_interval = g_move_alarm->detect_interval;
			move_alarm->max_width       = g_move_alarm->max_width;
			move_alarm->max_height      = g_move_alarm->max_height;
			memcpy(&(move_alarm->detect_area), &(g_move_alarm->detect_area), sizeof(JArea));
			memcpy(&(move_alarm->week), &(g_move_alarm->week), sizeof(JWeek));
			break;
		case ACTION_PACK:
			g_move_alarm->move_enable     = move_alarm->move_enable;
			g_move_alarm->sensitive_level = move_alarm->sensitive_level;
			g_move_alarm->detect_interval = move_alarm->detect_interval;
			g_move_alarm->max_width       = move_alarm->max_width;
			g_move_alarm->max_height      = move_alarm->max_height;
			memcpy(&(g_move_alarm->detect_area), &(move_alarm->detect_area), sizeof(JArea));
			memcpy(&(g_move_alarm->week), &(move_alarm->week), sizeof(JWeek));
			break;
		default:
			break;
	}
}
void process_lost_alarm(LostAlarmPacket *g_lost_alarm, 
									JLostAlarm *lost_alarm, JAction action)
{
	J_ASSERT(g_lost_alarm && lost_alarm);

	switch (action)
	{
		case ACTION_UNPACK:
			lost_alarm->lost_enable     = g_lost_alarm->lost_enable;
			lost_alarm->detect_interval = g_lost_alarm->detect_interval;
			memcpy(&(lost_alarm->week), &(g_lost_alarm->week), sizeof(JWeek));
			break;
		case ACTION_PACK:
			g_lost_alarm->lost_enable     = lost_alarm->lost_enable;
			g_lost_alarm->detect_interval = lost_alarm->detect_interval;
			memcpy(&(g_lost_alarm->week), &(lost_alarm->week), sizeof(JWeek));
			break;
		default:
			break;
	}
}
void process_hide_alarm(HideAlarmPacket *g_hide_alarm, 
									JHideAlarm *hide_alarm, JAction action)
{
	J_ASSERT(g_hide_alarm && hide_alarm);

	switch (action)
	{
		case ACTION_UNPACK:
			hide_alarm->hide_enable     = g_hide_alarm->hide_enable;
			hide_alarm->detect_interval = g_hide_alarm->detect_interval;
			hide_alarm->sensitive_level = g_hide_alarm->sensitive_level;
			hide_alarm->max_width       = g_hide_alarm->max_width;
			hide_alarm->max_height      = g_hide_alarm->max_height;
			memcpy(&(hide_alarm->detect_area), &(g_hide_alarm->detect_area), sizeof(JArea));
			memcpy(&(hide_alarm->week), &(g_hide_alarm->week), sizeof(JWeek));
			break;
		case ACTION_PACK:
			g_hide_alarm->hide_enable     = hide_alarm->hide_enable;
			g_hide_alarm->detect_interval = hide_alarm->detect_interval;
			g_hide_alarm->sensitive_level = hide_alarm->sensitive_level;
			g_hide_alarm->max_width       = hide_alarm->max_width;
			g_hide_alarm->max_height      = hide_alarm->max_height;
			memcpy(&(g_hide_alarm->detect_area), &(hide_alarm->detect_area), sizeof(JArea));
			memcpy(&(g_hide_alarm->week), &(hide_alarm->week), sizeof(JWeek));
			break;
		default:
			break;
	}
}
void process_io_alarm(IoAlarmPacket *g_io_alarm, JIoAlarm *io_alarm, JAction action)
{
	J_ASSERT(g_io_alarm && io_alarm);

	switch (action)
	{
		case ACTION_UNPACK:
			io_alarm->io_type    = g_io_alarm->io_type;
			io_alarm->alarm_enable    = g_io_alarm->alarm_enable;
			io_alarm->detect_interval = g_io_alarm->detect_interval;
			memcpy(&(io_alarm->week), &(g_io_alarm->week), sizeof(JWeek));
			break;
		case ACTION_PACK:
			g_io_alarm->io_type    = io_alarm->io_type;
			g_io_alarm->alarm_enable    = io_alarm->alarm_enable;
			g_io_alarm->detect_interval = io_alarm->detect_interval;
			memcpy(&(g_io_alarm->week), &(io_alarm->week), sizeof(JWeek));
			break;
		default:
			break;
	}
}
void process_joint_action(JointActionPacket *g_joint_action, 
		JJointAction *joint_action, JAction action)
{
	J_ASSERT(g_joint_action && joint_action);

	switch (action)
	{
		case ACTION_UNPACK:
			joint_action->alarm_type = g_joint_action->alarm_type;
			memcpy(&(joint_action->joint), &(g_joint_action->joint), sizeof(JJoint));
			break;
		case ACTION_PACK:
			g_joint_action->alarm_type = joint_action->alarm_type;
			memcpy(&(g_joint_action->joint), &(joint_action->joint), sizeof(JJoint));
			break;
		default:
			break;
	}
}
void process_control_ptz_cmd(PTZControlPacket *g_ptz_ctrl, 
		JPTZControl *ptz_ctrl, JAction action)
{
	J_ASSERT(g_ptz_ctrl && ptz_ctrl);

	switch (action)
	{
		case ACTION_UNPACK:
			ptz_ctrl->action = g_ptz_ctrl->action;
			ptz_ctrl->param  = g_ptz_ctrl->param;
			break;
		case ACTION_PACK:
			g_ptz_ctrl->action = ptz_ctrl->action;
			g_ptz_ctrl->param  = ptz_ctrl->param;
			break;
		default:
			break;
	}
}
void process_store_log(StoreLogPacket *g_store, 
		JStoreLog *store, JAction action)
{
	J_ASSERT(g_store && store);

	switch (action)
	{
		case ACTION_UNPACK:
			store->rec_type     = g_store->rec_type;
			store->beg_node     = g_store->beg_node;
			store->end_node     = g_store->end_node;
			memcpy(&store->beg_time, &g_store->beg_time, sizeof(JTime));
			memcpy(&store->end_time, &g_store->end_time, sizeof(JTime));
			
			store->node_count   = g_store->node_count;
			store->total_count  = g_store->total_count;
			store->sess_id      = g_store->sess_id;
			memcpy(&store->store, &g_store->store, sizeof(Store)*J_SDK_MAX_STORE_LOG_SIZE);
			break;
		case ACTION_PACK:
			g_store->rec_type     = store->rec_type;
			g_store->beg_node     = store->beg_node;
			g_store->end_node     = store->end_node;
			memcpy(&g_store->beg_time, &store->beg_time, sizeof(JTime));
			memcpy(&g_store->end_time, &store->end_time, sizeof(JTime));
			
			g_store->node_count   = store->node_count;
			g_store->total_count  = store->total_count;
			g_store->sess_id      = store->sess_id;
			memcpy(&g_store->store, &store->store, sizeof(Store)*J_SDK_MAX_STORE_LOG_SIZE);
			break;
		default:
			break;
	}
}
void process_firmware_upgrade(FirmwareUpgradePacket *g_upgrade, 
		JFirmwareUpgrade *upgrade, JAction action)
{
	J_ASSERT(g_upgrade && upgrade);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)upgrade->data, (const char*)g_upgrade->data);
			strcpy((char*)upgrade->addr, (const char*)g_upgrade->addr);
			upgrade->data_len    = g_upgrade->data_len;
			upgrade->file_len    = g_upgrade->file_len;
			upgrade->percent     = g_upgrade->percent;
			upgrade->sess_id     = g_upgrade->sess_id;
			break;
		case ACTION_PACK:
			strcpy((char*)g_upgrade->data, (const char*)upgrade->data);
			strcpy((char*)g_upgrade->addr, (const char*)upgrade->addr);
			g_upgrade->data_len    = upgrade->data_len;
			g_upgrade->file_len    = upgrade->file_len;
			g_upgrade->percent     = upgrade->percent;
			g_upgrade->sess_id     = upgrade->sess_id;
			break;
			
		default:
			break;
	}
}
void process_channel_info(ChannelInfoPacket *g_ch_info, 
		JChannelInfo *ch_info, JAction action)
{
	J_ASSERT(g_ch_info && ch_info);

	switch (action)
	{
		case ACTION_UNPACK:
            ch_info->ch_no     = g_ch_info->ch_no;
            ch_info->ch_type   = g_ch_info->ch_type;
            ch_info->ch_status = g_ch_info->ch_status;
			strcpy((char*)ch_info->ch_name, (const char*)g_ch_info->ch_name);
            memcpy(&ch_info->rmt_ch_info, &g_ch_info->rmt_ch_info, 
				sizeof(JRemoteChannelInfo));
			memcpy(&ch_info->rmt_dev_info, &g_ch_info->rmt_dev_info, 
				sizeof(JRemoteDeviceInfo));
            break;
		case ACTION_PACK:
            g_ch_info->ch_no     = ch_info->ch_no;
            g_ch_info->ch_type   = ch_info->ch_type;
            g_ch_info->ch_status = ch_info->ch_status;
			strcpy((char*)g_ch_info->ch_name, (const char*)ch_info->ch_name);
            memcpy(&g_ch_info->rmt_ch_info, &ch_info->rmt_ch_info, 
				sizeof(JRemoteChannelInfo));
			memcpy(&g_ch_info->rmt_dev_info, &ch_info->rmt_dev_info, 
				sizeof(JRemoteDeviceInfo));
			break;
		default:
			break;
	}
}
void process_picture_info(PictureInfoPacket *g_pic_info, 
		JPictureInfo *pic_info, JAction action)
{
	J_ASSERT(g_pic_info && pic_info);

	switch (action)
	{
		case ACTION_UNPACK:
            pic_info->mirror    = g_pic_info->mirror;
            pic_info->flip      = g_pic_info->flip;
            pic_info->hz        = g_pic_info->hz;
            pic_info->awb_mode  = g_pic_info->awb_mode;
            pic_info->awb_red   = g_pic_info->awb_red;
            pic_info->awb_blue  = g_pic_info->awb_blue;
            pic_info->wdr       = g_pic_info->wdr;
            pic_info->iris_type = g_pic_info->iris_type;
            pic_info->exp_compensation = g_pic_info->exp_compensation;
            pic_info->ae_mode = g_pic_info->ae_mode;
            break;
		case ACTION_PACK:
            g_pic_info->mirror    = pic_info->mirror;
            g_pic_info->flip      = pic_info->flip;
            g_pic_info->hz        = pic_info->hz;
            g_pic_info->awb_mode  = pic_info->awb_mode;
            g_pic_info->awb_red   = pic_info->awb_red;
            g_pic_info->awb_blue  = pic_info->awb_blue;
            g_pic_info->wdr       = pic_info->wdr;
            g_pic_info->iris_type = pic_info->iris_type;
            g_pic_info->exp_compensation = pic_info->exp_compensation;
            g_pic_info->ae_mode = pic_info->ae_mode;
			break;
		default:
			break;
	}
}
void process_wifi_config(WifiConfigPacket *g_wifi_cfg, 
		JWifiConfig *wifi_cfg, JAction action)
{
	J_ASSERT(g_wifi_cfg && wifi_cfg);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)wifi_cfg->essid, (const char*)g_wifi_cfg->essid);
			strcpy((char*)wifi_cfg->pwd,   (const char*)g_wifi_cfg->pwd);
            wifi_cfg->wifi_enable     = g_wifi_cfg->wifi_enable;
            wifi_cfg->encrypt_type    = g_wifi_cfg->encrypt_type;
            wifi_cfg->auth_mode       = g_wifi_cfg->auth_mode;
            wifi_cfg->secret_key_type = g_wifi_cfg->secret_key_type;
            wifi_cfg->wifi_st         = g_wifi_cfg->wifi_st;
            break;
		case ACTION_PACK:
			strcpy((char*)g_wifi_cfg->essid, (const char*)wifi_cfg->essid);
			strcpy((char*)g_wifi_cfg->pwd,   (const char*)wifi_cfg->pwd);
            g_wifi_cfg->wifi_enable     = wifi_cfg->wifi_enable;
            g_wifi_cfg->encrypt_type    = wifi_cfg->encrypt_type;
            g_wifi_cfg->auth_mode       = wifi_cfg->auth_mode;
            g_wifi_cfg->secret_key_type = wifi_cfg->secret_key_type;
            g_wifi_cfg->wifi_st         = wifi_cfg->wifi_st;
			break;
		default:
			break;
	}
}
void process_wifi_search(WifiSearchResPacket *g_wifi_search, 
		JWifiSearchRes *wifi_search, JAction action)
{
	J_ASSERT(g_wifi_search && wifi_search);

	switch (action)
	{
		case ACTION_UNPACK:
            wifi_search->count = g_wifi_search->count;
			memcpy(&wifi_search->wifi_ap, &g_wifi_search->wifi_ap, 
				sizeof(JWifiApInfo)*J_SDK_MAX_WIFI_AP_SIZE);
            break;
		case ACTION_PACK:
            g_wifi_search->count = wifi_search->count;
			memcpy(&g_wifi_search->wifi_ap, &wifi_search->wifi_ap, 
				sizeof(JWifiApInfo)*J_SDK_MAX_WIFI_AP_SIZE);
			break;
		default:
			break;
	}
}
void process_network_status(NetworkStatusPacket *g_net_status, 
		JNetworkStatus *net_status, JAction action)
{
	J_ASSERT(g_net_status && net_status);

	switch (action)
	{
		case ACTION_UNPACK:
            net_status->eth_st   = g_net_status->eth_st;
            net_status->wifi_st  = g_net_status->wifi_st;
            net_status->pppoe_st = g_net_status->pppoe_st;
            break;
		case ACTION_PACK:
            g_net_status->eth_st   = net_status->eth_st;
            g_net_status->wifi_st  = net_status->wifi_st;
            g_net_status->pppoe_st = net_status->pppoe_st;
			break;
		default:
			break;
	}
}
void process_control_device(ControlDevicePacket *g_cntrl_dev, 
		JControlDevice *cntrl_dev, JAction action)
{
	J_ASSERT(g_cntrl_dev && cntrl_dev);

	switch (action)
	{
		case ACTION_UNPACK:
            cntrl_dev->command = g_cntrl_dev->command;
            break;
		case ACTION_PACK:
            g_cntrl_dev->command = cntrl_dev->command;
			break;
		default:
			break;
	}
}
void process_ddns_config(DdnsConfigPacket *g_ddns_cfg, 
		JDdnsConfig *ddns_cfg, JAction action)
{
	J_ASSERT(g_ddns_cfg && ddns_cfg);

	switch (action)
	{
		case ACTION_UNPACK:
			strcpy((char*)ddns_cfg->ddns_account, (const char*)g_ddns_cfg->ddns_account);
			strcpy((char*)ddns_cfg->ddns_usr, (const char*)g_ddns_cfg->ddns_usr);
			strcpy((char*)ddns_cfg->ddns_pwd, (const char*)g_ddns_cfg->ddns_pwd);
            ddns_cfg->ddns_open  = g_ddns_cfg->ddns_open;
            ddns_cfg->ddns_type  = g_ddns_cfg->ddns_type;
            ddns_cfg->ddns_port  = g_ddns_cfg->ddns_port;
            ddns_cfg->ddns_times = g_ddns_cfg->ddns_times;
            break;
		case ACTION_PACK:
			strcpy((char*)g_ddns_cfg->ddns_account, (const char*)ddns_cfg->ddns_account);
			strcpy((char*)g_ddns_cfg->ddns_usr, (const char*)ddns_cfg->ddns_usr);
			strcpy((char*)g_ddns_cfg->ddns_pwd, (const char*)ddns_cfg->ddns_pwd);
            g_ddns_cfg->ddns_open  = ddns_cfg->ddns_open;
            g_ddns_cfg->ddns_type  = ddns_cfg->ddns_type;
            g_ddns_cfg->ddns_port  = ddns_cfg->ddns_port;
            g_ddns_cfg->ddns_times = ddns_cfg->ddns_times;
			break;
		default:
			break;
	}
}

void process_avd_config(AvdConfigPacket *g_avd_cfg, 
		JAvdConfig *avd_cfg, JAction action)
{
	J_ASSERT(g_avd_cfg && avd_cfg);

	switch (action)
	{
		case ACTION_UNPACK:
            avd_cfg->enable = g_avd_cfg->enable;
			memcpy(avd_cfg->sched_time, g_avd_cfg->sched_time, 
				sizeof(JSegment)*J_SDK_MAX_SEG_SZIE);
			memcpy(avd_cfg->avd_rule, g_avd_cfg->avd_rule, 
				sizeof(JAvdRule)*MAX_IVS_AVD_RULES);
            break;
		case ACTION_PACK:
            g_avd_cfg->enable = avd_cfg->enable;
			memcpy(g_avd_cfg->sched_time, avd_cfg->sched_time, 
				sizeof(JSegment)*J_SDK_MAX_SEG_SZIE);
			memcpy(g_avd_cfg->avd_rule, avd_cfg->avd_rule, 
				sizeof(JAvdRule)*MAX_IVS_AVD_RULES);
			break;
		default:
			break;
	}
}



