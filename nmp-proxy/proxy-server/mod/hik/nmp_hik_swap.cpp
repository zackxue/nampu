
#include <iconv.h>

#include "nmp_hik_swap.h"
//#include "cfg/nmp_config_info.h"


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
hik_process_version(char *nmp_version, size_t size, int *h_version, int flag)
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
hik_process_build_date(char *nmp_date, size_t size, int *h_date, int flag)
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

//代码转换:从一种编码转为另一种编码
static int code_convert(char *from_charset, char *to_charset, 
    char *inbuf, int inlen, char *outbuf, int outlen)
{
    int ret = -1;
    iconv_t cd;

    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0)
        return ret;

    memset(outbuf, 0, outlen);
    ret = iconv(cd, pin, (size_t*)&inlen, pout, (size_t*)&outlen);
    iconv_close(cd);

    return ret;
}

static __inline__ void
hik_process_sched_time(NET_DVR_SCHEDTIME (*hik_sched_time)[MAX_TIMESEGMENT_V30], 
        JWeek *week, int flag)
{
    int day_index, time_seg_index;
    JTime *time_start, *time_end;
    NET_DVR_SCHEDTIME *hik_alarm_time;
    
    switch (flag)
    {
        case SWAP_PACK:         
            for (day_index=0; day_index<MAX_DAYS; day_index++)
            {
            printf("day: %d\n", day_index);
                for (time_seg_index=0; time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/; 
                    time_seg_index++)
                {
                    week->days[day_index].seg[time_seg_index].enable = 1;
                    
                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);
                    
                    hik_alarm_time = &hik_sched_time[day_index][time_seg_index];
                    
                    if (hik_alarm_time)
                    {
                        time_start->hour   = (int)hik_alarm_time->byStartHour;
                        time_start->minute = (int)hik_alarm_time->byStartMin;
                        
                        time_end->hour   = (int)hik_alarm_time->byStopHour;
                        time_end->minute = (int)hik_alarm_time->byStopMin;

                        printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                            hik_alarm_time->byStartHour, hik_alarm_time->byStartMin, 
                            hik_alarm_time->byStopHour, hik_alarm_time->byStopMin);
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
            for (day_index=0; day_index<MAX_DAYS; day_index++)
            {
            printf("UNPACK--->day: %d\n", day_index);
                int seg = week->days[day_index].count;
                for (time_seg_index=0; (time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/ && 0 < seg--); 
                    time_seg_index++)
                {
                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);
                    
                    hik_alarm_time = &hik_sched_time[day_index][time_seg_index];
                    
                    hik_alarm_time->byStartHour = (BYTE)time_start->hour;
                    hik_alarm_time->byStartMin  = (BYTE)time_start->minute;
                    hik_alarm_time->byStopHour  = (BYTE)time_end->hour;
                    hik_alarm_time->byStopMin   = (BYTE)time_end->minute;
                    
                    printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                        hik_alarm_time->byStartHour, hik_alarm_time->byStartMin, 
                        hik_alarm_time->byStopHour, hik_alarm_time->byStopMin);
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

void hik_swap_device_info(NET_DVR_DEVICECFG_V40 *dev_cfg, 
        JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            hik_process_version((char*)dev_info->dev_version, 
                sizeof(dev_info->dev_version),
                (int*)&dev_cfg->dwSoftwareVersion, PACK_SOFTWARE);

            hik_process_version((char*)dev_info->hw_version, 
                sizeof(dev_info->hw_version),
                (int*)&dev_cfg->dwHardwareVersion, PACK_HARDEARE);
            
            hik_process_build_date((char*)dev_info->release_date, 
                sizeof(dev_info->release_date), 
                (int*)&dev_cfg->dwSoftwareBuildDate, PACK_BUILDDATE);
            
            dev_info->pu_type      = (JPuType)dev_cfg->byDVRType;
            dev_info->di_num       = (int)dev_cfg->byAlarmInPortNum;
            dev_info->do_num       = (int)dev_cfg->byAlarmOutPortNum;
            if (194 == (int)dev_cfg->byDVRType)
                dev_info->channel_num  = (int)dev_cfg->byIPChanNum;
            else
                dev_info->channel_num  = (int)dev_cfg->byChanNum;
            //dev_info->rs485_num    = (int)dev_cfg->byRS485Num;
            dev_info->rs232_num    = (int)dev_cfg->byRS232Num;
            break;
            
        case SWAP_UNPACK:
            hik_process_version((char*)dev_info->dev_version, 
                sizeof(dev_info->dev_version),
                (int*)&dev_cfg->dwSoftwareVersion, UNPACK_SOFTWARE);

            hik_process_version((char*)dev_info->hw_version, 
                sizeof(dev_info->hw_version),
                (int*)&dev_cfg->dwHardwareVersion, UNPACK_HARDEARE);
            
            hik_process_build_date((char*)dev_info->release_date, 
                sizeof(dev_info->release_date), 
                (int*)&dev_cfg->dwSoftwareBuildDate, UNPACK_BUILDDATE);
            
            dev_cfg->byDVRType         = dev_info->pu_type;
            dev_cfg->byAlarmInPortNum  = dev_info->di_num;
            dev_cfg->byAlarmOutPortNum = dev_info->do_num;
            dev_cfg->byChanNum         = dev_info->channel_num;
            dev_cfg->byRS485Num        = dev_info->rs485_num;
            dev_cfg->byRS232Num        = dev_info->rs232_num;
            break;
            
        default:
            break;
    }
    
    printf("dwSoftwareVersion  : 0x%08x  |  ", dev_cfg->dwSoftwareVersion);
    printf("dev_version : %s\n", dev_info->dev_version);
    printf("dwHardwareVersion  : 0x%08x  |  ", dev_cfg->dwHardwareVersion);
    printf("hw_version  : %s\n", dev_info->hw_version);
    printf("dwSoftwareBuildDate: 0x%08x  |  ", dev_cfg->dwSoftwareBuildDate);
    printf("release_date: %s\n", dev_info->release_date);

    printf("byDVRType        : %02d  |  ", (int)dev_cfg->byDVRType);
    printf("pu_type  : %d\n", dev_info->pu_type);
    printf("byAlarmInPortNum : %02d  |  ", (int)dev_cfg->byAlarmInPortNum);
    printf("di_num   : %d\n", dev_info->di_num);
    printf("byAlarmOutPortNum: %02d  |  ", (int)dev_cfg->byAlarmOutPortNum);
    printf("do_num   : %d\n", dev_info->do_num);
    printf("byChanNum        : %02d  |  ", (int)dev_cfg->byChanNum);
    printf("chan_num : %d\n", dev_info->channel_num);
    printf("byRS485Num       : %02d  |  ", (int)dev_cfg->byRS485Num);
    printf("rs485_num: %d\n", dev_info->rs485_num);
    printf("byRS232Num       : %02d  |  ", (int)dev_cfg->byRS232Num);
    printf("rs232_num: %d\n", dev_info->rs232_num);
    
    printf("byStartChan: %d\n", dev_cfg->byStartChan);
    printf("byIPChanNum: %d\n", dev_cfg->byIPChanNum);

    return ;
}

void hik_swap_encode_info(NET_DVR_COMPRESSION_INFO_V30 *compress_info, 
        JEncodeParameter *encode_para, int flag)
{
    int i, align;
    int video_bitrate_map[] = 
            {0, 16, 32, 48, 64, 80, 96, 128, 160, 192, 224, 256, 320, 384, 448, 512, 
            640, 768, 896, 1024, 1280, 1536, 1792, 2048};//, 3072, 4096, 8192, 16384};
    int video_frame_rate[] = 
        {0, 1, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56 ,60};
    
    NMP_ASSERT(compress_info && encode_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            if (16 > compress_info->dwVideoFrameRate && 0 <= compress_info->dwVideoFrameRate)
                encode_para->frame_rate = video_frame_rate[compress_info->dwVideoFrameRate];
            
            encode_para->i_frame_interval = compress_info->wIntervalFrameI;
            
            switch (compress_info->byVideoEncType)
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
            }

            switch (compress_info->byAudioEncType)
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
            }
            
            encode_para->resolution = compress_info->byResolution;  //<--todo
            encode_para->qp_value   = compress_info->byPicQuality;
            
            if (/*27*/23 >= compress_info->dwVideoBitrate && 1 < compress_info->dwVideoBitrate)
                encode_para->code_rate = video_bitrate_map[compress_info->dwVideoBitrate];
            
            if (J_SDK_CBR == compress_info->byBitrateType)
                encode_para->bit_rate = J_SDK_CBR;
            else
                encode_para->bit_rate = J_SDK_VBR;
            break;
            
        case SWAP_UNPACK:
            switch (encode_para->frame_rate)
            {
                case 0:
                    compress_info->dwVideoFrameRate = 0;
                    break;
                case 1:
                    compress_info->dwVideoFrameRate = 1;
                    break;
                default:
                    align = ALIGN(encode_para->frame_rate, 4);
                    compress_info->dwVideoFrameRate = align/4 + 1;
                    break;
            }

            compress_info->wIntervalFrameI = encode_para->i_frame_interval;
            
            switch (encode_para->video_type)
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
            }
            
            if (0 == encode_para->audio_enble)
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
            }
            
            compress_info->byResolution = encode_para->resolution;
            compress_info->byPicQuality = encode_para->qp_value;

            for (i=0; i</*27*/23; i++)
            {
                if (video_bitrate_map[i] >= (int)encode_para->code_rate)
                {
//                  compress_info->dwVideoBitrate = i;
                    break;
                }
            }
            compress_info->dwVideoBitrate = i;

            if (J_SDK_CBR == encode_para->bit_rate)
                compress_info->byBitrateType = J_SDK_CBR;
            else
                compress_info->byBitrateType = 0;
            break;
            
        default:
            break;
    }
    
    printf("dwVideoFrameRate: %02d  |  ", compress_info->dwVideoFrameRate);
    printf("frame_rate      : %d\n", encode_para->frame_rate);
    printf("wIntervalFrameI : %02d  |  ", compress_info->wIntervalFrameI);
    printf("i_frame_interval: %d\n", encode_para->i_frame_interval);
    printf("byVideoEncType  : %02d  |  ", compress_info->byVideoEncType);
    printf("video_type      : %d\n", encode_para->video_type);
    printf("byAudioEncType  : %02d  |  ", compress_info->byAudioEncType);
    printf("audio_type      : %d\n", encode_para->audio_type);
    printf("byResolution    : %02d  |  ", compress_info->byResolution);
    printf("resolution      : %d\n", encode_para->resolution);
    printf("byPicQuality    : %02d  |  ", compress_info->byPicQuality);
    printf("qp_value        : %d\n", encode_para->qp_value);
    printf("dwVideoBitrate  : %02d  |  ", compress_info->dwVideoBitrate);
    printf("code_rate       : %d\n", encode_para->code_rate);
    printf("byBitrateType   : %02d  |  ", compress_info->byBitrateType);
    printf("bit_rate        : %d\n", encode_para->bit_rate);

    return ;
}

void hik_swap_display_info(NET_DVR_VIDEOEFFECT *video_effect, 
        JDisplayParameter *display, int flag)
{
    NMP_ASSERT(video_effect && display);

    switch (flag)
    {
        case SWAP_PACK:
            display->bright     = video_effect->byBrightnessLevel;
            display->contrast   = video_effect->byContrastLevel;
            display->saturation = video_effect->bySaturationLevel;
            display->hue        = video_effect->byHueLevel;
            display->sharpness  = video_effect->bySharpnessLevel;
            break;
            
        case SWAP_UNPACK:
            video_effect->byBrightnessLevel = display->bright;
            video_effect->byContrastLevel   = display->contrast;
            video_effect->bySaturationLevel = display->saturation;
            video_effect->byHueLevel        = display->hue;
            video_effect->bySharpnessLevel = display->sharpness;
            break;
            
        default:
            break;
    }
    printf("byBrightness: %02d  |  ", video_effect->byBrightnessLevel);
    printf("bright    : %d\n", display->bright);
    printf("byContrast  : %02d  |  ", video_effect->byContrastLevel);
    printf("contrast  : %d\n", display->contrast);
    printf("bySaturation: %02d  |  ", video_effect->bySaturationLevel);
    printf("saturation: %d\n", display->saturation);
    printf("byHue       : %02d  |  ", video_effect->byHueLevel);
    printf("hue       : %d\n", display->hue);

    return ;
}

void hik_swap_video_effect(hik_video_effect_t *video_effect, 
        JDisplayParameter *display, int flag)
{
    NMP_ASSERT(video_effect && display);

    switch (flag)
    {
        case SWAP_PACK:
            display->bright     = video_effect->bright;
            display->contrast   = video_effect->contrast;
            display->saturation = video_effect->saturation;
            display->hue        = video_effect->hue;
            break;
            
        case SWAP_UNPACK:
            video_effect->bright     = display->bright;
            video_effect->contrast   = display->contrast;
            video_effect->saturation = display->saturation;
            video_effect->hue        = display->hue;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_osd_info(NET_DVR_PICCFG_V30 *pic_cfg, 
        JOSDParameter *osd_para, int flag)
{
    int out_len;
    char *out_buf;

    NMP_ASSERT(pic_cfg && osd_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            osd_para->time_enable    = pic_cfg->dwShowOsd;
            osd_para->time_display_x = pic_cfg->wOSDTopLeftX;
            osd_para->time_display_y = pic_cfg->wOSDTopLeftY;
            osd_para->text_enable    = pic_cfg->dwShowChanName;
            osd_para->text_display_x = pic_cfg->wShowNameTopLeftX;
            osd_para->text_display_y = pic_cfg->wShowNameTopLeftY;
            osd_para->max_width      = 704;
            osd_para->max_height     = 576;
            
            out_buf = (char*)osd_para->text_data;
            out_len = sizeof(osd_para->text_data)-1;
            code_convert((char*)"gb2312", (char*)"utf-8", 
                    (char*)pic_cfg->sChanName, 
                    strlen((const char*)pic_cfg->sChanName), 
                    out_buf, out_len);
            break;
            
        case SWAP_UNPACK:
            pic_cfg->dwShowOsd         = osd_para->time_enable;
            pic_cfg->wOSDTopLeftX      = osd_para->time_display_x;
            pic_cfg->wOSDTopLeftY      = osd_para->time_display_y;
            pic_cfg->dwShowChanName    = osd_para->text_enable;
            pic_cfg->wShowNameTopLeftX = osd_para->text_display_x;
            pic_cfg->wShowNameTopLeftY = osd_para->text_display_y;
            
            out_buf = (char*)pic_cfg->sChanName;
            out_len = NAME_LEN - 1;
            code_convert((char*)"utf-8", (char*)"gb2312", 
                    (char*)osd_para->text_data, 
                    strlen((const char*)osd_para->text_data), 
                    out_buf, out_len);
            break;
            
        default:
            break;
    }
    
    printf("dwShowOsd        : %02d  |  ", pic_cfg->dwShowOsd);
    printf("display_time  : %d\n", osd_para->time_enable);
    printf("wOSDTopLeftX     : %02d  |  ", pic_cfg->wOSDTopLeftX);
    printf("time_display_x: %d\n", osd_para->time_display_x);
    printf("wOSDTopLeftY     : %02d  |  ", pic_cfg->wOSDTopLeftY);
    printf("time_display_y: %d\n", osd_para->time_display_y);
    printf("dwShowChanName   : %02d  |  ", pic_cfg->dwShowChanName);
    printf("display_text  : %d\n", osd_para->text_enable);
    printf("wShowNameTopLeftX: %02d  |  ", pic_cfg->wShowNameTopLeftX);
    printf("text_display_x: %d\n", osd_para->text_display_x);
    printf("wShowNameTopLeftY: %02d  |  ", pic_cfg->wShowNameTopLeftY);
    printf("text_display_y: %d\n", osd_para->text_display_y);
    
    return ;
}

void hik_swap_record_info(NET_DVR_RECORD_V30 *record_cfg, 
        JRecordParameter *record_para, int flag)
{
    int day_index, time_seg_index;
    JTime *time_start, *time_end;

    NET_DVR_SCHEDTIME *hik_sched_time;
    
    NMP_ASSERT(record_cfg && record_para);
    
    switch (flag)
    {
        case SWAP_PACK:
            switch (record_cfg->dwPreRecordTime)
            {
                case 0:
                    record_para->pre_record = 0;
                    break;
                case 1:
                    record_para->pre_record = 5;
                    break;
                case 2:
                    record_para->pre_record = 10;
                    break;
                case 3:
                    record_para->pre_record = 15;
                    break;
                case 4:
                    record_para->pre_record = 20;
                    break;
                case 5:
                    record_para->pre_record = 25;
                    break;
                case 6:
                    record_para->pre_record = 30;
                    break;
                case 7:
                    record_para->pre_record = 35;
                    break;
                default:
                    break;
            }
            printf("dwPreRecordJTime: %02d       |  ", record_cfg->dwPreRecordTime);
            printf("pre_record: %d\n", record_para->pre_record);
            
            for (day_index=0; day_index<MAX_DAYS; day_index++)
            {
            printf("day: %d\n", day_index);
                for(time_seg_index=0; time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/; time_seg_index++)
                {
                    record_para->week.days[day_index].seg[time_seg_index].enable = 1;
                    
                    time_start = &record_para->week.days[day_index]\
                                    .seg[time_seg_index].time_start;
                    time_end = &record_para->week.days[day_index]\
                                    .seg[time_seg_index].time_end;
                    
                    hik_sched_time = &record_cfg->struRecordSched\
                                        [day_index][time_seg_index].struRecordTime;
                    
                    time_start->hour = hik_sched_time->byStartHour;
                    time_start->minute = hik_sched_time->byStartMin;
                    
                    time_end->hour = hik_sched_time->byStopHour;
                    time_end->minute = hik_sched_time->byStopMin;
                    
                    printf("enable    : %d\n", record_para->week\
                        .days[day_index].seg[time_seg_index].enable);
                    printf("SchedTime : %02d:%02d - %02d:%02d  |  ", 
                        hik_sched_time->byStartHour, hik_sched_time->byStartMin, 
                        hik_sched_time->byStopHour, hik_sched_time->byStopMin);
                    printf("time_seg  : %02d:%02d - %02d:%02d\n", 
                        time_start->hour, time_start->minute, 
                        time_end->hour, time_end->minute);
                }
                record_para->week.days[day_index].day_id = day_index;
                record_para->week.days[day_index].count = time_seg_index;
                record_para->week.days[day_index].all_day_enable = 
                    record_cfg->struRecAllDay[day_index].wAllDayRecord;
            }
            record_para->week.count = day_index;
            break;
            
        case SWAP_UNPACK:
            printf("dwPreRecordJTime: %02d       |  ", record_cfg->dwPreRecordTime);
            printf("pre_record: %d\n", record_para->pre_record);

            if (0 < record_para->week.count)
                record_cfg->dwRecord = 1;
            else
                record_cfg->dwRecord = 0;
            
            if (0 == record_para->pre_record)
                record_cfg->dwPreRecordTime = 0;
            else if (5 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 1;
            else if (10 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 2;
            else if (15 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 3;
            else if (20 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 4;
            else if (25 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 5;
            else if (30 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 6;
            else if (35 >= record_para->pre_record)
                record_cfg->dwPreRecordTime = 7;

            memset(record_cfg->struRecAllDay, 0, sizeof(record_cfg->struRecAllDay));
            memset(record_cfg->struRecordSched, 0, sizeof(record_cfg->struRecordSched));
            
            for (day_index=0; day_index<MAX_DAYS; day_index++)
            {
            printf("UNPACK--->day: %d\n", day_index);
                for(time_seg_index=0; time_seg_index<J_SDK_MAX_SEG_SZIE/*MAX_TIMESEGMENT_V30*/; time_seg_index++)
                {
                    time_start = &record_para->week.days[day_index]\
                        .seg[time_seg_index].time_start;
                    time_end = &record_para->week.days[day_index]\
                        .seg[time_seg_index].time_end;
                    hik_sched_time = &(record_cfg->struRecordSched\
                        [day_index][time_seg_index].struRecordTime);
                    
                    hik_sched_time->byStartHour = time_start->hour;
                    hik_sched_time->byStartMin = time_start->minute;
                    
                    hik_sched_time->byStopHour = time_end->hour;
                    hik_sched_time->byStopMin = time_end->minute;
                    
                    printf("enable    : %d\n", record_para->week\
                        .days[day_index].seg[time_seg_index].enable);
                    printf("SchedTime : %02d:%02d - %02d:%02d  |  ", 
                        hik_sched_time->byStartHour, hik_sched_time->byStartMin, 
                        hik_sched_time->byStopHour, hik_sched_time->byStopMin);
                    printf("time_seg  : %02d:%02d - %02d:%02d\n", 
                        time_start->hour, time_start->minute, 
                        time_end->hour, time_end->minute);
                }
                //record_para->week.days[day_index].count = time_seg_index;
                
                record_cfg->struRecAllDay[day_index].wAllDayRecord = 
                    record_para->week.days[day_index].all_day_enable;
            }
            //record_para->week.count = day_index;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_move_alarm_info(NET_DVR_MOTION_V30 *hik_motion, 
        JMoveAlarm *move_alarm, int flag)
{
    int i, j;
    NMP_ASSERT(hik_motion && move_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            move_alarm->move_enable     = hik_motion->byEnableHandleMotion;
            move_alarm->sensitive_level = hik_motion->byMotionSensitive;
            move_alarm->max_width       = 704;
            move_alarm->max_height      = 576;
            for (i=0; i<64; i++)
            {
                for (j=0; j<96; j++)
                {
                    if (hik_motion->byMotionScope[i][j])
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
            hik_process_sched_time(hik_motion->struAlarmTime, &move_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            hik_motion->byEnableHandleMotion = move_alarm->move_enable;
            hik_motion->byMotionSensitive = move_alarm->sensitive_level;
            printf("area.count: %d\n", move_alarm->detect_area.count);
            memset(hik_motion->byMotionScope, 0, sizeof(hik_motion->byMotionScope));
            if (0 < move_alarm->detect_area.count)
            {
                for (i=0; i<64; i++)
                {
                    for (j=0; j<96; j++)
                        hik_motion->byMotionScope[i][j] = 1;
                }
            }
            memset(hik_motion->struAlarmTime, 0, sizeof(hik_motion->struAlarmTime));
            hik_process_sched_time(hik_motion->struAlarmTime, &move_alarm->week, flag);
            break;
            
        default:
            break;
    }
    printf("byEnableHandleMotion: %02d   |  ", hik_motion->byEnableHandleMotion);
    printf("move_enable    : %d\n", move_alarm->move_enable);
    printf("byEnableHandleMotion: %02d   |  ", hik_motion->byMotionSensitive);
    printf("sensitive_level: %d\n", move_alarm->sensitive_level);
    
    return ;
}

void hik_swap_video_lost_info(NET_DVR_VILOST_V30 *hik_video_lost, 
        JLostAlarm *lost_alarm, int flag)
{
    NMP_ASSERT(hik_video_lost && lost_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            lost_alarm->lost_enable = hik_video_lost->byEnableHandleVILost;
            hik_process_sched_time(hik_video_lost->struAlarmTime, &lost_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            hik_video_lost->byEnableHandleVILost = lost_alarm->lost_enable;
            memset(hik_video_lost->struAlarmTime, 0, sizeof(hik_video_lost->struAlarmTime));
            hik_process_sched_time(hik_video_lost->struAlarmTime, &lost_alarm->week, flag);         
            break;
            
        default:
            break;
    }
    printf("byEnableHandleVILost: %02d   |  ", hik_video_lost->byEnableHandleVILost);
    printf("lost_enable    : %d\n", lost_alarm->lost_enable);
    
    return ;
}

void hik_swap_hide_alarm_info(NET_DVR_HIDEALARM_V30 *hik_hide_alarm, 
        JHideAlarm *hide_alarm, int flag)
{
    NMP_ASSERT(hik_hide_alarm && hide_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            if (0 < hik_hide_alarm->dwEnableHideAlarm)
                hide_alarm->hide_enable = 1;
            else
                hide_alarm->hide_enable = 0;
            hide_alarm->detect_area.count = 1;
            hide_alarm->detect_area.rect[0].left   = hik_hide_alarm->wHideAlarmAreaTopLeftX;
            hide_alarm->detect_area.rect[0].top    = hik_hide_alarm->wHideAlarmAreaTopLeftY;
            hide_alarm->detect_area.rect[0].right  = hik_hide_alarm->wHideAlarmAreaWidth;
            hide_alarm->detect_area.rect[0].bottom = hik_hide_alarm->wHideAlarmAreaHeight;
            hide_alarm->max_width                  = 704;
            hide_alarm->max_height                 = 576;
            hik_process_sched_time(hik_hide_alarm->struAlarmTime, &hide_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            hik_hide_alarm->dwEnableHideAlarm      = hide_alarm->hide_enable;
            hik_hide_alarm->wHideAlarmAreaTopLeftX = hide_alarm->detect_area.rect[0].left;
            hik_hide_alarm->wHideAlarmAreaTopLeftY = hide_alarm->detect_area.rect[0].top;
            hik_hide_alarm->wHideAlarmAreaWidth    = hide_alarm->detect_area.rect[0].right;
            hik_hide_alarm->wHideAlarmAreaHeight   = hide_alarm->detect_area.rect[0].bottom;
            memset(hik_hide_alarm->struAlarmTime, 0, sizeof(hik_hide_alarm->struAlarmTime));
            hik_process_sched_time(hik_hide_alarm->struAlarmTime, &hide_alarm->week, flag);
            break;
            
        default:
            break;
    }
    printf("dwEnableHideAlarm: %02d   |  ", hik_hide_alarm->dwEnableHideAlarm);
    printf("hide_enable    : %d\n", hide_alarm->hide_enable);
    
    return ;
}

void hik_swap_alarm_in_info(NET_DVR_ALARMINCFG_V30 *hik_alarm_in, 
        JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(hik_alarm_in && io_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            hik_process_sched_time(hik_alarm_in->struAlarmTime, &io_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            hik_alarm_in->byAlarmInHandle      = io_alarm->alarm_enable;
            memset(hik_alarm_in->struAlarmTime, 0, sizeof(hik_alarm_in->struAlarmTime));
            hik_process_sched_time(hik_alarm_in->struAlarmTime, &io_alarm->week, flag);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_alarm_out_info(NET_DVR_ALARMOUTCFG_V30 *hik_hide_out, 
        JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(hik_hide_out && io_alarm);
    
    switch (flag)
    {
        case SWAP_PACK:
            hik_process_sched_time(hik_hide_out->struAlarmOutTime, &io_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            memset(hik_hide_out->struAlarmOutTime, 0, sizeof(hik_hide_out->struAlarmOutTime));
            hik_process_sched_time(hik_hide_out->struAlarmOutTime, &io_alarm->week, flag);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_disk_list(NET_DVR_HDCFG *hd_cfg, 
        JDeviceDiskInfo *dev_disk, int flag)
{
    int i;
    NMP_ASSERT(hd_cfg && dev_disk);

    switch (flag)
    {
        case SWAP_PACK:
            dev_disk->disk_num = hd_cfg->dwHDCount;
            for (i=0; i<(int)hd_cfg->dwHDCount; i++)
            {
                dev_disk->disk[i].disk_no    = hd_cfg->struHDInfo[i].dwHDNo;
                dev_disk->disk[i].disk_type  = hd_cfg->struHDInfo[i].byHDType;
                //dev_disk->disk[i].status     = hd_cfg->struHDInfo[i].dwHdStatus;
                switch (hd_cfg->struHDInfo[i].dwHdStatus)
                {
                    case HD_STAT_OK:
                        dev_disk->disk[i].status = J_SDK_USING;
                        break;
                    case HD_STAT_UNFORMATTED:
                        break;
                    case HD_STAT_ERROR:
                        dev_disk->disk[i].status = J_SDK_UNMOUNT;
                        break;
                    case HD_STAT_SMART_FAILED:
                        break;
                    case HD_STAT_MISMATCH:
                        break;
                    case HD_STAT_IDLE:
                        dev_disk->disk[i].status = J_SDK_MOUNTED;
                        break;
                    case NET_HD_STAT_OFFLINE:
                        break;
                }
                dev_disk->disk[i].total_size = hd_cfg->struHDInfo[i].dwCapacity;
                dev_disk->disk[i].free_size  = hd_cfg->struHDInfo[i].dwFreeSpace;
                //dev_disk->disk[i].is_backup = hd_cfg->struHDInfo[i].byMode;
                //dev_disk->disk[i].sys_file_type = hd_cfg->struHDInfo[i].byMode;
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

void hik_swap_network_info(NET_DVR_NETCFG_V30 *net_cfg, 
        JNetworkInfo *net_info, int flag)
{
    int out_len;
    char *out_buf;
//  prx_server_config *config = NULL;
    NMP_ASSERT(net_cfg && net_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)net_info->network[J_SDK_ETH0].ip, 
                net_cfg->struEtherNet[0].struDVRIP.sIpV4, 
                sizeof(net_info->network[J_SDK_ETH0].ip)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].netmask, 
                net_cfg->struEtherNet[0].struDVRIPMask.sIpV4, 
                sizeof(net_info->network[J_SDK_ETH0].netmask)-1);
            strncpy((char*)net_info->network[J_SDK_ETH0].gateway, 
                net_cfg->struGatewayIpAddr.sIpV4, 
                sizeof(net_info->network[J_SDK_ETH0].gateway)-1);
            printf("mac: %s<--------------------\n", net_cfg->struEtherNet[0].byMACAddr);
            out_buf = (char*)net_info->network[J_SDK_ETH0].mac;
            out_len = sizeof(net_info->network[J_SDK_ETH0].mac)-1;
            code_convert((char*)"gb2312", (char*)"utf-8", 
                    (char*)net_cfg->struEtherNet[0].byMACAddr, 
                    strlen((const char*)net_cfg->struEtherNet[0].byMACAddr), 
                    out_buf, out_len);

            strncpy((char*)net_info->main_dns, 
                (const char*)net_cfg->struDnsServer1IpAddr.sIpV4, 
                sizeof(net_info->main_dns)-1);
            strncpy((char*)net_info->backup_dns, 
                (const char*)net_cfg->struDnsServer2IpAddr.sIpV4, 
                sizeof(net_info->backup_dns)-1);

            net_info->network[J_SDK_ETH0].type = J_SDK_ETH0;
            net_info->network[J_SDK_ETH0].dhcp_enable = net_cfg->byUseDhcp;
            net_info->network[J_SDK_ETH0].type = J_SDK_ETH0;

            net_info->web_port = net_cfg->wHttpPortNo;
            net_info->cmd_port = net_cfg->struEtherNet[0].wDVRPort;

//          config = config_get_server_config();
//          net_info->data_port = config->rtsp_port;
            break;

        case SWAP_UNPACK:
            strncpy((char*)net_cfg->struEtherNet[0].struDVRIP.sIpV4, 
                (const char*)net_info->network[J_SDK_ETH0].ip, 
                sizeof(net_cfg->struEtherNet[0].struDVRIP.sIpV4)-1);
            strncpy((char*)net_cfg->struEtherNet[0].struDVRIPMask.sIpV4, 
                (const char*)net_info->network[J_SDK_ETH0].netmask, 
                sizeof(net_cfg->struEtherNet[0].struDVRIPMask.sIpV4)-1);
            strncpy((char*)net_cfg->struGatewayIpAddr.sIpV4, 
                (const char*)net_info->network[J_SDK_ETH0].gateway, 
                sizeof(net_cfg->struGatewayIpAddr.sIpV4)-1);
            strncpy((char*)net_cfg->struEtherNet[0].byMACAddr, 
                (const char*)net_info->network[J_SDK_ETH0].mac, 
                sizeof(net_cfg->struEtherNet[0].byMACAddr)-1);

            strncpy((char*)net_cfg->struDnsServer1IpAddr.sIpV4, 
                (const char*)net_info->main_dns, 
                sizeof(net_cfg->struDnsServer1IpAddr.sIpV4)-1);
            strncpy((char*)net_cfg->struDnsServer2IpAddr.sIpV4, 
                (const char*)net_info->backup_dns, 
                sizeof(net_cfg->struDnsServer2IpAddr.sIpV4)-1);

            net_cfg->byUseDhcp = net_info->network[J_SDK_ETH0].dhcp_enable;
            net_cfg->wHttpPortNo = net_info->web_port;
            net_cfg->struEtherNet[0].wDVRPort = net_info->cmd_port;

/*          config = config_get_server_config();
            if (config->rtsp_port != net_info->data_port)
            {
                config->rtsp_port = net_info->data_port;
                if (!config_set_server_config(config))
                    config_save_server_config(config);
            }*/
            break;

        default:
            break;
    }

    return ;
}

void hik_swap_pppoe_info(NET_DVR_PPPOECFG *pppoe_cfg, 
        JPPPOEInfo *pppoe_info, int flag)
{
    NMP_ASSERT(pppoe_cfg && pppoe_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            pppoe_info->enable = pppoe_cfg->dwPPPOE;
            strncpy((char*)pppoe_info->ip, (const char*)pppoe_cfg->struPPPoEIP.sIpV4, 
                    sizeof(pppoe_info->ip)-1);
            strncpy((char*)pppoe_info->account, (const char*)pppoe_cfg->sPPPoEUser, 
                    sizeof(pppoe_info->account)-1);
            strncpy((char*)pppoe_info->passwd, (const char*)pppoe_cfg->sPPPoEPassword, 
                    sizeof(pppoe_info->passwd)-1);
            break;
            
        case SWAP_UNPACK:
            pppoe_cfg->dwPPPOE = pppoe_info->enable;
            strncpy((char*)pppoe_cfg->struPPPoEIP.sIpV4, (const char*)pppoe_info->ip, 
                    sizeof(pppoe_cfg->struPPPoEIP.sIpV4)-1);
            strncpy((char*)pppoe_cfg->sPPPoEUser, (const char*)pppoe_info->account, 
                    sizeof(pppoe_cfg->sPPPoEUser)-1);
            strncpy((char*)pppoe_cfg->sPPPoEPassword, (const char*)pppoe_info->passwd, 
                    sizeof(pppoe_cfg->sPPPoEPassword)-1);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_time_info(NET_DVR_TIME *hik_time, 
        JDeviceTime *dev_time, int flag)
{
    NMP_ASSERT(hik_time && dev_time);
    
    switch (flag)
    {
        case SWAP_PACK:
            dev_time->time.year   = hik_time->dwYear - 1900;
            dev_time->time.month  = hik_time->dwMonth;
            dev_time->time.date   = hik_time->dwDay;
            dev_time->time.hour   = hik_time->dwHour;
            dev_time->time.minute = hik_time->dwMinute;
            dev_time->time.second = hik_time->dwSecond;
            printf("SWAP_PACK\n");
            printf("dev_time: %d/%d/%d %d:%d:%d\n", 
                dev_time->time.year, 
                dev_time->time.month, 
                dev_time->time.date, 
                dev_time->time.hour, 
                dev_time->time.minute, 
                dev_time->time.second);
            printf("hik_time: %d/%d/%d %d:%d:%d\n", 
                hik_time->dwYear, 
                hik_time->dwMonth, 
                hik_time->dwDay, 
                hik_time->dwHour, 
                hik_time->dwMinute, 
                hik_time->dwSecond);
            break;
            
        case SWAP_UNPACK:
            hik_time->dwYear   = dev_time->time.year + 1900;
            hik_time->dwMonth  = dev_time->time.month;
            hik_time->dwDay    = dev_time->time.date;
            hik_time->dwHour   = dev_time->time.hour;
            hik_time->dwMinute = dev_time->time.minute;
            hik_time->dwSecond = dev_time->time.second;
            printf("SWAP_UNPACK\n");
            printf("dev_time: %d/%d/%d %d:%d:%d\n", 
                dev_time->time.year, 
                dev_time->time.month, 
                dev_time->time.date, 
                dev_time->time.hour, 
                dev_time->time.minute, 
                dev_time->time.second);
            printf("hik_time: %d/%d/%d %d:%d:%d\n", 
                hik_time->dwYear, 
                hik_time->dwMonth, 
                hik_time->dwDay, 
                hik_time->dwHour, 
                hik_time->dwMinute, 
                hik_time->dwSecond);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_zone_info(NET_DVR_ZONEANDDST *hik_zone, 
        JDeviceTime *dev_time, int flag)
{
    NMP_ASSERT(hik_zone && dev_time);
    
    switch (flag)
    {
        case SWAP_PACK:
            //dev_time->zone = hik_zone->;
            break;
            
        case SWAP_UNPACK:
            //hik_zone-> = dev_time->zone;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_ntp_info(NET_DVR_NTPPARA *ntp_para, 
        JDeviceNTPInfo *ntp_info, int flag)
{
    NMP_ASSERT(ntp_para && ntp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)ntp_info->ntp_server_ip, (const char*)ntp_para->sNTPServer, 
                    sizeof(ntp_info->ntp_server_ip)-1);
            ntp_info->time_interval = ntp_para->wInterval;
            ntp_info->ntp_enable    = ntp_para->byEnableNTP;
            break;
            
        case SWAP_UNPACK:
            strncpy((char*)ntp_para->sNTPServer, (const char*)ntp_info->ntp_server_ip, 
                    sizeof(ntp_para->sNTPServer)-1);
            ntp_para->wInterval   = ntp_info->time_interval;
            ntp_para->byEnableNTP = ntp_info->ntp_enable;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_ftp_info(NET_DVR_FTPCFG *ftp_cfg, 
        JFTPParameter *ftp_param, int flag)
{
    NMP_ASSERT(ftp_cfg && ftp_param);
    
    switch (flag)
    {
        case SWAP_PACK:
            ftp_param->ftp_port = ftp_cfg->dwFTPPort;
            strncpy((char*)ftp_param->ftp_ip, (const char*)ftp_cfg->sFTPIP, 
                    sizeof(ftp_param->ftp_ip)-1);
            strncpy((char*)ftp_param->ftp_usr, (const char*)ftp_cfg->sUserName, 
                    sizeof(ftp_param->ftp_usr)-1);
            strncpy((char*)ftp_param->ftp_pwd, (const char*)ftp_cfg->sPassword, 
                    sizeof(ftp_param->ftp_pwd)-1);
            //strncpy(ftp_param->ftp_path, ftp_cfg->, sizeof(ftp_param->ftp_path)
            break;
            
        case SWAP_UNPACK:
            ftp_cfg->dwFTPPort = ftp_param->ftp_port;
            strncpy((char*)ftp_cfg->sFTPIP, (const char*)ftp_param->ftp_ip, 
                    sizeof(ftp_cfg->sFTPIP)-1);
            strncpy((char*)ftp_cfg->sUserName, (const char*)ftp_param->ftp_usr, 
                    sizeof(ftp_cfg->sUserName)-1);
            strncpy((char*)ftp_cfg->sPassword, (const char*)ftp_param->ftp_pwd, 
                    sizeof(ftp_cfg->sPassword)-1);
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_store_log_info(JStoreLog *store_log_cfg, 
            JStoreLog *store_log_param, int flag)
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
                    case 0:
                        store_log_param->store[index].rec_type = 0x00000001;     //  定时录像     
                        break;
                    case 1:
                        store_log_param->store[index].rec_type = 0x00000100;     //  移动侦测对象   
                        break;
                    case 2:
                        store_log_param->store[index].rec_type = 0x00000010;     // 报警录像    
                        break;
                    case 4:
                        store_log_param->store[index].rec_type = (0x00000010 | 0x00000100);     // 报警录像 + 移动侦测 
                        break;
                    case 6:
                        store_log_param->store[index].rec_type = 0x00001000;     // 手动录像  
                        break;
                    case 0xff:
                        store_log_param->store[index].rec_type = 0xFFFFFFFF;     // 全部
                    default:
                        break;
                }
            }
            show_debug("total_count: %d\n", store_log_param->total_count);
            break;

        case SWAP_UNPACK:
            switch(store_log_cfg->rec_type)
            {
                case 0x00000001:                        // 定时录像 
                    store_log_param->rec_type = 0;
                    break;
                case 0x00000010:                        // 报警录像
                    store_log_param->rec_type = 2;
                    break;
                case 0x00000100:                        // 移动侦测对象
                    store_log_param->rec_type = 1;
                    break;
                case (0x00000010 | 0x00000100):         // 报警录像 + 移动侦测对象
                    store_log_param->rec_type = 4;
                    break;
                case 0x00001000:                        // 手动录像
                    store_log_param->rec_type = 6;
                    break;
                case 0xFFFFFFFF:    
                store_log_param->rec_type = 0xff;       // 全部
                break;
            }
            break;

        default:
            break;
    }

    return ;
}

void hik_swap_hide_info(NET_DVR_PICCFG_V30 *pic_cfg, 
        JHideParameter *hide_info, int flag)
{
    int i;
    NMP_ASSERT(pic_cfg && hide_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            hide_info->hide_enable = pic_cfg->dwEnableHide;
            for (i=0; i<MAX_SHELTERNUM; i++)
            {
                hide_info->hide_area.rect[i].left   = pic_cfg->struShelter[i].wHideAreaTopLeftX;
                hide_info->hide_area.rect[i].top    = pic_cfg->struShelter[i].wHideAreaTopLeftY;
                hide_info->hide_area.rect[i].right  = pic_cfg->struShelter[i].wHideAreaWidth;
                hide_info->hide_area.rect[i].bottom = pic_cfg->struShelter[i].wHideAreaHeight;
    printf("\n%d\n", i);
    printf("left  : %02d\n", hide_info->hide_area.rect[i].left);
    printf("top   : %02d\n", hide_info->hide_area.rect[i].top);
    printf("right : %02d\n", hide_info->hide_area.rect[i].right);
    printf("bottom: %02d\n", hide_info->hide_area.rect[i].bottom);
            }
            hide_info->hide_area.count = MAX_SHELTERNUM;
            hide_info->max_width       = 704;
            hide_info->max_height      = 576;
            break;
            
        case SWAP_UNPACK:
            pic_cfg->dwEnableHide = hide_info->hide_enable;
            memset(pic_cfg->struShelter, 0, sizeof(pic_cfg->struShelter));
            for (i=0; i<MAX_SHELTERNUM; i++)
            {
                pic_cfg->struShelter[i].wHideAreaTopLeftX = hide_info->hide_area.rect[i].left;
                pic_cfg->struShelter[i].wHideAreaTopLeftY = hide_info->hide_area.rect[i].top;
                pic_cfg->struShelter[i].wHideAreaWidth    = hide_info->hide_area.rect[i].right;
                pic_cfg->struShelter[i].wHideAreaHeight   = hide_info->hide_area.rect[i].bottom;
    printf("\n%d\n", i);
    printf("wHideAreaTopLeftX: %02d\n", pic_cfg->struShelter[i].wHideAreaTopLeftX);
    printf("wHideAreaTopLeftY: %02d\n", pic_cfg->struShelter[i].wHideAreaTopLeftY);
    printf("wHideAreaWidth   : %02d\n", pic_cfg->struShelter[i].wHideAreaWidth);
    printf("wHideAreaHeight  : %02d\n", pic_cfg->struShelter[i].wHideAreaHeight);
            }
            break;
            
        default:
            break;
    }
    printf("dwEnableHide: %02d   |  ", pic_cfg->dwEnableHide);
    printf("hide_enable    : %d\n", hide_info->hide_enable);
    
    return ;
}

void hik_swap_rs232_info(NET_DVR_SINGLE_RS232 *rs232_cfg, 
        JSerialParameter *serial_info, int flag)
{
    int no, i;
    int baud_rate[] = {50, 75, 110, 150, 300, 600, 1200, 2400, 
        4800, 9600, 19200, 38400, 57600, 76800, 115200};
    NMP_ASSERT(rs232_cfg && serial_info);
    
    switch (flag)
    {
    case SWAP_PACK:
        no = serial_info->serial_no;
        if (MAX_SERIAL_PORT > no)
        {
            if (0 <= rs232_cfg[no].dwBaudRate && 15 > rs232_cfg[no].dwBaudRate)
                serial_info->baud_rate = baud_rate[rs232_cfg[no].dwBaudRate];

            serial_info->data_bit = rs232_cfg[no].byDataBit + 5;
            
            switch (rs232_cfg[no].byStopBit)
            {
            case 0:
                serial_info->stop_bit = 1;
                break;
            case 1:
                serial_info->stop_bit = 2;
                break;
                
            default:
                break;
            }
            
            serial_info->verify    = rs232_cfg[no].byParity;
        }
        break;
        
    case SWAP_UNPACK:
        no = serial_info->serial_no;
        if (MAX_SERIAL_PORT > no)
        {
            for (i=0; i<15; i++)
            {
                if (baud_rate[i] == (int)serial_info->baud_rate)
                {
                    rs232_cfg[no].dwBaudRate = i;
                    break;
                }
            }
            rs232_cfg[no].byDataBit = serial_info->data_bit -5;
            
            switch (serial_info->stop_bit)
            {
            case 1:
                rs232_cfg[no].byStopBit = 0;
                break;
            case 2:
                rs232_cfg[no].byStopBit = 1;
                break;
                
            default:
                break;
            }
            
            rs232_cfg[no].byParity   = serial_info->verify;
        }
        break;
        
    default:
        break;
    }
    
    return ;
}

void hik_swap_rs485_info(NET_DVR_ALARM_RS485CFG *rs485_cfg, 
        JSerialParameter *serial_info, int flag)
{
    NMP_ASSERT(rs485_cfg && serial_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_smtp_info(NET_DVR_EMAILCFG_V30 *email_cfg, 
        JSMTPParameter *smtp_info, int flag)
{
    NMP_ASSERT(email_cfg && smtp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)smtp_info->mail_ip, (const char*)email_cfg->sSmtpServer, 
                    sizeof(smtp_info->mail_ip)-1);
            strncpy((char*)smtp_info->mail_addr, (const char*)email_cfg->struSender.sAddress, 
                    sizeof(smtp_info->mail_addr)-1);
            strncpy((char*)smtp_info->mail_usr, (const char*)email_cfg->sAccount, 
                    sizeof(smtp_info->mail_usr)-1);
            strncpy((char*)smtp_info->mail_pwd, (const char*)email_cfg->sPassword, 
                    sizeof(smtp_info->mail_pwd)-1);
            strncpy((char*)smtp_info->mail_rctp1, (const char*)email_cfg->struReceiver[0].sAddress, 
                    sizeof(smtp_info->mail_rctp1)-1);
            strncpy((char*)smtp_info->mail_rctp2, (const char*)email_cfg->struReceiver[1].sAddress, 
                    sizeof(smtp_info->mail_rctp2)-1);
            strncpy((char*)smtp_info->mail_rctp3, (const char*)email_cfg->struReceiver[2].sAddress, 
                    sizeof(smtp_info->mail_rctp3)-1);
            smtp_info->mail_port  = email_cfg->wSmtpPort;
            smtp_info->ssl_enable = email_cfg->byEnableSSL;
            break;
            
        case SWAP_UNPACK:
            strncpy((char*)email_cfg->sSmtpServer, (const char*)smtp_info->mail_ip, 
                    sizeof(email_cfg->sSmtpServer)-1);
            strncpy((char*)email_cfg->struSender.sAddress, (const char*)smtp_info->mail_addr, 
                    sizeof(email_cfg->struSender.sAddress)-1);
            strncpy((char*)email_cfg->sAccount, (const char*)smtp_info->mail_usr, 
                    sizeof(email_cfg->sAccount)-1);
            strncpy((char*)email_cfg->sPassword, (const char*)smtp_info->mail_pwd, 
                    sizeof(email_cfg->sPassword)-1);
            strncpy((char*)email_cfg->struReceiver[0].sAddress, (const char*)smtp_info->mail_rctp1, 
                    sizeof(email_cfg->struReceiver[0].sAddress)-1);
            strncpy((char*)email_cfg->struReceiver[1].sAddress, (const char*)smtp_info->mail_rctp2, 
                    sizeof(email_cfg->struReceiver[1].sAddress)-1);
            strncpy((char*)email_cfg->struReceiver[2].sAddress, (const char*)smtp_info->mail_rctp3, 
                    sizeof(email_cfg->struReceiver[2].sAddress)-1);
            email_cfg->wSmtpPort   = smtp_info->mail_port;
            email_cfg->byEnableSSL = smtp_info->ssl_enable;
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_ddns_info(NET_DVR_DDNSPARA_EX *ddns_cfg, 
        JDdnsConfig *ddns_info, int flag)
{
    NMP_ASSERT(ddns_cfg && ddns_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            ddns_info->ddns_open = ddns_cfg->byEnableDDNS;
            if (0 < ddns_cfg->byHostIndex)
                ddns_info->ddns_type  = ddns_cfg->byHostIndex - 1;
            strncpy((char*)ddns_info->ddns_account, (const char*)ddns_cfg->sDomainName, 
                    sizeof(ddns_info->ddns_account)-1);
            strncpy((char*)ddns_info->ddns_usr, (const char*)ddns_cfg->sUsername, 
                    sizeof(ddns_info->ddns_usr)-1);
            strncpy((char*)ddns_info->ddns_pwd, (const char*)ddns_cfg->sPassword, 
                    sizeof(ddns_info->ddns_pwd)-1);
            ddns_info->ddns_port  = ddns_cfg->wDDNSPort;
            //ddns_info->ddns_times  = ddns_cfg->;
            break;
            
        case SWAP_UNPACK:
            ddns_cfg->byEnableDDNS = ddns_info->ddns_open;
            ddns_cfg->byHostIndex = ddns_info->ddns_type + 1;
            strncpy((char*)ddns_cfg->sDomainName, (const char*)ddns_info->ddns_account, 
                    sizeof(ddns_cfg->sDomainName)-1);
            strncpy((char*)ddns_cfg->sUsername, (const char*)ddns_info->ddns_usr, 
                    sizeof(ddns_cfg->sUsername)-1);
            strncpy((char*)ddns_cfg->sPassword, (const char*)ddns_info->ddns_pwd, 
                    sizeof(ddns_cfg->sPassword)-1);
            ddns_cfg->wDDNSPort = ddns_info->ddns_port;
            break;
            
        default:
            break;
    }
    printf("byEnableDDNS: %02d   |  ", ddns_cfg->byEnableDDNS);
    printf("ddns_open    : %d\n", ddns_info->ddns_open);
    printf("byHostIndex : %02d   |  ", ddns_cfg->byHostIndex);
    printf("ddns_type    : %d\n", ddns_info->ddns_type);
    
    printf("sDomainName : %s     |  ", ddns_cfg->sDomainName);
    printf("ddns_account : %s\n", ddns_info->ddns_account);
    printf("sUsername   : %s     |  ", ddns_cfg->sUsername);
    printf("ddns_usr     : %s\n", ddns_info->ddns_usr);
    printf("sPassword   : %s     |  ", ddns_cfg->sPassword);
    printf("ddns_pwd     : %s\n", ddns_info->ddns_pwd);
    
    printf("wDDNSPort   : %02d   |  ", ddns_cfg->wDDNSPort);
    printf("ddns_port    : %d\n", ddns_info->ddns_port);
    
    return ;
}

void hik_swap_ptz_info(NET_DVR_PTZCFG *ptz_cfg, 
        JPTZParameter *ptz_info, int flag)
{
    NMP_ASSERT(ptz_cfg && ptz_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            //ptz_info->
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_cruise_way(NET_DVR_CRUISE_RET *hik_crz, 
        JCruiseWay *crz_way, int flag)
{
    int i;
    JCruisePoint *crz_pp;
    
    NMP_ASSERT(hik_crz && crz_way);
    
    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<32; i++)
            {
                crz_pp = &crz_way->crz_pp[i];
                crz_pp->preset = (int)hik_crz->struCruisePoint[i].PresetNum;
                crz_pp->speed  = (int)hik_crz->struCruisePoint[i].Speed;
                crz_pp->dwell  = (int)hik_crz->struCruisePoint[i].Dwell;
            }
            crz_way->pp_count = i;
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }
    
    return ;
}

void hik_swap_decoder_info(NET_DVR_DECODERCFG_V30 *dec_cfg, 
        JPTZParameter *ptz_info, int flag)
{
    int i;
    int baud_rate[] = {50, 75, 110, 150, 300, 600, 1200, 2400, 
        4800, 9600, 19200, 38400, 57600, 76800, 115200};
    
    NMP_ASSERT(dec_cfg && ptz_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            if (0 <= dec_cfg->dwBaudRate && 15 > dec_cfg->dwBaudRate)
                ptz_info->baud_rate = baud_rate[dec_cfg->dwBaudRate];

            switch (dec_cfg->byDataBit)
            {
            case 0:
                ptz_info->data_bit = 5;
                break;
            case 1:
                ptz_info->data_bit = 6;
                break;
            case 2:
                ptz_info->data_bit = 7;
                break;
            case 3:
                ptz_info->data_bit = 8;
                break;
                
            default:
                break;
            }
            
            switch (dec_cfg->byStopBit)
            {
            case 0:
                ptz_info->stop_bit = 1;
                break;
            case 1:
                ptz_info->stop_bit = 2;
                break;
                
            default:
                break;
            }

            ptz_info->protocol = dec_cfg->wDecoderType;
            ptz_info->ptz_addr = dec_cfg->wDecoderAddress;
            ptz_info->verify   = dec_cfg->byParity;
            break;
            
        case SWAP_UNPACK:
            for (i=0; i<15; i++)
            {
                if (baud_rate[i] == (int)ptz_info->baud_rate)
                {
                    dec_cfg->dwBaudRate = i;
                    break;
                }
            }
            
            switch (ptz_info->data_bit)
            {
            case 5:
                dec_cfg->byDataBit = 0;
                break;
            case 6:
                dec_cfg->byDataBit = 1;
                break;
            case 7:
                dec_cfg->byDataBit = 2;
                break;
            case 8:
                dec_cfg->byDataBit = 3;
                break;
                
            default:
                break;
            }
            
            switch (ptz_info->stop_bit)
            {
            case 1:
                dec_cfg->byStopBit = 0;
                break;
            case 2:
                dec_cfg->byStopBit = 1;
                break;
                
            default:
                break;
            }
            
            dec_cfg->wDecoderType    = ptz_info->protocol;
            dec_cfg->wDecoderAddress = ptz_info->ptz_addr;
            dec_cfg->byParity        = ptz_info->verify;
            break;
            
        default:
            break;
    }
    
    return ;
}




