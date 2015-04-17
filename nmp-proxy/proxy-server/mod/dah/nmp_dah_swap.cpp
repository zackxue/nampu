
#include "nmp_dah_swap.h"

enum
{
    PACK_SOFTWARE,      //软件版本
    PACK_HARDEARE,      //硬件版本
    PACK_BUILDDATE,     //生成日期
    
    UNPACK_SOFTWARE,
    UNPACK_HARDEARE,
    UNPACK_BUILDDATE,
};

static __inline__ void 
dah_swap_time(NET_TIME *d_time, JTime *nmp_time, int flag)
{
    NMP_ASSERT(d_time && nmp_time);

    switch(flag)
    {
        case SWAP_PACK:
            nmp_time->year    = d_time->dwYear - 1900;
            nmp_time->month   = d_time->dwMonth;
            nmp_time->date    = d_time->dwDay;
            nmp_time->hour    = d_time->dwHour;
            nmp_time->minute  = d_time->dwMinute;
            nmp_time->second  = d_time->dwSecond;
            break;

        case SWAP_UNPACK:
            d_time->dwYear   = nmp_time->year + 1900;
            d_time->dwMonth  = nmp_time->month;
            d_time->dwDay    = nmp_time->date;
            d_time->dwHour   = nmp_time->hour;
            d_time->dwMinute = nmp_time->minute;
            d_time->dwSecond = nmp_time->second;
            break;

        default:
            break;
    }
}


static __inline__ void 
dah_process_version(char *nmp_version, size_t size, int *h_version, int flag)
{
    int high, low;
    char *ptr;
    char tmp_version[J_SDK_MAX_VERSION_LEN];
    
    switch (flag)
    {
        case PACK_SOFTWARE:
            high = (*h_version & 0xffff0000) >> 16;
            low = *h_version & 0x0000ffff;
            
            snprintf((char*)nmp_version, size,
                "%d.%d", high, low);
            break;
            
        case PACK_HARDEARE:         
            high = (*h_version & 0xffff0000) >> 16;
            low = *h_version & 0x0000ffff;
            
            snprintf((char*)nmp_version, size,
                "%d.%d", high, low);
            break;
            
        case UNPACK_SOFTWARE:
            strncpy(tmp_version, nmp_version, sizeof(tmp_version)-1);
            tmp_version[sizeof(tmp_version)-1] = '\0';

            ptr = strtok(tmp_version, ".");
            if (ptr)
            {
                sscanf(ptr, "%d", &high);
                ptr = strtok(NULL, ".");
                if (ptr)
                {
                    sscanf(ptr, "%d", &low);
                }
            }

            *h_version = high << 16 | low;
            break;
            
        case UNPACK_HARDEARE:
            strncpy(tmp_version, nmp_version, sizeof(tmp_version)-1);
            tmp_version[sizeof(tmp_version)-1] = '\0';

            ptr = strtok(tmp_version, ".");
            if (ptr)
            {
                sscanf(ptr, "%d", &high);
                ptr = strtok(NULL, ".");
                if (ptr)
                {
                    sscanf(ptr, "%d", &low);
                }
            }

            *h_version = high << 16 | low;
            break;

        default:
            break;
    }
    
    return ;
}

static __inline__ void 
dah_process_build_date(char *nmp_date, size_t size, int *h_date, int flag)
{
    int year, month, day;
    char *ptr;
    char tmp_version[J_SDK_MAX_VERSION_LEN];

    switch (flag)
    {
        case PACK_BUILDDATE:
            year = (*h_date & 0xffff0000) >> 16;
            month = (*h_date & 0x0000ff00) >> 8;
            day = *h_date & 0x000000ff;
            snprintf((char*)nmp_date, size, "%d.%d.%d", year, month, day);
            break;

        case UNPACK_BUILDDATE:
            strncpy(tmp_version, nmp_date, sizeof(tmp_version)-1);
            tmp_version[sizeof(tmp_version)-1] = '\0';

            ptr = strtok(tmp_version, ".");
            if (ptr)
            {
                sscanf(ptr, "%d", &year);
                ptr = strtok(NULL, ".");
                if (ptr)
                {
                    sscanf(ptr, "%d", &month);
                    ptr = strtok(NULL, ".");
                    if (ptr)
                    {
                        sscanf(ptr, "%d", &day);
                    }
                }
            }

            *h_date = year << 16 | month << 8 | day;
            break;

        default:
            break;
    }
    
    return ;
}

static __inline__ void
dah_process_sched_time(DH_TSECT (*dah_sched_time)[DH_N_REC_TSECT], 
        JWeek *week, int flag)
{
    int day_index, time_seg_index;
    JTime *time_start, *time_end;
    DH_TSECT *dah_alarm_time;

    switch (flag)
    {
        case SWAP_PACK:         
            for (day_index=0; day_index<DH_N_WEEKS; day_index++)
            {
            printf("day: %d\n", day_index);
                for (time_seg_index=0; time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/; 
                    time_seg_index++)
                {
                    week->days[day_index].seg[time_seg_index].enable = 1;

                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);

                    dah_alarm_time = &dah_sched_time[day_index][time_seg_index];

                    if (dah_alarm_time)
                    {
                        time_start->hour   = (int)dah_alarm_time->iBeginHour;
                        time_start->minute = (int)dah_alarm_time->iBeginMin;
                        time_start->second = (int)dah_alarm_time->iBeginSec;

                        time_end->hour   = (int)dah_alarm_time->iEndHour;
                        time_end->minute = (int)dah_alarm_time->iEndMin;
                        time_end->second = (int)dah_alarm_time->iEndSec;
                        
                        printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                            dah_alarm_time->iBeginHour, dah_alarm_time->iBeginMin, 
                            dah_alarm_time->iEndHour, dah_alarm_time->iEndMin);
                        printf("time_seg: %02d:%02d - %02d:%02d\n", 
                            time_start->hour, time_start->minute, 
                            time_end->hour, time_end->minute);
                    }
                    else
                        break;
                }
                week->days[day_index].day_id = day_index;
                week->days[day_index].count = time_seg_index;
            }
            week->count = day_index;
            break;

        case SWAP_UNPACK:
            for (day_index=0; day_index<DH_N_WEEKS; day_index++)
            {
            printf("UNPACK--->day: %d\n", day_index);
                int seg = week->days[day_index].count;
                for (time_seg_index=0; (time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/ && 0 < seg--); 
                    time_seg_index++)
                {
                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);

                    dah_alarm_time = &dah_sched_time[day_index][time_seg_index];

                    dah_alarm_time->iBeginHour = (BYTE)time_start->hour;
                    dah_alarm_time->iBeginMin  = (BYTE)time_start->minute;
                    dah_alarm_time->iBeginSec  = (BYTE)time_start->minute;

                    dah_alarm_time->iEndHour  = (BYTE)time_end->hour;
                    dah_alarm_time->iEndMin   = (BYTE)time_end->minute;
                    dah_alarm_time->iEndSec   = (BYTE)time_end->minute;

                    printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                        dah_alarm_time->iBeginHour, dah_alarm_time->iBeginMin, 
                        dah_alarm_time->iEndHour, dah_alarm_time->iEndMin);
                    printf("time_seg: %02d:%02d - %02d:%02d\n", 
                        time_start->hour, time_start->minute, 
                        time_end->hour, time_end->minute);
                }
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_system_info(DHDEV_SYSTEM_ATTR_CFG *sys_cfg, JDeviceInfo *dev_info, int flag)
{
    DH_VERSION_INFO *dah_ver_info;
    NMP_ASSERT(sys_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            dah_ver_info = &sys_cfg->stVersion;
            dah_process_version((char*)dev_info->dev_version, 
                sizeof(dev_info->dev_version),
                (int*)&dah_ver_info->dwSoftwareVersion, PACK_SOFTWARE);

            dah_process_version((char*)dev_info->hw_version, 
                sizeof(dev_info->hw_version),
                (int*)&dah_ver_info->dwHardwareVersion, PACK_HARDEARE);
            
            dah_process_build_date((char*)dev_info->release_date, 
                sizeof(dev_info->release_date), 
                (int*)&dah_ver_info->dwSoftwareBuildDate, PACK_BUILDDATE);
            break;

        case SWAP_UNPACK:
            dah_ver_info = &sys_cfg->stVersion;
            dah_process_version((char*)dev_info->dev_version, 
                sizeof(dev_info->dev_version),
                (int*)&dah_ver_info->dwSoftwareVersion, UNPACK_SOFTWARE);

            dah_process_version((char*)dev_info->hw_version, 
                sizeof(dev_info->hw_version),
                (int*)&dah_ver_info->dwHardwareVersion, UNPACK_HARDEARE);

            dah_process_build_date((char*)dev_info->release_date, 
                sizeof(dev_info->release_date), 
                (int*)&dah_ver_info->dwSoftwareBuildDate, UNPACK_BUILDDATE);
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_device_info(NET_DEVICEINFO *dev_cfg, JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            dev_info->pu_type     = (JPuType)dev_cfg->byDVRType;
            dev_info->di_num      = (int)dev_cfg->byAlarmInPortNum;
            dev_info->do_num      = (int)dev_cfg->byAlarmOutPortNum;
            dev_info->channel_num = (int)dev_cfg->byChanNum;
            break;

        case SWAP_UNPACK:
            dev_cfg->byDVRType         = dev_info->pu_type;
            dev_cfg->byAlarmInPortNum  = dev_info->di_num;
            dev_cfg->byAlarmOutPortNum = dev_info->do_num;
            dev_cfg->byChanNum         = dev_info->channel_num;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_serial_info(DH_RS232_CFG *rs232_cfg, JSerialParameter *serial_info, int flag)
{
    int i, j;
    int baud_rate[10] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    NMP_ASSERT(rs232_cfg && serial_info);

    switch (flag)
    {
        case SWAP_PACK:
            serial_info->baud_rate = baud_rate[rs232_cfg->struComm.byBaudRate];
            serial_info->data_bit  = rs232_cfg->struComm.byDataBit + 5;
            if (0 == rs232_cfg->struComm.byStopBit)
                serial_info->stop_bit = 1;
            else
                serial_info->stop_bit = 2;
            serial_info->verify    = rs232_cfg->struComm.byParity;
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_MAX_232_NUM; i++)
            {
                for (j=0; j<10; j++)
                {
                    if ((int)serial_info->baud_rate <= baud_rate[j])
                    {
                        rs232_cfg[i].struComm.byBaudRate = j;
                        break;
                    }
                }
                rs232_cfg[i].struComm.byDataBit  = serial_info->data_bit - 5;
                rs232_cfg[i].struComm.byStopBit  = serial_info->stop_bit;
                rs232_cfg[i].struComm.byParity   = serial_info->verify;
            }
            break;

        default:
            break;
    }

    printf("byBaudRate: %02d   |  ", rs232_cfg->struComm.byBaudRate);
    printf("baud_rate    : %d\n", serial_info->baud_rate);
    printf("byDataBit : %02d   |  ", rs232_cfg->struComm.byDataBit);
    printf("data_bit     : %d\n", serial_info->data_bit);
    printf("byStopBit : %02d   |  ", rs232_cfg->struComm.byStopBit);
    printf("stop_bit     : %d\n", serial_info->stop_bit);
    printf("byParity  : %02d   |  ", rs232_cfg->struComm.byParity);
    printf("verify       : %d\n", serial_info->verify);

    return ;
}

void dah_swap_time_info(NET_TIME *dah_time, JDeviceTime *dev_time, int flag)
{
    NMP_ASSERT(dah_time && dev_time);

    switch (flag)
    {
        case SWAP_PACK:
            dev_time->time.year   = dah_time->dwYear;
            dev_time->time.month  = dah_time->dwMonth;
            dev_time->time.date   = dah_time->dwDay;
            dev_time->time.hour   = dah_time->dwHour;
            dev_time->time.minute = dah_time->dwMinute;
            dev_time->time.second = dah_time->dwSecond;
            break;

        case SWAP_UNPACK:
            dah_time->dwYear   = dev_time->time.year;
            dah_time->dwMonth  = dev_time->time.month;
            dah_time->dwDay    = dev_time->time.date;
            dah_time->dwHour   = dev_time->time.hour;
            dah_time->dwMinute = dev_time->time.minute;
            dah_time->dwSecond = dev_time->time.second;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_network_info(DHDEV_NET_CFG *net_cfg, JNetworkInfo *net_info, int flag)
{
    NMP_ASSERT(net_cfg && net_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)net_info->network[J_SDK_ETH0].ip, 
                net_cfg->stEtherNet[0].sDevIPAddr, 
                sizeof(net_info->network[J_SDK_ETH0].ip)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].netmask, 
                net_cfg->stEtherNet[0].sDevIPMask, 
                sizeof(net_info->network[J_SDK_ETH0].netmask)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].gateway, 
                net_cfg->stEtherNet[0].sGatewayIP, 
                sizeof(net_info->network[J_SDK_ETH0].gateway)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].mac, 
                net_cfg->stEtherNet[0].byMACAddr, 
                sizeof(net_info->network[J_SDK_ETH0].mac)-1);

            net_info->network[J_SDK_ETH0].type = J_SDK_ETH0;
            net_info->network[J_SDK_ETH0].dhcp_enable = 
                (int)((net_cfg->stEtherNet[0].bValid & 0x000000f0) >> 1);

            net_info->cmd_port = net_cfg->wTcpPort;
            net_info->web_port = net_cfg->wHttpPort;
            break;

        case SWAP_UNPACK:
            strncpy((char*)net_cfg->stEtherNet[0].sDevIPAddr, 
                (const char*)net_info->network[J_SDK_ETH0].ip, 
                sizeof(net_cfg->stEtherNet[0].sDevIPAddr)-1);
            strncpy((char*)net_cfg->stEtherNet[0].sDevIPMask, 
                (const char*)net_info->network[J_SDK_ETH0].netmask, 
                sizeof(net_cfg->stEtherNet[0].sDevIPMask)-1);
            strncpy((char*)net_cfg->stEtherNet[0].sGatewayIP, 
                (const char*)net_info->network[J_SDK_ETH0].gateway, 
                sizeof(net_cfg->stEtherNet[0].sGatewayIP)-1);
            strncpy((char*)net_cfg->stEtherNet[0].byMACAddr, 
                (const char*)net_info->network[J_SDK_ETH0].mac, 
                sizeof(net_cfg->stEtherNet[0].byMACAddr)-1);

            /*strncpy((char*)net_cfg->struDnsServer1IpAddr.sIpV4, 
                (const char*)net_info->main_dns, 
                sizeof(net_cfg->struDnsServer1IpAddr.sIpV4)-1);
            strncpy((char*)net_cfg->struDnsServer2IpAddr.sIpV4, 
                (const char*)net_info->backup_dns, 
                sizeof(net_cfg->struDnsServer2IpAddr.sIpV4)-1);*/

            net_cfg->stEtherNet[0].bValid = net_info->network[J_SDK_ETH0].dhcp_enable;

            net_cfg->wTcpPort  = net_info->cmd_port;
            net_cfg->wHttpPort = net_info->web_port;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_pppoe_info(DH_REMOTE_HOST *pppoe_cfg, JPPPOEInfo *pppoe_info, int flag)
{
    NMP_ASSERT(pppoe_cfg && pppoe_info);

    switch (flag)
    {
        case SWAP_PACK:
            pppoe_info->enable = pppoe_cfg->byEnable;
            pppoe_info->type   = pppoe_cfg->byAssistant;
            strncpy((char*)pppoe_info->ip, (const char*)pppoe_cfg->sHostIPAddr, 
                    sizeof(pppoe_info->ip)-1);
            strncpy((char*)pppoe_info->account, (const char*)pppoe_cfg->sHostUser, 
                    sizeof(pppoe_info->account)-1);
            strncpy((char*)pppoe_info->passwd, (const char*)pppoe_cfg->sHostPassword, 
                    sizeof(pppoe_info->passwd)-1);
            break;

        case SWAP_UNPACK:
            pppoe_cfg->byEnable    = pppoe_info->enable;
            if (0 == pppoe_info->type)
                pppoe_cfg->byAssistant = 0;
            else
                pppoe_cfg->byAssistant = 1;
            strncpy((char*)pppoe_cfg->sHostIPAddr, (const char*)pppoe_info->ip, 
                    sizeof(pppoe_cfg->sHostIPAddr)-1);
            strncpy((char*)pppoe_cfg->sHostUser, (const char*)pppoe_info->account, 
                    sizeof(pppoe_cfg->sHostUser)-1);
            strncpy((char*)pppoe_cfg->sHostPassword, (const char*)pppoe_info->passwd, 
                    sizeof(pppoe_cfg->sHostPassword)-1);
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_ntp_info(DHDEV_NTP_CFG *ntp_cfg, JDeviceNTPInfo *ntp_info, int flag)
{
    NMP_ASSERT(ntp_cfg && ntp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)ntp_info->ntp_server_ip, (const char*)ntp_cfg->szHostIp, 
                    sizeof(ntp_info->ntp_server_ip)-1);
            ntp_info->time_interval = ntp_cfg->nUpdateInterval;
            ntp_info->ntp_enable    = ntp_cfg->bEnable;
            ntp_info->time_zone     = ntp_cfg->nTimeZone;
            break;

        case SWAP_UNPACK:
            strncpy((char*)ntp_cfg->szHostIp, (const char*)ntp_info->ntp_server_ip, 
                    sizeof(ntp_cfg->szHostIp)-1);
            ntp_cfg->nUpdateInterval = ntp_info->time_interval;
            ntp_cfg->bEnable         = ntp_info->ntp_enable;
            ntp_cfg->nTimeZone       = ntp_info->time_zone;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_ftp_info(DHDEV_FTP_PROTO_CFG *ftp_cfg, JFTPParameter *ftp_info, int flag)
{
    NMP_ASSERT(ftp_cfg && ftp_info);

    switch (flag)
    {
        case SWAP_PACK:
            ftp_info->ftp_port = ftp_cfg->wHostPort;
            strncpy((char*)ftp_info->ftp_ip, (const char*)ftp_cfg->szHostIp, 
                    sizeof(ftp_info->ftp_ip)-1);
            strncpy((char*)ftp_info->ftp_usr, (const char*)ftp_cfg->szUserName, 
                    sizeof(ftp_info->ftp_usr)-1);
            strncpy((char*)ftp_info->ftp_pwd, (const char*)ftp_cfg->szPassword, 
                    sizeof(ftp_info->ftp_pwd)-1);
            strncpy((char*)ftp_info->ftp_path, (const char*)ftp_cfg->szDirName, 
                    sizeof(ftp_info->ftp_path)-1);
            break;

        case SWAP_UNPACK:
            ftp_cfg->wHostPort = ftp_info->ftp_port;
            strncpy((char*)ftp_cfg->szHostIp, (const char*)ftp_info->ftp_ip, 
                    sizeof(ftp_cfg->szHostIp)-1);
            strncpy((char*)ftp_cfg->szUserName, (const char*)ftp_info->ftp_usr, 
                    sizeof(ftp_cfg->szUserName)-1);
            strncpy((char*)ftp_cfg->szPassword, (const char*)ftp_info->ftp_pwd, 
                    sizeof(ftp_cfg->szPassword)-1);
            strncpy((char*)ftp_cfg->szDirName, (const char*)ftp_info->ftp_path, 
                    sizeof(ftp_cfg->szDirName)-1);
            break;

        default:
            break;
    }
    
    return ;
}

void dah_swap_smtp_info(DH_MAIL_CFG *email_cfg, JSMTPParameter *smtp_info, int flag)
{
    NMP_ASSERT(email_cfg && smtp_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)smtp_info->mail_ip, (const char*)email_cfg->sMailIPAddr, 
                    sizeof(smtp_info->mail_ip)-1);
            strncpy((char*)smtp_info->mail_addr, (const char*)email_cfg->sSenderAddr, 
                    sizeof(smtp_info->mail_addr)-1);
            strncpy((char*)smtp_info->mail_usr, (const char*)email_cfg->sUserName, 
                    sizeof(smtp_info->mail_usr)-1);
            strncpy((char*)smtp_info->mail_pwd, (const char*)email_cfg->sUserPsw, 
                    sizeof(smtp_info->mail_pwd)-1);
            strncpy((char*)smtp_info->mail_rctp1, (const char*)email_cfg->sDestAddr, 
                    sizeof(smtp_info->mail_rctp1)-1);
            strncpy((char*)smtp_info->mail_rctp2, (const char*)email_cfg->sCcAddr, 
                    sizeof(smtp_info->mail_rctp2)-1);
            strncpy((char*)smtp_info->mail_rctp3, (const char*)email_cfg->sBccAddr, 
                    sizeof(smtp_info->mail_rctp3)-1);
            smtp_info->mail_port  = email_cfg->wMailPort;
            //smtp_info->ssl_enable = email_cfg->byEnableSSL;
            break;

        case SWAP_UNPACK:
            strncpy((char*)email_cfg->sMailIPAddr, (const char*)smtp_info->mail_ip, 
                    sizeof(email_cfg->sMailIPAddr)-1);
            strncpy((char*)email_cfg->sSenderAddr, (const char*)smtp_info->mail_addr, 
                    sizeof(email_cfg->sSenderAddr)-1);
            strncpy((char*)email_cfg->sUserName, (const char*)smtp_info->mail_usr, 
                    sizeof(email_cfg->sUserName)-1);
            strncpy((char*)email_cfg->sUserPsw, (const char*)smtp_info->mail_pwd, 
                    sizeof(email_cfg->sUserPsw)-1);
            strncpy((char*)email_cfg->sDestAddr, (const char*)smtp_info->mail_rctp1, 
                    sizeof(email_cfg->sDestAddr)-1);
            strncpy((char*)email_cfg->sCcAddr, (const char*)smtp_info->mail_rctp2, 
                    sizeof(email_cfg->sCcAddr)-1);
            strncpy((char*)email_cfg->sBccAddr, (const char*)smtp_info->mail_rctp3, 
                    sizeof(email_cfg->sBccAddr)-1);
            email_cfg->wMailPort = smtp_info->mail_port;
            //email_cfg->byEnableSSL = smtp_info->ssl_enable;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_ddns_info(DH_DDNS_SERVER_CFG *ddns_cfg, JDdnsConfig *ddns_info, int flag)
{
    NMP_ASSERT(ddns_cfg && ddns_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)ddns_info->ddns_account, (const char*)ddns_cfg->szDomainName, 
                    sizeof(ddns_info->ddns_account)-1);
            strncpy((char*)ddns_info->ddns_usr, (const char*)ddns_cfg->szUserName, 
                    sizeof(ddns_info->ddns_usr)-1);
            strncpy((char*)ddns_info->ddns_pwd, (const char*)ddns_cfg->szUserPsw, 
                    sizeof(ddns_info->ddns_pwd)-1);
            ddns_info->ddns_open  = ddns_cfg->bEnable;
            //ddns_info->ddns_type  = ddns_cfg->;
            printf("Ddns type: %s\n", ddns_cfg->szServerType);
            ddns_info->ddns_port  = ddns_cfg->dwServerPort;
            ddns_info->ddns_times  = ddns_cfg->dwAlivePeriod;
            break;

        case SWAP_UNPACK:
            strncpy((char*)ddns_cfg->szDomainName, (const char*)ddns_info->ddns_account, 
                    sizeof(ddns_cfg->szDomainName)-1);
            strncpy((char*)ddns_cfg->szUserName, (const char*)ddns_info->ddns_usr, 
                    sizeof(ddns_cfg->szUserName)-1);
            strncpy((char*)ddns_cfg->szUserPsw, (const char*)ddns_info->ddns_pwd, 
                    sizeof(ddns_cfg->szUserPsw)-1);
            ddns_cfg->bEnable = ddns_info->ddns_open;
            //ddns_cfg->szServerType = ddns_info->;
            ddns_cfg->dwServerPort = ddns_info->ddns_port;
            ddns_cfg->dwAlivePeriod = ddns_info->ddns_times;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_disk_list(DH_HARDDISK_STATE *dh_disk, JDeviceDiskInfo *dev_disk, int flag)
{
    int i, state;
    NMP_ASSERT(dh_disk && dev_disk);

    switch (flag)
    {
        case SWAP_PACK:
            dev_disk->disk_num = dh_disk->dwDiskNum;
            for (i=0; i<(int)dh_disk->dwDiskNum; i++)
            {printf("disk no: [%d,%d]\n", i, dh_disk->stDisks[i].bDiskNum);
                dev_disk->disk[i].disk_no    = dh_disk->stDisks[i].bDiskNum;
                dev_disk->disk[i].disk_type  = dh_disk->stDisks[i].bSignal;
                state = dh_disk->stDisks[i].dwStatus & 0x0f;//低四位的值表示硬盘的状态，0-休眠,1-活动,2-故障
                dev_disk->disk[i].status = state ? (1==state?0:state) : 1;
                printf("status: %d, %d, %d\n", dev_disk->disk[i].status, dh_disk->stDisks[i].dwStatus, state);
                dev_disk->disk[i].total_size = dh_disk->stDisks[i].dwVolume;
                dev_disk->disk[i].free_size  = dh_disk->stDisks[i].dwFreeSpace;
                //dev_disk->disk[i].is_backup = hd_cfg->struHDInfo[i].byMode;
                //dev_disk->disk[i].sys_file_type = hd_cfg->struHDInfo[i].byMode;
            }
            break;

        case SWAP_UNPACK:
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_format_info(DISKCTRL_PARAM *dh_format, JFormatDisk *fmt_disk, int flag)
{
    NMP_ASSERT(dh_format && fmt_disk);

    switch (flag)
    {
        case SWAP_PACK:
            break;

        case SWAP_UNPACK:
            dh_format->dwSize   = sizeof(DISKCTRL_PARAM);
            dh_format->nIndex   = fmt_disk->disk_no;
            printf("disk no: %d\n", dh_format->nIndex);
            dh_format->ctrlType = 0;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_encode_info(DH_VIDEOENC_OPT *enc_cfg, JEncodeParameter *enc_info, int flag)
{
    int i;
    NMP_ASSERT(enc_cfg && enc_info);

    switch (flag)
    {
        case SWAP_PACK:
            enc_info->frame_rate = enc_cfg->byFramesPerSec;

            switch (enc_cfg->byImageSize)
            {
            case DH_CAPTURE_SIZE_QCIF:
                enc_info->resolution = J_SDK_VIDEO_QCIF;
                break;
            case DH_CAPTURE_SIZE_CIF:
                enc_info->resolution = J_SDK_VIDEO_CIF;
                break;
            case DH_CAPTURE_SIZE_HD1:
                enc_info->resolution = J_SDK_VIDEO_HD1;
                break;
            case DH_CAPTURE_SIZE_D1:
                enc_info->resolution = J_SDK_VIDEO_D1;
                break;
            case DH_CAPTURE_SIZE_QQVGA:
                enc_info->resolution = J_SDK_VIDEO_QQVGA;
                break;
            case DH_CAPTURE_SIZE_QVGA:
                enc_info->resolution = J_SDK_VIDEO_QVGA;
                break;
            case DH_CAPTURE_SIZE_VGA:
                enc_info->resolution = J_SDK_VIDEO_VGA;
                break;
            case DH_CAPTURE_SIZE_SVGA:
                enc_info->resolution = J_SDK_VIDEO_SVGA;
                break;
            case DH_CAPTURE_SIZE_UXGA:
                enc_info->resolution = J_SDK_VIDEO_UXGA;
                break;
            case DH_CAPTURE_SIZE_720:
                enc_info->resolution = J_SDK_VIDEO_720P;
                break;
            //case :
                //enc_info->resolution = J_SDK_VIDEO_960P;
                //break;
            case DH_CAPTURE_SIZE_1080:
                enc_info->resolution = J_SDK_VIDEO_1080P;
                break;
            }

            if (!enc_cfg->byImageQltyType)
                enc_info->qp_value = enc_cfg->byImageQlty-1;
            else
                enc_info->qp_value = 0;

            switch (enc_cfg->byEncodeMode)
            {
            case DH_CAPTURE_COMP_H264:
                enc_info->video_type = J_SDK_AV_VIDEO_H264;
                break;
            case DH_CAPTURE_COMP_DIVX_MPEG4:
            case DH_CAPTURE_COMP_MS_MPEG4:
            case DH_CAPTURE_COMP_MPEG2:
            case DH_CAPTURE_COMP_MPEG1:
                enc_info->video_type = J_SDK_AV_VIDEO_MPEG4;
                break;
            case DH_CAPTURE_COMP_MJPG:
                enc_info->video_type = J_SDK_AV_VIDEO_MJPEG;
                break;
            }

            //enc_info->audio_type = enc_cfg->wFormatTag;
            enc_info->audio_enble      = enc_cfg->byAudioEnable;
            enc_info->i_frame_interval = enc_cfg->bIFrameInterval * 1.7 + 0.5;
            enc_info->code_rate        = enc_cfg->wLimitStream;
            
            if (DH_CAPTURE_BRC_CBR == enc_cfg->byBitRateControl)
                enc_info->bit_rate = J_SDK_CBR;
            else
                enc_info->bit_rate = J_SDK_VBR;
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_REC_TYPE_NUM; i++)
            {
                enc_cfg[i].byFramesPerSec = enc_info->frame_rate;

                switch (enc_info->resolution)
                {
                case J_SDK_VIDEO_QCIF:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_QCIF;
                    break;
                case J_SDK_VIDEO_CIF:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_CIF;
                    break;
                case J_SDK_VIDEO_HD1:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_HD1;
                    break;
                case J_SDK_VIDEO_D1:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_D1;
                    break;
                case J_SDK_VIDEO_QQVGA:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_QQVGA;
                    break;
                case J_SDK_VIDEO_QVGA:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_QVGA;
                    break;
                case J_SDK_VIDEO_VGA:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_VGA;
                    break;
                case J_SDK_VIDEO_SVGA:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_SVGA;
                    break;
                case J_SDK_VIDEO_UXGA:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_UXGA;
                    break;
                case J_SDK_VIDEO_720P:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_720;
                    break;
                case J_SDK_VIDEO_1080P:
                    enc_cfg[i].byImageSize = (BYTE)DH_CAPTURE_SIZE_1080;
                    break;
                }

                if (!enc_cfg[i].byImageQltyType)
                    enc_cfg[i].byImageQlty = (BYTE)enc_info->qp_value + 1;
                else
                    enc_cfg[i].byImageQlty = 0;

                switch (enc_info->video_type)
                {
                case J_SDK_AV_VIDEO_H264:
                     enc_cfg[i].byEncodeMode = (BYTE)DH_CAPTURE_COMP_H264;
                    break;
                case J_SDK_AV_VIDEO_MPEG4:
                    enc_cfg[i].byEncodeMode = (BYTE)DH_CAPTURE_COMP_MS_MPEG4;
                    break;
                case J_SDK_AV_VIDEO_MJPEG:
                    enc_cfg[i].byEncodeMode = (BYTE)DH_CAPTURE_COMP_MJPG;
                    break;
                }

                enc_cfg[i].byAudioEnable   = (BYTE)enc_info->audio_enble;
                enc_cfg[i].bIFrameInterval = (BYTE)enc_info->i_frame_interval/1.7 + 0.5;
                enc_cfg[i].wLimitStream    = (BYTE)enc_info->code_rate;

                if (J_SDK_CBR == enc_info->bit_rate)
                    enc_cfg[i].byBitRateControl = (BYTE)DH_CAPTURE_BRC_CBR;
                else
                    enc_cfg[i].byBitRateControl = (BYTE)DH_CAPTURE_BRC_VBR;
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_display_info(DH_COLOR_CFG *color, JDisplayParameter *disp_info, int flag)
{
    int i;
    NMP_ASSERT(color && disp_info);

    switch (flag)
    {
        case SWAP_PACK:
            disp_info->bright     = color->byBrightness*2.5;
            disp_info->contrast   = color->byContrast*2.5;
            disp_info->saturation = color->bySaturation*2.5;
            disp_info->hue        = color->byHue*2.5;
            disp_info->sharpness  = color->byGain*2.5;
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_N_COL_TSECT; i++)
            {
                color[i].byBrightness = (BYTE)disp_info->bright/2.5;
                color[i].byContrast   = (BYTE)disp_info->contrast/2.5;
                color[i].bySaturation = (BYTE)disp_info->saturation/2.5;
                color[i].byHue        = (BYTE)disp_info->hue/2.5;
                color[i].byGainEn     = (BYTE)1;
                color[i].byGain       = (BYTE)disp_info->sharpness/2.5;
            }
            break;

        default:
            break;
    }
    printf("byBrightness: %02d  |  ", color->byBrightness);
    printf("bright    : %d\n", disp_info->bright);
    printf("byContrast  : %02d  |  ", color->byContrast);
    printf("contrast  : %d\n", disp_info->contrast);
    printf("bySaturation: %02d  |  ", color->bySaturation);
    printf("saturation: %d\n", disp_info->saturation);
    printf("byHue       : %02d  |  ", color->byHue);
    printf("hue       : %d\n", disp_info->hue);
    printf("byGain      : %02d  |  ", color->byGain);
    printf("sharpness : %d\n", disp_info->sharpness);

    return ;
}

void dah_swap_osd_info(DHDEV_CHANNEL_CFG *chn_cfg, JOSDParameter *osd_info, int flag)
{
    NMP_ASSERT(chn_cfg && osd_info);

    switch (flag)
    {
        case SWAP_PACK:
            osd_info->time_enable    = chn_cfg->stTimeOSD.bShow;
            osd_info->time_display_x = chn_cfg->stTimeOSD.rcRect.left * 704/8192;
            osd_info->time_display_y = chn_cfg->stTimeOSD.rcRect.top * 576/8192;
            osd_info->text_enable    = chn_cfg->stChannelOSD.bShow;
            osd_info->text_display_x = chn_cfg->stChannelOSD.rcRect.left * 704/8192;
            osd_info->text_display_y = chn_cfg->stChannelOSD.rcRect.top * 576/8192;
            osd_info->max_width      = 704;
            osd_info->max_height     = 576;
            strncpy((char*)osd_info->text_data, (const char*)chn_cfg->szChannelName, 
                sizeof(osd_info->text_data)-1);
            break;

        case SWAP_UNPACK:
            chn_cfg->stTimeOSD.bShow          = osd_info->time_enable;
            chn_cfg->stTimeOSD.rcRect.left    = osd_info->time_display_x * 8192/704;
            chn_cfg->stTimeOSD.rcRect.top     = osd_info->time_display_y * 8192/576;
            chn_cfg->stChannelOSD.bShow       = osd_info->text_enable;
            chn_cfg->stChannelOSD.rcRect.left = osd_info->text_display_x * 8192/704;
            chn_cfg->stChannelOSD.rcRect.top  = osd_info->text_display_y * 8192/576;
            strncpy((char*)chn_cfg->szChannelName, (const char*)osd_info->text_data, 
                sizeof(chn_cfg->szChannelName)-1);
            break;

        default:
            break;
    }
    printf("TimShow    : %d  |  ", chn_cfg->stTimeOSD.bShow);
    printf("time_enable    : %d\n", osd_info->time_enable);
    printf("ChannelShow: %d  |  ", chn_cfg->stChannelOSD.bShow);
    printf("text_enable    : %d\n", osd_info->text_enable);

    return ;
}

void dah_swap_record_info(DHDEV_RECORD_CFG *rec_cfg, JRecordParameter *rec_info, int flag)
{
//  int day_index, time_seg_index;
//  JTime *time_start, *time_end;

//  DH_TSECT *dah_sched_time;

    NMP_ASSERT(rec_cfg && rec_info);

    switch (flag)
    {
        case SWAP_PACK:
            rec_info->pre_record = rec_cfg->byPreRecordLen;
            dah_process_sched_time(rec_cfg->stSect, &rec_info->week, flag);
            break;

        case SWAP_UNPACK:
            rec_cfg->byPreRecordLen = rec_info->pre_record;
            memset(rec_cfg->stSect, 0, sizeof(rec_cfg->stSect));
            dah_process_sched_time(rec_cfg->stSect, &rec_info->week, flag);
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_move_alarm_info(DH_MOTION_DETECT_CFG *dah_motion, JMoveAlarm *move_alarm, int flag)
{
    int i;
    NMP_ASSERT(dah_motion && move_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            move_alarm->move_enable     = dah_motion->byMotionEn;
            move_alarm->sensitive_level = dah_motion->wSenseLevel;
            //move_alarm->detect_interval = dah_motion->;
            //move_alarm->detect_area = dah_motion->byDetected;// 检测区域，最多32*32块区域
            move_alarm->max_width       = 704;
            move_alarm->max_height      = 576;
            dah_process_sched_time(dah_motion->stSect, &move_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_MAX_VIDEO_IN_NUM; i++)
            {
                dah_motion[i].byMotionEn  = move_alarm->move_enable;
                dah_motion[i].wSenseLevel = move_alarm->sensitive_level;
                memset(dah_motion[i].stSect, 0, sizeof(dah_motion[i].stSect));
                dah_process_sched_time(dah_motion[i].stSect, &move_alarm->week, flag);
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_video_lost_info(DH_VIDEO_LOST_CFG *dah_video_lost, JLostAlarm *lost_alarm, int flag)
{
    int i;
    NMP_ASSERT(dah_video_lost && lost_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            lost_alarm->lost_enable = dah_video_lost->byAlarmEn;
            dah_process_sched_time(dah_video_lost->stSect, &lost_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_MAX_VIDEO_IN_NUM; i++)
            {
                dah_video_lost[i].byAlarmEn = lost_alarm->lost_enable;
                memset(dah_video_lost[i].stSect, 0, sizeof(dah_video_lost[i].stSect));
                dah_process_sched_time(dah_video_lost[i].stSect, &lost_alarm->week, flag);
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_hide_alarm_info(DH_BLIND_CFG *dah_hide_alarm, JHideAlarm *hide_alarm, int flag)
{
    int i;
    NMP_ASSERT(dah_hide_alarm && hide_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            hide_alarm->hide_enable = dah_hide_alarm->byBlindEnable;
            hide_alarm->max_width       = 704;
            hide_alarm->max_height      = 576;
            dah_process_sched_time(dah_hide_alarm->stSect, &hide_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_MAX_VIDEO_IN_NUM; i++)
            {
                dah_hide_alarm[i].byBlindEnable = hide_alarm->hide_enable;
                memset(dah_hide_alarm[i].stSect, 0, sizeof(dah_hide_alarm[i].stSect));
                dah_process_sched_time(dah_hide_alarm[i].stSect, &hide_alarm->week, flag);
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_alarm_in_info(DH_ALARMIN_CFG *dah_alarm_in, JIoAlarm *io_alarm, int flag)
{
    int i;
    NMP_ASSERT(dah_alarm_in && io_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            io_alarm->alarm_enable = dah_alarm_in->byAlarmEn;
            dah_process_sched_time(dah_alarm_in->stSect, &io_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            for (i=0; i<DH_MAX_VIDEO_IN_NUM; i++)
            {
                dah_alarm_in[i].byAlarmEn = io_alarm->alarm_enable;
                memset(dah_alarm_in[i].stSect, 0, sizeof(dah_alarm_in[i].stSect));
                dah_process_sched_time(dah_alarm_in[i].stSect, &io_alarm->week, flag);
            }
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_hide_info(DHDEV_VIDEOCOVER_CFG *vc_cfg, JHideParameter *hide_info, int flag)
{
    int i;
    NMP_ASSERT(vc_cfg && hide_info);

    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<(int)vc_cfg->bCoverCount; i++)
            {
                //if (vc_cfg->CoverBlock[i].bPriview)
                if (vc_cfg->CoverBlock[i].bEncode)
                    hide_info->hide_enable = 1;
                
                hide_info->hide_area.rect[i].left   = vc_cfg->CoverBlock[i].rcBlock.left * 704/8192;
                hide_info->hide_area.rect[i].top    = vc_cfg->CoverBlock[i].rcBlock.top * 576/8192;
                hide_info->hide_area.rect[i].right  = vc_cfg->CoverBlock[i].rcBlock.right * 704/8192;
                hide_info->hide_area.rect[i].bottom = vc_cfg->CoverBlock[i].rcBlock.bottom * 576/8192;
            }
            hide_info->hide_area.count = i;
            hide_info->max_width       = 704;
            hide_info->max_height      = 576;
            break;

        case SWAP_UNPACK:
            memset(vc_cfg->CoverBlock, 0, sizeof(vc_cfg->CoverBlock));
            for (i=0; i<(int)hide_info->hide_area.count; i++)
            {
                vc_cfg->CoverBlock[i].bBlockType = 0;
                vc_cfg->CoverBlock[i].bEncode    = hide_info->hide_enable;
                vc_cfg->CoverBlock[i].bPriview   = hide_info->hide_enable;

                vc_cfg->CoverBlock[i].rcBlock.left   = hide_info->hide_area.rect[i].left * 8192/704;
                vc_cfg->CoverBlock[i].rcBlock.top    = hide_info->hide_area.rect[i].top * 8192/576;
                vc_cfg->CoverBlock[i].rcBlock.right  = hide_info->hide_area.rect[i].right * 8192/704;
                vc_cfg->CoverBlock[i].rcBlock.bottom = hide_info->hide_area.rect[i].bottom * 8192/576;
            }
            vc_cfg->bCoverCount = i;
            break;

        default:
            break;
    }
//  printf("dwEnableHide: %02d   |  ", chn_cfg->byBlindEnable);
//  printf("hide_enable    : %d\n", hide_info->hide_enable);

    return ;
}

void dah_swap_ptz_info(CFG_PTZ_INFO *ptz_cfg, JPTZParameter *ptz_info, int flag)
{
    int i;
    int baud_rate[] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    NMP_ASSERT(ptz_cfg && ptz_info);

    switch (flag)
    {
        case SWAP_PACK:
            //for (i=0; i<MAX_DECPRO_LIST_SIZE; i++)
                //printf("%d.name: %s<------------------\n", i, ptz_cfg->stuDecProName[i].name);
            //16.name: PELCOD,23.name: PELCOP
            if (16 == ptz_cfg->nProName)
                ptz_info->protocol  = 0;
            else
                ptz_info->protocol  = 1;
            ptz_info->ptz_addr  = ptz_cfg->nDecoderAddress;
            ptz_info->baud_rate = baud_rate[ptz_cfg->struComm.byBaudRate];
            ptz_info->data_bit  = ptz_cfg->struComm.byDataBit + 5;
            if (0 == ptz_cfg->struComm.byStopBit)
                ptz_info->stop_bit = 1;
            else
                ptz_info->stop_bit = 2;
            ptz_info->verify    = ptz_cfg->struComm.byParity;
            break;

        case SWAP_UNPACK:
            ptz_cfg->nDecoderAddress = ptz_info->ptz_addr;
            if (0 == ptz_info->protocol)
                ptz_cfg->nProName = 16;
            else if (1 == ptz_info->protocol)
                ptz_cfg->nProName = 23;
            for (i=0; i<10; i++)
            {
                if ((int)ptz_info->baud_rate <= baud_rate[i])
                {
                    ptz_cfg->struComm.byBaudRate = i;
                    break;
                }
            }
            ptz_cfg->struComm.byDataBit = ptz_info->data_bit - 5;
            ptz_cfg->struComm.byStopBit = ptz_info->stop_bit;
            ptz_cfg->struComm.byParity  = ptz_info->verify;
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_store_log_info(dah_query_t *query_record, JStoreLog *store_log, int flag)
{
    int fin = 0, index = 0, count = 0;
    int count0 = 0, count1 = 0, count2 = 0;
    NET_RECORDFILE_INFO *rec_file;

    NMP_ASSERT(query_record && store_log);

    switch (flag)
    {
        case SWAP_PACK:
            rec_file = (NET_RECORDFILE_INFO*)query_record->buffer;
            while (TRUE)
            {
                if (query_record->filecount <= count)
                    break;

                switch (rec_file[count].nRecordFileType)
                {//0：普通录象；1：报警录象；2：移动检测；3：卡号录象；4：图片
                    case 0:
                        if ((int)store_log->beg_node <= count && 
                            (int)store_log->end_node >= count && 
                            (int)store_log->rec_type & 0x00000001)
                        {
                            if (index >= J_SDK_MAX_STORE_LOG_SIZE)
                                break;
                            
                            store_log->store[index].rec_type = 0x00000001;
                            fin = 1;
                        }
                        count0++;
                        break;
                    case 1:
                        if ((int)store_log->beg_node <= count && 
                            (int)store_log->end_node >= count && 
                            (int)store_log->rec_type & 0x00000010)
                        {
                            if (index >= J_SDK_MAX_STORE_LOG_SIZE)
                                break;
                            
                            store_log->store[index].rec_type = 0x00000010;
                            fin = 1;
                        }
                        count1++;
                        break;
                    case 2:
                        if ((int)store_log->beg_node <= count && 
                            (int)store_log->end_node >= count && 
                            (int)store_log->rec_type & 0x00000100)
                        {
                            if (index >= J_SDK_MAX_STORE_LOG_SIZE)
                                break;
                            
                            store_log->store[index].rec_type = 0x00000100;
                            fin = 1;
                        }
                        count2++;
                        break;
                        
                    default:
                        break;
                }

                if (fin)
                {
                    store_log->store[index].file_size = rec_file[count].size * 1024;    //byte
                    
                    dah_swap_time(&rec_file[count].starttime, 
                        &store_log->store[index].beg_time, SWAP_PACK);
                    dah_swap_time(&rec_file[count].endtime, 
                        &store_log->store[index].end_time, SWAP_PACK);
                    
                    index++;
                    fin = 0;
                }

                count++;
            }
            store_log->node_count  = index;

            if (store_log->rec_type | 0x00000001)
                store_log->total_count += count0;
            if (store_log->rec_type | 0x00000010)
                store_log->total_count += count1;
            if (store_log->rec_type | 0x00000100)
                store_log->total_count += count2;
            show_debug("total_count: %d\n", store_log->total_count);
            break;

        case SWAP_UNPACK:
            dah_swap_time(&query_record->start, &store_log->beg_time, SWAP_UNPACK);
            dah_swap_time(&query_record->end, &store_log->end_time, SWAP_UNPACK);
            break;

        default:
            break;
    }

    return ;
}

void dah_swap_ptz_control(dah_ptz_ctrl_t *dah_ptz_ctrl, JPTZControl *ptz_ctrl, int flag)
{
    NMP_ASSERT(dah_ptz_ctrl && ptz_ctrl);

    switch (flag)
    {
        case SWAP_PACK:
            ptz_ctrl->action = dah_ptz_ctrl->ptz_cmd;
            //ptz_ctrl->param;
            break;
            
        case SWAP_UNPACK:
            dah_ptz_ctrl->ptz_cmd = ptz_ctrl->action;
            break;
            
        default:
            break;
    }
    
    return ;
}




