
#include "nmp_bsm_swap.h"

void bsm_swap_device_info(HI_DEVICE_INFO *dev_cfg, 
        JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            dev_info->pu_type     = (JPuType)J_SDK_IPC;
            strncpy((char*)dev_info->dev_version, dev_cfg->aszServerSoftVersion, 
                sizeof(dev_info->dev_version)-1);
            /*dev_info->di_num      = (int)dev_cfg->byAlarmInPortNum;
            dev_info->do_num      = (int)dev_cfg->byAlarmOutPortNum;
            dev_info->channel_num = (int)dev_cfg->byChanNum;*/
            break;

        case SWAP_UNPACK:
            strncpy(dev_cfg->aszServerSoftVersion, (const char*)dev_info->dev_version, 
                sizeof(dev_cfg->aszServerSoftVersion)-1);
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_network_info(HI_S_NETINFO *net_cfg, 
        JNetworkInfo *net_info, int flag)
{
    NMP_ASSERT(net_cfg && net_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)net_info->network[J_SDK_ETH0].ip, 
                net_cfg->aszServerIP, 
                sizeof(net_info->network[J_SDK_ETH0].ip)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].netmask, 
                net_cfg->aszNetMask, 
                sizeof(net_info->network[J_SDK_ETH0].netmask)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].gateway, 
                net_cfg->aszGateWay, 
                sizeof(net_info->network[J_SDK_ETH0].gateway)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].mac, 
                net_cfg->aszMacAddr, 
                sizeof(net_info->network[J_SDK_ETH0].mac)-1);

            strncpy((char*)net_info->main_dns, 
                (const char*)net_cfg->aszFDNSIP, 
                sizeof(net_info->main_dns)-1);
            strncpy((char*)net_info->backup_dns, 
                (const char*)net_cfg->aszSDNSIP, 
                sizeof(net_info->backup_dns)-1);

            net_info->network[J_SDK_ETH0].type = J_SDK_ETH0;
            net_info->network[J_SDK_ETH0].dhcp_enable = net_cfg->s32DhcpFlag;
            break;

        case SWAP_UNPACK:
            strncpy((char*)net_cfg->aszServerIP, 
                (const char*)net_info->network[J_SDK_ETH0].ip, 
                sizeof(net_cfg->aszServerIP)-1);
            strncpy((char*)net_cfg->aszNetMask, 
                (const char*)net_info->network[J_SDK_ETH0].netmask, 
                sizeof(net_cfg->aszNetMask)-1);
            strncpy((char*)net_cfg->aszGateWay, 
                (const char*)net_info->network[J_SDK_ETH0].gateway, 
                sizeof(net_cfg->aszGateWay)-1);
            strncpy((char*)net_cfg->aszMacAddr, 
                (const char*)net_info->network[J_SDK_ETH0].mac, 
                sizeof(net_cfg->aszMacAddr)-1);

            strncpy((char*)net_cfg->aszFDNSIP, 
                (const char*)net_info->main_dns, 
                sizeof(net_cfg->aszFDNSIP)-1);
            strncpy((char*)net_cfg->aszSDNSIP, 
                (const char*)net_info->backup_dns, 
                sizeof(net_cfg->aszSDNSIP)-1);

            net_cfg->s32DhcpFlag = net_info->network[J_SDK_ETH0].dhcp_enable;
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_ftp_info(HI_S_FTP_PARAM *ftp_cfg, 
        JFTPParameter *ftp_param, int flag)
{
    NMP_ASSERT(ftp_cfg && ftp_param);
    
    switch (flag)
    {
        case SWAP_PACK:
            ftp_param->ftp_port = ftp_cfg->u32Port;
            strncpy((char*)ftp_param->ftp_ip, (const char*)ftp_cfg->sServer, 
                    sizeof(ftp_param->ftp_ip)-1);
            strncpy((char*)ftp_param->ftp_usr, (const char*)ftp_cfg->sUser, 
                    sizeof(ftp_param->ftp_usr)-1);
            strncpy((char*)ftp_param->ftp_pwd, (const char*)ftp_cfg->sPass, 
                    sizeof(ftp_param->ftp_pwd)-1);
            break;

        case SWAP_UNPACK:
            ftp_cfg->u32Port = ftp_param->ftp_port;
            strncpy((char*)ftp_cfg->sServer, (const char*)ftp_param->ftp_ip, 
                    sizeof(ftp_cfg->sServer)-1);
            strncpy((char*)ftp_cfg->sUser, (const char*)ftp_param->ftp_usr, 
                    sizeof(ftp_cfg->sUser)-1);
            strncpy((char*)ftp_cfg->sPass, (const char*)ftp_param->ftp_pwd, 
                    sizeof(ftp_cfg->sPass)-1);
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_smtp_info(HI_S_EMAIL_PARAM *email_cfg, 
        JSMTPParameter *smtp_info, int flag)
{
    NMP_ASSERT(email_cfg && smtp_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)smtp_info->mail_ip, (const char*)email_cfg->sServer, 
                    sizeof(smtp_info->mail_ip)-1);
            strncpy((char*)smtp_info->mail_addr, (const char*)email_cfg->sFrom, 
                    sizeof(smtp_info->mail_addr)-1);
            strncpy((char*)smtp_info->mail_usr, (const char*)email_cfg->sUser, 
                    sizeof(smtp_info->mail_usr)-1);
            strncpy((char*)smtp_info->mail_pwd, (const char*)email_cfg->sPass, 
                    sizeof(smtp_info->mail_pwd)-1);
            strncpy((char*)smtp_info->mail_rctp1, (const char*)email_cfg->sTo, 
                    sizeof(smtp_info->mail_rctp1)-1);
            /*strncpy((char*)smtp_info->mail_rctp2, (const char*)email_cfg->struReceiver[1].sAddress, 
                    sizeof(smtp_info->mail_rctp2)-1);
            strncpy((char*)smtp_info->mail_rctp3, (const char*)email_cfg->struReceiver[2].sAddress, 
                    sizeof(smtp_info->mail_rctp3)-1);*/
            smtp_info->mail_port  = email_cfg->u32Port;
            smtp_info->ssl_enable = email_cfg->u32Ssl;
            break;

        case SWAP_UNPACK:
            strncpy((char*)email_cfg->sServer, (const char*)smtp_info->mail_ip, 
                    sizeof(email_cfg->sServer)-1);
            strncpy((char*)email_cfg->sFrom, (const char*)smtp_info->mail_addr, 
                    sizeof(email_cfg->sFrom)-1);
            strncpy((char*)email_cfg->sUser, (const char*)smtp_info->mail_usr, 
                    sizeof(email_cfg->sUser)-1);
            strncpy((char*)email_cfg->sPass, (const char*)smtp_info->mail_pwd, 
                    sizeof(email_cfg->sPass)-1);
            strncpy((char*)email_cfg->sTo, (const char*)smtp_info->mail_rctp1, 
                    sizeof(email_cfg->sTo)-1);
            email_cfg->u32Port = smtp_info->mail_port;
            email_cfg->u32Ssl  = smtp_info->ssl_enable;
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_ddns_info(HI_S_DNS_PARAM *ddns_cfg, 
        JDdnsConfig *ddns_info, int flag)
{
    NMP_ASSERT(ddns_cfg && ddns_info);

    switch (flag)
    {
        case SWAP_PACK:
            ddns_info->ddns_open = ddns_cfg->u32Enable;
            //if (0 < ddns_cfg->byHostIndex)
              //  ddns_info->ddns_type  = ddns_cfg->byHostIndex - 1;
            strncpy((char*)ddns_info->ddns_account, (const char*)ddns_cfg->sDomain, 
                    sizeof(ddns_info->ddns_account)-1);
            strncpy((char*)ddns_info->ddns_usr, (const char*)ddns_cfg->sUser, 
                    sizeof(ddns_info->ddns_usr)-1);
            strncpy((char*)ddns_info->ddns_pwd, (const char*)ddns_cfg->sPass, 
                    sizeof(ddns_info->ddns_pwd)-1);
            //ddns_info->ddns_port  = ddns_cfg->wDDNSPort;
            //ddns_info->ddns_times  = ddns_cfg->;
            break;

        case SWAP_UNPACK:
            ddns_cfg->u32Enable = ddns_info->ddns_open;
            //ddns_cfg->byHostIndex = ddns_info->ddns_type + 1;
            strncpy((char*)ddns_cfg->sDomain, (const char*)ddns_info->ddns_account, 
                    sizeof(ddns_cfg->sDomain)-1);
            strncpy((char*)ddns_cfg->sUser, (const char*)ddns_info->ddns_usr, 
                    sizeof(ddns_cfg->sUser)-1);
            strncpy((char*)ddns_cfg->sPass, (const char*)ddns_info->ddns_pwd, 
                    sizeof(ddns_cfg->sPass)-1);
            //ddns_cfg->wDDNSPort = ddns_info->ddns_port;
            break;

        default:
            break;
    }
    printf("byEnableDDNS: %02d   |  ", ddns_cfg->u32Enable);
    printf("ddns_open    : %d\n", ddns_info->ddns_open);
    printf("byHostIndex : %s<===============\n", ddns_cfg->sServiceType);

    printf("sDomainName : %s     |  ", ddns_cfg->sDomain);
    printf("ddns_account : %s\n", ddns_info->ddns_account);
    printf("sUser   : %s     |  ", ddns_cfg->sUser);
    printf("ddns_usr     : %s\n", ddns_info->ddns_usr);
    printf("sPass   : %s     |  ", ddns_cfg->sPass);
    printf("ddns_pwd     : %s\n", ddns_info->ddns_pwd);

    return ;
}

void bsm_swap_disk_list(HI_S_DISK_INFO *disk_cfg, 
        JDeviceDiskInfo *dev_disk, int flag)
{
    int i;
    NMP_ASSERT(disk_cfg && dev_disk);
    
    switch (flag)
    {
        case SWAP_PACK:
            dev_disk->disk_num = disk_cfg->s32Num;
            for (i=0; i<(int)disk_cfg->s32Num; i++)
            {
                dev_disk->disk[i].disk_no    = i;
                dev_disk->disk[i].disk_type  = J_SDK_SATA;
                dev_disk->disk[i].status     = J_SDK_USING;
                dev_disk->disk[i].total_size = disk_cfg->sDisk[i].u32Total;
                dev_disk->disk[i].free_size  = disk_cfg->sDisk[i].u32Free;
                printf("\nindex: %d\n", i);
                printf("disk no   : %d\n", dev_disk->disk[i].disk_no);
                printf("total size: %d\n", dev_disk->disk[i].total_size);
                printf("free  size: %d\n", dev_disk->disk[i].free_size);
            }
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }
    
    return ;
}

void bsm_swap_video_info(HI_S_Video *video_cfg, 
        JEncodeParameter *enc_info, int flag)
{
    NMP_ASSERT(video_cfg && enc_info);

    switch (flag)
    {
        case SWAP_PACK:
            enc_info->frame_rate = video_cfg->u32Frame;

            printf("u32ImgQuality: %d\n", video_cfg->u32ImgQuality);
            if (video_cfg->u32ImgQuality)
                enc_info->qp_value = video_cfg->u32ImgQuality-1;
            else
                enc_info->qp_value = 0;

            enc_info->i_frame_interval = (int)video_cfg->u32Iframe;
            enc_info->code_rate        = video_cfg->u32Bitrate;

            if (video_cfg->blCbr)
                enc_info->bit_rate = J_SDK_CBR;
            else 
                enc_info->bit_rate = J_SDK_VBR;
            break;

        case SWAP_UNPACK:
            video_cfg->u32Frame = enc_info->frame_rate;
            video_cfg->u32ImgQuality = enc_info->qp_value + 1;
            printf("u32ImgQuality: %d\n", video_cfg->u32ImgQuality);
            printf("i_frame_interval: %d\n", enc_info->i_frame_interval);
            video_cfg->u32Iframe  = enc_info->i_frame_interval;
            video_cfg->u32Bitrate = enc_info->code_rate;

            if (J_SDK_CBR == enc_info->bit_rate)
                video_cfg->blCbr = HI_FALSE;
            else
                video_cfg->blCbr = HI_TRUE;
            break;

        default:
            break;
    }
    
    return ;
}

void bsm_swap_audio_info(HI_S_Audio *audio_cfg, 
        JEncodeParameter *enc_info, int flag)
{
    NMP_ASSERT(audio_cfg && enc_info);

    switch (flag)
    {
        case SWAP_PACK:
            enc_info->audio_enble = audio_cfg->blEnable;
            switch (audio_cfg->u32Type)
            {
            case HI_NET_DEV_AUDIO_TYPE_G711:
                enc_info->audio_type = J_SDK_AV_AUDIO_G711A;
                break;
            case HI_NET_DEV_AUDIO_TYPE_G726:
                enc_info->audio_type = J_SDK_AV_VIDEO_MPEG4;
                break;
            case HI_NET_DEV_AUDIO_TYPE_AMR:
                enc_info->audio_type = J_SDK_AV_VIDEO_MJPEG;
                break;
            }
            break;

        case SWAP_UNPACK:
            audio_cfg->blEnable = (HI_BOOL)enc_info->audio_enble;
            switch (enc_info->audio_type)
            {
            case J_SDK_AV_VIDEO_H264:
                 audio_cfg->u32Type = HI_NET_DEV_AUDIO_TYPE_G711;
                break;
            case J_SDK_AV_VIDEO_MPEG4:
                audio_cfg->u32Type = HI_NET_DEV_AUDIO_TYPE_G726;
                break;
            case J_SDK_AV_VIDEO_MJPEG:
                audio_cfg->u32Type = HI_NET_DEV_AUDIO_TYPE_AMR;
                break;
            }
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_display_info(HI_S_Display *dis_cfg, 
        JDisplayParameter *dis_info, int flag)
{
    NMP_ASSERT(dis_cfg && dis_info);

    switch (flag)
    {
        case SWAP_PACK:
            dis_info->bright     = dis_cfg->u32Brightness;
            dis_info->contrast   = dis_cfg->u32Contrast * (255/7);
            dis_info->saturation = dis_cfg->u32Saturation;
            dis_info->hue        = dis_cfg->u32Hue;
            dis_info->sharpness  = 128;
            break;

        case SWAP_UNPACK:
            dis_cfg->u32Brightness = dis_info->bright;
            dis_cfg->u32Contrast   = dis_info->contrast / (255/7);
            dis_cfg->u32Saturation = dis_info->saturation;
            dis_cfg->u32Hue        = dis_info->hue;
            break;

        default:
            break;
    }
    printf("u32Brightness: %02d  |  ", dis_cfg->u32Brightness);
    printf("bright    : %d\n", dis_info->bright);
    printf("u32Contrast  : %02d  |  ", dis_cfg->u32Contrast);
    printf("contrast  : %d\n", dis_info->contrast);
    printf("u32Saturation: %02d  |  ", dis_cfg->u32Saturation);
    printf("saturation: %d\n", dis_info->saturation);
    printf("u32Hue      : %02d  |  ", dis_cfg->u32Hue);
    printf("hue       : %d\n", dis_info->hue);
    printf("sharpness : %d\n", dis_info->sharpness);

    return ;
}

void bsm_swap_osd_info(HI_S_OSD *osd_cfg, 
        JOSDParameter *osd_info, int flag)
{
    NMP_ASSERT(osd_cfg && osd_info);

    switch (flag)
    {
        case SWAP_PACK:
            osd_info->time_enable    = osd_cfg->blEnTime;
            //osd_info->time_display_x = osd_cfg->stTimeOSD.rcRect.left * 704/8192;
            //osd_info->time_display_y = osd_cfg->stTimeOSD.rcRect.top * 576/8192;
            osd_info->text_enable    = osd_cfg->blEnName;
            //osd_info->text_display_x = osd_cfg->stChannelOSD.rcRect.left * 704/8192;
            //osd_info->text_display_y = osd_cfg->stChannelOSD.rcRect.top * 576/8192;
            osd_info->max_width      = 704;
            osd_info->max_height     = 576;
            strncpy((char*)osd_info->text_data, (const char*)osd_cfg->sName, 
                sizeof(osd_info->text_data)-1);
            break;

        case SWAP_UNPACK:
            osd_cfg->blEnTime  = (HI_BOOL)osd_info->time_enable;
            //osd_cfg->stTimeOSD.rcRect.left    = osd_info->time_display_x * 8192/704;
            //osd_cfg->stTimeOSD.rcRect.top     = osd_info->time_display_y * 8192/576;
            osd_cfg->blEnName  = (HI_BOOL)osd_info->text_enable;
            //osd_cfg->stChannelOSD.rcRect.left = osd_info->text_display_x * 8192/704;
            //osd_cfg->stChannelOSD.rcRect.top  = osd_info->text_display_y * 8192/576;
            strncpy((char*)osd_cfg->sName, (const char*)osd_info->text_data, 
                sizeof(osd_cfg->sName)-1);
            break;

        default:
            break;
    }
    printf("blEnTime    : %d  |  ", osd_cfg->blEnTime);
    printf("time_enable    : %d\n", osd_info->time_enable);
    printf("blEnName: %d  |  ", osd_cfg->blEnName);
    printf("text_enable    : %d\n", osd_info->text_enable);
    printf("sName : %s\n", osd_cfg->sName);

    return ;
}

void bsm_swap_move_alarm_info(HI_S_MD_PARAM *bsm_motion, 
        JMoveAlarm *move_alarm, int flag)
{
    NMP_ASSERT(bsm_motion && move_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            move_alarm->move_enable     = bsm_motion->blEnable;
            move_alarm->sensitive_level = bsm_motion->u32Sensitivity;
            move_alarm->detect_area.count = bsm_motion->u32Area;
            move_alarm->detect_area.rect[0].left = bsm_motion->u32X;
            move_alarm->detect_area.rect[0].top = bsm_motion->u32Y;
            move_alarm->detect_area.rect[0].right = bsm_motion->u32Width;
            move_alarm->detect_area.rect[0].bottom = bsm_motion->u32Height;
            move_alarm->max_width       = 704;
            move_alarm->max_height      = 576;
            //bsm_process_sched_time(bsm_motion->stSect, &move_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            bsm_motion->u32Channel = HI_NET_DEV_CHANNEL_1;
            bsm_motion->blEnable  = (HI_BOOL)move_alarm->move_enable;
            bsm_motion->u32Sensitivity = move_alarm->sensitive_level;
            bsm_motion->u32Area = 1;
            bsm_motion->u32X = move_alarm->detect_area.rect[0].left;
            bsm_motion->u32Y = move_alarm->detect_area.rect[0].top;
            bsm_motion->u32Width = move_alarm->detect_area.rect[0].right;
            bsm_motion->u32Height = move_alarm->detect_area.rect[0].bottom;
            break;

        default:
            break;
    }

    return ;
}

void bsm_swap_ptz_info(HI_S_PTZ *ptz_cfg, 
        JPTZParameter *ptz_info, int flag)
{
//  int baud_rate[] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    NMP_ASSERT(ptz_cfg && ptz_info);

    switch (flag)
    {
        case SWAP_PACK:
            ptz_info->protocol  = ptz_cfg->u32Protocol;
            ptz_info->ptz_addr  = ptz_cfg->u32Address;
            ptz_info->baud_rate = ptz_cfg->u32Baud;
            ptz_info->data_bit  = ptz_cfg->u32DataBit;
            ptz_info->stop_bit  = ptz_cfg->u32StopBit;
            ptz_info->verify    = ptz_cfg->u32Parity;
            break;

        case SWAP_UNPACK:
            ptz_cfg->u32Protocol = ptz_info->protocol;
            ptz_cfg->u32Address  = ptz_info->ptz_addr;
            ptz_cfg->u32Baud     = ptz_info->baud_rate;
            ptz_cfg->u32DataBit  = ptz_info->data_bit;
            ptz_cfg->u32StopBit  = ptz_info->stop_bit;
            ptz_cfg->u32Parity   = ptz_info->verify;
            break;

        default:
            break;
    }

    printf("u32Address : %02d   |  ", ptz_cfg->u32Address);
    printf("ptz_addr     : %d\n", ptz_info->ptz_addr);
    printf("u32Protocol: %02d   |  ", ptz_cfg->u32Protocol);
    printf("protocol     : %d\n", ptz_info->protocol);
    printf("u32Baud    : %02d   |  ", ptz_cfg->u32Baud);
    printf("baud_rate    : %d\n", ptz_info->baud_rate);
    printf("u32DataBit : %02d   |  ", ptz_cfg->u32DataBit);
    printf("data_bit     : %d\n", ptz_info->data_bit);
    printf("u32StopBit : %02d   |  ", ptz_cfg->u32StopBit);
    printf("stop_bit     : %d\n", ptz_info->stop_bit);
    printf("u32Parity  : %02d   |  ", ptz_cfg->u32Parity);
    printf("verify       : %d\n", ptz_info->verify);
    return ;
}


