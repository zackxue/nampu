
#include "nmp_proxy_sdk.h"

#include "nmp_hik_swap.h"
#include "nmp_hik_service.h"
#include "nmp_hik_handler.h"


#define DEF_HIK_FACTORY_INFO    "http://www.hikvision.com/cn/"


int hik_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_DEVICECFG_V40 hik_dev_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&hik_dev_cfg, 0, sizeof(NET_DVR_DEVICECFG_V40));

    hik_cfg.command = NET_DVR_GET_DEVICECFG_V40;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &hik_dev_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DEVICECFG_V40);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, (void*)&hik_cfg);
    if (!ret)
    {
        strncpy((char*)dev_info->manu_info, DEF_HIK_FACTORY_INFO, sizeof(dev_info->manu_info)-1);
        hik_swap_device_info(&hik_dev_cfg, dev_info, SWAP_PACK);
    }

    return ret;
}
int hik_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_RS232CFG_V30 rs232_cfg;
    NET_DVR_ALARM_RS485CFG rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&rs232_cfg, 0, sizeof(NET_DVR_RS232CFG_V30));
    memset(&rs485_cfg, 0, sizeof(NET_DVR_ALARM_RS485CFG));

    if (0 == serial_info->serial_no)
    {
        hik_cfg.command = NET_DVR_GET_ALARM_RS485CFG;
        hik_cfg.channel = channel+1;
        hik_cfg.buffer  = &rs485_cfg;
        hik_cfg.b_size  = sizeof(NET_DVR_ALARM_RS485CFG);
    }
    else if (1 == serial_info->serial_no)
    {
        hik_cfg.command = NET_DVR_GET_RS232CFG_V30;
        hik_cfg.channel = channel+1;
        hik_cfg.buffer  = &rs232_cfg;
        hik_cfg.b_size  = sizeof(NET_DVR_RS232CFG_V30);
    }

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
    {
        if (0 == serial_info->serial_no)
            hik_swap_rs485_info(&rs485_cfg, serial_info, SWAP_PACK);
        else if (1 == serial_info->serial_no)
            hik_swap_rs232_info(rs232_cfg.struRs232, serial_info, SWAP_PACK);
    }

    return ret;
}
int hik_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JSerialParameter *serial_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_RS232CFG_V30 rs232_cfg;
    //NET_DVR_ALARM_RS485CFG rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&rs232_cfg, 0, sizeof(NET_DVR_RS232CFG_V30));
    //memset(&rs485_cfg, 0, sizeof(NET_DVR_ALARM_RS485CFG));

    if (0 == serial_info->serial_no)
    {
    }
    else if (1 == serial_info->serial_no)
    {
        hik_cfg.command = NET_DVR_GET_RS232CFG_V30;
        hik_cfg.channel = channel+1;
        hik_cfg.buffer  = &rs232_cfg;
        hik_cfg.b_size  = sizeof(NET_DVR_RS232CFG_V30);

        ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
        if (!ret)
        {
            hik_cfg.command = NET_DVR_SET_RS232CFG_V30;
            hik_swap_rs232_info(rs232_cfg.struRs232, serial_info, SWAP_UNPACK);
        }
    }

    if (!ret)
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

    return ret;
}
int hik_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_TIME hik_time;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&hik_time, 0, sizeof(NET_DVR_TIME));

    hik_cfg.command = NET_DVR_GET_TIMECFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &hik_time;
    hik_cfg.b_size  = sizeof(NET_DVR_TIME);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
    {
        hik_swap_time_info(&hik_time, dev_time, SWAP_PACK);
        dev_time->zone = DEF_PROXY_TIME_ZOE;
    }

    return ret;
}
int hik_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JDeviceTime *dev_time;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_TIME hik_time;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&hik_time, 0, sizeof(NET_DVR_TIME));

    hik_cfg.command = NET_DVR_GET_TIMECFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &hik_time;
    hik_cfg.b_size  = sizeof(NET_DVR_TIME);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_TIMECFG;
        hik_swap_time_info(&hik_time, dev_time, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NTPPARA ntp_para;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ntp_para, 0, sizeof(NET_DVR_NTPPARA));

    hik_cfg.command = NET_DVR_GET_NTPCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ntp_para;
    hik_cfg.b_size  = sizeof(NET_DVR_NTPPARA);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_ntp_info(&ntp_para, ntp_info, SWAP_PACK);

    return ret;
}
int hik_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NTPPARA ntp_para;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ntp_para, 0, sizeof(NET_DVR_NTPPARA));

    hik_cfg.command = NET_DVR_GET_NTPCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ntp_para;
    hik_cfg.b_size  = sizeof(NET_DVR_NTPPARA);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_NTPCFG;
        hik_swap_ntp_info(&ntp_para, ntp_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NETCFG_V30 net_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&net_cfg, 0, sizeof(NET_DVR_NETCFG_V30));

    hik_cfg.command = NET_DVR_GET_NETCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &net_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_NETCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_network_info(&net_cfg, net_info, SWAP_PACK);

    return ret;
}
int hik_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NETCFG_V30 net_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&net_cfg, 0, sizeof(NET_DVR_NETCFG_V30));

    hik_cfg.command = NET_DVR_GET_NETCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &net_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_NETCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_NETCFG_V30;
        hik_swap_network_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NETCFG_V30 net_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&net_cfg, 0, sizeof(NET_DVR_NETCFG_V30));

    hik_cfg.command = NET_DVR_GET_NETCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &net_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_NETCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_pppoe_info(&net_cfg.struPPPoE, pppoe_info, SWAP_PACK);

    return ret;
}
int hik_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_NETCFG_V30 net_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&net_cfg, 0, sizeof(NET_DVR_NETCFG_V30));

    hik_cfg.command = NET_DVR_GET_NETCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &net_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_NETCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_NETCFG_V30;
        hik_swap_pppoe_info(&net_cfg.struPPPoE, pppoe_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_FTPCFG ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ftp_cfg, 0, sizeof(NET_DVR_FTPCFG));

    hik_cfg.command = NET_DVR_GET_FTPCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ftp_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_FTPCFG);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);
    
    return ret;
}
int hik_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_FTPCFG ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;
    
    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ftp_cfg, 0, sizeof(NET_DVR_FTPCFG));

    hik_cfg.command = NET_DVR_GET_FTPCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ftp_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_FTPCFG);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_FTPCFG;
        hik_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_EMAILCFG_V30 email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&email_cfg, 0, sizeof(NET_DVR_EMAILCFG_V30));

    hik_cfg.command = NET_DVR_GET_EMAILCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &email_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_EMAILCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_smtp_info(&email_cfg, smtp_info, SWAP_PACK);

    return ret;
}
int hik_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_EMAILCFG_V30 email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&email_cfg, 0, sizeof(NET_DVR_EMAILCFG_V30));

    hik_cfg.command = NET_DVR_GET_EMAILCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &email_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_EMAILCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_EMAILCFG_V30;
        hik_swap_smtp_info(&email_cfg, smtp_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_DDNSPARA_EX ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ddns_cfg, 0, sizeof(NET_DVR_DDNSPARA_EX));

    hik_cfg.command = NET_DVR_GET_DDNSCFG_EX;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ddns_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DDNSPARA_EX);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);
    
    return ret;
}
int hik_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_DDNSPARA_EX ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&ddns_cfg, 0, sizeof(NET_DVR_DDNSPARA_EX));

    hik_cfg.command = NET_DVR_GET_DDNSCFG_EX;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ddns_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DDNSPARA_EX);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_DDNSCFG_EX;
        hik_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    //NET_DVR_DDNSPARA_EX ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    //memset(&ddns_cfg, 0, sizeof(NET_DVR_DDNSPARA_EX));

    /*hik_cfg.command = ;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ddns_cfg;
    hik_cfg.b_size  = sizeof();

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);*/

    return ret;
}
int hik_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    //NET_DVR_DDNSPARA_EX ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    //memset(&ddns_cfg, 0, sizeof(NET_DVR_DDNSPARA_EX));

    /*hik_cfg.command = ;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &ddns_cfg;
    hik_cfg.b_size  = sizeof();

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);*/

    return ret;
}
int hik_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceDiskInfo *disk_list;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_HDCFG hd_cfg;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&hd_cfg, 0, sizeof(NET_DVR_HDCFG));

    hik_cfg.command = NET_DVR_GET_HDCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &hd_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_HDCFG);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_disk_list(&hd_cfg, disk_list, SWAP_PACK);

    return ret;
}
int hik_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JFormatDisk *fmt_disk;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    return (*hik_basic->ro.set_config)(srv, parm_id, &fmt_disk->disk_no);
}
int hik_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    //JControlDevice *ctrl;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    //ctrl = (JControlDevice*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    return (*hik_basic->ro.set_config)(srv, parm_id, /*ctrl*/pvalue);
}
int hik_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;
    NET_DVR_COMPRESSIONCFG_V30 cmp_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));
    memset(&cmp_cfg, 0, sizeof(NET_DVR_COMPRESSIONCFG_V30));

    hik_cfg.command = NET_DVR_GET_COMPRESSCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &cmp_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_COMPRESSIONCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_encode_info(&cmp_cfg.struNormHighRecordPara, enc_info, SWAP_PACK);
    
    /******* get format info *******/
    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
    {
        if (J_SDK_NTSC == pic_cfg.dwVideoFormat)
            enc_info->format = J_SDK_NTSC;
        else if (2 == pic_cfg.dwVideoFormat)
            enc_info->format = J_SDK_PAL;
        printf("dwVideoFormat   : %2d  |  ", pic_cfg.dwVideoFormat);
        printf("format          : %d\n", enc_info->format);
    }
    
    return ret;
}
int hik_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;
    NET_DVR_COMPRESSIONCFG_V30 cmp_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));
    memset(&cmp_cfg, 0, sizeof(NET_DVR_COMPRESSIONCFG_V30));

    hik_cfg.command = NET_DVR_GET_COMPRESSCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &cmp_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_COMPRESSIONCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_COMPRESSCFG_V30;
        hik_swap_encode_info(&cmp_cfg.struNormHighRecordPara, enc_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    if (!ret)
    {
        /******* set format info *******/
        hik_cfg.command = NET_DVR_GET_PICCFG_V30;
        hik_cfg.buffer  = &pic_cfg;
        hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

        ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
        if (!ret)
        {
            if (J_SDK_NTSC == enc_info->format)
                pic_cfg.dwVideoFormat = J_SDK_NTSC;
            else if (J_SDK_PAL == enc_info->format)
                pic_cfg.dwVideoFormat = 2;

            printf("dwVideoFormat   : %2d  |  ", pic_cfg.dwVideoFormat);
            printf("format          : %d\n", enc_info->format);

            hik_cfg.command = NET_DVR_SET_PICCFG_V30;
            ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
        }
    }

    return ret;
}
int hik_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_CAMERAPARAMCFG camera_para;
    
    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&camera_para, 0, sizeof(NET_DVR_CAMERAPARAMCFG));

    hik_cfg.command = NET_DVR_GET_CCDPARAMCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &camera_para;
    hik_cfg.b_size  = sizeof(NET_DVR_CAMERAPARAMCFG);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_display_info(&camera_para.struVideoEffect, dis_info, SWAP_PACK);

    return ret;
}
int hik_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_CAMERAPARAMCFG camera_para;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&camera_para, 0, sizeof(NET_DVR_CAMERAPARAMCFG));

    hik_cfg.command = NET_DVR_GET_CCDPARAMCFG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &camera_para;
    hik_cfg.b_size  = sizeof(NET_DVR_CAMERAPARAMCFG);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_GET_CCDPARAMCFG;
        hik_swap_display_info(&camera_para.struVideoEffect, dis_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_osd_info(&pic_cfg, osd_info, SWAP_PACK);

    return ret;
}
int hik_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_PICCFG_V30;
        hik_swap_osd_info(&pic_cfg, osd_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_DECODERCFG_V30 dec_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&dec_cfg, 0, sizeof(NET_DVR_DECODERCFG_V30));

    hik_cfg.command = NET_DVR_GET_DECODERCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &dec_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DECODERCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_decoder_info(&dec_cfg, ptz_info, SWAP_PACK);

    return ret;
}
int hik_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_DECODERCFG_V30 dec_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&dec_cfg, 0, sizeof(NET_DVR_DECODERCFG_V30));

    hik_cfg.command = NET_DVR_GET_DECODERCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &dec_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DECODERCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_DECODERCFG_V30;
        hik_swap_decoder_info(&dec_cfg, ptz_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_RECORD_V30 rec_cfg;
    NET_DVR_DEVICECFG_V40 dev_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&rec_cfg, 0, sizeof(NET_DVR_RECORD_V30));
    memset(&dev_cfg, 0, sizeof(NET_DVR_DEVICECFG_V40));

    hik_cfg.command = NET_DVR_GET_DEVICECFG_V40;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &dev_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DEVICECFG_V40);

    if (!(*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg))
        rec_info->auto_cover = dev_cfg.dwRecycleRecord;

    hik_cfg.command = NET_DVR_GET_RECORDCFG_V30;
    hik_cfg.buffer  = &rec_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_RECORD_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_record_info(&rec_cfg, rec_info, SWAP_PACK);

    return ret;
}
int hik_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_RECORD_V30 rec_cfg;
    NET_DVR_DEVICECFG_V40 dev_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&rec_cfg, 0, sizeof(NET_DVR_RECORD_V30));
    memset(&dev_cfg, 0, sizeof(NET_DVR_DEVICECFG_V40));

    hik_cfg.command = NET_DVR_GET_DEVICECFG_V40;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &dev_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_DEVICECFG_V40);

    if (!(*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg))
    {
        dev_cfg.dwRecycleRecord = rec_info->auto_cover;
        hik_cfg.command = NET_DVR_SET_DEVICECFG_V40;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    hik_cfg.command = NET_DVR_GET_RECORDCFG_V30;
    hik_cfg.buffer  = &rec_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_RECORD_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_RECORDCFG_V30;
        hik_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_hide_info(&pic_cfg, hide_info, SWAP_PACK);

    return ret;
}
int hik_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_PICCFG_V30;
        hik_swap_hide_info(&pic_cfg, hide_info, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_PACK);

    return ret;
}
int hik_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_PICCFG_V30;
        hik_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_PACK);

    return ret;
}
int hik_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_PICCFG_V30;
        hik_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_hide_alarm_info(&pic_cfg.struHideAlarm, hide_alarm, SWAP_PACK);

    return ret;
}
int hik_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_PICCFG_V30 pic_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&pic_cfg, 0, sizeof(NET_DVR_PICCFG_V30));

    hik_cfg.command = NET_DVR_GET_PICCFG_V30;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = &pic_cfg;
    hik_cfg.b_size  = sizeof(NET_DVR_PICCFG_V30);

    ret = (*hik_basic->ro.get_config)(srv, parm_id-1, &hik_cfg);
    if (!ret)
    {
        hik_cfg.command = NET_DVR_SET_PICCFG_V30;
        hik_swap_hide_alarm_info(&pic_cfg.struHideAlarm, hide_alarm, SWAP_UNPACK);
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return ret;
}
int hik_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_ALARMINCFG_V30 alarm_in;
    NET_DVR_ALARMOUTCFG_V30 alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&alarm_in, 0, sizeof(NET_DVR_ALARMINCFG_V30));
    memset(&alarm_out, 0, sizeof(NET_DVR_ALARMOUTCFG_V30));

    switch (type)
    {
        case ALARM_IN:
            hik_cfg.command = NET_DVR_GET_ALARMINCFG_V30;
            hik_cfg.channel = channel+1;
            hik_cfg.buffer  = &alarm_in;
            hik_cfg.b_size  = sizeof(NET_DVR_ALARMINCFG_V30);

            ret = (*hik_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hik_cfg);
            if (!ret)
                hik_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            hik_cfg.command = NET_DVR_GET_ALARMOUTCFG_V30;
            hik_cfg.channel = channel+1;
            hik_cfg.buffer  = &alarm_out;
            hik_cfg.b_size  = sizeof(NET_DVR_ALARMOUTCFG_V30);

            ret = (*hik_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hik_cfg);
            if (!ret)
                hik_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int hik_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    hik_service_basic_t *hik_basic;

    hik_config_t hik_cfg;
    NET_DVR_ALARMINCFG_V30 alarm_in;
    NET_DVR_ALARMOUTCFG_V30 alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));
    memset(&alarm_in, 0, sizeof(NET_DVR_ALARMINCFG_V30));
    memset(&alarm_out, 0, sizeof(NET_DVR_ALARMOUTCFG_V30));

    switch (type)
    {
        case ALARM_IN:
            hik_cfg.command = NET_DVR_GET_ALARMINCFG_V30;
            hik_cfg.channel = channel+1;
            hik_cfg.buffer  = &alarm_in;
            hik_cfg.b_size  = sizeof(NET_DVR_ALARMINCFG_V30);

            ret = (*hik_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hik_cfg);
            if (!ret)
            {
                hik_cfg.command = NET_DVR_SET_ALARMINCFG_V30;
                hik_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*hik_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hik_cfg);
            }
            break;
        case ALARM_OUT:
            hik_cfg.command = NET_DVR_GET_ALARMOUTCFG_V30;
            hik_cfg.channel = channel+1;
            hik_cfg.buffer  = &alarm_out;
            hik_cfg.b_size  = sizeof(NET_DVR_ALARMOUTCFG_V30);

            ret = (*hik_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hik_cfg);
            if (!ret)
            {
                hik_cfg.command = NET_DVR_SET_ALARMOUTCFG_V30;
                hik_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_UNPACK);
                ret = (*hik_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hik_cfg);
            }
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int hik_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JStoreLog *store;
    hik_config_t hik_cfg;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_config_t));

    //hik_cfg.command = GET_STORE_LOG;
    hik_cfg.channel = channel+1;
    hik_cfg.buffer  = store;
    hik_cfg.b_size  = sizeof(JStoreLog);

    hik_swap_store_log_info(store, store, SWAP_UNPACK);
    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
        hik_swap_store_log_info(store, store, SWAP_PACK);

    return ret;
}
int hik_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i, flag = 0, ret = 0;
    static int ptz_cmd = PTZ_STOP;
    int hik_cmd[] = {PAN_AUTO, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN, UP_LEFT, DOWN_LEFT, UP_RIGHT, DOWN_RIGHT, 
                    ZOOM_IN, ZOOM_OUT, FOCUS_FAR, FOCUS_NEAR, LIGHT_PWRON, 0, WIPER_PWRON, 0};
    int jxnmp_cmd[] = {PTZ_AUTO, PTZ_LEFT, PTZ_RIGHT, PTZ_UP, PTZ_DOWN, PTZ_LEFT_UP, PTZ_LEFT_DOWN, PTZ_RIGHT_UP, PTZ_RIGHT_DOWN, 
                    PTZ_ADD_ZOOM, PTZ_SUB_ZOOM, PTZ_ADD_FOCUS, PTZ_SUB_FOCUS, PTZ_TURN_ON, PTZ_TURN_OFF, PTZ_WIPERS_ON, PTZ_WIPERS_OFF};

    JPTZControl *ptz_ctrl;
    hik_ptz_ctrl_t hik_cfg;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    hik_cfg.channel = channel+1;
    if (PTZ_STOP == ptz_cmd)
    {
        for (i=0; i<(int)(sizeof(jxnmp_cmd)/sizeof(*jxnmp_cmd)); i++)
        {
            if ((int)ptz_ctrl->action == (int)jxnmp_cmd[i])
                break;
        }

        if (PTZ_TURN_OFF == jxnmp_cmd[i] || PTZ_WIPERS_OFF == jxnmp_cmd[i])
        {
            hik_cfg.stop = 1;
            hik_cfg.ptz_cmd = hik_cmd[i-1];
        }
        else
        {
            hik_cfg.stop = 0;
            hik_cfg.ptz_cmd = hik_cmd[i];
            
            ptz_cmd = hik_cfg.ptz_cmd;
        }
    }
    else if (PTZ_STOP == ptz_ctrl->action)
    {
        hik_cfg.stop = 1;
        hik_cfg.ptz_cmd = ptz_cmd;
        
        ptz_cmd = PTZ_STOP;
    }
    else
    {
        for (i=0; i<(int)(sizeof(jxnmp_cmd)/sizeof(*jxnmp_cmd)); i++)
        {
            if ((int)ptz_ctrl->action == (int)jxnmp_cmd[i])
                break;
        }

        if (ptz_cmd == hik_cmd[i])
        {
            flag = 1;
        }
        else if (PTZ_TURN_OFF == jxnmp_cmd[i] || PTZ_WIPERS_OFF == jxnmp_cmd[i])
        {
            hik_cfg.stop = 1;
            hik_cfg.ptz_cmd = ptz_cmd;

            ptz_cmd = PTZ_STOP;
        }
        else
        {
            hik_cfg.stop = 1;
            hik_cfg.ptz_cmd = ptz_cmd;
            (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

            hik_cfg.stop = 0;
            hik_cfg.ptz_cmd = hik_cmd[i];

            ptz_cmd = hik_cfg.ptz_cmd;
        }
    }

    if (!flag)
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    else
        ret = 0;

    return ret;
}
int hik_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JPPConfig *pp_cfg;
    hik_preset_t hik_cfg;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    pp_cfg = (JPPConfig*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    hik_cfg.channel = channel+1;
    hik_cfg.preset_no = pp_cfg->pp.preset;
    show_debug("%s() preset_no: %d<--------------------\n", __FUNCTION__, hik_cfg.preset_no);
    switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
            hik_cfg.preset_cmd = SET_PRESET;
            break;
        case PTZ_USE_PP:
            hik_cfg.preset_cmd = GOTO_PRESET;
            break;
        case PTZ_DEL_PP:
            hik_cfg.preset_cmd = CLE_PRESET;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

    return ret;
}
int hik_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JCruiseWay *crz_way;
    hik_service_basic_t *hik_basic;

    hik_cruise_t hik_cfg;
    NET_DVR_CRUISE_RET hik_crz;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_cruise_t));
    memset(&hik_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    hik_cfg.channel = channel+1;
    hik_cfg.crz_no  = crz_way->crz_info.crz_no;
    hik_cfg.input   = (int)&hik_crz;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, hik_cfg.crz_no);

    ret = (*hik_basic->ro.get_config)(srv, parm_id, &hik_cfg);
    if (!ret)
    {
        sprintf((char*)crz_way->crz_info.crz_name, 
            "巡航路径 %d", crz_way->crz_info.crz_no+1);
        hik_swap_cruise_way(&hik_crz, crz_way, SWAP_PACK);
    }

    return ret;
}
int hik_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JCruiseConfig *crz_cfg;
    hik_service_basic_t *hik_basic;

    hik_cruise_t hik_cfg;
    NET_DVR_CRUISE_RET hik_crz;

    NMP_ASSERT(srv && pvalue);

    crz_cfg = (JCruiseConfig*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    memset(&hik_cfg, 0, sizeof(hik_cruise_t));
    memset(&hik_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    hik_cfg.channel = channel+1;
    hik_cfg.crz_no  = crz_cfg->crz_no;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, hik_cfg.crz_no);
    switch (crz_cfg->action)
    {
        case PTZ_START_CRZ:
            hik_cfg.crz_cmd = RUN_SEQ;
            break;
        case PTZ_STOP_CRZ:
            hik_cfg.crz_cmd = STOP_SEQ;
            break;
        case PTZ_DEL_CRZ:
            hik_cfg.crz_cmd = CLE_PRE_SEQ;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

    return ret;
}
int hik_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hik_cruise_t hik_cfg;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    hik_cfg.channel = channel+1;
    hik_cfg.crz_no  = crz_way->crz_info.crz_no;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, hik_cfg.crz_no);

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        hik_cfg.crz_cmd = FILL_PRE_SEQ;
        hik_cfg.input   = crz_point->preset;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

        hik_cfg.crz_cmd = SET_SEQ_SPEED;
        hik_cfg.input   = crz_point->speed;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

        hik_cfg.crz_cmd = SET_SEQ_DWELL;
        hik_cfg.input   = crz_point->dwell;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return 0;
}
int hik_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hik_cruise_t hik_cfg;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    hik_cfg.channel   = channel+1;
    hik_cfg.crz_no = crz_way->crz_info.crz_no;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, hik_cfg.crz_no);

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        hik_cfg.crz_cmd = FILL_PRE_SEQ;
        hik_cfg.input   = crz_point->preset;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

        hik_cfg.crz_cmd = SET_SEQ_SPEED;
        hik_cfg.input   = crz_point->speed;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);

        hik_cfg.crz_cmd = SET_SEQ_DWELL;
        hik_cfg.input   = crz_point->dwell;
        (*hik_basic->ro.set_config)(srv, parm_id, &hik_cfg);
    }

    return 0;
}
int hik_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    //JDevCap *dev_cap;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(srv && pvalue);

    //dev_cap = (JDevCap*)pvalue;
    hik_basic = (hik_service_basic_t*)srv->tm;

    return (*hik_basic->ro.get_config)(srv, parm_id, /*dev_cap*/pvalue);
}


