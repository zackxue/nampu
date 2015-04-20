
#include "nmp_proxy_sdk.h"

#include "nmp_hie_swap.h"
#include "nmp_hie_service.h"
#include "nmp_hie_handler.h"


#define DEF_HIE_TIME_ZOE            8
#define DEF_HIE_NO_CHANNEL          0xFFFFFFFF
#define DEF_HIE_FACTORY_INFO        "http://"

#define DEF_HIE_GET_SUCCESSFUL  0

int hie_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    hie_service_basic_t *hie_basic;
	
	ConfigInformation hie_cfg;
	HY_DVR_DEVICE_INFO dev_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&dev_cfg, 0, sizeof(HY_DVR_DEVICE_INFO));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_DEVICE_INFO);
    hie_cfg.dwMainCommand = HY_DVR_GET_DEVICEINFO;
	hie_cfg.dwAssistCommand = DEVICEINFO_ALL;

    ret = (*hie_basic->ro.get_config)(srv, parm_id, (void*)&hie_cfg);
    if (!ret)
    {
        hie_swap_device_info((HY_DVR_DEVICE_INFO *)hie_cfg.sConfig, dev_info, SWAP_PACK);
        //strncpy((char*)dev_info->manu_info, DEF_HIE_FACTORY_INFO, 
        //    sizeof(dev_info->manu_info)-1);
    }

    return ret;
}
int hie_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    hie_service_basic_t *hie_basic;

	ConfigInformation hie_cfg;
    HY_DVR_SERIAL serial_cfg;//´®¿ÚÀàÐÍ: 1-232, 2-485


    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&serial_cfg, 0, sizeof(HY_DVR_SERIAL));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(ConfigInformation);
    hie_cfg.dwMainCommand = HY_DVR_GET_SERIALCFG;
	hie_cfg.dwAssistCommand = SERIALCFG_ALL;

    //serial_cfg.dwSize = sizeof(HB_NET_SERIALCFG);
    //serial_cfg.dwSerialType = serial_info->serial_no ? 1 : 2;

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_serial_info((HY_DVR_SERIAL *)hie_cfg.sConfig, serial_info, SWAP_PACK);

    return ret;
}
int hie_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    hie_service_basic_t *hie_basic;

	ConfigInformation hie_cfg;
    HY_DVR_SERIAL serial_cfg;


    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&serial_cfg, 0, sizeof(HY_DVR_SERIAL));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(ConfigInformation);
    hie_cfg.dwMainCommand = HY_DVR_GET_SERIALCFG;
	hie_cfg.dwAssistCommand = SERIALCFG_ALL;


    //serial_cfg.dwSize = sizeof(HB_NET_SERIALCFG);
    //serial_cfg.dwSerialType = serial_info->serial_no ? 1 : 2;

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwMainCommand = HY_DVR_GET_SERIALCFG;
        hie_swap_serial_info((HY_DVR_SERIAL *)hie_cfg.sConfig, serial_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }

    return ret;
}

int hie_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    hie_service_basic_t *hie_basic;

    HY_DVR_TIME hie_time;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&hie_time, 0, sizeof(HY_DVR_TIME));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(ConfigInformation);
    hie_cfg.dwMainCommand = HY_DVR_GET_SYSTIME ;
	hie_cfg.dwAssistCommand = SYSTIME_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_time_info((HY_DVR_TIME *)hie_cfg.sConfig, dev_time, SWAP_PACK);

    return ret;
}
int hie_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    hie_service_basic_t *hie_basic;

    HY_DVR_TIME hie_time;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&hie_time, 0, sizeof(HY_DVR_TIME));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(ConfigInformation);
    hie_cfg.dwMainCommand = HY_DVR_GET_SYSTIME ;
	hie_cfg.dwAssistCommand = SYSTIME_ALL;

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwMainCommand = HY_DVR_SET_SYSTIME;
        hie_swap_time_info((HY_DVR_TIME *)hie_cfg.sConfig, dev_time, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }

    return ret;
}

int hie_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JDeviceNTPInfo *ntp_info;
    hie_service_basic_t *hie_basic;
    
    HB_NET_NTPCFG ntp_cfg;
    HB_NET_DSTTIME dst_time;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ntp_cfg, 0, sizeof(HB_NET_NTPCFG));
    memset(&dst_time, 0, sizeof(HB_NET_DSTTIME));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_NTPCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &ntp_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_NTPCFG);
    ntp_cfg.dwSize = sizeof(HB_NET_NTPCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_PACK);

    hie_cfg.dwCommand    = HB_NET_GET_DSTTIME;
    hie_cfg.pOutBuffer   = &dst_time;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_DSTTIME);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        ntp_info->dst_enable = dst_time.wEanble;*/

    return ret;
}
int hie_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JDeviceNTPInfo *ntp_info;
    hie_service_basic_t *hie_basic;

    HB_NET_NTPCFG ntp_cfg;
    HB_NET_DSTTIME dst_time;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ntp_cfg, 0, sizeof(HB_NET_NTPCFG));
    memset(&dst_time, 0, sizeof(HB_NET_DSTTIME));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_NTPCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &ntp_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_NTPCFG);
    ntp_cfg.dwSize = sizeof(HB_NET_NTPCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_NTPCFG;
        hie_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
        if (ret)
            goto End;
    }

    hie_cfg.dwCommand   = HB_NET_GET_DSTTIME;
    hie_cfg.pInBuffer   = &dst_time;
    hie_cfg.dwInBufSize = sizeof(HB_NET_DSTTIME);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_DSTTIME;
        dst_time.wEanble = ntp_info->dst_enable;
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

//End:
    return ret;
}

int hie_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
   	int ret;
    JNetworkInfo *net_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_NET_CFG net_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HY_DVR_NET_CFG));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_NET_CFG);
    hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
	hie_cfg.dwAssistCommand = NETCFG_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_net_info((HY_DVR_NET_CFG *)hie_cfg.sConfig, net_info, SWAP_PACK);

    return ret;
}
int hie_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JNetworkInfo *net_info;
    hie_service_basic_t *hie_basic;

    HB_NET_NETCFG net_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HB_NET_NETCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_NETCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &net_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_NETCFG);
    net_cfg.dwSize = sizeof(HB_NET_NETCFG);
    
    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_NETCFG;
        hie_swap_net_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_PPPOE_CONF net_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HY_DVR_PPPOE_CONF));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_PPPOE_CONF);
    hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
	hie_cfg.dwAssistCommand = NETCFG_PPPOE_CONF;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_pppoe_info((HY_DVR_PPPOE_CONF *)hie_cfg.sConfig, pppoe_info, SWAP_PACK);

    return ret;
}
int hie_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_PPPOE_CONF net_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&net_cfg, 0, sizeof(HY_DVR_PPPOE_CONF));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_PPPOE_CONF);
    hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
	hie_cfg.dwAssistCommand = NETCFG_PPPOE_CONF;


    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwMainCommand = HY_DVR_SET_NETCFG;
        hie_swap_pppoe_info((HY_DVR_PPPOE_CONF *)hie_cfg.sConfig, pppoe_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }

    return ret;
}

int hie_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JFTPParameter *ftp_info;
    hie_service_basic_t *hie_basic;

    HB_NET_FTPRECORDCFG ftp_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ftp_cfg, 0, sizeof(HB_NET_FTPRECORDCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_FTPRECORDCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &ftp_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_FTPRECORDCFG);
    ftp_cfg.dwSize = sizeof(HB_NET_FTPRECORDCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);*/
    
    return ret;
}
int hie_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JFTPParameter *ftp_info;
    hie_service_basic_t *hie_basic;

    HB_NET_FTPRECORDCFG ftp_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ftp_cfg, 0, sizeof(HB_NET_FTPRECORDCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_FTPRECORDCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &ftp_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_FTPRECORDCFG);
    ftp_cfg.dwSize = sizeof(HB_NET_FTPRECORDCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_FTPRECORDCFG;
        hie_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JSMTPParameter *smtp_info;
    hie_service_basic_t *hie_basic;
    
    HB_NET_SMTPCFG smtp_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&smtp_cfg, 0, sizeof(HB_NET_SMTPCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_SMTPCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &smtp_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_SMTPCFG);
    smtp_cfg.dwSize = sizeof(HB_NET_SMTPCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_smtp_info(&smtp_cfg, smtp_info, SWAP_PACK);*/

    return ret;
}
int hie_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JSMTPParameter *smtp_info;
    hie_service_basic_t *hie_basic;

    HB_NET_SMTPCFG smtp_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&smtp_cfg, 0, sizeof(HB_NET_SMTPCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_SMTPCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &smtp_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_SMTPCFG);
    smtp_cfg.dwSize = sizeof(HB_NET_SMTPCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_SMTPCFG;
        hie_swap_smtp_info(&smtp_cfg, smtp_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hie_service_basic_t *hie_basic;
    
    HY_DVR_DDNS_CONF ddns_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ddns_cfg, 0, sizeof(HY_DVR_DDNS_CONF));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_DDNS_CONF);
    hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
	hie_cfg.dwAssistCommand = NETCFG_DDNS_CONF;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_ddns_info((HY_DVR_DDNS_CONF *)hie_cfg.sConfig, ddns_info, SWAP_PACK);

    return ret;
}
int hie_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_DDNS_CONF ddns_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&ddns_cfg, 0, sizeof(HY_DVR_DDNS_CONF));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_DDNS_CONF);
    hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
	hie_cfg.dwAssistCommand = NETCFG_DDNS_CONF;


    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_swap_ddns_info((HY_DVR_DDNS_CONF *)hie_cfg.sConfig, ddns_info, SWAP_UNPACK);
        hie_cfg.dwMainCommand = HY_DVR_GET_NETCFG;
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }

    return ret;
}
int hie_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    return ret;
}
int hie_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    return ret;
}
int hie_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceDiskInfo *disk_list;
    hie_service_basic_t *hie_basic;

    HY_DVR_STORAGE_CFG work_stat;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&hie_cfg, 0, sizeof(ConfigInformation));
    memset(&work_stat, 0, sizeof(HY_DVR_STORAGE_CFG));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_STORAGE_CFG);
    hie_cfg.dwMainCommand = HY_DVR_GET_STORAGEINFO;
	hie_cfg.dwAssistCommand = STORAGEINFO_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_disk_list((HY_DVR_STORAGE_CFG *)hie_cfg.sConfig, disk_list, SWAP_PACK);

    return ret;
}
int hie_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JFormatDisk *fmt_disk;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    return (*hie_basic->ro.set_config)(srv, parm_id, &fmt_disk->disk_no);
}

int hie_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    hie_basic = (hie_service_basic_t*)srv->tm;

    return (*hie_basic->ro.set_config)(srv, parm_id, pvalue);
}

int hie_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JEncodeParameter *enc_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_COMPRESSION_CFG    encode_info;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&encode_info, 0, sizeof(HY_DVR_COMPRESSION_CFG));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

    hie_cfg.dwConfigLen = sizeof(HY_DVR_COMPRESSION_CFG);
    hie_cfg.dwMainCommand = HY_DVR_GET_COMPRESSCFG;
	hie_cfg.dwAssistCommand = COMPRESSCFG_ALL;


    if (!(*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg))
        hie_swap_encode_info((HY_DVR_COMPRESSION_CFG *)hie_cfg.sConfig, enc_info, SWAP_PACK, channel);

    return ret;
}
int hie_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret = 0;
	 JEncodeParameter *enc_info;
	 hie_service_basic_t *hie_basic;
	
	 HY_DVR_COMPRESSION_CFG    encode_info;
	 ConfigInformation hie_cfg;
	
	 NMP_ASSERT(srv && pvalue);
	
	 enc_info = (JEncodeParameter*)pvalue;
	 hie_basic = (hie_service_basic_t*)srv->tm;
	
	 memset(&encode_info, 0, sizeof(HY_DVR_COMPRESSION_CFG));
	 memset(&hie_cfg, 0, sizeof(ConfigInformation));
	
	 hie_cfg.dwConfigLen = sizeof(HY_DVR_COMPRESSION_CFG);
	 hie_cfg.dwMainCommand = HY_DVR_GET_COMPRESSCFG;
	 hie_cfg.dwAssistCommand = COMPRESSCFG_ALL;
	
	
	 if (!(*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg))
	 {
		hie_swap_encode_info((HY_DVR_COMPRESSION_CFG *)hie_cfg.sConfig, enc_info, SWAP_UNPACK, channel);
		hie_cfg.dwMainCommand = HY_DVR_SET_COMPRESSCFG;
		ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
	 }

    return ret;
}

int hie_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JDisplayParameter *dis_info;
    hie_service_basic_t *hie_basic;

    HB_NET_VIDEOEFFECT vid_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&vid_cfg, 0, sizeof(HB_NET_VIDEOEFFECT));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_VEFF;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &vid_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_VIDEOEFFECT);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_display_info(&vid_cfg.Default_VideoParam, dis_info, SWAP_PACK);
*/
    return ret;
}

int hie_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JDisplayParameter *dis_info;
    hie_service_basic_t *hie_basic;

    HB_NET_VIDEOEFFECT vid_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&vid_cfg, 0, sizeof(HB_NET_VIDEOEFFECT));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_VEFF;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &vid_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_VIDEOEFFECT);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_VEFF;
        hie_swap_display_info(&vid_cfg.Default_VideoParam, dis_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_PIC_CFG pic_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HY_DVR_PIC_CFG));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

	hie_cfg.dwConfigLen = sizeof(HY_DVR_PIC_CFG);
	hie_cfg.dwMainCommand = HY_DVR_GET_PICCFG;
	hie_cfg.dwAssistCommand = NETCFG_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_osd_info(&pic_cfg, osd_info, SWAP_PACK, channel);

    return ret;
}
int hie_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    hie_service_basic_t *hie_basic;
	
    HY_DVR_PIC_CFG pic_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HY_DVR_PIC_CFG));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

	hie_cfg.dwConfigLen = sizeof(HY_DVR_PIC_CFG);
	hie_cfg.dwMainCommand = HY_DVR_GET_PICCFG;
	hie_cfg.dwAssistCommand = NETCFG_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwMainCommand = HY_DVR_SET_PICCFG;
        hie_swap_osd_info((HY_DVR_PIC_CFG *)hie_cfg.sConfig, osd_info, SWAP_UNPACK, channel);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }

    return ret;
}

int hie_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JPTZParameter *ptz_info;
    hie_service_basic_t *hie_basic;

    HB_NET_DECODERCFG dec_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&dec_cfg, 0, sizeof(HB_NET_DECODERCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_DECODERCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &dec_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_DECODERCFG);
    dec_cfg.dwSize = sizeof(HB_NET_DECODERCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_decoder_info(&dec_cfg, ptz_info, SWAP_PACK);*/

    return ret;
}
int hie_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JPTZParameter *ptz_info;
    hie_service_basic_t *hie_basic;

    HB_NET_DECODERCFG dec_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&dec_cfg, 0, sizeof(HB_NET_DECODERCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_DECODERCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &dec_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_DECODERCFG);
    dec_cfg.dwSize = sizeof(HB_NET_DECODERCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_DECODERCFG;
        hie_swap_decoder_info(&dec_cfg, ptz_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    hie_service_basic_t *hie_basic;

    HY_DVR_RECORD_SCHED rec_cfg;
    ConfigInformation hie_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&rec_cfg, 0, sizeof(HY_DVR_RECORD_SCHED));
    memset(&hie_cfg, 0, sizeof(ConfigInformation));

	hie_cfg.dwConfigLen = sizeof(HY_DVR_RECORD_SCHED);
	hie_cfg.dwMainCommand = HY_DVR_GET_RECORDCFG;
	hie_cfg.dwAssistCommand = RECORDCFG_ALL;


    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_record_info((HY_DVR_RECORD_SCHED *)hie_cfg.sConfig, rec_info, SWAP_PACK, channel);

    return ret;
}
int hie_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JRecordParameter *rec_info;
    hie_service_basic_t *hie_basic;

    HB_NET_RECORDCFG rec_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&rec_cfg, 0, sizeof(HB_NET_RECORDCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_RECORDCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &rec_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_RECORDCFG);
    rec_cfg.dwSize = sizeof(HB_NET_RECORDCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_RECORDCFG;
        hie_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JHideParameter *hide_info;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &pic_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
    {
        hide_info->hide_enable = pic_cfg.byShelter;
        hie_swap_hide_info(pic_cfg.struShelter, hide_info, SWAP_PACK);
    }*/
    
    return ret;
}
int hie_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JHideParameter *hide_info;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &pic_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_PICCFG;
        pic_cfg.byShelter = hide_info->hide_enable;
        hie_swap_hide_info(pic_cfg.struShelter, hide_info, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JMoveAlarm *motion;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &pic_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_PACK);*/

    return ret;
}
int hie_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JMoveAlarm *motion;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &pic_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_PICCFG;
        hie_swap_move_alarm_info(&pic_cfg.struMotion, motion, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JLostAlarm *video_lost;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &pic_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_PACK);*/

    return ret;
}
int hie_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JLostAlarm *video_lost;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &pic_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_PICCFG;
        hie_swap_video_lost_info(&pic_cfg.struVILost, video_lost, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JHideAlarm *hide_alarm;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &pic_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
        hie_swap_hide_alarm_info(&pic_cfg.struHide, hide_alarm, SWAP_PACK);*/

    return ret;
}
int hie_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JHideAlarm *hide_alarm;
    hie_service_basic_t *hie_basic;

    HB_NET_PICCFG pic_cfg;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pic_cfg, 0, sizeof(HB_NET_PICCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize      = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwCommand   = HB_NET_GET_PICCFG;
    hie_cfg.dwChannel   = channel;
    hie_cfg.pInBuffer   = &pic_cfg;
    hie_cfg.dwInBufSize = sizeof(HB_NET_PICCFG);
    pic_cfg.dwSize = sizeof(HB_NET_PICCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id-1, &hie_cfg);
    if (!ret)
    {
        hie_cfg.dwCommand = HB_NET_SET_PICCFG;
        hie_swap_hide_alarm_info(&pic_cfg.struHide, hide_alarm, SWAP_UNPACK);
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_cfg);
    }*/

    return ret;
}

int hie_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret = 0;
    /*JIoAlarm *io_alarm;
    hie_service_basic_t *hie_basic;

    HB_NET_ALARMINCFG alarm_in;
    HB_NET_ALARMOUTCFG alarm_out;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&alarm_in, 0, sizeof(HB_NET_ALARMINCFG));
    memset(&alarm_out, 0, sizeof(HB_NET_ALARMOUTCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize    = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwChannel = channel;

    switch (type)
    {
        case ALARM_IN:
            hie_cfg.dwCommand    = HB_NET_GET_ALARMINCFG;
            hie_cfg.pOutBuffer   = &alarm_in;
            hie_cfg.dwOutBufSize = sizeof(HB_NET_ALARMINCFG);
            alarm_in.dwSize = sizeof(HB_NET_ALARMINCFG);

            ret = (*hie_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hie_cfg);
            if (!ret)
                hie_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            hie_cfg.dwCommand    = HB_NET_GET_ALARMOUTCFG;
            hie_cfg.pOutBuffer   = &alarm_out;
            hie_cfg.dwOutBufSize = sizeof(HB_NET_ALARMOUTCFG);
            alarm_out.dwSize = sizeof(HB_NET_ALARMOUTCFG);

            ret = (*hie_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hie_cfg);
            if (!ret)
                hie_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }*/

    return ret;
}
int hie_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret = 0;
    /*JIoAlarm *io_alarm;
    hie_service_basic_t *hie_basic;

    HB_NET_ALARMINCFG alarm_in;
    HB_NET_ALARMOUTCFG alarm_out;
    HB_NET_SETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&alarm_in, 0, sizeof(HB_NET_ALARMINCFG));
    memset(&alarm_out, 0, sizeof(HB_NET_ALARMOUTCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_SETDEVCONFIG));

    hie_cfg.dwSize    = sizeof(HB_NET_SETDEVCONFIG);
    hie_cfg.dwChannel = channel;

    switch (type)
    {
        case ALARM_IN:
            hie_cfg.dwCommand   = HB_NET_GET_ALARMINCFG;
            hie_cfg.pInBuffer   = &alarm_in;
            hie_cfg.dwInBufSize = sizeof(HB_NET_ALARMINCFG);
            alarm_in.dwSize = sizeof(HB_NET_ALARMINCFG);

            ret = (*hie_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hie_cfg);
            if (!ret)
            {
                hie_cfg.dwCommand = HB_NET_SET_ALARMINCFG;
                hie_swap_alarm_in_info(&alarm_in, io_alarm, SWAP_UNPACK);
                ret = (*hie_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hie_cfg);
            }
            break;
        case ALARM_OUT:
            hie_cfg.dwCommand   = HB_NET_GET_ALARMOUTCFG;
            hie_cfg.pInBuffer   = &alarm_out;
            hie_cfg.dwInBufSize = sizeof(HB_NET_ALARMOUTCFG);
            alarm_out.dwSize = sizeof(HB_NET_ALARMOUTCFG);

            ret = (*hie_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &hie_cfg);
            if (!ret)
            {
                hie_cfg.dwCommand = HB_NET_SET_ALARMOUTCFG;
                hie_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_UNPACK);
                ret = (*hie_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &hie_cfg);
            }
            break;

        default:
            ret = -1;
            break;
    }*/

    return ret;
}
int hie_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JStoreLog *store;
    get_store_t get_store;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    get_store.channel = channel+1;
    get_store.buffer  = store;
    get_store.b_size  = sizeof(JStoreLog);

    hie_swap_store_log_info(store, store, SWAP_UNPACK);
    ret = (*hie_basic->ro.get_config)(srv, parm_id, &get_store);
    if (!ret)
        hie_swap_store_log_info(store, store, SWAP_PACK);*/

    return ret;
}

int hie_ptz_control(struct service *srv, int channel, int parm_id, void *pvalue)
{
    //int i, flag = 0,
	int ret = 0;
    #if 0
    static int ptz_cmd = PTZ_STOP;
    int hie_cmd[] = {PAN_AUTO, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN, /*UP_LEFT, DOWN_LEFT, UP_RIGHT, DOWN_RIGHT,*/ 
                    ZOOM_IN, ZOOM_OUT, FOCUS_FAR, FOCUS_NEAR, LIGHT_PWRON, 0, WIPER_PWRON, 0,
                    SET_PRESET, GOTO_PRESET, CLE_PRESET};
    int jxnmp_cmd[] = {PTZ_AUTO, PTZ_LEFT, PTZ_RIGHT, PTZ_UP, PTZ_DOWN, /*PTZ_LEFT_UP, PTZ_LEFT_DOWN, PTZ_RIGHT_UP, PTZ_RIGHT_DOWN,*/ 
                    PTZ_ADD_ZOOM, PTZ_SUB_ZOOM, PTZ_ADD_FOCUS, PTZ_SUB_FOCUS, PTZ_TURN_ON, PTZ_TURN_OFF, PTZ_WIPERS_ON, PTZ_WIPERS_OFF,
                    PTZ_SET_PP, PTZ_USE_PP, PTZ_DEL_PP};
    
    JPTZControl *ptz_ctrl;
    hie_service_basic_t *hie_basic;

    HB_NET_PTZCTRL hie_ptz_ctrl;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&hie_ptz_ctrl, 0, sizeof(HB_NET_PTZCTRL));

    hie_ptz_ctrl.dwSize = sizeof(HB_NET_PTZCTRL);
    hie_ptz_ctrl.dwChannel = channel;
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
            hie_ptz_ctrl.dwPTZCmd = ALL_STOP;
        }
        else
        {
            hie_ptz_ctrl.dwPTZCmd = hie_cmd[i];
            ptz_cmd = hie_ptz_ctrl.dwPTZCmd;
        }
    }
    else if (PTZ_STOP == ptz_ctrl->action)
    {
        hie_ptz_ctrl.dwPTZCmd = ALL_STOP;
        ptz_cmd = PTZ_STOP;
    }
    else
    {
        for (i=0; i<(int)(sizeof(jxnmp_cmd)/sizeof(*jxnmp_cmd)); i++)
        {
            if (jxnmp_cmd[i] == (int)ptz_ctrl->action)
                break;
        }

        if (ptz_cmd == hie_cmd[i])
        {
            flag = 1;
        }
        else if (PTZ_TURN_OFF == jxnmp_cmd[i] || PTZ_WIPERS_OFF == jxnmp_cmd[i])
        {
            hie_ptz_ctrl.dwPTZCmd = ALL_STOP;
            ptz_cmd = PTZ_STOP;
        }
        else
        {
            hie_ptz_ctrl.dwPTZCmd = ALL_STOP;
            (*hie_basic->ro.set_config)(srv, parm_id, &hie_ptz_ctrl);

            hie_ptz_ctrl.dwPTZCmd = hie_cmd[i];
            ptz_cmd = hie_ptz_ctrl.dwPTZCmd;
        }

    }

    if (!flag)
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_ptz_ctrl);
    else
        ret = 0;
#endif
    return ret;
}
int hie_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JPPConfig *pp_cfg;
    hie_service_basic_t *hie_basic;

    HB_NET_PTZCTRL hie_ptz_ctrl;

    NMP_ASSERT(srv && pvalue);

    pp_cfg = (JPPConfig*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&hie_ptz_ctrl, 0, sizeof(HB_NET_PTZCTRL));

    hie_ptz_ctrl.dwSize = sizeof(HB_NET_PTZCTRL);
    hie_ptz_ctrl.dwChannel = channel;
    hie_ptz_ctrl.dwIndex = pp_cfg->pp.preset;
//    ptz_ctrl.dwSpeed = ;
    
    switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
            hie_ptz_ctrl.dwPTZCmd = SET_PRESET;
            break;
        case PTZ_USE_PP:
            hie_ptz_ctrl.dwPTZCmd = GOTO_PRESET;
            break;
        case PTZ_DEL_PP:
            hie_ptz_ctrl.dwPTZCmd = CLE_PRESET;
            break;
        default:
            ret = -1;
            break;
    }

    if (!ret)
        ret = (*hie_basic->ro.set_config)(srv, parm_id, &hie_ptz_ctrl);*/

    return ret;
}
int hie_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseWay *crz_way;
    hie_service_basic_t *hie_basic;

    HB_NET_PRESETPOLLCFG pp_cfg;
    HB_NET_GETDEVCONFIG hie_cfg;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

    memset(&pp_cfg, 0, sizeof(HB_NET_PRESETPOLLCFG));
    memset(&hie_cfg, 0, sizeof(HB_NET_GETDEVCONFIG));

    hie_cfg.dwSize       = sizeof(HB_NET_GETDEVCONFIG);
    hie_cfg.dwCommand    = HB_NET_GET_PRESETPOLL;
    hie_cfg.dwChannel    = channel;
    hie_cfg.pOutBuffer   = &pp_cfg;
    hie_cfg.dwOutBufSize = sizeof(HB_NET_PRESETPOLLCFG);

    ret = (*hie_basic->ro.get_config)(srv, parm_id, &hie_cfg);
    if (!ret)
    {
        sprintf((char*)crz_way->crz_info.crz_name, 
            "å·¡èˆªè·¯å¾„ %d", crz_way->crz_info.crz_no+1);
        hie_swap_cruise_way(&pp_cfg, crz_way, SWAP_PACK);
    }*/

    return ret;
}
int hie_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseConfig *crz_cfg;
    hie_service_basic_t *hie_basic;

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
int hie_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

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
int hie_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    /*JCruiseWay *crz_way;
    JCruisePoint *crz_point;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    crz_way = (JCruiseWay*)pvalue;
    hie_basic = (hie_service_basic_t*)srv->tm;

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
int hie_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(srv && pvalue);

    hie_basic = (hie_service_basic_t*)srv->tm;

    return (*hie_basic->ro.get_config)(srv, parm_id, pvalue);
}




