
#include "nmp_bsm_swap.h"
#include "nmp_bsm_service.h"
#include "nmp_bsm_handler.h"

int bsm_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceInfo *dev_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_DEVICE_INFO dev_cfg;

    NMP_ASSERT(srv && pvalue);

    dev_info = (JDeviceInfo*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&dev_cfg, 0, sizeof(HI_DEVICE_INFO));

    bsm_cfg.command = HI_NET_DEV_CMD_DEVICE_INFO;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &dev_cfg;
    bsm_cfg.b_size  = sizeof(HI_DEVICE_INFO);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_device_info(&dev_cfg, dev_info, SWAP_PACK);

    return ret;
}
int bsm_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_NET_EXT net_ext;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&net_ext, 0, sizeof(HI_S_NET_EXT));

    bsm_cfg.command = HI_NET_DEV_CMD_NET_EXT;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &net_ext;
    bsm_cfg.b_size  = sizeof(HI_S_NET_EXT);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
    {
        net_info->web_port = net_ext.sHttpPort.u32HttpPort;
        bsm_swap_network_info(&net_ext.sNetInfo, net_info, SWAP_PACK);
    }

    return ret;
}
int bsm_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *net_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_NET_EXT net_ext;

    NMP_ASSERT(srv && pvalue);

    net_info = (JNetworkInfo*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&net_ext, 0, sizeof(HI_S_NET_EXT));

    bsm_cfg.command = HI_NET_DEV_CMD_NET_EXT;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &net_ext;
    bsm_cfg.b_size  = sizeof(HI_S_NET_EXT);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        net_ext.sHttpPort.u32HttpPort = net_info->web_port;
        bsm_swap_network_info(&net_ext.sNetInfo, net_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, (void*)&bsm_cfg);
    }

    return ret;
}
int bsm_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JFTPParameter *ftp_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_FTP_PARAM ftp_cfg;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&ftp_cfg, 0, sizeof(HI_S_FTP_PARAM));

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    bsm_cfg.command = HI_NET_DEV_CMD_FTP_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &ftp_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_FTP_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, &bsm_cfg);
    if (!ret)
        bsm_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_PACK);
    
    return ret;
}
int bsm_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JFTPParameter *ftp_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_FTP_PARAM ftp_cfg;

    NMP_ASSERT(srv && pvalue);

    ftp_info = (JFTPParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&ftp_cfg, 0, sizeof(HI_S_FTP_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_FTP_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &ftp_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_FTP_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, &bsm_cfg);
    if (!ret)
    {
        bsm_swap_ftp_info(&ftp_cfg, ftp_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    return ret;
}
int bsm_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JSMTPParameter *smtp_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_EMAIL_PARAM email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&email_cfg, 0, sizeof(HI_S_EMAIL_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_EMAIL_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &email_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_EMAIL_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, &bsm_cfg);
    if (!ret)
        bsm_swap_smtp_info(&email_cfg, smtp_info, SWAP_PACK);

    return ret;
}
int bsm_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    JSMTPParameter *smtp_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_EMAIL_PARAM email_cfg;

    NMP_ASSERT(srv && pvalue);

    smtp_info = (JSMTPParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&email_cfg, 0, sizeof(HI_S_EMAIL_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_EMAIL_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &email_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_EMAIL_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, &bsm_cfg);
    if (!ret)
    {
        bsm_swap_smtp_info(&email_cfg, smtp_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    return ret;
}
int bsm_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_DNS_PARAM dns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&dns_cfg, 0, sizeof(HI_S_DNS_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_DNS_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &dns_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_DNS_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, &bsm_cfg);
    if (!ret)
        bsm_swap_ddns_info(&dns_cfg, ddns_info, SWAP_PACK);

    return ret;
}
int bsm_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDdnsConfig *ddns_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_DNS_PARAM dns_cfg;

    NMP_ASSERT(srv && pvalue);

    ddns_info = (JDdnsConfig*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&dns_cfg, 0, sizeof(HI_S_DNS_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_DNS_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &dns_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_DNS_PARAM);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, &bsm_cfg);
    if (!ret)
    {
        bsm_swap_ddns_info(&dns_cfg, ddns_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    return ret;
}
int bsm_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    JDeviceDiskInfo *disk_list;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_DISK_INFO disk_cfg;

    NMP_ASSERT(srv && pvalue);

    disk_list = (JDeviceDiskInfo*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&disk_cfg, 0, sizeof(HI_S_DISK_INFO));

    bsm_cfg.command = HI_NET_NVR_CMD_DISK_INFO;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &disk_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_DISK_INFO);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, &bsm_cfg);
    if (!ret)
        bsm_swap_disk_list(&disk_cfg, disk_list, SWAP_PACK);

    return ret;
}
int bsm_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
    JFormatDisk *fmt_disk;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_DISK_FORMAT disk_fmt;

    NMP_ASSERT(srv && pvalue);

    fmt_disk = (JFormatDisk*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&disk_fmt, 0, sizeof(HI_DISK_FORMAT));

    bsm_cfg.command = HI_NET_NVR_CMD_DISK_FORMAT;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &disk_fmt;
    bsm_cfg.b_size  = sizeof(HI_DISK_FORMAT);
    disk_fmt.s32DiskNum = fmt_disk->disk_no;

    return (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
}
int bsm_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
    bsm_service_basic_t *bsm_basic;

    NMP_ASSERT(srv && pvalue);

    bsm_basic = (bsm_service_basic_t*)srv->tm;

    return (*bsm_basic->ro.set_config)(srv, parm_id, pvalue);
}
int bsm_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_Video video_cfg;
    HI_S_Audio audio_cfg;
    HI_S_Resolution resolution;

    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&video_cfg, 0, sizeof(HI_S_Video));
    memset(&audio_cfg, 0, sizeof(HI_S_Audio));
    memset(&resolution, 0, sizeof(HI_S_Resolution));

    bsm_cfg.command = HI_NET_DEV_CMD_VIDEO_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &video_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Video);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_video_info(&video_cfg, enc_info, SWAP_PACK);

    bsm_cfg.command = HI_NET_DEV_CMD_AUDIO_INPUT;
    bsm_cfg.buffer  = &enc_info->au_in_mode;
    bsm_cfg.b_size  = sizeof(int);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
    {
        switch (enc_info->au_in_mode)
        {
            case AUDIO_INPUT_MIC:
                enc_info->au_in_mode = J_SDK_AUDIO_MIC_IN;
                break;
            case AUDIO_INPUT_LINE:
                enc_info->au_in_mode = J_SDK_AUDIO_LINE_IN;
                break;
        }
    }

    bsm_cfg.command = HI_NET_DEV_CMD_FREQUENCY;
    bsm_cfg.buffer  = &enc_info->format;
    bsm_cfg.b_size  = sizeof(int);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
    {
        switch (enc_info->format)
        {
            case FREQ_50HZ_PAL:
                enc_info->format = J_SDK_PAL;
                break;
            case FREQ_60HZ_NTSC:
                enc_info->format = J_SDK_NTSC;
                break;
        }
    }

    bsm_cfg.command = HI_NET_DEV_CMD_AUDIO_PARAM;
    bsm_cfg.buffer  = &audio_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Audio);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_audio_info(&audio_cfg, enc_info, SWAP_PACK);

    bsm_cfg.command = HI_NET_DEV_CMD_RESOLUTION;
    bsm_cfg.buffer  = &resolution;
    bsm_cfg.b_size  = sizeof(HI_S_Resolution);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
    {
        switch (resolution.u32Resolution)
        {
        case HI_NET_DEV_RESOLUTION_VGA:
            enc_info->resolution = J_SDK_VIDEO_VGA;
            break;
        case HI_NET_DEV_RESOLUTION_QVGA:
            enc_info->resolution = J_SDK_VIDEO_QVGA;
            break;
        case HI_NET_DEV_RESOLUTION_QQVGA:
            enc_info->resolution = J_SDK_VIDEO_QQVGA;
            break;
        case HI_NET_DEV_RESOLUTION_D1:
            enc_info->resolution = J_SDK_VIDEO_D1;
            break;
        case HI_NET_DEV_RESOLUTION_CIF:
            enc_info->resolution = J_SDK_VIDEO_CIF;
            break;
        case HI_NET_DEV_RESOLUTION_QCIF:
            enc_info->resolution = J_SDK_VIDEO_QCIF;
            break;
        case HI_NET_DEV_RESOLUTION_720P:
            enc_info->resolution = J_SDK_VIDEO_720P;
            break;
        }
    }

    return ret;
}
int bsm_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JEncodeParameter *enc_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_Video video_cfg;
    HI_S_Audio audio_cfg;
    HI_S_Resolution resolution;

    NMP_ASSERT(srv && pvalue);

    enc_info = (JEncodeParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&video_cfg, 0, sizeof(HI_S_Video));
    memset(&audio_cfg, 0, sizeof(HI_S_Audio));
    memset(&resolution, 0, sizeof(HI_S_Resolution));

    bsm_cfg.command = HI_NET_DEV_CMD_VIDEO_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &video_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Video);
    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_video_info(&video_cfg, enc_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    if (ret)
        goto END;

    bsm_cfg.command = HI_NET_DEV_CMD_AUDIO_PARAM;
    bsm_cfg.buffer  = &audio_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Audio);
    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_audio_info(&audio_cfg, enc_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    if (ret)
        goto END;

    bsm_cfg.command = HI_NET_DEV_CMD_AUDIO_INPUT;
    bsm_cfg.buffer  = &enc_info->au_in_mode;
    bsm_cfg.b_size  = sizeof(int);
    switch (enc_info->au_in_mode)
    {
        case J_SDK_AUDIO_MIC_IN:
            enc_info->au_in_mode = AUDIO_INPUT_MIC;
            break;
        case J_SDK_AUDIO_LINE_IN:
            enc_info->au_in_mode = AUDIO_INPUT_LINE;
            break;
    }
    ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);

    if (ret)
        goto END;

    bsm_cfg.command = HI_NET_DEV_CMD_FREQUENCY;
    bsm_cfg.buffer  = &enc_info->format;
    bsm_cfg.b_size  = sizeof(int);

    switch (enc_info->format)
    {
        case J_SDK_PAL:
            enc_info->format = FREQ_50HZ_PAL;
            break;
        case J_SDK_NTSC:
            enc_info->format = FREQ_60HZ_NTSC;
            break;
    }
    ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);

    if (ret)
        goto END;

    bsm_cfg.command = HI_NET_DEV_CMD_RESOLUTION;
    bsm_cfg.buffer  = &resolution;
    bsm_cfg.b_size  = sizeof(HI_S_Resolution);
    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        switch (enc_info->resolution)
        {
        case J_SDK_VIDEO_VGA:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_VGA;
            break;
        case J_SDK_VIDEO_QVGA:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_QVGA;
            break;
        case J_SDK_VIDEO_QQVGA:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_QQVGA;
            break;
        case J_SDK_VIDEO_D1:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_D1;
            break;
        case J_SDK_VIDEO_CIF:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_CIF;
            break;
        case J_SDK_VIDEO_QCIF:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_QCIF;
            break;
        case J_SDK_VIDEO_720P:
            resolution.u32Resolution = HI_NET_DEV_RESOLUTION_720P;
            break;
        }
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

END:
    return ret;
}
int bsm_get_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_Display dis_cfg;
    HI_S_Sharpness sharp;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&dis_cfg, 0, sizeof(HI_S_Display));
    memset(&sharp, 0, sizeof(HI_S_Sharpness));

    bsm_cfg.command = HI_NET_DEV_CMD_DISPLAY;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &dis_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Display);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_display_info(&dis_cfg, dis_info, SWAP_PACK);

    /*bsm_cfg.command = ;
    bsm_cfg.buffer  = &sharp;
    bsm_cfg.b_size  = sizeof(HI_S_Sharpness);
    if (!(*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg))
        dis_info->sharpness = sharp.u32Sharpness;*/

    return ret;
}
int bsm_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDisplayParameter *dis_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_Display dis_cfg;

    NMP_ASSERT(srv && pvalue);

    dis_info = (JDisplayParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&dis_cfg, 0, sizeof(HI_S_Display));

    bsm_cfg.command = HI_NET_DEV_CMD_DISPLAY;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &dis_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_Display);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_display_info(&dis_cfg, dis_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    return ret;
}
int bsm_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_OSD osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&osd_cfg, 0, sizeof(HI_S_OSD));

    bsm_cfg.command = HI_NET_DEV_CMD_OSD_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &osd_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_OSD);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_osd_info(&osd_cfg, osd_info, SWAP_PACK);

    return ret;
}
int bsm_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JOSDParameter *osd_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_OSD osd_cfg;

    NMP_ASSERT(srv && pvalue);

    osd_info = (JOSDParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&osd_cfg, 0, sizeof(HI_S_OSD));

    bsm_cfg.command = HI_NET_DEV_CMD_OSD_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &osd_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_OSD);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_osd_info(&osd_cfg, osd_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);
    }

    return ret;
}
int bsm_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&ptz_cfg, 0, sizeof(HI_S_PTZ));

    bsm_cfg.command  = HI_NET_DEV_CMD_PTZ_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer   = &ptz_cfg;
    bsm_cfg.b_size   = sizeof(HI_S_PTZ);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_PACK);

    return ret;
}
int bsm_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JPTZParameter *ptz_info;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_PTZ ptz_cfg;

    NMP_ASSERT(srv && pvalue);

    ptz_info = (JPTZParameter*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&ptz_cfg, 0, sizeof(HI_S_PTZ));

    bsm_cfg.command = HI_NET_DEV_CMD_PTZ_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &ptz_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_PTZ);

    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_ptz_info(&ptz_cfg, ptz_info, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, (void*)&bsm_cfg);
    }

    return ret;
}
int bsm_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_MD_PARAM alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&alarm_cfg, 0, sizeof(HI_S_MD_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_MD_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &alarm_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_MD_PARAM);

    alarm_cfg.u32Channel = HI_NET_DEV_CHANNEL_1;
    ret = (*bsm_basic->ro.get_config)(srv, parm_id, (void*)&bsm_cfg);
    if (!ret)
        bsm_swap_move_alarm_info(&alarm_cfg, motion, SWAP_PACK);

    return ret;
}
int bsm_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JMoveAlarm *motion;
    bsm_service_basic_t *bsm_basic;

    bsm_config_t bsm_cfg;
    HI_S_MD_PARAM alarm_cfg;

    NMP_ASSERT(srv && pvalue);

    motion = (JMoveAlarm*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&alarm_cfg, 0, sizeof(HI_S_MD_PARAM));

    bsm_cfg.command = HI_NET_DEV_CMD_MD_PARAM;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &alarm_cfg;
    bsm_cfg.b_size  = sizeof(HI_S_MD_PARAM);

    alarm_cfg.u32Channel = HI_NET_DEV_CHANNEL_1;
    ret = (*bsm_basic->ro.get_config)(srv, parm_id-1, (void*)&bsm_cfg);
    if (!ret)
    {
        bsm_swap_move_alarm_info(&alarm_cfg, motion, SWAP_UNPACK);
        ret = (*bsm_basic->ro.set_config)(srv, parm_id, (void*)&bsm_cfg);
    }

    return ret;
}
int bsm_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = -1;
    int i, count;
    JPTZControl *ptz_ctrl;
    bsm_service_basic_t *bsm_basic;

    int ptz_cmd1[] = {PTZ_STOP, PTZ_LEFT, PTZ_RIGHT, PTZ_UP, PTZ_DOWN, PTZ_ADD_ZOOM, 
                        PTZ_SUB_ZOOM, PTZ_ADD_FOCUS, PTZ_SUB_FOCUS};
    int ptz_cmd2[] = {PTZ_USE_PP, PTZ_SET_PP, PTZ_DEL_PP};
    int ptz_cmd3[] = {PTZ_TURN_ON, PTZ_TURN_OFF, PTZ_WIPERS_ON, PTZ_WIPERS_OFF, PTZ_AUTO};

    int std_cmd[] = {HI_NET_DEV_CTRL_PTZ_STOP, 
            HI_NET_DEV_CTRL_PTZ_LEFT, HI_NET_DEV_CTRL_PTZ_RIGHT, 
            HI_NET_DEV_CTRL_PTZ_UP, HI_NET_DEV_CTRL_PTZ_DOWN, 
            HI_NET_DEV_CTRL_PTZ_ZOOMOUT, HI_NET_DEV_CTRL_PTZ_ZOOMIN, 
            HI_NET_DEV_CTRL_PTZ_FOCUSOUT, HI_NET_DEV_CTRL_PTZ_FOCUSIN};
    int prs_cmd[] = {HI_NET_DEV_CTRL_PTZ_GOTO_PRESET, HI_NET_DEV_CTRL_PTZ_SET_PRESET, 
            HI_NET_DEV_CTRL_PTZ_CLE_PRESET};
    int ext_cmd[] = {HI_NET_DEV_CTRL_PTZ_LIGHT_ON, HI_NET_DEV_CTRL_PTZ_LIGHT_OFF, 
            HI_NET_DEV_CTRL_PTZ_WIPER_ON, HI_NET_DEV_CTRL_PTZ_WIPER_OFF, HI_NET_DEV_CTRL_PTZ_AUTO_ON};

    bsm_config_t bsm_cfg;
    bsm_ptz_ctrl_std_t ctrl_std;
    bsm_ptz_ctrl_prs_t ctrl_prs;
    bsm_ptz_ctrl_ext_t ctrl_ext;

    NMP_ASSERT(srv && pvalue);

    ptz_ctrl = (JPTZControl*)pvalue;
    bsm_basic = (bsm_service_basic_t*)srv->tm;

    memset(&bsm_cfg, 0, sizeof(bsm_config_t));
    memset(&ctrl_std, 0, sizeof(bsm_ptz_ctrl_std_t));
    memset(&ctrl_prs, 0, sizeof(bsm_ptz_ctrl_prs_t));
    memset(&ctrl_ext, 0, sizeof(bsm_ptz_ctrl_ext_t));

    bsm_cfg.command = BSM_PTZ_CTRL_STD;
    bsm_cfg.channel = channel;
    bsm_cfg.buffer  = &ctrl_std;
    bsm_cfg.b_size  = sizeof(bsm_ptz_ctrl_std_t);

    count = sizeof(ptz_cmd1)/sizeof(int);
    for (i=0; i<count; i++)
    {
        if (ptz_cmd1[i] == (int)ptz_ctrl->action)
        {
            ctrl_std.cmd = std_cmd[i];
            ctrl_std.speed = HI_NET_DEV_CTRL_PTZ_SPEED_MAX/2;
            goto SET_PARM;
        }
    }

    bsm_cfg.command = BSM_PTZ_CTRL_PRS;
    bsm_cfg.buffer  = &ctrl_prs;
    bsm_cfg.b_size  = sizeof(bsm_ptz_ctrl_prs_t);

    count = sizeof(ptz_cmd2)/sizeof(int);
    for (i=0; i<count; i++)
    {
        if (ptz_cmd2[i] == (int)ptz_ctrl->action)
        {
            ctrl_prs.cmd = prs_cmd[i];
            ctrl_prs.preset = ptz_ctrl->param;
            goto SET_PARM;
        }
    }

    bsm_cfg.command = BSM_PTZ_CTRL_EXT;
    bsm_cfg.buffer  = &ctrl_ext;
    bsm_cfg.b_size  = sizeof(bsm_ptz_ctrl_ext_t);

    count = sizeof(ptz_cmd3)/sizeof(int);
    for (i=0; i<count; i++)
    {
        if (ptz_cmd3[i] == (int)ptz_ctrl->action)
        {
            ctrl_ext.cmd = ext_cmd[i];
            goto SET_PARM;
        }
    }

    return ret;

SET_PARM:
    return (*bsm_basic->ro.set_config)(srv, parm_id, &bsm_cfg);

}
int bsm_get_capability_set(struct service *srv, int channel, int parm_id, void *pvalue)
{
    bsm_service_basic_t *bsm_basic;

    NMP_ASSERT(srv && pvalue);

    bsm_basic = (bsm_service_basic_t*)srv->tm;

    return (*bsm_basic->ro.get_config)(srv, parm_id, pvalue);
}


