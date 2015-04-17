
#include "nmp_proxy_sdk.h"

#include "nmp_dah_swap.h"
#include "nmp_dah_service.h"
#include "nmp_dah_handler.h"


#define DEF_DAH_FACTORY_INFO        "http://www.dahuatech.com/"

#define DEF_DAH_ALL_CHANNEL         0xFFFFFFFF
#define DEF_DAH_GET_SUCCESSFUL      0x0
#define DEF_DAH_MAX_RECORD_FILE     168
#define DEF_DAH_WAIT_TIME           1000


int dah_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    NET_DEVICEINFO *dev_cfg;
    dah_service_t *dah_srv;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_COMM_CFG        serial_cfg;
    DHDEV_SYSTEM_ATTR_CFG sys_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    dah_srv  = (dah_service_t*)srv;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&serial_cfg, 0, sizeof(DHDEV_COMM_CFG));
    memset(&sys_cfg, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));

    dah_cfg.command  = DH_DEV_DEVICECFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &sys_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_SYSTEM_ATTR_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, (void*)&dah_cfg);
    if (!ret)
    {
        dev_cfg = &dah_srv->dah_info;
        strncpy((char*)dev_info->manu_info, DEF_DAH_FACTORY_INFO, 
            sizeof(dev_info->manu_info)-1);
        dah_swap_system_info(&sys_cfg, dev_info, SWAP_PACK);
        dah_swap_device_info(dev_cfg, dev_info, SWAP_PACK);
    }

    dah_cfg.command = DH_DEV_COMMCFG;
    dah_cfg.buffer  = &serial_cfg;
    dah_cfg.b_size  = sizeof(DHDEV_COMM_CFG);

    ret = (*dah_basic->ro.get_config)(srv, parm_id, (void*)&dah_cfg);
    if (!ret)
    {
        dev_info->rs232_num = serial_cfg.dw232FuncNameNum;
    }

    return ret;
}
int dah_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_COMM_CFG cmd_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&cmd_cfg, 0, sizeof(DHDEV_COMM_CFG));

    dah_cfg.command  = DH_DEV_COMMCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &cmd_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_COMM_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_serial_info(cmd_cfg.st232, serial_info, SWAP_PACK);
    }

    return ret;
}
int dah_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_COMM_CFG cmd_cfg;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&cmd_cfg, 0, sizeof(DHDEV_COMM_CFG));

    dah_cfg.command  = DH_DEV_COMMCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &cmd_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_COMM_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_serial_info(cmd_cfg.st232, serial_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    NET_TIME dah_time;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&dah_time, 0, sizeof(NET_TIME));

    dah_cfg.command  = DH_DEV_TIMECFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &dah_time;
    dah_cfg.b_size   = sizeof(NET_TIME);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_time_info(&dah_time, dev_time, SWAP_PACK);

    return ret;
}
int dah_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *dev_time;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    NET_TIME dah_time;

    NMP_ASSERT(srv && pvalue);

    dev_time = (JDeviceTime*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&dah_time, 0, sizeof(NET_TIME));
    
    dah_cfg.command  = DH_DEV_TIMECFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &dah_time;
    dah_cfg.b_size   = sizeof(NET_TIME);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_time_info(&dah_time, dev_time, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NTP_CFG ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&ntp_cfg, 0, sizeof(DHDEV_NTP_CFG));

    dah_cfg.command  = DH_DEV_NTP_CFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &ntp_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NTP_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_PACK);

    return ret;
}
int dah_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NTP_CFG ntp_cfg;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&ntp_cfg, 0, sizeof(DHDEV_NTP_CFG));

    dah_cfg.command  = DH_DEV_NTP_CFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &ntp_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NTP_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_ntp_info(&ntp_cfg, ntp_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_network_info(&net_cfg, net_info, SWAP_PACK);

    return ret;
}
int dah_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_network_info(&net_cfg, net_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_pppoe_info(&net_cfg.struPppoe, pppoe_info, SWAP_PACK);

    return ret;
}
int dah_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPPPOEInfo *pppoe_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    pppoe_info = (JPPPOEInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_pppoe_info(&net_cfg.struPppoe, pppoe_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_FTP_PROTO_CFG ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&ftp_cfg, 0, sizeof(DHDEV_FTP_PROTO_CFG));

    dah_cfg.command  = DH_DEV_FTP_PROTO_CFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &ftp_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_FTP_PROTO_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);

    return ret;
}
int dah_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JFTPParameter *ftp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_FTP_PROTO_CFG ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&ftp_cfg, 0, sizeof(DHDEV_FTP_PROTO_CFG));

    dah_cfg.command  = DH_DEV_FTP_PROTO_CFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &ftp_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_FTP_PROTO_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_smtp_info(&net_cfg.struMail, smtp_info, SWAP_PACK);

    return ret;
}
int dah_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSMTPParameter *smtp_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_NET_CFG net_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&net_cfg, 0, sizeof(DHDEV_NET_CFG));

    dah_cfg.command  = DH_DEV_NETCFG;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &net_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_NET_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_smtp_info(&net_cfg.struMail, smtp_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_MULTI_DDNS_CFG multi_ddns;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&multi_ddns, 0, sizeof(DHDEV_MULTI_DDNS_CFG));

    dah_cfg.command  = DH_DEV_MULTI_DDNS;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &multi_ddns;
    dah_cfg.b_size   = sizeof(DHDEV_MULTI_DDNS_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_ddns_info(multi_ddns.struDdnsServer, ddns_info, SWAP_PACK);

    return ret;
}
int dah_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_MULTI_DDNS_CFG multi_ddns;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&multi_ddns, 0, sizeof(DHDEV_MULTI_DDNS_CFG));
    
    dah_cfg.command  = DH_DEV_MULTI_DDNS;
    dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
    dah_cfg.buffer   = &multi_ddns;
    dah_cfg.b_size   = sizeof(DHDEV_MULTI_DDNS_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_ddns_info(multi_ddns.struDdnsServer, ddns_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    return ret;
}
int dah_set_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JUPNPParameter *upnp_info;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(srv && pvalue);

    upnp_info = (JUPNPParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    return ret;
}
int dah_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceDiskInfo *disk_list;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DH_HARDDISK_STATE dh_disk;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&dh_disk, 0, sizeof(DH_HARDDISK_STATE));

    dah_cfg.command  = DH_DEVSTATE_DISK;
    dah_cfg.buffer   = &dh_disk;
    dah_cfg.b_size   = sizeof(DH_HARDDISK_STATE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_disk_list(&dh_disk, disk_list, SWAP_PACK);

    return ret;
}
int dah_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int i, ret;
    JFormatDisk *fmt_disk;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DISKCTRL_PARAM dh_format;
    DH_HARDDISK_STATE dh_disk;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&dh_format, 0, sizeof(DISKCTRL_PARAM));
    memset(&dh_disk, 0, sizeof(DH_HARDDISK_STATE));

    dah_cfg.command  = DH_DEVSTATE_DISK;
    dah_cfg.buffer   = &dh_disk;
    dah_cfg.b_size   = sizeof(DH_HARDDISK_STATE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        for (i=0; i<(int)dh_disk.dwDiskNum; i++)
        {
            if (dh_disk.stDisks[i].bDiskNum == (int)fmt_disk->disk_no)
            {
                memcpy(&dh_format.stuDisk, &dh_disk.stDisks[i], sizeof(NET_DEV_DISKSTATE));
                break;
            }
        }

        dah_cfg.command  = DH_CTRL_DISK;
        dah_cfg.channel  = DEF_DAH_ALL_CHANNEL;
        dah_cfg.buffer   = &dh_format;
        dah_cfg.b_size   = sizeof(DISKCTRL_PARAM);
        dah_cfg.waittime = DEF_DAH_WAIT_TIME;

        dah_swap_format_info(&dh_format, fmt_disk, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(srv && pvalue);

    dah_basic = (dah_service_basic_t*)srv->tm;

    return (*dah_basic->ro.set_config)(srv, parm_id, pvalue);
}
int dah_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_SYSTEM_ATTR_CFG sys_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&sys_cfg, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_encode_info(chn_cfg.stMainVideoEncOpt, enc_info, SWAP_PACK);

    dah_cfg.command  = DH_DEV_DEVICECFG;
    dah_cfg.buffer   = &sys_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_SYSTEM_ATTR_CFG);

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        enc_info->format = sys_cfg.byVideoStandard;

    return ret;
}
int dah_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_SYSTEM_ATTR_CFG sys_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;
    
    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&sys_cfg, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_encode_info(chn_cfg.stMainVideoEncOpt, enc_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    dah_cfg.command  = DH_DEV_DEVICECFG;
    dah_cfg.buffer   = &sys_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_SYSTEM_ATTR_CFG);

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        sys_cfg.byVideoStandard = enc_info->format;
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_display_info(chn_cfg.stColorCfg, dis_info, SWAP_PACK);

    return ret;
}
int dah_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_display_info(chn_cfg.stColorCfg, dis_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_osd_info(&chn_cfg, osd_info, SWAP_PACK);

    return ret;
}
int dah_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_CHANNEL_CFG chn_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&chn_cfg, 0, sizeof(DHDEV_CHANNEL_CFG));

    dah_cfg.command  = DH_DEV_CHANNELCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &chn_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_CHANNEL_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_osd_info(&chn_cfg, osd_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    dah_service_basic_t *dah_basic;

    dah_new_config_t dah_cfg;
    CFG_PTZ_INFO ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(CFG_PTZ_INFO));
    memset(&dah_cfg, 0, sizeof(dah_new_config_t));

    dah_cfg.command  = (char*)CFG_CMD_PTZ;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = (char*)&ptz_cfg;
    dah_cfg.b_size   = sizeof(CFG_PTZ_INFO);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_PACK);
    }

    return ret;
}
int dah_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    dah_service_basic_t *dah_basic;

    dah_new_config_t dah_cfg;
    CFG_PTZ_INFO ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&ptz_cfg, 0, sizeof(CFG_PTZ_INFO));
    memset(&dah_cfg, 0, sizeof(dah_new_config_t));

    dah_cfg.command  = (char*)CFG_CMD_PTZ;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = (char*)&ptz_cfg;
    dah_cfg.b_size   = sizeof(CFG_PTZ_INFO);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_RECORD_CFG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&rec_cfg, 0, sizeof(DHDEV_RECORD_CFG));

    dah_cfg.command  = DH_DEV_RECORDCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &rec_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_RECORD_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_record_info(&rec_cfg, rec_info, SWAP_PACK);
    }

    return ret;
}
int dah_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JRecordParameter *rec_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_RECORD_CFG rec_cfg;

    NMP_ASSERT(srv && pvalue);

    rec_info = (JRecordParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&rec_cfg, 0, sizeof(DHDEV_RECORD_CFG));

    dah_cfg.command  = DH_DEV_RECORDCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &rec_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_RECORD_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_record_info(&rec_cfg, rec_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }
    
    return ret;
}
int dah_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_VIDEOCOVER_CFG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&vc_cfg, 0, sizeof(DHDEV_VIDEOCOVER_CFG));

    dah_cfg.command  = DH_DEV_VIDEO_COVER;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &vc_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_VIDEOCOVER_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_hide_info(&vc_cfg, hide_info, SWAP_PACK);

    return ret;
}
int dah_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideParameter *hide_info;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_VIDEOCOVER_CFG vc_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_info = (JHideParameter*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&vc_cfg, 0, sizeof(DHDEV_VIDEOCOVER_CFG));

    dah_cfg.command  = DH_DEV_VIDEO_COVER;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &vc_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_VIDEOCOVER_CFG);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_hide_info(&vc_cfg, hide_info, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_move_alarm_info(alarm_cfg.struMotion, motion, SWAP_PACK);
    }

    return ret;
}
int dah_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_move_alarm_info(alarm_cfg.struMotion, motion, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_video_lost_info(alarm_cfg.struVideoLost, video_lost, SWAP_PACK);
    }

    return ret;
}
int dah_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JLostAlarm *video_lost;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    video_lost = (JLostAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_video_lost_info(alarm_cfg.struVideoLost, video_lost, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
    {
        dah_swap_hide_alarm_info(alarm_cfg.struBlind, hide_alarm, SWAP_PACK);
    }

    return ret;
}
int dah_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JHideAlarm *hide_alarm;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    hide_alarm = (JHideAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    ret = (*dah_basic->ro.get_config)(srv, parm_id-1, &dah_cfg);
    if (!ret)
    {
        dah_swap_hide_alarm_info(alarm_cfg.struBlind, hide_alarm, SWAP_UNPACK);
        ret = (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
    }

    return ret;
}
int dah_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.channel  = channel;
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            ret = (*dah_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &dah_cfg);
            if (!ret)
                dah_swap_alarm_in_info(alarm_cfg.struLocalAlmIn, io_alarm, SWAP_PACK);
            break;
        case ALARM_OUT:
            ret = (*dah_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &dah_cfg);
            if (!ret)
                ;//dah_swap_alarm_out_info(&alarm_out, io_alarm, SWAP_PACK);
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int dah_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
    int ret;
    JIoAlarm *io_alarm;
    dah_service_basic_t *dah_basic;

    dah_config_t dah_cfg;
    DHDEV_ALARM_SCHEDULE alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    io_alarm = (JIoAlarm*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_config_t));
    memset(&alarm_cfg, 0, sizeof(DHDEV_ALARM_SCHEDULE));

    dah_cfg.command  = DH_DEV_ALARMCFG;
    dah_cfg.channel  = channel;
    dah_cfg.buffer   = &alarm_cfg;
    dah_cfg.b_size   = sizeof(DHDEV_ALARM_SCHEDULE);
    dah_cfg.waittime = DEF_DAH_WAIT_TIME;

    switch (type)
    {
        case ALARM_IN:
            ret = (*dah_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &dah_cfg);
            if (!ret)
            {
                dah_swap_alarm_in_info(alarm_cfg.struLocalAlmIn, io_alarm, SWAP_UNPACK);
                ret = (*dah_basic->ro.set_config)(srv, SET_IO_ALARM_CONFIG, &dah_cfg);
            }
            break;
        case ALARM_OUT:
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
int dah_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JStoreLog *store;
    dah_service_basic_t *dah_basic;

    dah_query_t dah_cfg;
    NET_RECORDFILE_INFO rec_file[DEF_DAH_MAX_RECORD_FILE];

    NMP_ASSERT(srv && pvalue);

    store = (JStoreLog*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_query_t));
    memset(rec_file, 0, sizeof(NET_RECORDFILE_INFO)*DEF_DAH_MAX_RECORD_FILE);

    dah_cfg.channel   = channel;
    dah_cfg.file_type = 0;  //所有录像文件
    dah_cfg.card_id   = NULL;
    dah_cfg.buffer    = (char*)rec_file;
    dah_cfg.buf_size  = sizeof(NET_RECORDFILE_INFO)*DEF_DAH_MAX_RECORD_FILE;
    dah_cfg.waittime  = DEF_DAH_WAIT_TIME * 2;

    dah_swap_store_log_info(&dah_cfg, store, SWAP_UNPACK);
    ret = (*dah_basic->ro.get_config)(srv, parm_id, &dah_cfg);
    if (!ret)
        dah_swap_store_log_info(&dah_cfg, store, SWAP_PACK);

    return ret;
}
int dah_ptz_control(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JPTZControl *ptz_ctrl;
    dah_service_basic_t *dah_basic;

    dah_ptz_ctrl_t dah_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    dah_basic = (dah_service_basic_t*)srv->tm;

    memset(&dah_cfg, 0, sizeof(dah_ptz_ctrl_t));

    dah_swap_ptz_control(&dah_cfg, ptz_ctrl, SWAP_UNPACK);

    return (*dah_basic->ro.set_config)(srv, parm_id, &dah_cfg);
}
int dah_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(srv && pvalue);

    dah_basic = (dah_service_basic_t*)srv->tm;

    return (*dah_basic->ro.get_config)(srv, parm_id, pvalue);
}


