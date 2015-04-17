#include <arpa/inet.h>
#include "nmp_hbn_swap.h"

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
hbn_process_version(char *nmp_version, size_t size, int *h_version, int flag)
{
    int higher, high, low;
    char *ptr;
    char tmp_version[J_SDK_MAX_VERSION_LEN];
    
    switch (flag)
    {
        case PACK_SOFTWARE:
            higher = (*h_version & 0xff000000) >> 24;
            high = (*h_version & 0x00ff0000) >> 16;
            low = *h_version & 0x0000ffff;
            
            snprintf((char*)nmp_version, size,
                "%d.%d.%d", higher, high, low);
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
                sscanf(ptr, "%d", &higher);
                ptr = strtok(NULL, ".");
                if (ptr)
                {
                    sscanf(ptr, "%d", &high);
                    ptr = strtok(NULL, ".");
                    if (ptr)
                    {
                        sscanf(ptr, "%d", &low);
                    }
                }
            }

            *h_version = higher << 24 | high << 16 | low;
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
hbn_process_build_date(char *nmp_date, size_t size, int *h_date, int flag)
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
            
            snprintf((char*)nmp_date, size,
                "%d.%d.%d", year, month, day);
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
hbn_process_sched_time(HB_NET_SCHEDTIME (*hbn_sched_time)[HB_MAX_TIMESEGMENT], 
        JWeek *week, int flag)
{
    int index, seg_count, *enable;
    int day_index, time_seg_index;
    JTime *time_start, *time_end;
    HB_NET_SCHEDTIME *hbn_alarm_time;
    
    switch (flag)
    {
        case SWAP_PACK:
            for (index=0; index<HB_MAX_DAYS; index++)
            {
                day_index = index ? ((index==HB_MAX_DAYS-1) ? 0 : index) : HB_MAX_DAYS-1;
            printf("day: %d\n", day_index);
                for (time_seg_index=0; time_seg_index<HB_MAX_TIMESEGMENT; 
                    time_seg_index++)
                {
                    time_start = &week->days[day_index].seg[time_seg_index].time_start;
                    time_end = &week->days[day_index].seg[time_seg_index].time_end;
                    enable = (int*)&week->days[day_index].seg[time_seg_index].enable;
                    
                    hbn_alarm_time = &hbn_sched_time[index][time_seg_index];
                    
                    //if (hbn_alarm_time)
                    {
                        *enable = (int)hbn_alarm_time->byEnable;
                        time_start->hour   = (int)hbn_alarm_time->byStartHour;
                        time_start->minute = (int)hbn_alarm_time->byStartMin;
                        time_start->second = (int)hbn_alarm_time->byStartSec;
                        
                        time_end->hour   = (int)hbn_alarm_time->byStopHour;
                        time_end->minute = (int)hbn_alarm_time->byStopMin;
                        time_end->second = (int)hbn_alarm_time->byStopSec;
                        
                        printf("enable    : %d\n", *enable);
                        printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                            hbn_alarm_time->byStartHour, hbn_alarm_time->byStartMin, 
                            hbn_alarm_time->byStopHour, hbn_alarm_time->byStopMin);
                        printf("time_seg: %02d:%02d - %02d:%02d\n", 
                            time_start->hour, time_start->minute, 
                            time_end->hour, time_end->minute);
                    }
                    //else
                        //break;
                }
                week->days[day_index].day_id = day_index;
                week->days[day_index].count = time_seg_index;
            }
            week->count = index;
            break;
            
        case SWAP_UNPACK:
            for (index=0; index<HB_MAX_DAYS; index++)
            {
                day_index = index ? ((index==HB_MAX_DAYS-1) ? 0 : index) : HB_MAX_DAYS-1;
            printf("UNPACK--->day: %d\n", day_index);
                seg_count = week->days[day_index].count;
                for (time_seg_index=0; (time_seg_index<HB_MAX_TIMESEGMENT && 0 < seg_count--); 
                    time_seg_index++)
                {
                    time_start = &week->days[day_index].seg[time_seg_index].time_start;
                    time_end = &week->days[day_index].seg[time_seg_index].time_end;
                    enable = (int*)&week->days[day_index].seg[time_seg_index].enable;
                    
                    hbn_alarm_time = &hbn_sched_time[index][time_seg_index];
                    
                    hbn_alarm_time->byEnable    = (BYTE)*enable;
                    hbn_alarm_time->byStartHour = (BYTE)time_start->hour;
                    hbn_alarm_time->byStartMin  = (BYTE)time_start->minute;
                    hbn_alarm_time->byStartSec  = (BYTE)time_start->second;
                    
                    hbn_alarm_time->byStopHour  = (BYTE)time_end->hour;
                    hbn_alarm_time->byStopMin   = (BYTE)time_end->minute;
                    hbn_alarm_time->byStopSec   = (BYTE)time_end->second;
                    
                    printf("enable    : %d\n", *enable);
                    printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                        hbn_alarm_time->byStartHour, hbn_alarm_time->byStartMin, 
                        hbn_alarm_time->byStopHour, hbn_alarm_time->byStopMin);
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



void hbn_swap_device_info(HB_NET_DEVICECFG *dev_cfg, JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            snprintf((char*)dev_info->dev_version, sizeof(dev_info->dev_version), 
                "%s", dev_cfg->sSoftwareVer);
            snprintf((char*)dev_info->hw_version, sizeof(dev_info->hw_version), 
                "%s", dev_cfg->sHardwareVer);
            printf("dev_cfg->sPanelVer: %s\n", dev_cfg->sPanelVer);
            //snprintf((char*)dev_info->release_date, sizeof(dev_info->release_date), 
              //  "%s", dev_cfg->sPanelVer);

            dev_info->pu_type     = (JPuType)dev_cfg->dwDevType;//DVR类型, 1:DVR 2:ATM DVR 3:DVS
            dev_info->di_num      = (int)dev_cfg->byAlarmInPortNum;
            dev_info->do_num      = (int)dev_cfg->byAlarmOutPortNum;
            dev_info->channel_num = (int)dev_cfg->byChanNum;
            dev_info->rs485_num   = (int)dev_cfg->byRS485Num;
            dev_info->rs232_num   = (int)dev_cfg->byRS232Num;
            break;

        case SWAP_UNPACK:
            break;

        default:
            break;
    }

    return ;
}

void hbn_swap_serial_info(HB_NET_SERIALCFG *serial_cfg, JSerialParameter *serial_info, int flag)
{
    int i, baud_rate[] = {2400, 2400, 4800, 9600, 19200, 38400};
    NMP_ASSERT(serial_cfg && serial_info);

    switch (flag)
    {
    case SWAP_PACK:
        serial_info->serial_no = (1==serial_cfg->dwSerialType) ? 1 : 0;
        if (0 <= serial_cfg->dwBaudRate && 5 > serial_cfg->dwBaudRate)
            serial_info->baud_rate = baud_rate[serial_cfg->dwBaudRate];

        serial_info->data_bit = serial_cfg->byDataBit;
        serial_info->stop_bit = serial_cfg->byStopBit;
        serial_info->verify   = serial_cfg->byParity;//0-NONE, 1-ODD, 2-EVEN, 3-SPACE
        break;

    case SWAP_UNPACK:
        serial_cfg->dwSerialType = serial_info->serial_no ? 1: 2;
        for (i=0; i<5; i++)
        {
            if (baud_rate[i] >= (int)serial_info->baud_rate)
            {
                serial_cfg->dwBaudRate = baud_rate[i];
                break;
            }
        }
        serial_cfg->byDataBit = serial_info->data_bit;
        serial_cfg->byStopBit = serial_info->stop_bit;
        serial_cfg->byParity  = serial_info->verify;
        break;

    default:
        break;
    }

    printf("serial_cfg->dwSerialType: %d | %d\n", 
        (int)serial_cfg->dwSerialType, serial_info->serial_no);
    return ;
}

void hbn_swap_time_info(HB_NET_TIME *hbn_time, JDeviceTime *dev_time, int flag)
{
    NMP_ASSERT(hbn_time && dev_time);

    switch (flag)
    {
        case SWAP_PACK:
            dev_time->time.year   = (int)hbn_time->dwYear - 1900;
            dev_time->time.month  = (int)hbn_time->dwMonth;
            dev_time->time.date   = (int)hbn_time->dwDay;
            dev_time->time.hour   = (int)hbn_time->dwHour;
            dev_time->time.minute = (int)hbn_time->dwMinute;
            dev_time->time.second = (int)hbn_time->dwSecond;
            printf("SWAP_PACK\n");
            printf("dev_time: %d/%d/%d %d:%d:%d\n", 
                (int)dev_time->time.year, 
                (int)dev_time->time.month, 
                (int)dev_time->time.date, 
                (int)dev_time->time.hour, 
                (int)dev_time->time.minute, 
                (int)dev_time->time.second);
            printf("hbn_time: %d/%d/%d %d:%d:%d\n", 
                (int)hbn_time->dwYear, 
                (int)hbn_time->dwMonth, 
                (int)hbn_time->dwDay, 
                (int)hbn_time->dwHour, 
                (int)hbn_time->dwMinute, 
                (int)hbn_time->dwSecond);
            dev_time->zone = DEF_PROXY_TIME_ZOE;
            break;

        case SWAP_UNPACK:
            hbn_time->dwYear   = (int)dev_time->time.year + 1900;
            hbn_time->dwMonth  = (int)dev_time->time.month;
            hbn_time->dwDay    = (int)dev_time->time.date;
            hbn_time->dwHour   = (int)dev_time->time.hour;
            hbn_time->dwMinute = (int)dev_time->time.minute;
            hbn_time->dwSecond = (int)dev_time->time.second;
            printf("SWAP_UNPACK\n");
            printf("dev_time: %d/%d/%d %d:%d:%d\n", 
                (int)dev_time->time.year, 
                (int)dev_time->time.month, 
                (int)dev_time->time.date, 
                (int)dev_time->time.hour, 
                (int)dev_time->time.minute, 
                (int)dev_time->time.second);
            printf("hbn_time: %d/%d/%d %d:%d:%d\n", 
                (int)hbn_time->dwYear, 
                (int)hbn_time->dwMonth, 
                (int)hbn_time->dwDay, 
                (int)hbn_time->dwHour, 
                (int)hbn_time->dwMinute, 
                (int)hbn_time->dwSecond);
            break;

        default:
            break;
    }

    return ;
}

void hbn_swap_ntp_info(HB_NET_NTPCFG *ntp_cfg, JDeviceNTPInfo *ntp_info, int flag)
{
    NMP_ASSERT(ntp_cfg && ntp_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)ntp_info->ntp_server_ip, (const char*)ntp_cfg->sServer, 
                    sizeof(ntp_info->ntp_server_ip)-1);
            ntp_info->time_interval = ntp_cfg->dwSyncPeriod;
            ntp_info->time_zone     = ntp_cfg->nTimeZone;
            ntp_info->ntp_enable    = ntp_cfg->dwAuto;
            break;

        case SWAP_UNPACK:
            strncpy((char*)ntp_cfg->sServer, (const char*)ntp_info->ntp_server_ip, 
                    sizeof(ntp_cfg->sServer)-1);
            ntp_cfg->dwSyncPeriod = ntp_info->time_interval;
            ntp_cfg->dwSyncUnit = 1;//0-分钟, 1-小时, 2-天, 3-星期, 4-月
            ntp_cfg->nTimeZone = ntp_info->time_zone;
            ntp_cfg->dwAuto = ntp_info->ntp_enable;
            break;

        default:
            break;
    }

    return ;
}

void hbn_swap_net_info(HB_NET_NETCFG *net_cfg, JNetworkInfo *net_info, int flag)
{
    NMP_ASSERT(net_cfg && net_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            inet_ntop(AF_INET, (void*)&net_cfg->struEtherNet[0].dwDevIP, 
                (char*)net_info->network[J_SDK_ETH0].ip, 
                sizeof(net_info->network[J_SDK_ETH0].ip));

            inet_ntop(AF_INET, (void*)&net_cfg->struEtherNet[0].dwDevIPMask, 
                (char*)net_info->network[J_SDK_ETH0].netmask, 
                sizeof(net_info->network[J_SDK_ETH0].netmask));

            inet_ntop(AF_INET, (void*)&net_cfg->dwGateWayIP, 
                (char*)net_info->network[J_SDK_ETH0].gateway, 
                sizeof(net_info->network[J_SDK_ETH0].gateway));

            inet_ntop(AF_INET, (void*)&net_cfg->dwDNSIP, 
                (char*)net_info->main_dns, sizeof(net_info->main_dns));

            strncpy((char*)net_info->network[J_SDK_ETH0].mac, 
                (const char*)net_cfg->struEtherNet[0].byMACAddr, 
                sizeof(net_info->network[J_SDK_ETH0].mac)-1);

            net_info->web_port  = net_cfg->dwHttpPort;
            net_info->cmd_port  = net_cfg->struEtherNet[0].wDevPort;
            break;

        case SWAP_UNPACK:
            inet_pton(AF_INET, (const char*)net_info->network[J_SDK_ETH0].ip, 
                &net_cfg->struEtherNet[0].dwDevIP);
            inet_pton(AF_INET, (const char*)net_info->network[J_SDK_ETH0].netmask, 
                &net_cfg->struEtherNet[0].dwDevIPMask);
            inet_pton(AF_INET, (const char*)net_info->network[J_SDK_ETH0].gateway, 
                &net_cfg->dwGateWayIP);
            inet_pton(AF_INET, (const char*)net_info->main_dns, 
                &net_cfg->dwDNSIP);
            strncpy((char*)net_cfg->struEtherNet[0].byMACAddr, 
                (const char*)net_info->network[J_SDK_ETH0].mac, 
                sizeof(net_cfg->struEtherNet[0].byMACAddr)-1);

            net_cfg->dwHttpPort = net_info->web_port;
            net_cfg->struEtherNet[0].wDevPort = net_info->cmd_port;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_pppoe_info(HB_NET_NETCFG *pppoe_cfg, JPPPOEInfo *pppoe_info, int flag)
{
    NMP_ASSERT(pppoe_cfg && pppoe_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            pppoe_info->enable = pppoe_cfg->dwPPPOE;
            inet_ntop(AF_INET, (void*)&pppoe_cfg->dwPPPOEIP, 
                    (char*)pppoe_info->ip, sizeof(pppoe_info->ip));
            strncpy((char*)pppoe_info->account, (const char*)pppoe_cfg->sPPPOEUser, 
                    sizeof(pppoe_info->account)-1);
            strncpy((char*)pppoe_info->passwd, (const char*)pppoe_cfg->sPPPOEPwd, 
                    sizeof(pppoe_info->passwd)-1);
            break;
            
        case SWAP_UNPACK:
            pppoe_cfg->dwPPPOE = pppoe_info->enable;
            inet_pton(AF_INET, (const char*)pppoe_info->ip, &pppoe_cfg->dwPPPOEIP);
            strncpy((char*)pppoe_cfg->sPPPOEUser, (const char*)pppoe_info->account, 
                    sizeof(pppoe_cfg->sPPPOEUser)-1);
            strncpy((char*)pppoe_cfg->sPPPOEPwd, (const char*)pppoe_info->passwd, 
                    sizeof(pppoe_cfg->sPPPOEPwd)-1);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_ftp_info(HB_NET_FTPRECORDCFG *ftp_cfg, JFTPParameter *ftp_info, int flag)
{
    NMP_ASSERT(ftp_cfg && ftp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)ftp_info->ftp_ip, (const char*)ftp_cfg->sServer, 
                    sizeof(ftp_info->ftp_ip)-1);
            strncpy((char*)ftp_info->ftp_usr, (const char*)ftp_cfg->sUser, 
                    sizeof(ftp_info->ftp_usr)-1);
            strncpy((char*)ftp_info->ftp_pwd, (const char*)ftp_cfg->sPwd, 
                    sizeof(ftp_info->ftp_pwd)-1);
            strncpy((char*)ftp_info->ftp_path, (const char*)ftp_cfg->sPath, 
                    sizeof(ftp_info->ftp_path)-1);
            ftp_info->ftp_port = ftp_cfg->dwPort;
            break;
            
        case SWAP_UNPACK:
            strncpy((char*)ftp_cfg->sServer, (const char*)ftp_info->ftp_ip, 
                    sizeof(ftp_cfg->sServer)-1);
            strncpy((char*)ftp_cfg->sUser, (const char*)ftp_info->ftp_usr, 
                    sizeof(ftp_cfg->sUser)-1);
            strncpy((char*)ftp_cfg->sPwd, (const char*)ftp_info->ftp_pwd, 
                    sizeof(ftp_cfg->sPwd)-1);
            strncpy((char*)ftp_cfg->sPath, (const char*)ftp_info->ftp_pwd, 
                    sizeof(ftp_cfg->sPath)-1);
            ftp_cfg->dwPort = ftp_info->ftp_port;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_smtp_info(HB_NET_SMTPCFG *smtp_cfg, JSMTPParameter *smtp_info, int flag)
{
    NMP_ASSERT(smtp_cfg && smtp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)smtp_info->mail_ip, (const char*)smtp_cfg->sHost, 
                    sizeof(smtp_info->mail_ip)-1);
            strncpy((char*)smtp_info->mail_addr, (const char*)smtp_cfg->sSendAddr, 
                    sizeof(smtp_info->mail_addr)-1);
            strncpy((char*)smtp_info->mail_usr, (const char*)smtp_cfg->sUser, 
                    sizeof(smtp_info->mail_usr)-1);
            strncpy((char*)smtp_info->mail_pwd, (const char*)smtp_cfg->sPwd, 
                    sizeof(smtp_info->mail_pwd)-1);
            //sRecvAddr//TO:邮件地址,多个以';'分隔。
            strncpy((char*)smtp_info->mail_rctp1, (const char*)smtp_cfg->sRecvAddr, 
                    sizeof(smtp_info->mail_rctp1)-1);
            /*strncpy((char*)smtp_info->mail_rctp2, (const char*)smtp_cfg->struReceiver[1].sAddress, 
                    sizeof(smtp_info->mail_rctp2)-1);
            strncpy((char*)smtp_info->mail_rctp3, (const char*)smtp_cfg->struReceiver[2].sAddress, 
                    sizeof(smtp_info->mail_rctp3)-1);*/
            smtp_info->mail_port  = smtp_cfg->dwPort;
            smtp_info->ssl_enable = smtp_cfg->byUseSSL;
            break;
            
        case SWAP_UNPACK:
            strncpy((char*)smtp_cfg->sHost, (const char*)smtp_info->mail_ip, 
                    sizeof(smtp_cfg->sHost)-1);
            strncpy((char*)smtp_cfg->sSendAddr, (const char*)smtp_info->mail_addr, 
                    sizeof(smtp_cfg->sSendAddr)-1);
            strncpy((char*)smtp_cfg->sUser, (const char*)smtp_info->mail_usr, 
                    sizeof(smtp_cfg->sUser)-1);
            strncpy((char*)smtp_cfg->sPwd, (const char*)smtp_info->mail_pwd, 
                    sizeof(smtp_cfg->sPwd)-1);
            strncpy((char*)smtp_cfg->sRecvAddr, (const char*)smtp_info->mail_rctp1, 
                    sizeof(smtp_cfg->sRecvAddr)-1);
            /*strncpy((char*)smtp_cfg->struReceiver[1].sAddress, (const char*)smtp_info->mail_rctp2, 
                    sizeof(smtp_cfg->struReceiver[1].sAddress)-1);
            strncpy((char*)smtp_cfg->struReceiver[2].sAddress, (const char*)smtp_info->mail_rctp3, 
                    sizeof(smtp_cfg->struReceiver[2].sAddress)-1);*/
            smtp_cfg->dwPort   = smtp_info->mail_port;
            smtp_cfg->byUseSSL = smtp_info->ssl_enable;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_ddns_info(HB_NET_DDNSCFG *ddns_cfg, JDdnsConfig *ddns_info, int flag)
{
    NMP_ASSERT(ddns_cfg && ddns_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            ddns_info->ddns_open = 1;
            ddns_info->ddns_type = ddns_cfg->wServer;//域名服务器：0 hanbang.org.cn；1 oray.net；2 dyndns.com；3 no-ip.com；4 ddns.hbgk.net
            strncpy((char*)ddns_info->ddns_account, (const char*)ddns_cfg->sName, 
                    sizeof(ddns_info->ddns_account)-1);
            strncpy((char*)ddns_info->ddns_usr, (const char*)ddns_cfg->sUser, 
                    sizeof(ddns_info->ddns_usr)-1);
            strncpy((char*)ddns_info->ddns_pwd, (const char*)ddns_cfg->sPWD, 
                    sizeof(ddns_info->ddns_pwd)-1);
            //ddns_info->ddns_port = ddns_cfg->;
            //ddns_info->ddns_times  = ddns_cfg->;
            break;
            
        case SWAP_UNPACK:
            ddns_cfg->wServer = 0;
            strncpy((char*)ddns_cfg->sName, (const char*)ddns_info->ddns_account, 
                    sizeof(ddns_cfg->sName)-1);
            strncpy((char*)ddns_cfg->sUser, (const char*)ddns_info->ddns_usr, 
                    sizeof(ddns_cfg->sUser)-1);
            strncpy((char*)ddns_cfg->sPWD, (const char*)ddns_info->ddns_pwd, 
                    sizeof(ddns_cfg->sPWD)-1);
            ddns_cfg->wServer = ddns_info->ddns_type;
            ddns_cfg->byAutoCon = 1;
            ddns_cfg->bySave = 1;
            break;
            
        default:
            break;
    }
    printf("wServer     : %02d   |  ", ddns_cfg->wServer);
    printf("ddns_type    : %d\n", ddns_info->ddns_type);
    
    printf("sDomainName : %s     |  ", ddns_cfg->sName);
    printf("ddns_account : %s\n", ddns_info->ddns_account);
    printf("sUsername   : %s     |  ", ddns_cfg->sUser);
    printf("ddns_usr     : %s\n", ddns_info->ddns_usr);
    printf("sPassword   : %s     |  ", ddns_cfg->sPWD);
    printf("ddns_pwd     : %s\n", ddns_info->ddns_pwd);
    
    return ;
}

void hbn_swap_encode_info(HB_NET_COMPRESSINFO *cmp_info, JEncodeParameter *encode_para, int flag)
{
    int video_bitrate_map[] = 
            {0, 100, 128, 256, 512, 1024, 1536, 2048, 3072, 4096};
    
    NMP_ASSERT(cmp_info && encode_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            encode_para->frame_rate = cmp_info->dwFrameRate;
//          encode_para->i_frame_interval = compress_info->wIntervalFrameI;
            
            /*switch (compress_info->byVideoEncType)
            {
                case 0xff://无效
                    break;
                case 0://私有264
                    //encode_para->video_type = ;
                    break;
                case 1://标准h264
                    encode_para->video_type = J_SDK_AV_VIDEO_H264;
                    break;
                case 2://标准mpeg4
                    encode_para->video_type = J_SDK_AV_VIDEO_MPEG4;
                    break;
                case 7://M-JPEG
                    encode_para->video_type = J_SDK_AV_VIDEO_MJPEG;
                    break;
            }*/

            encode_para->audio_enble = cmp_info->byAudio;
            /*switch (compress_info->byAudioEncType)
            {
                case 0xff://无效
                    encode_para->audio_enble = 0;
                    break;
                case 0:
                    break;
                case 1://G711_U
                    encode_para->audio_enble = 1;
                    encode_para->audio_type = J_SDK_AV_AUDIO_G711U;
                    break;
                case 2://G711_A
                    encode_para->audio_enble = 1;
                    encode_para->audio_type = J_SDK_AV_AUDIO_G711A;
                    break;
            }*/
            
            switch (cmp_info->byResolution)
            {
                case 0://CIF
                    encode_para->resolution = J_SDK_VIDEO_CIF;
                    break;
                case 1://HD1
                    encode_para->resolution = J_SDK_VIDEO_HD1;
                    break;
                case 2://D1
                    encode_para->resolution = J_SDK_VIDEO_D1;
                    break;
                case 3://QCIF
                    encode_para->resolution = J_SDK_VIDEO_QCIF;
                    break;
                case 4://720P
                    encode_para->resolution = J_SDK_VIDEO_720P;
                    break;
                case 5://1080P
                    encode_para->resolution = J_SDK_VIDEO_1080P;
                    break;
            }

            encode_para->qp_value = cmp_info->byPicQuality-1;
            
            //if (8 >= cmp_info->dwBitRate && 1 < cmp_info->dwBitRate)
                encode_para->code_rate = /*video_bitrate_map[*/cmp_info->dwBitRate;//];

            switch (cmp_info->byBitRateType)
            {
                case 0://变码率
                    encode_para->bit_rate = J_SDK_CBR;
                    break;
                case 1://定码率
                    encode_para->bit_rate = J_SDK_VBR;
                    break;
                case 2://定画质
                    encode_para->bit_rate = J_SDK_FIXQP;
                    break;
            }
            break;
            
        case SWAP_UNPACK:
            cmp_info->dwFrameRate = encode_para->frame_rate == 1 ? 
                2 : encode_para->frame_rate;

//          compress_info->wIntervalFrameI = encode_para->i_frame_interval;
            
            /*switch (encode_para->video_type)
            {
                case J_SDK_AV_VIDEO_H264:
                    compress_info->byVideoEncType = 1;//标准h264
                    break;
                case J_SDK_AV_VIDEO_MPEG4:
                    compress_info->byVideoEncType = 2;//标准mpeg4
                    break;
                case J_SDK_AV_VIDEO_MJPEG:
                    compress_info->byVideoEncType = 7;//M-JPEG
                    break;
                default:
                    compress_info->byVideoEncType = 0xff;//无效
                    break;
            }*/

            cmp_info->byAudio = encode_para->audio_enble;

            /*if (0 == encode_para->audio_enble)
                compress_info->byAudioEncType = 0xff;//无效
            else
            {
                switch (encode_para->audio_type)
                {
                    case J_SDK_AV_AUDIO_G711U:
                        compress_info->byAudioEncType = 1;//G711_U
                        break;
                    case J_SDK_AV_AUDIO_G711A:
                        compress_info->byAudioEncType = 2;//G711_A
                        break;
                }
            }*/
            
            cmp_info->byResolution = encode_para->resolution;
            cmp_info->byPicQuality = encode_para->qp_value+1;

            int i;
            for (i=0; i<23; i++)
            {
                if (video_bitrate_map[i] >= (int)encode_para->code_rate)
                {
                    cmp_info->dwBitRate = video_bitrate_map[i];
                    break;
                }
            }

            switch (encode_para->bit_rate)
            {
                case J_SDK_CBR://变码率
                    cmp_info->byBitRateType = 0;
                    break;
                case J_SDK_VBR://定码率
                    cmp_info->byBitRateType = 1;
                    break;
                case J_SDK_FIXQP://定画质
                    cmp_info->byBitRateType = 2;
                    break;
                default:
                    cmp_info->byBitRateType = 0;
                    break;
            }
            
        default:
            break;
    }
    
    printf("dwFrameRate     : %02d  |  ", (int)cmp_info->dwFrameRate);
    printf("frame_rate      : %d\n", (int)encode_para->frame_rate);
//  printf("wIntervalFrameI : %02d  |  ", compress_info->wIntervalFrameI);
//  printf("i_frame_interval: %d\n", encode_para->i_frame_interval);
//  printf("byVideoEncType  : %02d  |  ", compress_info->byVideoEncType);
//  printf("video_type      : %d\n", encode_para->video_type);
//  printf("byAudioEncType  : %02d  |  ", compress_info->byAudioEncType);
//  printf("audio_type      : %d\n", encode_para->audio_type);
    printf("byResolution    : %02d  |  ", (int)cmp_info->byResolution);
    printf("resolution      : %d\n", (int)encode_para->resolution);
    printf("byPicQuality    : %02d  |  ", (int)cmp_info->byPicQuality);
    printf("qp_value        : %d\n", (int)encode_para->qp_value);
    printf("dwBitRate       : %02d  |  ", (int)cmp_info->dwBitRate);
    printf("code_rate       : %d\n", (int)encode_para->code_rate);
    printf("byBitRateType   : %02d  |  ", (int)cmp_info->byBitRateType);
    printf("bit_rate        : %d\n", (int)encode_para->bit_rate);

    return ;
}

void hbn_swap_display_info(HB_NET_VIDEOPARAM *vid_cfg, JDisplayParameter *display, int flag)
{
    NMP_ASSERT(vid_cfg && display);

    switch (flag)
    {
        case SWAP_PACK:
            display->bright     = vid_cfg->dwBrightValue;
            display->contrast   = vid_cfg->dwContrastValue;
            display->saturation = vid_cfg->dwSaturationValue;
            display->hue        = vid_cfg->dwHueValue;
            //display->sharpness  = ;
            break;

        case SWAP_UNPACK:
            vid_cfg->dwBrightValue = display->bright;
            vid_cfg->dwContrastValue  = display->contrast;
            vid_cfg->dwSaturationValue = display->saturation;
            vid_cfg->dwHueValue        = display->hue;
            break;

        default:
            break;
    }
    printf("byBright    : %02d  |  ", (int)vid_cfg->dwBrightValue);
    printf("bright      : %d\n", (int)display->bright);
    printf("byContrast  : %02d  |  ", (int)vid_cfg->dwContrastValue);
    printf("contrast    : %d\n", (int)display->contrast);
    printf("bySaturation: %02d  |  ", (int)vid_cfg->dwSaturationValue);
    printf("saturation  : %d\n", (int)display->saturation);
    printf("byHue       : %02d  |  ", (int)vid_cfg->dwHueValue);
    printf("hue         : %d\n", (int)display->hue);
    //printf("dwSharp     : %02d  |  ", (int)vid_cfg->dwSharp);
    //printf("sharpness   : %d\n", (int)display->sharpness);

    return ;
}

void hbn_swap_osd_info(HB_NET_PICCFG *pic_cfg, JOSDParameter *osd_para, int flag)
{
    NMP_ASSERT(pic_cfg && osd_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            osd_para->time_enable    = pic_cfg->dwShowTime;
            osd_para->time_display_x = pic_cfg->wOSDTopLeftX;
            osd_para->time_display_y = pic_cfg->wOSDTopLeftY;
            osd_para->text_enable    = pic_cfg->dwShowChanName;
            osd_para->text_display_x = pic_cfg->wNameTopLeftX;
            osd_para->text_display_y = pic_cfg->wNameTopLeftY;
            osd_para->max_width      = 704;
            osd_para->max_height     = 576;
            
            snprintf((char*)osd_para->text_data, sizeof(osd_para->text_data),
                "%s", (const char*)pic_cfg->sChanName);
            break;
            
        case SWAP_UNPACK:
            pic_cfg->dwShowTime     = osd_para->time_enable;
            pic_cfg->wOSDTopLeftX   = osd_para->time_display_x;
            pic_cfg->wOSDTopLeftY   = osd_para->time_display_y;
            pic_cfg->dwShowChanName = osd_para->text_enable;
            pic_cfg->wNameTopLeftX  = osd_para->text_display_x;
            pic_cfg->wNameTopLeftY  = osd_para->text_display_y;
            
            snprintf((char*)pic_cfg->sChanName, sizeof(pic_cfg->sChanName),
                "%s", (const char*)osd_para->text_data);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_decoder_info(HB_NET_DECODERCFG *dec_cfg, JPTZParameter *ptz_info, int flag)
{
    int i;
    int baud_rate[] = {2400, 2400, 4800, 9600, 19200, 38400};
    NMP_ASSERT(dec_cfg && ptz_info);

    switch (flag)
    {
        case SWAP_PACK:
            if (0 <= dec_cfg->dwBaudRate && 5 > dec_cfg->dwBaudRate)
                ptz_info->baud_rate = baud_rate[dec_cfg->dwBaudRate];

            ptz_info->data_bit = dec_cfg->byDataBit;
            ptz_info->stop_bit = dec_cfg->byStopBit;
            ptz_info->protocol = dec_cfg->wDecoderType;
            ptz_info->ptz_addr = dec_cfg->wDecoderAddr;
            ptz_info->verify   = dec_cfg->byParity;
            break;
            
        case SWAP_UNPACK:
            for (i=0; i<5; i++)
            {
                if (baud_rate[i] == (int)ptz_info->baud_rate)
                {
                    dec_cfg->dwBaudRate = i;
                    break;
                }
            }
            dec_cfg->byDataBit    = ptz_info->data_bit;
            dec_cfg->byStopBit    = ptz_info->stop_bit;
            dec_cfg->wDecoderType = ptz_info->protocol;
            dec_cfg->wDecoderAddr = ptz_info->ptz_addr;
            dec_cfg->byParity     = ptz_info->verify;
            break;
            
        default:
            break;
    }
printf("dec_cfg->dwBaudRate  : %d\n", (int)dec_cfg->dwBaudRate);
printf("dec_cfg->byDataBit   : %d\n", (int)dec_cfg->byDataBit);
printf("dec_cfg->byStopBit   : %d\n", (int)dec_cfg->byStopBit);
printf("dec_cfg->wDecoderAddr: %d\n", (int)dec_cfg->wDecoderAddr);
printf("dec_cfg->byParity    : %d\n", (int)dec_cfg->byParity);
    
    return ;
}

void hbn_swap_record_info(HB_NET_RECORDCFG *rec_cfg, JRecordParameter *rec_para, int flag)
{
    int index, *enable;
    int day_index, time_seg_index;
    JTime *time_start, *time_end;

//    HB_NET_RECORDDAY *rec_day;
    HB_NET_SCHEDTIME *sched_time;
    
    NMP_ASSERT(rec_cfg && rec_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            //rec_para-> = rec_cfg->dwRecord;
            for (index=0; index<HB_MAX_DAYS; index++)
            {
                day_index = index ? ((index==HB_MAX_DAYS-1) ? 0 : index) : HB_MAX_DAYS-1;
            printf("day: %d, %d\n", day_index, index);
                for(time_seg_index=0; time_seg_index<HB_MAX_TIMESEGMENT; time_seg_index++)
                {
                    time_start = &rec_para->week.days[day_index]
                                    .seg[time_seg_index].time_start;
                    time_end = &rec_para->week.days[day_index]
                                    .seg[time_seg_index].time_end;
                    enable = (int*)&rec_para->week.days[day_index]
                                    .seg[time_seg_index].enable;
                    
                    sched_time = &rec_cfg->struRecSched[index][time_seg_index].strRecordTime;
                    
                    *enable = sched_time->byEnable;
                    time_start->hour   = sched_time->byStartHour;
                    time_start->minute = sched_time->byStartMin;
                    time_start->second = sched_time->byStartSec;
                    
                    time_end->hour   = sched_time->byStopHour;
                    time_end->minute = sched_time->byStopMin;
                    time_end->second = sched_time->byStopSec;
                    
                    printf("enable    : %d\n", *enable);
                    printf("SchedTime : %02d:%02d - %02d:%02d  |  ", 
                        sched_time->byStartHour, sched_time->byStartMin, 
                        sched_time->byStopHour, sched_time->byStopMin);
                    printf("time_seg  : %02d:%02d - %02d:%02d\n", 
                        time_start->hour, time_start->minute, 
                        time_end->hour, time_end->minute);
                }
                
                rec_para->week.days[day_index].day_id = day_index;
                rec_para->week.days[day_index].count = time_seg_index;
                rec_para->week.days[day_index].all_day_enable = 0;
//                  rec_cfg->struRecAllDay[day_index].bAllDayRecord;
            }
            rec_para->week.count = index;
            break;
            
        case SWAP_UNPACK:
            memset(rec_cfg->struRecAllDay, 0, sizeof(rec_cfg->struRecAllDay));
            memset(rec_cfg->struRecSched, 0, sizeof(rec_cfg->struRecSched));
            
            for (index=0; index<HB_MAX_DAYS; index++)
            {
                day_index = index ? ((index==HB_MAX_DAYS-1) ? 0 : index) : HB_MAX_DAYS-1;
            printf("UNPACK--->day: %d\n", day_index);
                for(time_seg_index=0; time_seg_index<HB_MAX_TIMESEGMENT; time_seg_index++)
                {
                    time_start = &rec_para->week.days[day_index]
                        .seg[time_seg_index].time_start;
                    time_end = &rec_para->week.days[day_index]
                        .seg[time_seg_index].time_end;
                    enable = (int*)&rec_para->week.days[day_index]
                                    .seg[time_seg_index].enable;
                    
                    sched_time = &rec_cfg->struRecSched[index][time_seg_index].strRecordTime;
                    
                    sched_time->byEnable = *enable;
                    sched_time->byStartHour = time_start->hour;
                    sched_time->byStartMin = time_start->minute;
                    sched_time->byStartSec = time_start->second;
                    
                    sched_time->byStopHour = time_end->hour;
                    sched_time->byStopMin = time_end->minute;
                    sched_time->byStopSec = time_end->second;
                    
                    printf("enable    : %d\n", *enable);
                    printf("    SchedTime : %02d:%02d - %02d:%02d  |  ", 
                        sched_time->byStartHour, sched_time->byStartMin, 
                        sched_time->byStopHour, sched_time->byStopMin);
                    printf("    time_seg  : %02d:%02d - %02d:%02d\n", 
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

void hbn_swap_hide_info(HB_NET_SHELTER *shelter, JHideParameter *hide_info, int flag)
{
    NMP_ASSERT(shelter && hide_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            hide_info->hide_area.count = 1;
            hide_info->hide_area.rect[0].left   = shelter->rect.wTopLeftX;
            hide_info->hide_area.rect[0].top    = shelter->rect.wTopLeftY;
            hide_info->hide_area.rect[0].right  = shelter->rect.wWidth;
            hide_info->hide_area.rect[0].bottom = shelter->rect.wHeight;
            hide_info->max_width  = 704;
            hide_info->max_height = 576;
            break;

        case SWAP_UNPACK:
            shelter->rect.wTopLeftX = hide_info->hide_area.rect[0].left;
            shelter->rect.wTopLeftY = hide_info->hide_area.rect[0].top;
            shelter->rect.wWidth    = hide_info->hide_area.rect[0].right;
            shelter->rect.wHeight   = hide_info->hide_area.rect[0].bottom;
            break;
            
        default:
            break;
    }
    printf("wHideAreaTopLeftX: %02d  |  ", shelter->rect.wTopLeftX);
    printf("left  : %02d\n", hide_info->hide_area.rect[0].left);
    printf("wHideAreaTopLeftY: %02d  |  ", shelter->rect.wTopLeftY);
    printf("top   : %02d\n", hide_info->hide_area.rect[0].top);
    printf("wHideAreaWidth   : %02d  |  ", shelter->rect.wWidth);
    printf("right : %02d\n", hide_info->hide_area.rect[0].right);
    printf("wHideAreaHeight  : %02d  |  ", shelter->rect.wHeight);
    printf("bottom: %02d\n", hide_info->hide_area.rect[0].bottom);


    return ;
}

void hbn_swap_move_alarm_info(HB_NET_MOTION *motion, JMoveAlarm *move_alarm, int flag)
{
    int i, j;
    NMP_ASSERT(motion && move_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            move_alarm->move_enable     = motion->byEnable;
            move_alarm->sensitive_level = motion->byMotionSensitive;
            move_alarm->max_width       = 704;
            move_alarm->max_height      = 576;
            for (i=0; i<HB_MOTION_SCOPE_HIGHT; i++)
            {
                for (j=0; j<HB_MOTION_SCOPE_WIDTH; j++)
                {
                    if (motion->byMotionScope[i][j])
                    {
                        move_alarm->detect_area.count = 1;
                        move_alarm->detect_area.rect[0].left   = 2;
                        move_alarm->detect_area.rect[0].top    = 2;
                        move_alarm->detect_area.rect[0].right  = 700;
                        move_alarm->detect_area.rect[0].bottom = 572;
                        break;
                    }
                }
            }
            hbn_process_sched_time(motion->struTime, &move_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            motion->byEnable = move_alarm->move_enable;
            motion->byMotionSensitive = move_alarm->sensitive_level;
            if (0 < move_alarm->detect_area.count)
            {
                for (i=0; i<HB_MOTION_SCOPE_HIGHT; i++)
                {
                    for (j=0; j<HB_MOTION_SCOPE_WIDTH; j++)
                        motion->byMotionScope[i][j] = 1;
                }
            }
            memset(motion->struTime, 0, sizeof(motion->struTime));
            hbn_process_sched_time(motion->struTime, &move_alarm->week, flag);
            break;
            
        default:
            break;
    }
    printf("byEnable: %02d   |  ", motion->byEnable);
    printf("move_enable    : %d\n", move_alarm->move_enable);
    printf("byEnable: %02d   |  ", motion->byMotionSensitive);
    printf("sensitive_level: %d\n", move_alarm->sensitive_level);
    
    return ;
}

void hbn_swap_video_lost_info(HB_NET_VILOST *video_lost, JLostAlarm *lost_alarm, int flag)
{
    NMP_ASSERT(video_lost && lost_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            lost_alarm->lost_enable = video_lost->byEnable;
            hbn_process_sched_time(video_lost->struTime, &lost_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            video_lost->byEnable = lost_alarm->lost_enable;
            memset(video_lost->struTime, 0, sizeof(video_lost->struTime));
            hbn_process_sched_time(video_lost->struTime, &lost_alarm->week, flag);          
            break;
            
        default:
            break;
    }
    printf("byEnable: %d   |  ", video_lost->byEnable);
    printf("lost_enable    : %d\n", lost_alarm->lost_enable);
    
    return ;
}

void hbn_swap_hide_alarm_info(HB_NET_HIDEALARM *hbn_hide_alarm, JHideAlarm *hide_alarm, int flag)
{
    NMP_ASSERT(hide_alarm && hide_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            hide_alarm->hide_enable = hbn_hide_alarm->dwEnable;
            hide_alarm->detect_area.count = 1;
            hide_alarm->detect_area.rect[0].left   = hbn_hide_alarm->rect.wTopLeftX;
            hide_alarm->detect_area.rect[0].top    = hbn_hide_alarm->rect.wTopLeftY;
            hide_alarm->detect_area.rect[0].right  = hbn_hide_alarm->rect.wWidth;
            hide_alarm->detect_area.rect[0].bottom = hbn_hide_alarm->rect.wHeight;
            hide_alarm->max_width                  = 704;
            hide_alarm->max_height                 = 576;
            hbn_process_sched_time(hbn_hide_alarm->struTime, &hide_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            hbn_hide_alarm->dwEnable       = hide_alarm->hide_enable;
            hbn_hide_alarm->rect.wTopLeftX = hide_alarm->detect_area.rect[0].left;
            hbn_hide_alarm->rect.wTopLeftY = hide_alarm->detect_area.rect[0].top;
            hbn_hide_alarm->rect.wWidth    = hide_alarm->detect_area.rect[0].right;
            hbn_hide_alarm->rect.wHeight   = hide_alarm->detect_area.rect[0].bottom;
            memset(hbn_hide_alarm->struTime, 0, sizeof(hbn_hide_alarm->struTime));
            hbn_process_sched_time(hbn_hide_alarm->struTime, &hide_alarm->week, flag);
            break;

        default:
            break;
    }
    printf("dwEnable : %02d   |  ", (int)hbn_hide_alarm->dwEnable);
    printf("hide_enable    : %d\n", (int)hide_alarm->hide_enable);
    printf("wTopLeftX: %02d   |  ", (int)hbn_hide_alarm->rect.wTopLeftX);
    printf("left     : %d\n", (int)hide_alarm->detect_area.rect[0].left);
    printf("wTopLeftY: %02d   |  ", (int)hbn_hide_alarm->rect.wTopLeftY);
    printf("top      : %d\n", (int)hide_alarm->detect_area.rect[0].top);
    printf("wWidth: %02d   |  ", (int)hbn_hide_alarm->rect.wWidth);
    printf("right    : %d\n", (int)hide_alarm->detect_area.rect[0].right);
    printf("wHeight  : %02d   |  ", (int)hbn_hide_alarm->rect.wHeight);
    printf("bottom   : %d\n", (int)hide_alarm->detect_area.rect[0].bottom);

    return ;
}

void hbn_swap_alarm_in_info(HB_NET_ALARMINCFG *alarm_in, JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(alarm_in && io_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            io_alarm->alarm_enable = alarm_in->byAlarmInHandle;
            hbn_process_sched_time(alarm_in->struTime, &io_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            alarm_in->byAlarmInHandle = io_alarm->alarm_enable;
            memset(alarm_in->struTime, 0, sizeof(alarm_in->struTime));
            hbn_process_sched_time(alarm_in->struTime, &io_alarm->week, flag);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_alarm_out_info(HB_NET_ALARMOUTCFG *alarm_out, JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(alarm_out && io_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            hbn_process_sched_time(alarm_out->struTime, &io_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            memset(alarm_out->struTime, 0, sizeof(alarm_out->struTime));
            hbn_process_sched_time(alarm_out->struTime, &io_alarm->week, flag);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hbn_swap_disk_list(HB_NET_DISKSTAT *hbn_disk, JDeviceDiskInfo *dev_disk, int flag)
{
    int i, j = 0;
    NMP_ASSERT(hbn_disk && dev_disk);

    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<HB_MAX_HARDDISK; i++)
            {
                if (hbn_disk[i].dwVolume)
                {
                    dev_disk->disk[j].disk_no    = i;
                    dev_disk->disk[j].status     = hbn_disk[i].dwStat;
                    dev_disk->disk[j].total_size = hbn_disk[i].dwVolume;
                    dev_disk->disk[j].free_size  = hbn_disk[i].dwFreeSpace;
                    printf("\nindex: %d\n", j);
                    printf("disk no   : %d\n", (int)dev_disk->disk[j].disk_no);
                    printf("total size: %d\n", (int)dev_disk->disk[j].total_size);
                    printf("free  size: %d\n", (int)dev_disk->disk[j].free_size);
                    j++;
                }
            }
            dev_disk->disk_num = j;
            break;

        case SWAP_UNPACK:
            break;

        default:
            break;
    }

    return ;
}

/*void hbn_swap_format_info(struct hbn_format *hbn_format, JFormatDisk *format_disk, int flag)
{
    NMP_ASSERT(hbn_format && format_disk);
    
    switch (flag)
    {
        case SWAP_PACK:
            format_disk->disk_no = hbn_format->disk_no;
            break;
            
        case SWAP_UNPACK:
            hbn_format->disk_no = format_disk->disk_no;
            break;
            
        default:
            break;
    }
    
    return ;
}*/

void hbn_swap_store_log_info(JStoreLog *store_log_cfg, JStoreLog *store_log_param, int flag)
{
    int index;
    NMP_ASSERT(store_log_cfg && store_log_param);

    switch (flag)
    {
        case SWAP_PACK:
            for (index = 0; index < (int)store_log_param->node_count; index++)
            {
                switch(store_log_param->store[index].rec_type)
                {
                    case HB_NET_RECSCHEDULE://  定时录像    
                        store_log_param->store[index].rec_type = TIMER_RECODE; 
                        break;
                    case HB_NET_RECMOTION://  移动侦测对象   
                        store_log_param->store[index].rec_type = MOVE_RECODE;
                        break;
                    case HB_NET_RECALARM:// 报警录像    
                        store_log_param->store[index].rec_type = ALARM_RECODE;
                        break;
                    /*case 4:// 报警录像 + 移动侦测 
                        store_log_param->store[index].rec_type = (0x00000010 | 0x00000100);
                        break;*/
                    case HB_NET_RECMANUAL:// 手动录像  
                        store_log_param->store[index].rec_type = MANUAL_RECODE;
                        break;
                    case HB_NET_REC_ALL:// 全部
                        store_log_param->store[index].rec_type = ALL_RECODE;
                    default:
                        break;
                }
            }
            show_debug("total_count: %d\n", (int)store_log_param->total_count);
            break;

        case SWAP_UNPACK:
            switch(store_log_cfg->rec_type)
            {
                case TIMER_RECODE:// 定时录像 
                    store_log_param->rec_type = HB_REC_SCHEDULE;
                    break;
                case ALARM_RECODE:// 报警录像
                    store_log_param->rec_type = HB_REC_ALARM;
                    break;
                case MOVE_RECODE:// 移动侦测录像
                    store_log_param->rec_type = HB_REC_MOTION;
                    break;
                /*case (ALARM_RECODE | MOVE_RECODE):            // 报警录像 + 移动侦测录像
                    store_log_param->rec_type = 4;
                    break;*/
                case MANUAL_RECODE:// 手动录像
                    store_log_param->rec_type = HB_NET_RECMANUAL;
                    break;
                case ALL_RECODE:// 全部
                    store_log_param->rec_type = HB_REC_ALL;
                break;
            }
            break;

        default:
            break;
    }

    return ;
}

void hbn_swap_cruise_way(HB_NET_PRESETPOLLCFG *pp_cfg, JCruiseWay *crz_way, int flag)
{
    int i, j=0;
    JCruisePoint *crz_pp;
    HB_NET_PRESET *preset;
    
    NMP_ASSERT(pp_cfg && crz_way);
    
    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<16; i++)
            {
                preset = &pp_cfg->struPreset[i];
                if (255 != preset->wPreset)
                {
                    crz_pp = &crz_way->crz_pp[j++];
                    crz_pp->preset = (int)preset->wPreset;
                    //crz_pp->speed = (int)preset->;
                    crz_pp->dwell = (int)preset->wTime * 60/99;
                }
            }
            crz_way->pp_count = j;
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }
    
    return ;
}

