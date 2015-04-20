#include <string.h>
#include <unistd.h>

#include "nmp_proxy_sdk.h"
#include "nmp_resolve_host.h"

#include "nmp_hie_handler.h"
#include "nmp_hie_channel.h"
#include "nmp_hie_service.h"
//#include "nmp_hie_talk.h"


static const handler_table_t 
hie_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     hie_get_device_info },
    { GET_SERIAL_CONFIG,     hie_get_serial_info },
    { SET_SERIAL_CONFIG,     hie_set_serial_info },
    { GET_DEVICE_TIME,       hie_get_device_time },
    { SET_DEVICE_TIME,       hie_set_device_time },
    { GET_NTP_CONFIG,        hie_get_ntp_info },
    { SET_NTP_CONFIG,        hie_set_ntp_info },
    { GET_NETWORK_CONFIG,    hie_get_network_info },
    { SET_NETWORK_CONFIG,    hie_set_network_info },
    { GET_PPPOE_CONFIG,      hie_get_pppoe_info },
    { SET_PPPOE_CONFIG,      hie_set_pppoe_info },
    { GET_FTP_CONFIG,        hie_get_ftp_info },
    { SET_FTP_CONFIG,        hie_set_ftp_info },
    { GET_SMTP_CONFIG,       hie_get_smtp_info },
    { SET_SMTP_CONFIG,       hie_set_smtp_info },
    { GET_DDNS_CONFIG,       hie_get_ddns_info },
    { SET_DDNS_CONFIG,       hie_set_ddns_info },
    { GET_UPNP_CONFIG,       hie_get_upnp_info },
    { SET_UPNP_CONFIG,       hie_set_upnp_info },
    { GET_DISK_LIST,         hie_get_disk_list},
    { SET_DISK_FORMAT,       hie_format_disk },
    { CONTROL_DEVICE_CMD,    hie_control_device },

    { GET_ENCODE_CONFIG,     hie_get_encode_info },
    { SET_ENCODE_CONFIG,     hie_set_encode_info },
    { GET_DISPLAY_CONFIG,    hie_get_display_info },
    { SET_DISPLAY_CONFIG,    hie_set_display_info },
    { GET_OSD_CONFIG,        hie_get_osd_info },
    { SET_OSD_CONFIG,        hie_set_osd_info },
    { GET_PTZ_CONFIG,        hie_get_ptz_info },
    { SET_PTZ_CONFIG,        hie_set_ptz_info },
    { GET_RECORD_CONFIG,     hie_get_record_info },
    { SET_RECORD_CONFIG,     hie_set_record_info },
    { GET_HIDE_CONFIG,       hie_get_hide_info },
    { SET_HIDE_CONFIG,       hie_set_hide_info },

    { GET_MOTION_CONFIG,     hie_get_move_alarm_info },
    { SET_MOTION_CONFIG,     hie_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, hie_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, hie_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, hie_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, hie_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   hie_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   hie_set_io_alarm_info },

    { GET_STORE_LOG,         hie_get_store_log },

    { CONTROL_PTZ_CMD,       hie_ptz_control },
    { SET_PRESET_CONFIG,     hie_set_preset_point },
    { GET_CRUISE_CONFIG,     hie_get_cruise_way },
    { SET_CRUISE_CONFIG,     hie_set_cruise_way },
    { ADD_CRUISE_CONFIG,     hie_add_cruise_way },
    { MDF_CRUISE_CONFIG,     hie_modify_cruise_way },

    { GET_CAPABILITY_SET,    hie_get_capability_set },
};

static __inline__ JTime *
hie_get_local_time(JTime *ts)
{
    time_t t;
    struct tm *local;

    t = time(NULL);
    local = localtime(&t);

    ts->year   = local->tm_year;
    ts->month  = local->tm_mon + 1;
    ts->date   = local->tm_mday;
    ts->hour   = local->tm_hour;
    ts->minute = local->tm_min;
    ts->second = local->tm_sec;

    return ts;
}


static __inline__ void 
hie_time_swap(TimeInfo *time, JTime *jtime, int flag)
{
    switch(flag)
    {
        case SWAP_UNPACK:
            time->wYear   = jtime->year + 1900;
            time->wMonth  = jtime->month;
            time->wDay    = jtime->date;
            time->wHour   = jtime->hour;
            time->wMinute = jtime->minute;
            time->wSecond = jtime->second;
            break;
        case SWAP_PACK:
            jtime->year    = time->wYear - 1900;
            jtime->month   = time->wMonth;
            jtime->date    = time->wDay;
            jtime->hour    = time->wHour;
            jtime->minute  = time->wMinute;
            jtime->second  = time->wSecond;
            break;

        default:
            break;
    }
}

static void 
hie_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int 
hie_logout(HUSER user_id)
{
    if (!HieClient_UserLogout(user_id))
    {
        show_debug("HieClient_UserLogout() successful, user_id: %p\n", user_id);
        return 0;
    }
    else
        return -1;
}
static __inline__ void 
hie_login(hie_service_t *hie_srv)
{
    char addr[MAX_IP_LEN];
    int state, error = 0;
    HUSER user_id = HIE_INVALID_HANDLE;

    proxy_sdk_t sdk_info;
    UserLoginPara login_para;

    NMP_ASSERT(hie_srv);

    memset(&sdk_info, 0, sizeof(proxy_sdk_t));
    memset(&login_para, 0, sizeof(UserLoginPara));

    if (proxy_get_device_private(hie_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        login_para.dwCommandPort = sdk_info.dev_port;
        strncpy(login_para.sServerIP, sdk_info.dev_host, MAX_ADDRESS_LEN);
        strncpy(login_para.sUName,    sdk_info.username, USERNAME_LEN);
        strncpy(login_para.sUPass,    sdk_info.password, USERPASS_LEN);

        error = HieClient_UserLogin(&user_id, &login_para);

        switch(error)
        {
            case eErrorConnectFailed:
            {
                show_debug("eErrorConnectFailed\n");
                break;
            }
            case eErrorServerReject:
            {
                show_debug("eErrorServerReject\n");
                break;
            }
            case eErrorUserName:
            {
                show_debug("eErrorUserName\n");
                break;
            }
            case eErrorUserPass:
            {
                show_debug("eErrorUserPass\n");
                break;
            }
            case eErrorUserNumOverflow:
            {
                show_debug("eErrorUserNumOverflow\n");
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (HIE_INVALID_HANDLE != user_id)
    {
        state = HIE_LOGIN;
        error = 0;
        show_debug("hie_login() successful, user_id: %p\n", user_id);
    }
    else
    {
        state = HIE_LOGOUT;

        if (strlen(addr))
        {
            show_warn("hie_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                hie_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    hie_set_user_id(hie_srv, user_id);
    hie_set_state(hie_srv, (HIE_STATE_E)state, error);

    return ;
}

//brief  强制删除已登录用户回调
static int 
hie_del_user_force_cb(HUSER user_id, DWORD user_data)
{
    proxy_device_t *prx_dev;
    hie_service_t *hie_srv;
    hie_service_basic_t *hie_basic;
    show_warn("hie_del_user_force_cb() user_id: %p!!!\n", user_id);

    hie_basic = (hie_service_basic_t*)user_data;

    prx_dev = proxy_find_device_by_user_id((int)user_id, 
                hie_basic->tm.service_name);
    if (prx_dev)
    {
        hie_srv = (hie_service_t*)prx_dev->sdk_srv;
        if (hie_srv)
        {
            hie_set_user_id(hie_srv, HIE_INVALID_HANDLE);
            hie_set_state(hie_srv, HIE_LOGOUT, 0);
        }
        proxy_device_ref(prx_dev);
    }

    return 0;
}

static int 
hie_init(service_template_t *self)
{
    int ret = 0, err;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(self);

    hie_basic = (hie_service_basic_t*)self;

    if ((err = HieClient_Start()))
    {
        ret = -1;
        show_warn("HieClient_Start() failure, err: %d!\n", err);
    }
    else
    {
        HieClient_DeleteUserForceCB(hie_del_user_force_cb, (DWORD)hie_basic);
        
    }

    return ret;
}
static int 
hie_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    HieClient_Stop();
    return 0;
}

static struct service *
hie_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    hie_service_t *hie_srv;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(self && init_data);

    hie_basic = (hie_service_basic_t*)self;
    prx_dev   = (proxy_device_t*)init_data;

    hie_srv = (hie_service_t*)nmp_new0(hie_service_t, 1);
    if (hie_srv)
    {
        hie_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        hie_srv->base.tm = self;
        hie_srv->parm.lock = nmp_mutex_new();
        hie_srv->parm.user_id = HIE_INVALID_HANDLE;
        hie_srv->parm.state = HIE_LOGOUT;
        hie_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接
                                                //设备，而不是等待整个策略周期
        hie_srv->rtsp.lock = nmp_mutex_new();
        memset(&hie_srv->rtsp.real_header, 0, sizeof(hie_strm_header_t));
        hie_srv->rtsp.real_strm_list.list = NULL;
        hie_srv->rtsp.rec_strm_list.list = NULL;

        hie_srv->owner = prx_dev;
    }

    return (struct service *)hie_srv;
}
static void 
hie_delete_service(service_template_t *self, struct service *srv)
{
    hie_service_t *hie_srv;
    hie_service_basic_t *hie_basic;

    NMP_ASSERT(self && srv);

    hie_srv = (hie_service_t*)srv;
    hie_basic = (hie_service_basic_t*)self;

    hie_basic->total_count -= 1;

    hie_set_user_id(hie_srv, HIE_INVALID_HANDLE);               //注销登录
    hie_set_state(hie_srv, HIE_LOGOUT, 0);

    /* 清理视频流链表 */
    hie_cleanup_stream_info(&hie_srv->rtsp);

    if (hie_srv->parm.lock)
        nmp_mutex_free(hie_srv->parm.lock);
    if (hie_srv->rtsp.lock)
        nmp_mutex_free(hie_srv->rtsp.lock);             //销毁相关锁

    memset(hie_srv, 0, sizeof(hie_service_t));          //内存块归零
    nmp_del(hie_srv, hie_service_t, 1);                   //释放内存块
    return ;
}

static int 
hie_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int ret = -1;
    hie_service_t *hie_srv;
    hie_service_basic_t *hie_basic;

    HANDLE user_id;//, handle;
    eRemoteDeviceControl ctrl_code;

    NMP_ASSERT(self && srv);

    hie_srv = (hie_service_t*)srv;
    hie_basic = (hie_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!hie_logout(hie_get_user_id(&hie_srv->parm)))
            {
                ret = 0;
                hie_set_user_id(hie_srv, HIE_INVALID_HANDLE);
                hie_set_state(hie_srv, HIE_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            hie_login(hie_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            sleep(2);
            ctrl_code = eDeviceSetDefault;
            goto CONTROL_DEVICE;
        case CTRL_CMD_REBOOT:
            sleep(2);
            ctrl_code = eDeviceReboot;
            goto CONTROL_DEVICE;
        case CTRL_CMD_SHUTDOWN:
            sleep(2);
            ctrl_code = eDeviceHalt;
            goto CONTROL_DEVICE;
        case CTRL_CMD_FRMTDISK:
            /*handle = HB_NET_FormatDisk((LONG)hie_get_user_id(&hie_srv->parm), 
                        *((int*)parm));
            if (HIE_INVALID_HANDLE != handle)
            {
                msg_t *msg;
                HB_NET_FORMATINFO frmt_info;
                FormatProgressPacket *frmt_pos;

                int disk_no  = *((int*)parm);
                int disk_pos = -1;
                int disk_state = -1;

                memset(&frmt_info, 0, sizeof(HB_NET_FORMATINFO));
                frmt_info.dwSize = sizeof(HB_NET_FORMATINFO);
                frmt_info.pCurFormatDisk = (DWORD*)&disk_no;
                frmt_info.pCurDiskPos    = (DWORD*)&disk_pos;
                frmt_info.pFormatStat    = (DWORD*)&disk_state;

                frmt_pos = (FormatProgressPacket*)nmp_alloc0(sizeof(FormatProgressPacket));
                msg = alloc_new_msg_2(SUBMIT_FORMAT_PROGRESS_ID, frmt_pos, 
                        sizeof(FormatProgressPacket), 0, hie_free_msg_data);

                do
                {
                    sleep(1);
                    if (HB_NET_GetFormatProgress((LONG)handle, &frmt_info))
                    {
                        show_debug("=============>>hie format disk: [%d, %d]\n", 
                            disk_state, disk_pos);
                        switch (disk_state)
                        {
                            case 0://正在格式化
                                //show_info("Formating!!!!!!!!!!!\n");
                                break;
                            case 1://格式化完成
                                //show_info("Format Success!!!\n");
                                break;
                            case 2://格式化错误
                                disk_pos = -1;
                                break;
                            case 3://要格式化的磁盘不存在
                                disk_pos = -1;
                                break;
                            case 4://格式化中途错误
                                disk_pos = -1;
                                break;
                            case 5://磁盘正在被使用
                                disk_pos = -1;
                                break;
                        }
                    }
                    else
                    {
                        show_warn("HB_NET_GetFormatProgress(), Error: %d\n", 
                            (int)HB_NET_GetLastError());
                        break;
                    }

                    frmt_pos->disk_no  = disk_no;
                    frmt_pos->progress = disk_pos;

                    (*hie_srv->owner->plt_srv->tm->control_service)(
                        hie_srv->owner->plt_srv->tm, 
                        hie_srv->owner->plt_srv, CTRL_CMD_SUBMIT, msg);

                    if (100 == disk_pos)
                        break;
                    else if (-1 == disk_pos)
                        break;
                }while (1);
                free_msg(msg);
                HB_NET_CloseFormat((LONG)handle);
            }
            else
                show_warn("HB_NET_FormatDisk(%d), Error: %d\n", 
                    *((int*)parm)+1, (int)HB_NET_GetLastError());*/
            break;
        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (HUSER)parm;
            if (user_id == hie_get_user_id(&hie_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&hie_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&hie_strm_opt;
            ret = 0;
            break;
        /*case CTRL_CMD_OBTAIN_TALK_OPT:
            *(int*)parm = (int)&hie_talk_opt;
            ret = 0;
            break;*/

        default:
            break;
    }

    return ret;

CONTROL_DEVICE:
    user_id = hie_get_user_id(&hie_srv->parm);
    return HieClient_DeviceControl(user_id, ctrl_code);
}
static int 
hie_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hie_service_t *hie_srv;

    NMP_ASSERT(self && srv);

    hie_srv = (hie_service_t*)srv;

    switch (hie_get_state(&hie_srv->parm, &state_timer))
    {
        case HIE_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hie_srv->owner);
                    if (task)
                    {
                        hie_set_state(hie_srv, HIE_LOGING, 0);
                        proxy_thread_pool_push(task);
                    }
                    else
                    {
                        proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                        ret = -2;
                    }
                }
            }
            break;

        case HIE_LOGING:
            break;

        case HIE_LOGIN:
            break;
    }

    return ret;
}

static int 
hie_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = -1;
    HUSER user_id;

    JDevCap *dev_cap;
//    get_store_t *get_store;

    hie_service_t *hie_srv;

    NMP_ASSERT(srv && parm);

    hie_srv = (hie_service_t*)srv;
    user_id = hie_get_user_id(&hie_srv->parm);

    if (HIE_INVALID_HANDLE == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    switch (parm_id)
    {
        case GET_DEVICE_CONFIG:
            goto GET_CFG;
        case GET_SERIAL_CONFIG:
            goto GET_CFG;
        case GET_DEVICE_TIME:
            goto GET_CFG;
        case GET_NTP_CONFIG:
            goto GET_CFG;
        case GET_NETWORK_CONFIG:
            goto GET_CFG;
        case GET_PPPOE_CONFIG:
            goto GET_CFG;
        case GET_FTP_CONFIG:
            goto GET_CFG;
        case GET_SMTP_CONFIG:
            goto GET_CFG;
        case GET_DDNS_CONFIG:
            goto GET_CFG;
        case GET_UPNP_CONFIG:
            goto GET_CFG;
        case GET_DISK_LIST:
            goto GET_CFG;

        case GET_ENCODE_CONFIG:
            goto GET_CFG;
        case GET_DISPLAY_CONFIG:
            goto GET_CFG;
        case GET_OSD_CONFIG:
            goto GET_CFG;
        case GET_PTZ_CONFIG:
            goto GET_CFG;
        case GET_RECORD_CONFIG:
            goto GET_CFG;
        case GET_HIDE_CONFIG:
            goto GET_CFG;
        case GET_MOTION_CONFIG:
            goto GET_CFG;
        case GET_VIDEO_LOST_CONFIG:
            goto GET_CFG;
        case GET_HIDE_ALARM_CONFIG:
            goto GET_CFG;
        case GET_IO_ALARM_CONFIG:
            goto GET_CFG;

        case GET_STORE_LOG:
            /*get_store = (get_store_t*)parm;
            ret = hie_find_record_file(user_id, get_store->channel, 
                    (JStoreLog*)get_store->buffer);*/
            break;

        case GET_CRUISE_CONFIG:
            break;

        case GET_CAPABILITY_SET:
            dev_cap = (JDevCap*)parm;
            dev_cap->cap0 = DEV_CAP_VIDEO_IN | DEV_CAP_AUDIO | DEV_CAP_IRIS | DEV_CAP_PTZ | 
                DEV_CAP_ALARM_IN | DEV_CAP_ALARM_OUT | DEV_CAP_STORAGE | DEV_CAP_WEB | DEV_CAP_PLATFORM | 
                DEV_CAP_INTELLIGENT_ANALYSIS | DEV_CAP_UPDATE | DEV_CAP_VIDEO_OUT;// | DEV_CAP_IR;
            dev_cap->ftp_enable = 0;
            dev_cap->upnp_enable = 0;
            dev_cap->chn_cap.size = sizeof(JChnCap);
            dev_cap->chn_cap.encode = VIDEO_ENCODE_H264_E | VIDEO_ENCODE_MJPEG_E | 
                VIDEO_ENCODE_JPEG_E | VIDEO_ENCODE_MPEG4_E;
            dev_cap->chn_cap.supp_mask = 1;
            dev_cap->chn_cap.mask_count = 1;
            dev_cap->chn_cap.supp_hide_alarm = 1;
            dev_cap->chn_cap.hide_alarm_count = 1;
            dev_cap->chn_cap.supp_move_alarm = 1;
            dev_cap->chn_cap.move_alarm_count = 1;
            dev_cap->chn_cap.supp_video_lost_alarm = 1;
            dev_cap->chn_cap.osd_count = 1;
            dev_cap->chn_cap.stream_count = 2;
        dev_cap->chn_cap.stream_supp_resolution[0] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
            CAP_VIDEO_HD1 | CAP_VIDEO_D1 | CAP_VIDEO_720P | CAP_VIDEO_1080P;
        dev_cap->chn_cap.stream_supp_resolution[1] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
            CAP_VIDEO_HD1 | CAP_VIDEO_D1 | CAP_VIDEO_720P | CAP_VIDEO_1080P;
        dev_cap->chn_cap.stream_supp_resolution[2] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
            CAP_VIDEO_HD1 | CAP_VIDEO_D1 | CAP_VIDEO_720P | CAP_VIDEO_1080P;
        dev_cap->chn_cap.stream_supp_resolution[3] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
            CAP_VIDEO_HD1 | CAP_VIDEO_D1 | CAP_VIDEO_720P | CAP_VIDEO_1080P;
            dev_cap->chn_cap.stream_max_frate_rate[0] = 30;
            dev_cap->chn_cap.img_cap = IMA_BRIGHTNESS | IMA_CONTRAST | IMA_SATURATION | IMA_HUE;
            ret = 0;
            break;

        default:
            ret = -1;
            show_warn("parm_id Invalid!!!!!\n");
            break;
    }

    return ret;

GET_CFG:
    ret = HieClient_GetConfigV2(user_id, (ConfigInformationV2*)parm);
    show_debug("HieClient_GetConfigV2: %s<-------------------------------\n", 
        !ret ? "Success" : "Failure");
    if (!ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", ret);
        return -1;
    }
}
static int 
hie_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int cmd, ret = -1;
    HUSER user_id;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hie_service_t *hie_srv;

    NMP_ASSERT(srv && parm_id && parm);

    hie_srv = (hie_service_t*)srv;
    user_id = hie_get_user_id(&hie_srv->parm);

    if (HIE_INVALID_HANDLE == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    switch (parm_id)
    {
        case SET_SERIAL_CONFIG:
            goto SET_CFG;
        case SET_DEVICE_TIME:
            goto SET_CFG;
        case SET_NTP_CONFIG:
            goto SET_CFG;
        case SET_NETWORK_CONFIG:
            goto SET_CFG;
        case SET_PPPOE_CONFIG:
            goto SET_CFG;
        case SET_FTP_CONFIG:
            goto SET_CFG;
        case SET_SMTP_CONFIG:
            goto SET_CFG;
        case SET_DDNS_CONFIG:
            goto SET_CFG;
        case SET_UPNP_CONFIG:
            goto SET_CFG;

        case SET_DISK_FORMAT:
            ctrl = proxy_new_ctrl(srv, CTRL_CMD_FRMTDISK, (void*)parm);
            if (!ctrl)
                ret = -1;
            else
            {
                task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                        proxy_free_ctrl, hie_srv->owner);
                if (task)
                {
                    proxy_thread_pool_push(task);
                    ret = 0;
                }
                else
                {
                    proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                    ret = -1;
                }
            }
            break;

        case CONTROL_DEVICE_CMD:
            switch (((JControlDevice*)parm)->command)
            {
                case SHUTDOWN_DEVICE:
                    ret = 0;
                    cmd = CTRL_CMD_SHUTDOWN;
                    break;
                case RESTART_DEVICE:
                    ret = 0;
                    cmd = CTRL_CMD_REBOOT;
                    break;
                case RESTORE_DEFAULT:
                    ret = 0;
                    cmd = CTRL_CMD_RESET;
                    break;

                case DETECT_DEAL_PIX:
                    ret = -1;
                    break;
                case DETECT_IRIS:
                    ret = -1;
                    break;

                default:
                    ret = -1;
                    break;
            }

            if (!ret)
            {
                ctrl = proxy_new_ctrl(srv, cmd, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hie_srv->owner);
                    if (task)
                    {
                        proxy_thread_pool_push(task);
                        ret = 0;
                    }
                    else
                    {
                        proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                        ret = -1;
                    }
                }
            }
            break;

        case SET_ENCODE_CONFIG:
            goto SET_CFG;
        case SET_DISPLAY_CONFIG:
            goto SET_CFG;
        case SET_OSD_CONFIG:
            goto SET_CFG;
        case SET_PTZ_CONFIG:
            goto SET_CFG;
        case SET_RECORD_CONFIG:
            goto SET_CFG;
        case SET_HIDE_CONFIG:
            goto SET_CFG;
        case SET_MOTION_CONFIG:
            goto SET_CFG;
        case SET_VIDEO_LOST_CONFIG:
            goto SET_CFG;
        case SET_HIDE_ALARM_CONFIG:
            goto SET_CFG;
        case SET_IO_ALARM_CONFIG:
            goto SET_CFG;

        case CONTROL_PTZ_CMD:
        case SET_PRESET_CONFIG:
            /*ret = HB_NET_PTZControl(user_id, (HB_NET_PTZCTRL*)parm);
            show_debug("HB_NET_PTZControl: %s<------------------------------\n", 
                ret ? "Success" : "Failure");*/
            break;

        case SET_CRUISE_CONFIG:
        case ADD_CRUISE_CONFIG:
        case MDF_CRUISE_CONFIG:
            break;

        default:
            ret = -1;
            break;
    }

    return ret;

SET_CFG:
    ret = HieClient_SetConfigV2(user_id, (ConfigInformationV2*)parm);
    show_debug("HieClient_SetConfigV2: %s<-------------------------------\n", 
        !ret ? "Success" : "Failure");
    if (!ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", ret);
        return -1;
    }
}

hie_service_basic_t hie_srv_basic = 
{
    {
        g_sdk_items[SDK_HIE].sdk_name,
        hie_init,
        hie_cleanup,
        hie_create_service,
        hie_delete_service,
        hie_control_service,
        hie_check_service,
    },
    {
        hie_get_device_config,
        hie_set_device_config,
    },
    0,
};

//////////////////////////////////////////////////////////////////////////
int 
hie_get_state(hie_parm_t *pm, int *state_timer)
{
    int state;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    state = pm->state;
    if (state_timer)
        *state_timer = pm->state_timer++;
    nmp_mutex_unlock(pm->lock);

    return state;
}
int 
hie_set_state(hie_service_t *hie_srv, HIE_STATE_E state, int error)
{
    int flag, old;
    hie_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(hie_srv);

    old = hie_get_state(&hie_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("HIE State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case HIE_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case HIE_LOGING:
            flag = STATE_LOGING;
            break;
        case HIE_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return HIE_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(hie_srv->owner, &dev_state))
    {
        pm = &hie_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }
    
done:
    return old;
}
HUSER 
hie_get_user_id(hie_parm_t *pm)
{
    HUSER user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
HUSER 
hie_set_user_id(hie_service_t *hie_srv, HUSER user_id)
{
    HUSER old;
    hie_parm_t *pm;

    NMP_ASSERT(hie_srv);

    pm = &hie_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (HIE_INVALID_HANDLE != old)
        hie_logout(old);

    if (HIE_INVALID_HANDLE == user_id)
        hie_stop_all_stream(&hie_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
hie_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (HIE_INVALID_HANDLE != strm_info->handle)
    {
        //删除流媒体数据回调
        HieClient_StreamMediaCB((HANDLE)strm_info->handle, NULL, 0);

        //停止实时流
        HieClient_StreamMediaControl((HANDLE)strm_info->handle, eTaskStop);

        //断开实时流连接
        HieClient_RealStreamDisconnect((HANDLE)strm_info->handle);

        strm_info->handle = HIE_INVALID_HANDLE;
    }

    return ;
}
static void 
hie_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (HIE_INVALID_HANDLE != strm_info->handle)
    {
        //删除流媒体数据回调
        HieClient_StreamMediaCB((HANDLE)strm_info->handle, NULL, 0);

        //停止历史流
        HieClient_StreamMediaControl((HANDLE)strm_info->handle, eTaskStop);

        //销毁历史流通道
        HieClient_HistoryStreamDestroy((HANDLE)strm_info->handle);

        strm_info->handle = HIE_INVALID_HANDLE;
    }

    return ;
}

static void hie_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void hie_stop_all_stream(struct hie_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        hie_stop_one_stream, (void*)hie_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        hie_stop_one_stream, (void*)hie_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
hie_cleanup_stream_info(struct hie_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        hie_stop_real_stream, sizeof(hie_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        hie_stop_record_stream, sizeof(hie_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}

/*static __inline__ BOOL
hie_find_record_file(LONG user_id, int channel, JStoreLog *store_log)
{
    BOOL retval = FALSE;
    HB_NET_FILEFINDCOND find_cond;
    HB_NET_FINDDATA     find_data;
    int type, find_handle, index = 0, log_node = 0;

    memset(&find_cond, 0, sizeof(HB_NET_FILEFINDCOND));
    find_cond.dwSize = sizeof(HB_NET_FILEFINDCOND);
    find_cond.dwFileType = (HB_NET_RECTYPE_E)store_log->rec_type;
    find_cond.dwChannel  = channel;
    hie_time_swap(&find_cond.struStartTime, &store_log->beg_time, SWAP_UNPACK);
    hie_time_swap(&find_cond.struStopTime, &store_log->end_time, SWAP_UNPACK);

    find_handle = HB_NET_FindFile(user_id, &find_cond);
    if(-1 == find_handle)
    {
        show_warn("HB_NET_FindFile(%d), FileType: %d\n", 
            (int)HB_NET_GetLastError(), find_cond.dwFileType);
        return retval;
    }

    memset(&find_data, 0, sizeof(HB_NET_FINDDATA));
    find_data.dwSize = sizeof(HB_NET_FINDDATA);

    while(TRUE)
    {
        type = HB_NET_FindNextFile(find_handle, &find_data);
        if(type == HB_NET_ISFINDING)
        {
            usleep(0.5*1000*1000);
            continue;
        }
        else if(type == HB_NET_FILE_SUCCESS)
        {
            if (index >= J_SDK_MAX_STORE_LOG_SIZE)
            {
                retval = TRUE;
                break;
            }

            if ((int)store_log->beg_node <= log_node && 
                (int)store_log->end_node >= log_node)
            {
                store_log->store[index].rec_type = find_data.dwFileType;
                store_log->store[index].file_size = find_data.dwFileSize;
                hie_time_swap(&find_data.struStartTime, 
                    &store_log->store[index].beg_time, SWAP_PACK);
                hie_time_swap(&find_data.struStopTime, 
                    &store_log->store[index].end_time, SWAP_PACK);

                index++;
            }

            log_node++;
        }
        else if(type == HB_NET_FILE_NOFIND || type == HB_NET_NOMOREFILE)
        {
            show_warn("HB_NET_FILE_NOFIND or HB_NET_NOMOREFILE\n");
            store_log->node_count = index;
            store_log->total_count = log_node;

            retval = TRUE;
            break;
        }
        else if (type == HB_NET_FILE_EXCEPTION)
        {
            show_warn("HB_NET_FILE_EXCEPTION!!!!Error: %d\n", (int)HB_NET_GetLastError());
            break;
        }
        else
        {
            store_log->node_count  = index;
            store_log->total_count = log_node;

            retval = TRUE;
            break;
        }
    }

    HB_NET_FindFileClose(find_handle);
    return retval;
}*/




