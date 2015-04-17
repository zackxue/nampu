
#include "nmp_proxy_sdk.h"

#include "nmp_hbn_swap.h"
#include "nmp_hbn_service.h"
#include "nmp_hbn_handler.h"


#define DEF_HBN_TIME_ZOE            8
#define DEF_HBN_NO_CHANNEL      0xFFFFFFFF
#define DEF_HBN_FACTORY_INFO        "http://www.hbgk.net/cn/"

#define DEF_HBN_GET_SUCCESSFUL  0

int hbn_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_DEVICECFG dev_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&dev_cfg, 0, sizeof(HB_NET_DEVICECFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_DEVICECFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &dev_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_DEVICECFG);
    dev_cfg.dwSize = sizeof(HB_NET_DEVICECFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, (void*)&hbn_cfg);
    if (!ret)
    {
        hbn_swap_device_info(&dev_cfg, dev_info, SWAP_PACK);
        strncpy((char*)dev_info->manu_info, DEF_HBN_FACTORY_INFO, 
            sizeof(dev_info->manu_info)-1);
    }

    return ret;
}
int hbn_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_SERIALCFG serial_cfg;//´®¿ÚÀàÐÍ: 1-232, 2-485
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&serial_cfg, 0, sizeof(HB_NET_SERIALCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_SERIALCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &serial_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_SERIALCFG);

    serial_cfg.dwSize = sizeof(HB_NET_SERIALCFG);
    serial_cfg.dwSerialType = serial_info->serial_no ? 1 : 2;

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_serial_info(&serial_cfg, serial_info, SWAP_PACK);

    return ret;
}
int hbn_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_SERIALCFG serial_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&serial_cfg, 0, sizeof(HB_NET_SERIALCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_SERIALCFG;
    hbn_cfg.dwChannel   = 0;
    hbn_cfg.pInBuffer   = &serial_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_SERIALCFG);

    serial_cfg.dwSize = sizeof(HB_NET_SERIALCFG);
    serial_cfg.dwSerialType = serial_info->serial_no ? 1 : 2;

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_SERIALCFG;
        hbn_swap_serial_info(&serial_cfg, serial_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    hbn_service_basic_t *hbn_basic;

    HB_NET_TIME hbn_time;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&hbn_time, 0, sizeof(HB_NET_TIME));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_TIMECFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &hbn_time;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_TIME);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_time_info(&hbn_time, dev_time, SWAP_PACK);

    return ret;
}
int hbn_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    hbn_service_basic_t *hbn_basic;

    HB_NET_TIME hbn_time;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&hbn_time, 0, sizeof(HB_NET_TIME));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_TIMECFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &hbn_time;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_TIME);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_TIMECFG;
        hbn_swap_time_info(&hbn_time, dev_time, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    hbn_service_basic_t *hbn_basic;
    
    HB_NET_NTPCFG ntp_cfg;
    HB_NET_DSTTIME dst_time;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ntp_cfg, 0, sizeof(HB_NET_NTPCFG));
    memset(&dst_time, 0, sizeof(HB_NET_DSTTIME));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_NTPCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &ntp_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_NTPCFG);
    ntp_cfg.dwSize = sizeof(HB_NET_NTPCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_PACK);

    hbn_cfg.dwCommand    = HB_NET_GET_DSTTIME;
    hbn_cfg.pOutBuffer   = &dst_time;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_DSTTIME);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        ntp_info->dst_enable = dst_time.wEanble;

    return ret;
}
int hbn_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_NTPCFG ntp_cfg;
    HB_NET_DSTTIME dst_time;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ntp_cfg, 0, sizeof(HB_NET_NTPCFG));
    memset(&dst_time, 0, sizeof(HB_NET_DSTTIME));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_NTPCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &ntp_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_NTPCFG);
    ntp_cfg.dwSize = sizeof(HB_NET_NTPCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_NTPCFG;
        hbn_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
        if (ret)
            goto End;
    }

    hbn_cfg.dwCommand   = HB_NET_GET_DSTTIME;
    hbn_cfg.pInBuffer   = &dst_time;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_DSTTIME);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_DSTTIME;
        dst_time.wEanble = ntp_info->dst_enable;
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

End:
    return ret;
}

int hbn_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_NETCFG net_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HB_NET_NETCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_NETCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &net_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_NETCFG);
    net_cfg.dwSize = sizeof(HB_NET_NETCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_net_info(&net_cfg, net_info, SWAP_PACK);

    return ret;
}
int hbn_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_NETCFG net_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HB_NET_NETCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_NETCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &net_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_NETCFG);
    net_cfg.dwSize = sizeof(HB_NET_NETCFG);
    
    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_NETCFG;
        hbn_swap_net_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_NETCFG net_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HB_NET_NETCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_NETCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &net_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_NETCFG);
    net_cfg.dwSize = sizeof(HB_NET_NETCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_pppoe_info(&net_cfg, pppoe_info, SWAP_PACK);

    return ret;
}
int hbn_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_NETCFG net_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HB_NET_NETCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_NETCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &net_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_NETCFG);
    net_cfg.dwSize = sizeof(HB_NET_NETCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_NETCFG;
        hbn_swap_pppoe_info(&net_cfg, pppoe_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_FTPRECORDCFG ftp_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ftp_cfg, 0, sizeof(HB_NET_FTPRECORDCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_FTPRECORDCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &ftp_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_FTPRECORDCFG);
    ftp_cfg.dwSize = sizeof(HB_NET_FTPRECORDCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);
    
    return ret;
}
int hbn_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_FTPRECORDCFG ftp_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ftp_cfg, 0, sizeof(HB_NET_FTPRECORDCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_FTPRECORDCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &ftp_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_FTPRECORDCFG);
    ftp_cfg.dwSize = sizeof(HB_NET_FTPRECORDCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_FTPRECORDCFG;
        hbn_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    hbn_service_basic_t *hbn_basic;
    
    HB_NET_SMTPCFG smtp_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&smtp_cfg, 0, sizeof(HB_NET_SMTPCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_SMTPCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &smtp_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_SMTPCFG);
    smtp_cfg.dwSize = sizeof(HB_NET_SMTPCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_smtp_info(&smtp_cfg, smtp_info, SWAP_PACK);

    return ret;
}
int hbn_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_SMTPCFG smtp_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&smtp_cfg, 0, sizeof(HB_NET_SMTPCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_SMTPCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &smtp_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_SMTPCFG);
    smtp_cfg.dwSize = sizeof(HB_NET_SMTPCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_SMTPCFG;
        hbn_swap_smtp_info(&smtp_cfg, smtp_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hbn_service_basic_t *hbn_basic;
    
    HB_NET_DDNSCFG ddns_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ddns_cfg, 0, sizeof(HB_NET_DDNSCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_DDNSCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &ddns_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_DDNSCFG);
    ddns_cfg.dwSize = sizeof(HB_NET_DDNSCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_PACK);
    
    return ret;
}
int hbn_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_DDNSCFG ddns_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&ddns_cfg, 0, sizeof(HB_NET_DDNSCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_DDNSCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &ddns_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_DDNSCFG);
    ddns_cfg.dwSize = sizeof(HB_NET_DDNSCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_swap_ddns_info(&ddns_cfg, ddns_info, SWAP_UNPACK);
        hbn_cfg.dwCommand = HB_NET_SET_DDNSCFG;
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}
int hbn_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    return ret;
}
int hbn_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    return ret;
}
int hbn_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret, bytes = 0;
    JDeviceDiskInfo *disk_list;
    hbn_service_basic_t *hbn_basic;

    HB_NET_WORKSTAT work_stat;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&hbn_cfg, 0, sizeof(get_store_t));
    memset(&work_stat, 0, sizeof(HB_NET_WORKSTAT));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_WORK_STAT;
    hbn_cfg.dwChannel    = 0;
    hbn_cfg.pOutBuffer   = &work_stat;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_WORKSTAT);
    hbn_cfg.pBytesRet    = &bytes;
    work_stat.dwSize     = sizeof(HB_NET_WORKSTAT);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_disk_list(work_stat.struHardDiskStat, disk_list, SWAP_PACK);

    return ret;
}
int hbn_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JFormatDisk *fmt_disk;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    return (*hbn_basic->ro.set_config)(srv, parm_id, &fmt_disk->disk_no);
}

int hbn_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    hbn_basic = (hbn_service_basic_t*)srv->tm;

    return (*hbn_basic->ro.set_config)(srv, parm_id, pvalue);
}

int hbn_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JEncodeParameter *enc_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_COMPRESSCFG cmp_cfg;
    HB_NET_IFRAMERATE  ifrm_rate;
    HB_NET_VIDEOSYS    video_sys;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&cmp_cfg, 0, sizeof(HB_NET_COMPRESSCFG));
    memset(&ifrm_rate, 0, sizeof(HB_NET_IFRAMERATE));
    memset(&video_sys, 0, sizeof(HB_NET_VIDEOSYS));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwChannel    = channel;

    hbn_cfg.dwCommand    = HB_NET_GET_COMPRESSCFG;
    hbn_cfg.pOutBuffer   = &cmp_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_COMPRESSCFG);
    cmp_cfg.dwSize = sizeof(HB_NET_COMPRESSCFG);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg))
        hbn_swap_encode_info(&cmp_cfg.struRecord, enc_info, SWAP_PACK);

    hbn_cfg.dwCommand    = HB_NET_GET_IFRAMERATE;
    hbn_cfg.pOutBuffer   = &ifrm_rate;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_IFRAMERATE);
    ifrm_rate.wChannel = channel;
    ifrm_rate.dwSize = sizeof(HB_NET_IFRAMERATE);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg))
    {
        printf("channel: %d, %d\n", ifrm_rate.wChannel, (int)ifrm_rate.dwIFrameRate);
        if (hbn_cfg.dwChannel == ifrm_rate.wChannel)
            enc_info->i_frame_interval = ifrm_rate.dwIFrameRate;
    }

    hbn_cfg.dwCommand    = HB_NET_GET_VIDEOSYS;
    hbn_cfg.pOutBuffer   = &video_sys;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_VIDEOSYS);
    video_sys.dwSize = sizeof(HB_NET_VIDEOSYS);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg))
    {
        printf("VideoFormat: %d\n", (int)video_sys.wSys);
        enc_info->format = (--video_sys.wSys > 1) ? video_sys.wSys-1 : video_sys.wSys;
    }

    return ret;
}
int hbn_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JEncodeParameter *enc_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_COMPRESSCFG cmp_cfg;
    HB_NET_IFRAMERATE  ifrm_rate;
    HB_NET_VIDEOSYS    video_sys;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&cmp_cfg, 0, sizeof(HB_NET_COMPRESSCFG));
    memset(&ifrm_rate, 0, sizeof(HB_NET_IFRAMERATE));
    memset(&video_sys, 0, sizeof(HB_NET_VIDEOSYS));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwChannel   = channel;

    hbn_cfg.dwCommand   = HB_NET_GET_COMPRESSCFG;
    hbn_cfg.pInBuffer   = &cmp_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_COMPRESSCFG);
    cmp_cfg.dwSize = sizeof(HB_NET_COMPRESSCFG);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg))
    {
        hbn_cfg.dwCommand = HB_NET_SET_COMPRESSCFG;
        hbn_swap_encode_info(&cmp_cfg.struRecord, enc_info, SWAP_UNPACK);
        ret += (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }
    else
        ret += -1;

    hbn_cfg.dwCommand   = HB_NET_GET_IFRAMERATE;
    hbn_cfg.pInBuffer   = &ifrm_rate;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_IFRAMERATE);
    ifrm_rate.dwSize = sizeof(HB_NET_IFRAMERATE);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg))
    {
        hbn_cfg.dwCommand = HB_NET_SET_IFRAMERATE;
        ifrm_rate.dwIFrameRate = enc_info->i_frame_interval;
        ret += (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }
    else
        ret += -1;

    hbn_cfg.dwCommand   = HB_NET_GET_VIDEOSYS;
    hbn_cfg.pInBuffer   = &video_sys;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_VIDEOSYS);
    video_sys.dwSize = sizeof(HB_NET_VIDEOSYS);

    if (!(*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg))
    {
        hbn_cfg.dwCommand = HB_NET_SET_VIDEOSYS;
        video_sys.wSys = enc_info->format+1;
        ret += (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }
    else
        ret += -1;

    return ret;
}

int hbn_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_VIDEOEFFECT vid_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&vid_cfg, 0, sizeof(HB_NET_VIDEOEFFECT));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_VEFF;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &vid_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_VIDEOEFFECT);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_display_info(&vid_cfg.Default_VideoParam, dis_info, SWAP_PACK);

    return ret;
}

int hbn_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_VIDEOEFFECT vid_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&vid_cfg, 0, sizeof(HB_NET_VIDEOEFFECT));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_VEFF;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &vid_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_VIDEOEFFECT);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_VEFF;
        hbn_swap_display_info(&vid_cfg.Default_VideoParam, dis_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pic_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_osd_info(&pic_cfg, osd_info, SWAP_PACK);

    return ret;
}
int hbn_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &pic_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_PICCFG;
        hbn_swap_osd_info(&pic_cfg, osd_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_DECODERCFG dec_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&dec_cfg, 0, sizeof(HB_NET_DECODERCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_DECODERCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &dec_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_DECODERCFG);
    dec_cfg.dwSize = sizeof(HB_NET_DECODERCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_decoder_info(&dec_cfg, ptz_info, SWAP_PACK);

    return ret;
}
int hbn_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_DECODERCFG dec_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&dec_cfg, 0, sizeof(HB_NET_DECODERCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_DECODERCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &dec_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_DECODERCFG);
    dec_cfg.dwSize = sizeof(HB_NET_DECODERCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_DECODERCFG;
        hbn_swap_decoder_info(&dec_cfg, ptz_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_RECORDCFG rec_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&rec_cfg, 0, sizeof(HB_NET_RECORDCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_RECORDCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &rec_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_RECORDCFG);
    rec_cfg.dwSize = sizeof(HB_NET_RECORDCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_record_info(&rec_cfg, rec_info, SWAP_PACK);

    return ret;
}
int hbn_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_RECORDCFG rec_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&rec_cfg, 0, sizeof(HB_NET_RECORDCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_RECORDCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &rec_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_RECORDCFG);
    rec_cfg.dwSize = sizeof(HB_NET_RECORDCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_RECORDCFG;
        hbn_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pic_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
    {
        hide_info->hide_enable = pic_cfg.byShelter;
        hbn_swap_hide_info(pic_cfg.struShelter, hide_info, SWAP_PACK);
    }
    
    return ret;
}
int hbn_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &pic_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_PICCFG;
        pic_cfg.byShelter = hide_info->hide_enable;
        hbn_swap_hide_info(pic_cfg.struShelter, hide_info, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pic_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_PACK);

    return ret;
}
int hbn_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &pic_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_PICCFG;
        hbn_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pic_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_PACK);

    return ret;
}
int hbn_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &pic_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_PICCFG;
        hbn_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pic_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
        hbn_swap_hide_alarm_info(&pic_cfg.struHide, hide_alarm, SWAP_PACK);

    return ret;
}
int hbn_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hbn_cfg.dwChannel   = channel;
    hbn_cfg.pInBuffer   = &pic_cfg;
    hbn_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id-1, &hbn_cfg);
    if (!ret)
    {
        hbn_cfg.dwCommand = HB_NET_SET_PICCFG;
        hbn_swap_hide_alarm_info(&pic_cfg.struHide, hide_alarm, SWAP_UNPACK);
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_cfg);
    }

    return ret;
}

int hbn_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    hbn_service_basic_t *hbn_basic;

    HB_NET_ALARMINCFG alarm_in;
    HB_NET_ALARMOUTCFG alarm_out;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&alarm_in, 0, sizeof(HB_NET_ALARMINCFG));
    memset(&alarm_out, 0, sizeof(HB_NET_ALARMOUTCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize    = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwChannel = channel;

    switch (type)
    {
        case ALARM_IN:
            hbn_cfg.dwCommand    = HB_NET_GET_ALARMINCFG;
            hbn_cfg.pOutBuffer   = &alarm_in;
            hbn_cfg.dwOutBufSize = sizeof(HB_NET_ALARMINCFG);
            alarm_in.dwSize = sizeof(HB_NET_ALARMINCFG);

            ret = (*hbn_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hbn_cfg);
            if (!ret)
                hbn_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            hbn_cfg.dwCommand    = HB_NET_GET_ALARMOUTCFG;
            hbn_cfg.pOutBuffer   = &alarm_out;
            hbn_cfg.dwOutBufSize = sizeof(HB_NET_ALARMOUTCFG);
            alarm_out.dwSize = sizeof(HB_NET_ALARMOUTCFG);

            ret = (*hbn_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hbn_cfg);
            if (!ret)
                hbn_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int hbn_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    hbn_service_basic_t *hbn_basic;

    HB_NET_ALARMINCFG alarm_in;
    HB_NET_ALARMOUTCFG alarm_out;
    HB_NET_SETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&alarm_in, 0, sizeof(HB_NET_ALARMINCFG));
    memset(&alarm_out, 0, sizeof(HB_NET_ALARMOUTCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hbn_cfg.dwSize    = sizeof(HB_NET_SETDEVCONFIG);
    hbn_cfg.dwChannel = channel;

    switch (type)
    {
        case ALARM_IN:
            hbn_cfg.dwCommand   = HB_NET_GET_ALARMINCFG;
            hbn_cfg.pInBuffer   = &alarm_in;
            hbn_cfg.dwInBufSize = sizeof(HB_NET_ALARMINCFG);
            alarm_in.dwSize = sizeof(HB_NET_ALARMINCFG);

            ret = (*hbn_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hbn_cfg);
            if (!ret)
            {
                hbn_cfg.dwCommand = HB_NET_SET_ALARMINCFG;
                hbn_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*hbn_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hbn_cfg);
            }
            break;
        case ALARM_OUT:
            hbn_cfg.dwCommand   = HB_NET_GET_ALARMOUTCFG;
            hbn_cfg.pInBuffer   = &alarm_out;
            hbn_cfg.dwInBufSize = sizeof(HB_NET_ALARMOUTCFG);
            alarm_out.dwSize = sizeof(HB_NET_ALARMOUTCFG);

            ret = (*hbn_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hbn_cfg);
            if (!ret)
            {
                hbn_cfg.dwCommand = HB_NET_SET_ALARMOUTCFG;
                hbn_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_UNPACK);
                ret = (*hbn_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hbn_cfg);
            }
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int hbn_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JStoreLog *store;
    get_store_t get_store;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    get_store.channel = channel+1;
    get_store.buffer  = store;
    get_store.b_size  = sizeof(JStoreLog);

    hbn_swap_store_log_info(store, store, SWAP_UNPACK);
    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &get_store);
    if (!ret)
        hbn_swap_store_log_info(store, store, SWAP_PACK);

    return ret;
}

int hbn_ptz_control(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i, flag = 0, ret = 0;
    static int ptz_cmd = PTZ_STOP;
    int hbn_cmd[] = {PAN_AUTO, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN, /*UP_LEFT, DOWN_LEFT, UP_RIGHT, DOWN_RIGHT,*/ 
                    ZOOM_IN, ZOOM_OUT, FOCUS_FAR, FOCUS_NEAR, LIGHT_PWRON, 0, WIPER_PWRON, 0,
                    SET_PRESET, GOTO_PRESET, CLE_PRESET};
    int jxnmp_cmd[] = {PTZ_AUTO, PTZ_LEFT, PTZ_RIGHT, PTZ_UP, PTZ_DOWN, /*PTZ_LEFT_UP, PTZ_LEFT_DOWN, PTZ_RIGHT_UP, PTZ_RIGHT_DOWN,*/ 
                    PTZ_ADD_ZOOM, PTZ_SUB_ZOOM, PTZ_ADD_FOCUS, PTZ_SUB_FOCUS, PTZ_TURN_ON, PTZ_TURN_OFF, PTZ_WIPERS_ON, PTZ_WIPERS_OFF,
                    PTZ_SET_PP, PTZ_USE_PP, PTZ_DEL_PP};
    
    JPTZControl *ptz_ctrl;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PTZCTRL hbn_ptz_ctrl;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&hbn_ptz_ctrl, 0, sizeof(HB_NET_PTZCTRL));

    hbn_ptz_ctrl.dwSize = sizeof(HB_NET_PTZCTRL);
    hbn_ptz_ctrl.dwChannel = channel;
//    ptz_ctrl.dwIndex = ;
//    ptz_ctrl.dwSpeed = ;

    if (PTZ_STOP == ptz_cmd)
    {
        for (i=0; i<(int)(sizeof(jxnmp_cmd)/sizeof(*jxnmp_cmd)); i++)
        {
            if (jxnmp_cmd[i] == (int)ptz_ctrl->action)
                break;
        }
        
        if (PTZ_TURN_OFF == jxnmp_cmd[i] || PTZ_WIPERS_OFF == jxnmp_cmd[i])
        {
            hbn_ptz_ctrl.dwPTZCmd = ALL_STOP;
        }
        else
        {
            hbn_ptz_ctrl.dwPTZCmd = hbn_cmd[i];
            ptz_cmd = hbn_ptz_ctrl.dwPTZCmd;
        }
    }
    else if (PTZ_STOP == ptz_ctrl->action)
    {
        hbn_ptz_ctrl.dwPTZCmd = ALL_STOP;
        ptz_cmd = PTZ_STOP;
    }
    else
    {
        for (i=0; i<(int)(sizeof(jxnmp_cmd)/sizeof(*jxnmp_cmd)); i++)
        {
            if (jxnmp_cmd[i] == (int)ptz_ctrl->action)
                break;
        }

        if (ptz_cmd == hbn_cmd[i])
        {
            flag = 1;
        }
        else if (PTZ_TURN_OFF == jxnmp_cmd[i] || PTZ_WIPERS_OFF == jxnmp_cmd[i])
        {
            hbn_ptz_ctrl.dwPTZCmd = ALL_STOP;
            ptz_cmd = PTZ_STOP;
        }
        else
        {
            hbn_ptz_ctrl.dwPTZCmd = ALL_STOP;
            (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_ptz_ctrl);

            hbn_ptz_ctrl.dwPTZCmd = hbn_cmd[i];
            ptz_cmd = hbn_ptz_ctrl.dwPTZCmd;
        }

    }

    if (!flag)
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_ptz_ctrl);
    else
        ret = 0;

    return ret;
}
int hbn_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JPPConfig *pp_cfg;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PTZCTRL hbn_ptz_ctrl;

    NMP_ASSERT(srv && pvalue);

    pp_cfg = (JPPConfig*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&hbn_ptz_ctrl, 0, sizeof(HB_NET_PTZCTRL));

    hbn_ptz_ctrl.dwSize = sizeof(HB_NET_PTZCTRL);
    hbn_ptz_ctrl.dwChannel = channel;
    hbn_ptz_ctrl.dwIndex = pp_cfg->pp.preset;
//    ptz_ctrl.dwSpeed = ;
    
    switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
            hbn_ptz_ctrl.dwPTZCmd = SET_PRESET;
            break;
        case PTZ_USE_PP:
            hbn_ptz_ctrl.dwPTZCmd = GOTO_PRESET;
            break;
        case PTZ_DEL_PP:
            hbn_ptz_ctrl.dwPTZCmd = CLE_PRESET;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*hbn_basic->ro.set_config)(srv, parm_id, &hbn_ptz_ctrl);

    return ret;
}
int hbn_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JCruiseWay *crz_way;
    hbn_service_basic_t *hbn_basic;

    HB_NET_PRESETPOLLCFG pp_cfg;
    HB_NET_GETDEVCONFIG hbn_cfg;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    memset(&pp_cfg, 0, sizeof(HB_NET_PRESETPOLLCFG));
    memset(&hbn_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hbn_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hbn_cfg.dwCommand    = HB_NET_GET_PRESETPOLL;
    hbn_cfg.dwChannel    = channel;
    hbn_cfg.pOutBuffer   = &pp_cfg;
    hbn_cfg.dwOutBufSize = sizeof(HB_NET_PRESETPOLLCFG);

    ret = (*hbn_basic->ro.get_config)(srv, parm_id, &hbn_cfg);
    if (!ret)
    {
        sprintf((char*)crz_way->crz_info.crz_name, 
            "å·¡èˆªè·¯å¾„ %d", crz_way->crz_info.crz_no+1);
        hbn_swap_cruise_way(&pp_cfg, crz_way, SWAP_PACK);
    }

    return ret;
}
int hbn_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseConfig *crz_cfg;
    hbn_service_basic_t *hbn_basic;

    pvalue.channel = channel;
    pvalue.cruise_no = packet->crz_cfg.crz_no;
    switch (packet->crz_cfg.action)
    {
        case PTZ_START_CRZ:
            pvalue.cruise_cmd = RUN_SEQ;
            break;
        case PTZ_STOP_CRZ:
            pvalue.cruise_cmd = STOP_SEQ;
            break;
        case PTZ_DEL_CRZ:
            pvalue.cruise_cmd = CLE_PRE_SEQ;
            break;
        default:
            break;
    }
    ret = proxy_set_device_parm(dev, parm_id, &pvalue);*/
    
    return ret;
}
int hbn_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    pvalue.channel = channel;
    pvalue.cruise_no = packet->crz_way.crz_info.crz_no;

    for (i=0; i<(int)packet->crz_way.pp_count; i++)
    {
        point = &packet->crz_way.crz_pp[i];
        pvalue.cruise_cmd = FILL_PRE_SEQ;
        pvalue.input = point->preset;
        proxy_set_device_parm(dev, parm_id, &pvalue);
        
        pvalue.cruise_cmd = SET_SEQ_SPEED;
        pvalue.input = point->speed;
        proxy_set_device_parm(dev, parm_id, &pvalue);
        
        pvalue.cruise_cmd = SET_SEQ_DWELL;
        pvalue.input = point->dwell;
        proxy_set_device_parm(dev, parm_id, &pvalue);
    }*/

    return ret;
}
int hbn_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hbn_basic = (hbn_service_basic_t*)srv->tm;

    pvalue.channel = channel;
    pvalue.cruise_no = packet->crz_way.crz_info.crz_no;

    for (i=0; i<(int)packet->crz_way.pp_count; i++)
    {
        point = &packet->crz_way.crz_pp[i];
        pvalue.cruise_cmd = FILL_PRE_SEQ;
        pvalue.input = point->preset;
        proxy_set_device_parm(dev, parm_id, &pvalue);

        pvalue.cruise_cmd = SET_SEQ_SPEED;
        pvalue.input = point->speed;
        proxy_set_device_parm(dev, parm_id, &pvalue);

        pvalue.cruise_cmd = SET_SEQ_DWELL;
        pvalue.input = point->dwell;
        proxy_set_device_parm(dev, parm_id, &pvalue);
    }*/

    return ret;
}
int hbn_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(srv && pvalue);

    hbn_basic = (hbn_service_basic_t*)srv->tm;

    return (*hbn_basic->ro.get_config)(srv, parm_id, pvalue);
}



