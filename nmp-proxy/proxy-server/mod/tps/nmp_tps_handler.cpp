
#include "nmp_proxy_sdk.h"

#include "nmp_tps_swap.h"
#include "nmp_tps_service.h"
#include "nmp_tps_handler.h"


#define DEF_TPS_FACTORY_INFO        "http://www.xiongmaitech.com/"

#define DEF_TPS_ALL_CHANNEL         0xFFFFFFFF
#define DEF_TPS_GET_SUCCESSFUL      0x0
#define DEF_TPS_MAX_RECORD_FILE     256
#define DEF_TPS_WAIT_TIME           1000


int tps_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    tps_service_t *tps_srv;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*H264_DVR_DEVICEINFO dev_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    tps_srv  = (tps_service_t*)srv;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&dev_cfg, 0, sizeof(H264_DVR_DEVICEINFO));

    tps_cfg.command  = E_SDK_CONFIG_SYSINFO;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &dev_cfg;
    tps_cfg.b_size   = sizeof(H264_DVR_DEVICEINFO);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, (void*)&tps_cfg);
    if (!ret)
    {
        strncpy((char*)dev_info->manu_info, DEF_TPS_FACTORY_INFO, 
            sizeof(dev_info->manu_info)-1);
        tps_swap_device_info(&dev_cfg, dev_info, SWAP_PACK);
        //dev_info->rs485_num = 1;
    }*/

    return -1;//ret;
}
int tps_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_STR_RS485CONFIG_ALL rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&rs485_cfg, 0, sizeof(SDK_STR_RS485CONFIG_ALL));

    tps_cfg.command  = E_SDK_CONFIG_COMM485;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &rs485_cfg;
    tps_cfg.b_size   = sizeof(SDK_STR_RS485CONFIG_ALL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_serial_info(rs485_cfg.ptzAll, serial_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_STR_RS485CONFIG_ALL rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&rs485_cfg, 0, sizeof(SDK_STR_RS485CONFIG_ALL));

    tps_cfg.command  = E_SDK_CONFIG_COMM485;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &rs485_cfg;
    tps_cfg.b_size   = sizeof(SDK_STR_RS485CONFIG_ALL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_serial_info(rs485_cfg.ptzAll, serial_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_NORMAL normal_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&normal_cfg, 0, sizeof(SDK_CONFIG_NORMAL));

    tps_cfg.command  = E_SDK_CONFIG_SYSNORMAL;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &normal_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_NORMAL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_sys_time(&normal_cfg.sysTime, dev_time, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_NORMAL normal_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&normal_cfg, 0, sizeof(SDK_CONFIG_NORMAL));

    tps_cfg.command  = E_SDK_CONFIG_SYSNORMAL;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &normal_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_NORMAL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_sys_time(&normal_cfg.sysTime, dev_time, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetNTPConfig ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ntp_cfg, 0, sizeof(SDK_NetNTPConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_NTP;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ntp_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetNTPConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetNTPConfig ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ntp_cfg, 0, sizeof(SDK_NetNTPConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_NTP;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ntp_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetNTPConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_NET_COMMON net_cfg;
    SDK_NetDNSConfig dns_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&net_cfg, 0, sizeof(SDK_CONFIG_NET_COMMON));
    memset(&dns_cfg, 0, sizeof(SDK_NetDNSConfig));

    tps_cfg.command  = E_SDK_CONFIG_SYSNET;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &net_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_NET_COMMON);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_network_info(&net_cfg, net_info, SWAP_PACK);
    else
        goto done;

    tps_cfg.command  = E_SDK_CONFIG_NET_DNS;
    tps_cfg.buffer   = &dns_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetDNSConfig);

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_dns_info(&dns_cfg, net_info, SWAP_PACK);
*/
done:
    return -1;//ret;
}
int tps_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_NET_COMMON net_cfg;
    SDK_NetDNSConfig dns_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&net_cfg, 0, sizeof(SDK_CONFIG_NET_COMMON));
    memset(&dns_cfg, 0, sizeof(SDK_NetDNSConfig));

    tps_cfg.command  = E_SDK_CONFIG_SYSNET;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &net_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_NET_COMMON);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_network_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
        if (ret)
            goto done;
    }
    else
        goto done;

    tps_cfg.command  = E_SDK_CONFIG_NET_DNS;
    tps_cfg.buffer   = &dns_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetDNSConfig);

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_dns_info(&dns_cfg, net_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

done:
    return -1;//ret;
}
int tps_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetPPPoEConfig pppoe_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&pppoe_cfg, 0, sizeof(SDK_NetPPPoEConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_PPPOE;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &pppoe_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetPPPoEConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_pppoe_info(&pppoe_cfg, pppoe_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetPPPoEConfig pppoe_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&pppoe_cfg, 0, sizeof(SDK_NetPPPoEConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_PPPOE;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &pppoe_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetPPPoEConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_pppoe_info(&pppoe_cfg, pppoe_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_FtpServerConfig ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ftp_cfg, 0, sizeof(SDK_FtpServerConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_FTPSERVER;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ftp_cfg;
    tps_cfg.b_size   = sizeof(SDK_FtpServerConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_FtpServerConfig ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ftp_cfg, 0, sizeof(SDK_FtpServerConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_FTPSERVER;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ftp_cfg;
    tps_cfg.b_size   = sizeof(SDK_FtpServerConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetEmailConfig email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&email_cfg, 0, sizeof(SDK_NetEmailConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_EMAIL;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &email_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetEmailConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_smtp_info(&email_cfg, smtp_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetEmailConfig email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&email_cfg, 0, sizeof(SDK_NetEmailConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_EMAIL;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &email_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetEmailConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_smtp_info(&email_cfg, smtp_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetDDNSConfigALL ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ddns_cfg, 0, sizeof(SDK_NetDDNSConfigALL));

    tps_cfg.command  = E_SDK_CONFIG_NET_DDNS;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ddns_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetDDNSConfigALL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetDDNSConfigALL ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&ddns_cfg, 0, sizeof(SDK_NetDDNSConfigALL));

    tps_cfg.command  = E_SDK_CONFIG_NET_DDNS;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &ddns_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetDDNSConfigALL);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetUPNPConfig upnp_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&upnp_cfg, 0, sizeof(SDK_NetUPNPConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_UPNP;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &upnp_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetUPNPConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_upnp_info(&upnp_cfg, upnp_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_NetUPNPConfig upnp_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&upnp_cfg, 0, sizeof(SDK_NetUPNPConfig));

    tps_cfg.command  = E_SDK_CONFIG_NET_UPNP;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;
    tps_cfg.buffer   = &upnp_cfg;
    tps_cfg.b_size   = sizeof(SDK_NetUPNPConfig);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_upnp_info(&upnp_cfg, upnp_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceDiskInfo *disk_list;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_StorageDeviceInformationAll dh_disk;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&dh_disk, 0, sizeof(SDK_StorageDeviceInformationAll));

    tps_cfg.command  = E_SDK_CONFIG_DISK_INFO;
    tps_cfg.buffer   = &dh_disk;
    tps_cfg.b_size   = sizeof(SDK_StorageDeviceInformationAll);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_disk_list(&dh_disk, disk_list, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFormatDisk *fmt_disk;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_StorageDeviceInformationAll dh_disk;
    SDK_StorageDeviceControl storage_ctl;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&dh_disk, 0, sizeof(SDK_StorageDeviceInformationAll));
    memset(&storage_ctl, 0, sizeof(SDK_StorageDeviceControl));

    tps_cfg.command  = E_SDK_CONFIG_DISK_INFO;
    tps_cfg.buffer   = &dh_disk;
    tps_cfg.b_size   = sizeof(SDK_StorageDeviceInformationAll);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, GET_DISK_LIST, &tps_cfg);
    if (!ret)
    {
        tps_swap_format_info(&storage_ctl, &dh_disk, fmt_disk);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &storage_ctl);
    }*/

    return -1;//ret;
}
int tps_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(srv && pvalue);

    tps_basic = (tps_service_basic_t*)srv->tm;

    return (*tps_basic->ro.set_config)(srv, parm_id, pvalue);
}
int tps_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_ENCODE enc_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&enc_cfg, 0, sizeof(SDK_CONFIG_ENCODE));

    tps_cfg.command  = E_SDK_CONFIG_SYSENCODE;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &enc_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_encode_info(&enc_cfg, enc_info, SWAP_PACK);
*/
    /*tps_cfg.command  = DH_DEV_DEVICECFG;
    tps_cfg.buffer   = &sys_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        enc_info->format = sys_cfg.byVideoStandard;*/

    return -1;//ret;
}
int tps_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_CONFIG_ENCODE enc_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&enc_cfg, 0, sizeof(SDK_CONFIG_ENCODE));

    tps_cfg.command  = E_SDK_CONFIG_SYSENCODE;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &enc_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_encode_info(&enc_cfg, enc_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VIDEOCOLOR *color;
    SDK_CONFIG_VIDEOCOLOR vc_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
     memset(&vc_cfg, 0, sizeof(SDK_CONFIG_VIDEOCOLOR));

    tps_cfg.command  = E_SDK_VIDEOCOLOR;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vc_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_VIDEOCOLOR);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        color = &vc_cfg.dstVideoColor[1];
        tps_swap_display_info(&color->dstColor, dis_info, SWAP_PACK);
        color = &vc_cfg.dstVideoColor[0];
        tps_swap_display_info(&color->dstColor, dis_info, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VIDEOCOLOR *color;
    SDK_CONFIG_VIDEOCOLOR vc_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_CONFIG_VIDEOCOLOR));

    tps_cfg.command  = E_SDK_VIDEOCOLOR;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vc_cfg;
    tps_cfg.b_size   = sizeof(SDK_CONFIG_VIDEOCOLOR);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        color = &vc_cfg.dstVideoColor[0];
        tps_swap_display_info(&color->dstColor, dis_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
        color = &vc_cfg.dstVideoColor[1];
        tps_swap_display_info(&color->dstColor, dis_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VideoWidgetConfigAll osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&osd_cfg, 0, sizeof(SDK_VideoWidgetConfigAll));

    tps_cfg.command  = E_SDK_CONFIG_OUT_MODE;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;//channel;
    tps_cfg.buffer   = &osd_cfg;
    tps_cfg.b_size   = sizeof(SDK_VideoWidgetConfigAll);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_osd_info(&osd_cfg.vVideoWidegetConfigAll[channel], osd_info, SWAP_PACK);
*/
    return -1;//ret;
}
int tps_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VideoWidgetConfigAll osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&osd_cfg, 0, sizeof(SDK_VideoWidgetConfigAll));

    tps_cfg.command  = E_SDK_CONFIG_OUT_MODE;
    tps_cfg.channel  = DEF_TPS_ALL_CHANNEL;//channel;
    tps_cfg.buffer   = &osd_cfg;
    tps_cfg.b_size   = sizeof(SDK_VideoWidgetConfigAll);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_osd_info(&osd_cfg.vVideoWidegetConfigAll[channel], osd_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_STR_CONFIG_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(SDK_STR_CONFIG_PTZ));
    memset(&tps_cfg, 0, sizeof(tps_config_t));

    tps_cfg.command  = E_SDK_CONFIG_PTZ;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = (char*)&ptz_cfg;
    tps_cfg.b_size   = sizeof(SDK_STR_CONFIG_PTZ);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        tps_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_STR_CONFIG_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(SDK_STR_CONFIG_PTZ));
    memset(&tps_cfg, 0, sizeof(tps_config_t));

    tps_cfg.command  = E_SDK_CONFIG_PTZ;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = (char*)&ptz_cfg;
    tps_cfg.b_size   = sizeof(SDK_STR_CONFIG_PTZ);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_RECORDCONFIG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&rec_cfg, 0, sizeof(SDK_RECORDCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_RECORD;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &rec_cfg;
    tps_cfg.b_size   = sizeof(SDK_RECORDCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        tps_swap_record_info(&rec_cfg, rec_info, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_RECORDCONFIG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&rec_cfg, 0, sizeof(SDK_RECORDCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_RECORD;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &rec_cfg;
    tps_cfg.b_size   = sizeof(SDK_RECORDCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/
    
    return -1;//ret;
}
int tps_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    /*int ret;
    JHideParameter *hide_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    SDK_BLINDDETECTCONFIG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_BLINDDETECTCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_SHELTER;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vc_cfg;
    tps_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
        tps_swap_hide_info(&vc_cfg, hide_info, SWAP_PACK);*/

    return -1;//ret;
}
int tps_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    /*int ret;
    JHideParameter *hide_info;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    SDK_BLINDDETECTCONFIG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_BLINDDETECTCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_SHELTER;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vc_cfg;
    tps_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        //tps_swap_hide_info(&vc_cfg, hide_info, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_MOTIONCONFIG motion_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&motion_cfg, 0, sizeof(SDK_MOTIONCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_MOTION;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &motion_cfg;
    tps_cfg.b_size   = sizeof(SDK_MOTIONCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        tps_swap_move_alarm_info(&motion_cfg, motion, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_MOTIONCONFIG motion_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&motion_cfg, 0, sizeof(SDK_MOTIONCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_MOTION;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &motion_cfg;
    tps_cfg.b_size   = sizeof(SDK_MOTIONCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_move_alarm_info(&motion_cfg, motion, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VIDEOLOSSCONFIG vl_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&vl_cfg, 0, sizeof(SDK_VIDEOLOSSCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_VIDEO_LOSS;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vl_cfg;
    tps_cfg.b_size   = sizeof(SDK_VIDEOLOSSCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        tps_swap_video_lost_info(&vl_cfg, video_lost, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_VIDEOLOSSCONFIG vl_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&vl_cfg, 0, sizeof(SDK_VIDEOLOSSCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_VIDEO_LOSS;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &vl_cfg;
    tps_cfg.b_size   = sizeof(SDK_VIDEOLOSSCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_video_lost_info(&vl_cfg, video_lost, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_BLINDDETECTCONFIG blind_detect;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&blind_detect, 0, sizeof(SDK_BLINDDETECTCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_SHELTER;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &blind_detect;
    tps_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        tps_swap_hide_alarm_info(&blind_detect, hide_alarm, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_BLINDDETECTCONFIG blind_detect;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&blind_detect, 0, sizeof(SDK_BLINDDETECTCONFIG));

    tps_cfg.command  = E_SDK_CONFIG_SHELTER;
    tps_cfg.channel  = channel;
    tps_cfg.buffer   = &blind_detect;
    tps_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    ret = (*tps_basic->ro.get_config)(srv, parm_id-1, &tps_cfg);
    if (!ret)
    {
        tps_swap_hide_alarm_info(&blind_detect, hide_alarm, SWAP_UNPACK);
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
    }*/

    return -1;//ret;
}
int tps_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_ALARM_INPUTCONFIG alarm_in;
    SDK_AlarmOutConfig    alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&alarm_in, 0, sizeof(SDK_ALARM_INPUTCONFIG));
    memset(&alarm_out, 0, sizeof(SDK_AlarmOutConfig));

    tps_cfg.channel  = channel;
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            tps_cfg.command  = E_SDK_CONFIG_ALARM_IN;
            tps_cfg.buffer   = &alarm_in;
            tps_cfg.b_size   = sizeof(SDK_ALARM_INPUTCONFIG);
            ret = (*tps_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &tps_cfg);
            if (!ret)
                tps_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            tps_cfg.command  = E_SDK_CONFIG_ALARM_OUT;
            tps_cfg.buffer   = &alarm_out;
            tps_cfg.b_size   = sizeof(SDK_AlarmOutConfig);
            ret = (*tps_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &tps_cfg);
            if (!ret)
                tps_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }*/

    return -1;//ret;
}
int tps_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    tps_service_basic_t *tps_basic;

    tps_config_t tps_cfg;
    /*SDK_ALARM_INPUTCONFIG alarm_in;
    SDK_AlarmOutConfig    alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_config_t));
    memset(&alarm_in, 0, sizeof(SDK_ALARM_INPUTCONFIG));
    memset(&alarm_out, 0, sizeof(SDK_AlarmOutConfig));

    tps_cfg.channel  = channel;
    tps_cfg.waittime = DEF_TPS_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            tps_cfg.command  = E_SDK_CONFIG_ALARM_IN;
            tps_cfg.buffer   = &alarm_in;
            tps_cfg.b_size   = sizeof(SDK_ALARM_INPUTCONFIG);
            ret = (*tps_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &tps_cfg);
            if (!ret)
            {
                tps_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*tps_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &tps_cfg);
            }
            break;
        case ALARM_OUT:
            tps_cfg.command  = E_SDK_CONFIG_ALARM_OUT;
            tps_cfg.buffer   = &alarm_out;
            tps_cfg.b_size   = sizeof(SDK_AlarmOutConfig);
            ret = (*tps_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &tps_cfg);
            if (!ret)
            {
                tps_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*tps_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &tps_cfg);
            }
            break;

        default:
            ret = -1;
            break;
    }*/

    return -1;//ret;
}
#define deposit_file_type(type)                                     \
            for (i=filecount; i<tps_cfg.filecount+filecount; i++)   \
            {/*借用hWnd，记录文件类型 */                \
                file_data[i].hWnd = (void*)type;                    \
            }
int tps_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i, ret = 0;
    JStoreLog *store;
    tps_service_basic_t *tps_basic;

    static int sess_id = 0;
    static int file_type = 0;
    static int filecount = 0;
    /*static H264_DVR_FILE_DATA file_data[DEF_TPS_MAX_RECORD_FILE];

    tps_query_t tps_cfg;

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    if (-1 != (int)store->sess_id && 
        file_type == (int)store->rec_type)
    {
        goto done;
    }
    else
    {
        file_type = (int)store->rec_type;
        store->sess_id = sess_id++;
        filecount = 0;
    }

    memset(&tps_cfg, 0, sizeof(tps_query_t));
    memset(file_data, 0, sizeof(H264_DVR_FILE_DATA)*DEF_TPS_MAX_RECORD_FILE);

    tps_cfg.channel   = channel;
    tps_cfg.buffer    = (char*)file_data;
    //tps_cfg.buf_size  = sizeof(H264_DVR_FILE_DATA)*DEF_TPS_MAX_RECORD_FILE;
    tps_cfg.max_count = DEF_TPS_MAX_RECORD_FILE;
    tps_swap_store_log_info(&tps_cfg, store, SWAP_UNPACK);

    if (store->rec_type & TIMER_RECODE)
    {
        tps_cfg.file_type = SDK_RECORD_REGULAR;
        ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
        if (!ret)
        {
            deposit_file_type(TIMER_RECODE);
            filecount += tps_cfg.filecount;
        }
    }
    if (store->rec_type & ALARM_RECODE)
    {
        tps_cfg.file_type = SDK_RECORD_ALARM;
        tps_cfg.max_count-= filecount;
        tps_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
        if (!ret)
        {
            deposit_file_type(ALARM_RECODE);
            filecount += tps_cfg.filecount;
        }
    }
    if (store->rec_type & MOVE_RECODE)
    {
        tps_cfg.file_type = SDK_RECORD_DETECT;
        tps_cfg.max_count-= filecount;
        tps_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
        if (!ret)
        {
            deposit_file_type(MOVE_RECODE);
            filecount += tps_cfg.filecount;
        }
    }
    if (store->rec_type & MANUAL_RECODE)
    {
        tps_cfg.file_type = SDK_RECORD_MANUAL;
        tps_cfg.max_count-= filecount;
        tps_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
        if (!ret)
        {
            deposit_file_type(MANUAL_RECODE);
            filecount += tps_cfg.filecount;
        }
    }

done:
    tps_cfg.buffer    = (char*)file_data;
    tps_cfg.filecount = filecount;
    if (!ret)
        tps_swap_store_log_info(&tps_cfg, store, SWAP_PACK);*/

    return -1;//ret;
}
int tps_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JPTZControl *ptz_ctrl;
    tps_service_basic_t *tps_basic;

    tps_ptz_ctrl_t tps_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_ptz_ctrl_t));

    tps_cfg.channel = channel;
    //tps_swap_ptz_cmd(&tps_cfg, ptz_ctrl, SWAP_UNPACK);

    return (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);
}
int tps_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JPPConfig *pp_cfg;
    tps_service_basic_t *tps_basic;

    tps_ptz_ctrl_t tps_cfg;

    NMP_ASSERT(srv && pvalue);

    pp_cfg = (JPPConfig*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_ptz_ctrl_t));

    tps_cfg.channel = channel;

    /*switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
            tps_cfg.ptz_cmd = EXTPTZ_POINT_SET_CONTROL;
            break;
        case PTZ_USE_PP:
            tps_cfg.ptz_cmd = EXTPTZ_POINT_MOVE_CONTROL;
            break;
        case PTZ_DEL_PP:
            tps_cfg.ptz_cmd = EXTPTZ_POINT_DEL_CONTROL;
            break;
        default:
            ret = -1;
            break;
    }*/

    if (!ret)
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);

    return -1;//ret;
}
int tps_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JCruiseWay *crz_way;
    tps_service_basic_t *tps_basic;

    tps_cruise_t tps_cfg;
    //NET_DVR_CRUISE_RET tps_crz;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_cruise_t));
    //memset(&tps_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    /*tps_cfg.channel = channel+1;
    tps_cfg.crz_no  = crz_way->crz_info.crz_no;
    tps_cfg.input   = (int)&tps_crz;

    ret = (*tps_basic->ro.get_config)(srv, parm_id, &tps_cfg);
    if (!ret)
    {
        sprintf((char*)crz_way->crz_info.crz_name, 
            "宸¤埅璺緞 %d", crz_way->crz_info.crz_no+1);
        //tps_swap_cruise_way(&tps_crz, crz_way, SWAP_PACK);
    }*/

    return -1;//ret;
}
int tps_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JCruiseConfig *crz_cfg;
    tps_service_basic_t *tps_basic;

    tps_cruise_t tps_cfg;
    /*NET_DVR_CRUISE_RET tps_crz;

    NMP_ASSERT(srv && pvalue);

    crz_cfg = (JCruiseConfig*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    memset(&tps_cfg, 0, sizeof(tps_cruise_t));
    memset(&tps_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    tps_cfg.channel = channel;
    tps_cfg.crz_no  = crz_cfg->crz_no;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, tps_cfg.crz_no);
    switch (crz_cfg->action)
    {
        case PTZ_START_CRZ:
            tps_cfg.crz_cmd = RUN_SEQ;
            break;
        case PTZ_STOP_CRZ:
            tps_cfg.crz_cmd = STOP_SEQ;
            break;
        case PTZ_DEL_CRZ:
            tps_cfg.crz_cmd = CLE_PRE_SEQ;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);*/

    return -1;//ret;
}
int tps_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    tps_cruise_t tps_cfg;
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    tps_cfg.channel = channel;
    tps_cfg.crz_no  = crz_way->crz_info.crz_no;

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        /*tps_cfg.crz_cmd = FILL_PRE_SEQ;
        tps_cfg.input   = crz_point->preset;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);

        tps_cfg.crz_cmd = SET_SEQ_SPEED;
        tps_cfg.input   = crz_point->speed;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);

        tps_cfg.crz_cmd = SET_SEQ_DWELL;
        tps_cfg.input   = crz_point->dwell;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);*/
    }

    return 0;
}
int tps_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    tps_cruise_t tps_cfg;
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    tps_basic = (tps_service_basic_t*)srv->tm;

    tps_cfg.channel   = channel;
    tps_cfg.crz_no = crz_way->crz_info.crz_no;

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        /*tps_cfg.crz_cmd = FILL_PRE_SEQ;
        tps_cfg.input   = crz_point->preset;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);

        tps_cfg.crz_cmd = SET_SEQ_SPEED;
        tps_cfg.input   = crz_point->speed;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);

        tps_cfg.crz_cmd = SET_SEQ_DWELL;
        tps_cfg.input   = crz_point->dwell;
        (*tps_basic->ro.set_config)(srv, parm_id, &tps_cfg);*/
    }

    return 0;
}
int tps_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(srv && pvalue);

    tps_basic = (tps_service_basic_t*)srv->tm;

    return (*tps_basic->ro.get_config)(srv, parm_id, pvalue);
}


