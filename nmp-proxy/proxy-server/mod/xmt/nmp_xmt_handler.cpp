
#include "nmp_proxy_sdk.h"

#include "nmp_xmt_swap.h"
#include "nmp_xmt_service.h"
#include "nmp_xmt_handler.h"


#define DEF_XMT_FACTORY_INFO        "http://www.xiongmaitech.com/"

#define DEF_XMT_ALL_CHANNEL         0xFFFFFFFF
#define DEF_XMT_GET_SUCCESSFUL      0x0
#define DEF_XMT_MAX_RECORD_FILE     256
#define DEF_XMT_WAIT_TIME           1000


int xmt_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    xmt_service_t *xmt_srv;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    H264_DVR_DEVICEINFO dev_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    xmt_srv  = (xmt_service_t*)srv;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&dev_cfg, 0, sizeof(H264_DVR_DEVICEINFO));

    xmt_cfg.command  = E_SDK_CONFIG_SYSINFO;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &dev_cfg;
    xmt_cfg.b_size   = sizeof(H264_DVR_DEVICEINFO);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, (void*)&xmt_cfg);
    if (!ret)
    {
        strncpy((char*)dev_info->manu_info, DEF_XMT_FACTORY_INFO, 
            sizeof(dev_info->manu_info)-1);
        xmt_swap_device_info(&dev_cfg, dev_info, SWAP_PACK);
        //dev_info->rs485_num = 1;
    }

    return ret;
}
int xmt_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_STR_RS485CONFIG_ALL rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&rs485_cfg, 0, sizeof(SDK_STR_RS485CONFIG_ALL));

    xmt_cfg.command  = E_SDK_CONFIG_COMM485;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &rs485_cfg;
    xmt_cfg.b_size   = sizeof(SDK_STR_RS485CONFIG_ALL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_serial_info(rs485_cfg.ptzAll, serial_info, SWAP_PACK);

    return ret;
}
int xmt_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_STR_RS485CONFIG_ALL rs485_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&rs485_cfg, 0, sizeof(SDK_STR_RS485CONFIG_ALL));

    xmt_cfg.command  = E_SDK_CONFIG_COMM485;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &rs485_cfg;
    xmt_cfg.b_size   = sizeof(SDK_STR_RS485CONFIG_ALL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_serial_info(rs485_cfg.ptzAll, serial_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_NORMAL normal_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&normal_cfg, 0, sizeof(SDK_CONFIG_NORMAL));

    xmt_cfg.command  = E_SDK_CONFIG_SYSNORMAL;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &normal_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_NORMAL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_sys_time(&normal_cfg.sysTime, dev_time, SWAP_PACK);

    return ret;
}
int xmt_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_NORMAL normal_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&normal_cfg, 0, sizeof(SDK_CONFIG_NORMAL));

    xmt_cfg.command  = E_SDK_CONFIG_SYSNORMAL;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &normal_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_NORMAL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_sys_time(&normal_cfg.sysTime, dev_time, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetNTPConfig ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ntp_cfg, 0, sizeof(SDK_NetNTPConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_NTP;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ntp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetNTPConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_PACK);

    return ret;
}
int xmt_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetNTPConfig ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ntp_cfg, 0, sizeof(SDK_NetNTPConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_NTP;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ntp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetNTPConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_NET_COMMON net_cfg;
    SDK_NetDNSConfig dns_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&net_cfg, 0, sizeof(SDK_CONFIG_NET_COMMON));
    memset(&dns_cfg, 0, sizeof(SDK_NetDNSConfig));

    xmt_cfg.command  = E_SDK_CONFIG_SYSNET;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &net_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_NET_COMMON);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_network_info(&net_cfg, net_info, SWAP_PACK);
    else
        goto done;

    xmt_cfg.command  = E_SDK_CONFIG_NET_DNS;
    xmt_cfg.buffer   = &dns_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetDNSConfig);

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_dns_info(&dns_cfg, net_info, SWAP_PACK);
    
done:
    return ret;
}
int xmt_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_NET_COMMON net_cfg;
    SDK_NetDNSConfig dns_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&net_cfg, 0, sizeof(SDK_CONFIG_NET_COMMON));
    memset(&dns_cfg, 0, sizeof(SDK_NetDNSConfig));

    xmt_cfg.command  = E_SDK_CONFIG_SYSNET;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &net_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_NET_COMMON);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_network_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
        if (ret)
            goto done;
    }
    else
        goto done;

    xmt_cfg.command  = E_SDK_CONFIG_NET_DNS;
    xmt_cfg.buffer   = &dns_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetDNSConfig);

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_dns_info(&dns_cfg, net_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

done:
    return ret;
}
int xmt_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetPPPoEConfig pppoe_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&pppoe_cfg, 0, sizeof(SDK_NetPPPoEConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_PPPOE;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &pppoe_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetPPPoEConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_pppoe_info(&pppoe_cfg, pppoe_info, SWAP_PACK);

    return ret;
}
int xmt_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetPPPoEConfig pppoe_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&pppoe_cfg, 0, sizeof(SDK_NetPPPoEConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_PPPOE;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &pppoe_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetPPPoEConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_pppoe_info(&pppoe_cfg, pppoe_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_FtpServerConfig ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ftp_cfg, 0, sizeof(SDK_FtpServerConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_FTPSERVER;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ftp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_FtpServerConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);

    return ret;
}
int xmt_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_FtpServerConfig ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ftp_cfg, 0, sizeof(SDK_FtpServerConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_FTPSERVER;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ftp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_FtpServerConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetEmailConfig email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&email_cfg, 0, sizeof(SDK_NetEmailConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_EMAIL;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &email_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetEmailConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_smtp_info(&email_cfg, smtp_info, SWAP_PACK);

    return ret;
}
int xmt_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetEmailConfig email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&email_cfg, 0, sizeof(SDK_NetEmailConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_EMAIL;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &email_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetEmailConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_smtp_info(&email_cfg, smtp_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetDDNSConfigALL ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ddns_cfg, 0, sizeof(SDK_NetDDNSConfigALL));

    xmt_cfg.command  = E_SDK_CONFIG_NET_DDNS;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ddns_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetDDNSConfigALL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);

    return ret;
}
int xmt_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetDDNSConfigALL ddns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&ddns_cfg, 0, sizeof(SDK_NetDDNSConfigALL));

    xmt_cfg.command  = E_SDK_CONFIG_NET_DDNS;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &ddns_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetDDNSConfigALL);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetUPNPConfig upnp_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&upnp_cfg, 0, sizeof(SDK_NetUPNPConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_UPNP;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &upnp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetUPNPConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_upnp_info(&upnp_cfg, upnp_info, SWAP_PACK);

    return ret;
}
int xmt_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_NetUPNPConfig upnp_cfg;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&upnp_cfg, 0, sizeof(SDK_NetUPNPConfig));

    xmt_cfg.command  = E_SDK_CONFIG_NET_UPNP;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;
    xmt_cfg.buffer   = &upnp_cfg;
    xmt_cfg.b_size   = sizeof(SDK_NetUPNPConfig);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_upnp_info(&upnp_cfg, upnp_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceDiskInfo *disk_list;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_StorageDeviceInformationAll dh_disk;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&dh_disk, 0, sizeof(SDK_StorageDeviceInformationAll));

    xmt_cfg.command  = E_SDK_CONFIG_DISK_INFO;
    xmt_cfg.buffer   = &dh_disk;
    xmt_cfg.b_size   = sizeof(SDK_StorageDeviceInformationAll);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_disk_list(&dh_disk, disk_list, SWAP_PACK);

    return ret;
}
int xmt_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFormatDisk *fmt_disk;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_StorageDeviceInformationAll dh_disk;
    SDK_StorageDeviceControl storage_ctl;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&dh_disk, 0, sizeof(SDK_StorageDeviceInformationAll));
    memset(&storage_ctl, 0, sizeof(SDK_StorageDeviceControl));

    xmt_cfg.command  = E_SDK_CONFIG_DISK_INFO;
    xmt_cfg.buffer   = &dh_disk;
    xmt_cfg.b_size   = sizeof(SDK_StorageDeviceInformationAll);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, GET_DISK_LIST, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_format_info(&storage_ctl, &dh_disk, fmt_disk);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &storage_ctl);
    }

    return ret;
}
int xmt_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(srv && pvalue);

    xmt_basic = (xmt_service_basic_t*)srv->tm;

    return (*xmt_basic->ro.set_config)(srv, parm_id, pvalue);
}
int xmt_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_ENCODE enc_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&enc_cfg, 0, sizeof(SDK_CONFIG_ENCODE));

    xmt_cfg.command  = E_SDK_CONFIG_SYSENCODE;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &enc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_encode_info(&enc_cfg, enc_info, SWAP_PACK);

    /*xmt_cfg.command  = DH_DEV_DEVICECFG;
    xmt_cfg.buffer   = &sys_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        enc_info->format = sys_cfg.byVideoStandard;*/

    return ret;
}
int xmt_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_CONFIG_ENCODE enc_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&enc_cfg, 0, sizeof(SDK_CONFIG_ENCODE));

    xmt_cfg.command  = E_SDK_CONFIG_SYSENCODE;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &enc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_ENCODE);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_encode_info(&enc_cfg, enc_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VIDEOCOLOR *color;
    SDK_CONFIG_VIDEOCOLOR vc_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
     memset(&vc_cfg, 0, sizeof(SDK_CONFIG_VIDEOCOLOR));

    xmt_cfg.command  = E_SDK_VIDEOCOLOR;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_VIDEOCOLOR);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        color = &vc_cfg.dstVideoColor[1];
        xmt_swap_display_info(&color->dstColor, dis_info, SWAP_PACK);
        color = &vc_cfg.dstVideoColor[0];
        xmt_swap_display_info(&color->dstColor, dis_info, SWAP_PACK);
    }

    return ret;
}
int xmt_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VIDEOCOLOR *color;
    SDK_CONFIG_VIDEOCOLOR vc_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_CONFIG_VIDEOCOLOR));

    xmt_cfg.command  = E_SDK_VIDEOCOLOR;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_CONFIG_VIDEOCOLOR);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        color = &vc_cfg.dstVideoColor[0];
        xmt_swap_display_info(&color->dstColor, dis_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
        color = &vc_cfg.dstVideoColor[1];
        xmt_swap_display_info(&color->dstColor, dis_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VideoWidgetConfigAll osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&osd_cfg, 0, sizeof(SDK_VideoWidgetConfigAll));

    xmt_cfg.command  = E_SDK_CONFIG_OUT_MODE;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;//channel;
    xmt_cfg.buffer   = &osd_cfg;
    xmt_cfg.b_size   = sizeof(SDK_VideoWidgetConfigAll);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_osd_info(&osd_cfg.vVideoWidegetConfigAll[channel], osd_info, SWAP_PACK);

    return ret;
}
int xmt_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VideoWidgetConfigAll osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&osd_cfg, 0, sizeof(SDK_VideoWidgetConfigAll));

    xmt_cfg.command  = E_SDK_CONFIG_OUT_MODE;
    xmt_cfg.channel  = DEF_XMT_ALL_CHANNEL;//channel;
    xmt_cfg.buffer   = &osd_cfg;
    xmt_cfg.b_size   = sizeof(SDK_VideoWidgetConfigAll);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_osd_info(&osd_cfg.vVideoWidegetConfigAll[channel], osd_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_STR_CONFIG_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(SDK_STR_CONFIG_PTZ));
    memset(&xmt_cfg, 0, sizeof(xmt_config_t));

    xmt_cfg.command  = E_SDK_CONFIG_PTZ;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = (char*)&ptz_cfg;
    xmt_cfg.b_size   = sizeof(SDK_STR_CONFIG_PTZ);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_PACK);
    }

    return ret;
}
int xmt_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_STR_CONFIG_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(SDK_STR_CONFIG_PTZ));
    memset(&xmt_cfg, 0, sizeof(xmt_config_t));

    xmt_cfg.command  = E_SDK_CONFIG_PTZ;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = (char*)&ptz_cfg;
    xmt_cfg.b_size   = sizeof(SDK_STR_CONFIG_PTZ);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_RECORDCONFIG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&rec_cfg, 0, sizeof(SDK_RECORDCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_RECORD;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &rec_cfg;
    xmt_cfg.b_size   = sizeof(SDK_RECORDCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_record_info(&rec_cfg, rec_info, SWAP_PACK);
    }

    return ret;
}
int xmt_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_RECORDCONFIG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&rec_cfg, 0, sizeof(SDK_RECORDCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_RECORD;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &rec_cfg;
    xmt_cfg.b_size   = sizeof(SDK_RECORDCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }
    
    return ret;
}
int xmt_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    /*int ret;
    JHideParameter *hide_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_BLINDDETECTCONFIG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_BLINDDETECTCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_SHELTER;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
        xmt_swap_hide_info(&vc_cfg, hide_info, SWAP_PACK);*/

    return -1;//ret;
}
int xmt_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    /*int ret;
    JHideParameter *hide_info;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_BLINDDETECTCONFIG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&vc_cfg, 0, sizeof(SDK_BLINDDETECTCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_SHELTER;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vc_cfg;
    xmt_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        //xmt_swap_hide_info(&vc_cfg, hide_info, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }*/

    return -1;//ret;
}
int xmt_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_MOTIONCONFIG motion_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&motion_cfg, 0, sizeof(SDK_MOTIONCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_MOTION;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &motion_cfg;
    xmt_cfg.b_size   = sizeof(SDK_MOTIONCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_move_alarm_info(&motion_cfg, motion, SWAP_PACK);
    }

    return ret;
}
int xmt_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_MOTIONCONFIG motion_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&motion_cfg, 0, sizeof(SDK_MOTIONCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_MOTION;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &motion_cfg;
    xmt_cfg.b_size   = sizeof(SDK_MOTIONCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_move_alarm_info(&motion_cfg, motion, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VIDEOLOSSCONFIG vl_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&vl_cfg, 0, sizeof(SDK_VIDEOLOSSCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_VIDEO_LOSS;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vl_cfg;
    xmt_cfg.b_size   = sizeof(SDK_VIDEOLOSSCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_video_lost_info(&vl_cfg, video_lost, SWAP_PACK);
    }

    return ret;
}
int xmt_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_VIDEOLOSSCONFIG vl_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&vl_cfg, 0, sizeof(SDK_VIDEOLOSSCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_VIDEO_LOSS;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &vl_cfg;
    xmt_cfg.b_size   = sizeof(SDK_VIDEOLOSSCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_video_lost_info(&vl_cfg, video_lost, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_BLINDDETECTCONFIG blind_detect;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&blind_detect, 0, sizeof(SDK_BLINDDETECTCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_SHELTER;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &blind_detect;
    xmt_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_hide_alarm_info(&blind_detect, hide_alarm, SWAP_PACK);
    }

    return ret;
}
int xmt_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_BLINDDETECTCONFIG blind_detect;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&blind_detect, 0, sizeof(SDK_BLINDDETECTCONFIG));

    xmt_cfg.command  = E_SDK_CONFIG_SHELTER;
    xmt_cfg.channel  = channel;
    xmt_cfg.buffer   = &blind_detect;
    xmt_cfg.b_size   = sizeof(SDK_BLINDDETECTCONFIG);
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id-1, &xmt_cfg);
    if (!ret)
    {
        xmt_swap_hide_alarm_info(&blind_detect, hide_alarm, SWAP_UNPACK);
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
    }

    return ret;
}
int xmt_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_ALARM_INPUTCONFIG alarm_in;
    SDK_AlarmOutConfig    alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&alarm_in, 0, sizeof(SDK_ALARM_INPUTCONFIG));
    memset(&alarm_out, 0, sizeof(SDK_AlarmOutConfig));

    xmt_cfg.channel  = channel;
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            xmt_cfg.command  = E_SDK_CONFIG_ALARM_IN;
            xmt_cfg.buffer   = &alarm_in;
            xmt_cfg.b_size   = sizeof(SDK_ALARM_INPUTCONFIG);
            ret = (*xmt_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &xmt_cfg);
            if (!ret)
                xmt_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            xmt_cfg.command  = E_SDK_CONFIG_ALARM_OUT;
            xmt_cfg.buffer   = &alarm_out;
            xmt_cfg.b_size   = sizeof(SDK_AlarmOutConfig);
            ret = (*xmt_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &xmt_cfg);
            if (!ret)
                xmt_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int xmt_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    xmt_service_basic_t *xmt_basic;

    xmt_config_t xmt_cfg;
    SDK_ALARM_INPUTCONFIG alarm_in;
    SDK_AlarmOutConfig    alarm_out;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_config_t));
    memset(&alarm_in, 0, sizeof(SDK_ALARM_INPUTCONFIG));
    memset(&alarm_out, 0, sizeof(SDK_AlarmOutConfig));

    xmt_cfg.channel  = channel;
    xmt_cfg.waittime = DEF_XMT_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            xmt_cfg.command  = E_SDK_CONFIG_ALARM_IN;
            xmt_cfg.buffer   = &alarm_in;
            xmt_cfg.b_size   = sizeof(SDK_ALARM_INPUTCONFIG);
            ret = (*xmt_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &xmt_cfg);
            if (!ret)
            {
                xmt_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*xmt_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &xmt_cfg);
            }
            break;
        case ALARM_OUT:
            xmt_cfg.command  = E_SDK_CONFIG_ALARM_OUT;
            xmt_cfg.buffer   = &alarm_out;
            xmt_cfg.b_size   = sizeof(SDK_AlarmOutConfig);
            ret = (*xmt_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &xmt_cfg);
            if (!ret)
            {
                xmt_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*xmt_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &xmt_cfg);
            }
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
#define deposit_file_type(type)                                     \
            for (i=filecount; i<xmt_cfg.filecount+filecount; i++)   \
            {/*借用hWnd，记录文件类型 */                \
                file_data[i].hWnd = (void*)type;                    \
            }
int xmt_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i, ret = 0;
    JStoreLog *store;
    xmt_service_basic_t *xmt_basic;

    static int sess_id = 0;
    static int file_type = 0;
    static int filecount = 0;
    static H264_DVR_FILE_DATA file_data[DEF_XMT_MAX_RECORD_FILE];

    xmt_query_t xmt_cfg;

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

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

    memset(&xmt_cfg, 0, sizeof(xmt_query_t));
    memset(file_data, 0, sizeof(H264_DVR_FILE_DATA)*DEF_XMT_MAX_RECORD_FILE);

    xmt_cfg.channel   = channel;
    xmt_cfg.buffer    = (char*)file_data;
    //xmt_cfg.buf_size  = sizeof(H264_DVR_FILE_DATA)*DEF_XMT_MAX_RECORD_FILE;
    xmt_cfg.max_count = DEF_XMT_MAX_RECORD_FILE;
    xmt_swap_store_log_info(&xmt_cfg, store, SWAP_UNPACK);

    if (store->rec_type & TIMER_RECODE)
    {
        xmt_cfg.file_type = SDK_RECORD_REGULAR;
        ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
        if (!ret)
        {
            deposit_file_type(TIMER_RECODE);
            filecount += xmt_cfg.filecount;
        }
    }
    if (store->rec_type & ALARM_RECODE)
    {
        xmt_cfg.file_type = SDK_RECORD_ALARM;
        xmt_cfg.max_count-= filecount;
        xmt_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
        if (!ret)
        {
            deposit_file_type(ALARM_RECODE);
            filecount += xmt_cfg.filecount;
        }
    }
    if (store->rec_type & MOVE_RECODE)
    {
        xmt_cfg.file_type = SDK_RECORD_DETECT;
        xmt_cfg.max_count-= filecount;
        xmt_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
        if (!ret)
        {
            deposit_file_type(MOVE_RECODE);
            filecount += xmt_cfg.filecount;
        }
    }
    if (store->rec_type & MANUAL_RECODE)
    {
        xmt_cfg.file_type = SDK_RECORD_MANUAL;
        xmt_cfg.max_count-= filecount;
        xmt_cfg.buffer    = (char*)file_data + 
            (filecount*sizeof(H264_DVR_FILE_DATA));
        ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
        if (!ret)
        {
            deposit_file_type(MANUAL_RECODE);
            filecount += xmt_cfg.filecount;
        }
    }

done:
    xmt_cfg.buffer    = (char*)file_data;
    xmt_cfg.filecount = filecount;
    if (!ret)
        xmt_swap_store_log_info(&xmt_cfg, store, SWAP_PACK);

    return ret;
}
int xmt_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JPTZControl *ptz_ctrl;
    xmt_service_basic_t *xmt_basic;

    xmt_ptz_ctrl_t xmt_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_ptz_ctrl_t));

    xmt_cfg.channel = channel;
    xmt_swap_ptz_cmd(&xmt_cfg, ptz_ctrl, SWAP_UNPACK);

    return (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);
}
int xmt_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JPPConfig *pp_cfg;
    xmt_service_basic_t *xmt_basic;

    xmt_ptz_ctrl_t xmt_cfg;

    NMP_ASSERT(srv && pvalue);

    pp_cfg = (JPPConfig*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_ptz_ctrl_t));

    xmt_cfg.channel = channel;

    switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
            xmt_cfg.ptz_cmd = EXTPTZ_POINT_SET_CONTROL;
            break;
        case PTZ_USE_PP:
            xmt_cfg.ptz_cmd = EXTPTZ_POINT_MOVE_CONTROL;
            break;
        case PTZ_DEL_PP:
            xmt_cfg.ptz_cmd = EXTPTZ_POINT_DEL_CONTROL;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);

    return ret;
}
int xmt_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JCruiseWay *crz_way;
    xmt_service_basic_t *xmt_basic;

    xmt_cruise_t xmt_cfg;
    //NET_DVR_CRUISE_RET xmt_crz;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_cruise_t));
    //memset(&xmt_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    /*xmt_cfg.channel = channel+1;
    xmt_cfg.crz_no  = crz_way->crz_info.crz_no;
    xmt_cfg.input   = (int)&xmt_crz;

    ret = (*xmt_basic->ro.get_config)(srv, parm_id, &xmt_cfg);
    if (!ret)
    {
        sprintf((char*)crz_way->crz_info.crz_name, 
            "宸¤埅璺緞 %d", crz_way->crz_info.crz_no+1);
        //xmt_swap_cruise_way(&xmt_crz, crz_way, SWAP_PACK);
    }*/

    return ret;
}
int xmt_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JCruiseConfig *crz_cfg;
    xmt_service_basic_t *xmt_basic;

    xmt_cruise_t xmt_cfg;
    /*NET_DVR_CRUISE_RET xmt_crz;

    NMP_ASSERT(srv && pvalue);

    crz_cfg = (JCruiseConfig*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    memset(&xmt_cfg, 0, sizeof(xmt_cruise_t));
    memset(&xmt_crz, 0, sizeof(NET_DVR_CRUISE_RET));

    xmt_cfg.channel = channel;
    xmt_cfg.crz_no  = crz_cfg->crz_no;
    show_debug("%s() crz_no: %d<--------------------\n", __FUNCTION__, xmt_cfg.crz_no);
    switch (crz_cfg->action)
    {
        case PTZ_START_CRZ:
            xmt_cfg.crz_cmd = RUN_SEQ;
            break;
        case PTZ_STOP_CRZ:
            xmt_cfg.crz_cmd = STOP_SEQ;
            break;
        case PTZ_DEL_CRZ:
            xmt_cfg.crz_cmd = CLE_PRE_SEQ;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);*/

    return ret;
}
int xmt_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    xmt_cruise_t xmt_cfg;
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    xmt_cfg.channel = channel;
    xmt_cfg.crz_no  = crz_way->crz_info.crz_no;

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        /*xmt_cfg.crz_cmd = FILL_PRE_SEQ;
        xmt_cfg.input   = crz_point->preset;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);

        xmt_cfg.crz_cmd = SET_SEQ_SPEED;
        xmt_cfg.input   = crz_point->speed;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);

        xmt_cfg.crz_cmd = SET_SEQ_DWELL;
        xmt_cfg.input   = crz_point->dwell;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);*/
    }

    return 0;
}
int xmt_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i;
    JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    xmt_cruise_t xmt_cfg;
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    xmt_basic = (xmt_service_basic_t*)srv->tm;

    xmt_cfg.channel   = channel;
    xmt_cfg.crz_no = crz_way->crz_info.crz_no;

    for (i=0; i<(int)crz_way->pp_count; i++)
    {
        crz_point = &crz_way->crz_pp[i];

        /*xmt_cfg.crz_cmd = FILL_PRE_SEQ;
        xmt_cfg.input   = crz_point->preset;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);

        xmt_cfg.crz_cmd = SET_SEQ_SPEED;
        xmt_cfg.input   = crz_point->speed;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);

        xmt_cfg.crz_cmd = SET_SEQ_DWELL;
        xmt_cfg.input   = crz_point->dwell;
        (*xmt_basic->ro.set_config)(srv, parm_id, &xmt_cfg);*/
    }

    return 0;
}
int xmt_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(srv && pvalue);

    xmt_basic = (xmt_service_basic_t*)srv->tm;

    return (*xmt_basic->ro.get_config)(srv, parm_id, pvalue);
}



