#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_hbn_handler.h"
#include "nmp_hbn_channel.h"
#include "nmp_hbn_srv_impl.h"
#include "nmp_hbn_service.h"
#include "nmp_hbn_talk.h"

extern void hb_log_set_level(int level);

static __inline__ BOOL
hbn_find_record_file(LONG user_id, int channel, JStoreLog *store_log);


static __inline__ void 
hbn_time_swap(HB_NET_TIME *time, JTime *jtime, int flag)
{
    switch(flag)
    {
        case SWAP_UNPACK:
            time->dwYear   = jtime->year + 1900;
            time->dwMonth  = jtime->month;
            time->dwDay    = jtime->date;
            time->dwHour   = jtime->hour;
            time->dwMinute = jtime->minute;
            time->dwSecond = jtime->second;
            time->dwMillisecond = 0;
            break;
        case SWAP_PACK:
            jtime->year    = time->dwYear - 1900;
            jtime->month   = time->dwMonth;
            jtime->date    = time->dwDay;
            jtime->hour    = time->dwHour;
            jtime->minute  = time->dwMinute;
            jtime->second  = time->dwSecond;
            break;

        default:
            break;
    }
}

static void hbn_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int hbn_logout(long user_id)
{
    if (HB_NET_Logout(user_id))
    {
        show_debug("hbn_logout() successful, user_id: %d\n", (int)user_id);
        return 0;
    }
    else
        return -1;
}
static __inline__ void hbn_login(hbn_service_t *hbn_srv)
{
    char addr[MAX_IP_LEN];
    int state, error = 0;
    int user_id = HBN_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(hbn_srv);

    memset(&sdk_info, 0, sizeof(proxy_sdk_t));
    if (proxy_get_device_private(hbn_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        user_id = HB_NET_Login(addr, (WORD)sdk_info.dev_port, 
                    sdk_info.username, sdk_info.password, 
                    &hbn_srv->hbn_info);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (HBN_LOGOUT != user_id)
    {
        state = HBN_LOGIN;
        error = 0;
        show_debug("hbn_login() successful, user_id: %d\n", user_id);
    }
    else
    {
#if 0
    static int allow = 0;
    if (allow > 500)
    {
#endif
        state = HBN_LOGOUT;
        error = (int)HB_NET_GetLastError();
#if 0
    }
    else
    {
        user_id = allow++ + 500;
        state = HBN_LOGIN;
        error = 0;
    }
#endif

        if (strlen(addr))
        {
            show_warn("hbn_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                hbn_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    hbn_set_user_id(hbn_srv, user_id);
    hbn_set_state(hbn_srv, (HBN_STATE_E)state, error);

    return ;
}

static int hbn_init(service_template_t *self)
{
    int ret = 0;
    HB_NET_ALARMPARAM alarm;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(self);

    hbn_basic = (hbn_service_basic_t*)self;

    hb_log_set_level(-1);

    if (!HB_NET_Init())
    {
        ret = -1;
        show_warn("HB_NET_Init() failure!\n");
    }
    else
    {
        memset(&alarm, 0, sizeof(HB_NET_ALARMPARAM));
        alarm.dwSize = sizeof(HB_NET_ALARMPARAM);
        alarm.pfnCallback = hbn_process_alarm_info;
        alarm.pContext = (void*)hbn_basic;

        if (!HB_NET_SetDevAlarmCallback(&alarm))
        {
            ret = -1;
            show_debug("HB_NET_SetDevAlarmCallback() failure!!!!!!!!!!!!\n");
        }
    }

    return ret;
}
static int hbn_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    HB_NET_Cleanup();
    return 0;
}

static struct service *
hbn_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    hbn_service_t *hbn_srv;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(self && init_data);

    hbn_basic = (hbn_service_basic_t*)self;
    prx_dev   = (proxy_device_t*)init_data;

    hbn_srv = (hbn_service_t*)nmp_new0(hbn_service_t, 1);
    if (hbn_srv)
    {
        hbn_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        hbn_srv->base.tm = self;
        hbn_srv->parm.lock = nmp_mutex_new();
        hbn_srv->parm.user_id = HBN_LOGOUT;
        hbn_srv->parm.state = HBN_LOGOUT;
        hbn_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接
                                                //设备，而不是等待整个策略周期
        hbn_srv->rtsp.lock = nmp_mutex_new();
        memset(&hbn_srv->rtsp.real_header, 0, sizeof(hbn_strm_header_t));
        hbn_srv->rtsp.real_strm_list.list = NULL;
        hbn_srv->rtsp.rec_strm_list.list = NULL;

        hbn_srv->owner = prx_dev;

        memset(&hbn_srv->hbn_info, 0, sizeof(HB_NET_DEVICEINFO));
        hbn_srv->hbn_info.dwSize = sizeof(HB_NET_DEVICEINFO);
    }

    return (struct service *)hbn_srv;
}
static void 
hbn_delete_service(service_template_t *self, struct service *srv)
{
    hbn_service_t *hbn_srv;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(self && srv);

    hbn_srv = (hbn_service_t*)srv;
    hbn_basic = (hbn_service_basic_t*)self;

    hbn_basic->total_count -= 1;

    hbn_set_user_id(hbn_srv, HBN_LOGOUT);               //注销登录
    hbn_set_state(hbn_srv, HBN_LOGOUT, 0);

    /* 清理视频流链表 */
    hbn_cleanup_stream_info(&hbn_srv->rtsp);

    if (hbn_srv->parm.lock)
        nmp_mutex_free(hbn_srv->parm.lock);
    if (hbn_srv->rtsp.lock)
        nmp_mutex_free(hbn_srv->rtsp.lock);             //销毁相关锁

    memset(hbn_srv, 0, sizeof(hbn_service_t));          //内存块归零
    nmp_del(hbn_srv, hbn_service_t, 1);                   //释放内存块
    return ;
}

static int 
hbn_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, handle, ret = -1;
    hbn_service_t *hbn_srv;
    hbn_service_basic_t *hbn_basic;

    NMP_ASSERT(self && srv);

    hbn_srv = (hbn_service_t*)srv;
    hbn_basic = (hbn_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!hbn_logout((LONG)hbn_get_user_id(&hbn_srv->parm)))
            {
                ret = 0;
                hbn_set_user_id(hbn_srv, HBN_LOGOUT);
                hbn_set_state(hbn_srv, HBN_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            hbn_login(hbn_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            sleep(2);
            if (HB_NET_RestoreConfig((LONG)hbn_get_user_id(&hbn_srv->parm)))
            {
                ret = 0;
                hbn_set_user_id(hbn_srv, HBN_LOGOUT);
                hbn_set_state(hbn_srv, HBN_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_REBOOT:
            sleep(2);
            if (HB_NET_Reboot((LONG)hbn_get_user_id(&hbn_srv->parm)))
            {
                ret = 0;
                hbn_set_user_id(hbn_srv, HBN_LOGOUT);
                hbn_set_state(hbn_srv, HBN_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_SHUTDOWN:
            sleep(2);
            if (HB_NET_ShutDown((LONG)hbn_get_user_id(&hbn_srv->parm)))
            {
                ret = 0;
                hbn_set_user_id(hbn_srv, HBN_LOGOUT);
                hbn_set_state(hbn_srv, HBN_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_FRMTDISK:
            handle = HB_NET_FormatDisk((LONG)hbn_get_user_id(&hbn_srv->parm), 
                        *((int*)parm));
            if (HBN_INVALID_HANDLE != handle)
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
                        sizeof(FormatProgressPacket), 0, hbn_free_msg_data);

                do
                {
                    sleep(1);
                    if (HB_NET_GetFormatProgress((LONG)handle, &frmt_info))
                    {
                        show_debug("=============>>hbn format disk: [%d, %d]\n", 
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

                    (*hbn_srv->owner->plt_srv->tm->control_service)(
                        hbn_srv->owner->plt_srv->tm, 
                        hbn_srv->owner->plt_srv, CTRL_CMD_SUBMIT, msg);

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
                    *((int*)parm)+1, (int)HB_NET_GetLastError());
            break;
        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == hbn_get_user_id(&hbn_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&hbn_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&hbn_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            *(int*)parm = (int)&hbn_talk_opt;
            ret = 0;
            break;

        default:
            break;
    }

    return ret;
}
static int 
hbn_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hbn_service_t *hbn_srv;

    NMP_ASSERT(self && srv);

    hbn_srv = (hbn_service_t*)srv;

    switch (hbn_get_state(&hbn_srv->parm, &state_timer))
    {
        case HBN_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                hbn_set_user_id(hbn_srv, HBN_LOGOUT);
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hbn_srv->owner);
                    if (task)
                    {
                        hbn_set_state(hbn_srv, HBN_LOGING, 0);
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

        case HBN_LOGING:
            break;

        case HBN_LOGIN:
            break;
    }

    return ret;
}

static int 
hbn_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;
    int user_id;

    JDevCap *dev_cap;
    get_store_t *get_store;

    hbn_service_t *hbn_srv;

    NMP_ASSERT(srv && parm);

    hbn_srv = (hbn_service_t*)srv;
    user_id = hbn_get_user_id(&hbn_srv->parm);

    if (HBN_LOGOUT == user_id)
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
            get_store = (get_store_t*)parm;
            ret = hbn_find_record_file(user_id, get_store->channel, 
                    (JStoreLog*)get_store->buffer);
            break;

        case GET_CRUISE_CONFIG:
            /*cruise = (hbn_cruise_t*)parm;
            show_debug("crz_no: %d<--------------------\n", cruise->crz_no);
            ret = NET_DVR_GetPTZCruise(user_id, 
                    (LONG)cruise->channel, 
                    (LONG)cruise->crz_no, 
                    (NET_DVR_CRUISE_RET*)cruise->input);
            show_debug("NET_DVR_GetDVRConfig: %s<-------------------------------\n", 
                (ret == TRUE) ? "Success" : "Failure");
            if (FALSE == ret)
                show_debug("ERROR: %d<-------------------------------\n", (int)HB_NET_GetLastError());*/
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
            ret = TRUE;
            break;

        default:
            ret = FALSE;
            show_warn("parm_id Invalid!!!!!\n");
            break;
    }

    if (TRUE == ret)
        return 0;
    else
        return -1;

GET_CFG:
    ret = HB_NET_GetDevConfig((LONG)user_id, (HB_NET_GETDEVCONFIG*)parm);
    show_debug("HB_NET_GetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", (int)HB_NET_GetLastError());
        return -1;
    }
}
static int 
hbn_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;
    int user_id, cmd;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hbn_service_t *hbn_srv;

    NMP_ASSERT(srv && parm_id && parm);

    hbn_srv = (hbn_service_t*)srv;
    user_id = hbn_get_user_id(&hbn_srv->parm);

    if (HBN_LOGOUT == user_id)
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
                ret = FALSE;
            else
            {
                task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                        proxy_free_ctrl, hbn_srv->owner);
                if (task)
                {
                    proxy_thread_pool_push(task);
                    ret = TRUE;
                }
                else
                {
                    proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                    ret = FALSE;
                }
            }
            break;

        case CONTROL_DEVICE_CMD:
            switch (((JControlDevice*)parm)->command)
            {
                case SHUTDOWN_DEVICE:
                    ret = TRUE;
                    cmd = CTRL_CMD_SHUTDOWN;
                    break;
                case RESTART_DEVICE:
                    ret = TRUE;
                    cmd = CTRL_CMD_REBOOT;
                    break;
                case RESTORE_DEFAULT:
                    ret = TRUE;
                    cmd = CTRL_CMD_RESET;
                    break;

                case DETECT_DEAL_PIX:
                    ret = FALSE;
                    break;
                case DETECT_IRIS:
                    ret = FALSE;
                    break;

                default:
                    ret = FALSE;
                    break;
            }

            if (ret)
            {
                ctrl = proxy_new_ctrl(srv, cmd, NULL);
                if (!ctrl)
                    ret = FALSE;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hbn_srv->owner);
                    if (task)
                    {
                        proxy_thread_pool_push(task);
                        ret = TRUE;
                    }
                    else
                    {
                        proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                        ret = FALSE;
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
            ret = HB_NET_PTZControl(user_id, (HB_NET_PTZCTRL*)parm);
            show_debug("HB_NET_PTZControl: %s<------------------------------\n", 
                ret ? "Success" : "Failure");
            break;

        case SET_CRUISE_CONFIG:
        case ADD_CRUISE_CONFIG:
        case MDF_CRUISE_CONFIG:
            break;

        default:
            ret = FALSE;
            break;
    }

    if (TRUE == ret)
        return 0;
    else
        return -1;

SET_CFG:
    ret = HB_NET_SetDevConfig((LONG)user_id, (HB_NET_SETDEVCONFIG*)parm);
    show_debug("HB_NET_SetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", 
            (int)HB_NET_GetLastError());
        return -1;
    }
}

hbn_service_basic_t hbn_srv_basic = 
{
    {
        g_sdk_items[SDK_HBN].sdk_name,
        hbn_init,
        hbn_cleanup,
        hbn_create_service,
        hbn_delete_service,
        hbn_control_service,
        hbn_check_service,
    },
    {
        hbn_get_device_config,
        hbn_set_device_config,
    },
    0,
};

//////////////////////////////////////////////////////////////////////////
int hbn_get_state(hbn_parm_t *pm, int *state_timer)
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
int hbn_set_state(hbn_service_t *hbn_srv, HBN_STATE_E state, int error)
{
    int flag, old;
    hbn_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(hbn_srv);

    old = hbn_get_state(&hbn_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("HBN State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case HBN_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case HBN_LOGING:
            flag = STATE_LOGING;
            break;
        case HBN_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return HBN_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(hbn_srv->owner, &dev_state))
    {
        pm = &hbn_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }
    
done:
    return old;
}
int hbn_get_user_id(hbn_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int hbn_set_user_id(hbn_service_t *hbn_srv, int user_id)
{
    int old;
    hbn_parm_t *pm;

    NMP_ASSERT(hbn_srv);

    pm = &hbn_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (HBN_LOGOUT != old)
        hbn_logout(old);

    if (HBN_LOGOUT == user_id)
        hbn_stop_all_stream(&hbn_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
hbn_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (HBN_INVALID_HANDLE != strm_info->handle)
    {
        HB_NET_StopRealPlay((LONG)strm_info->handle);
        strm_info->handle = HBN_INVALID_HANDLE;
    }

    return ;
}
static void 
hbn_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (HBN_INVALID_HANDLE != strm_info->handle)
    {
        HB_NET_StopPlayBack((LONG)strm_info->handle);
        strm_info->handle = HBN_INVALID_HANDLE;
    }

    return ;
}

static void hbn_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void hbn_stop_all_stream(struct hbn_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        hbn_stop_one_stream, (void*)hbn_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        hbn_stop_one_stream, (void*)hbn_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
hbn_cleanup_stream_info(struct hbn_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        hbn_stop_real_stream, sizeof(hbn_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        hbn_stop_record_stream, sizeof(hbn_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}

static __inline__ BOOL
hbn_find_record_file(LONG user_id, int channel, JStoreLog *store_log)
{
    BOOL retval = FALSE;
    HB_NET_FILEFINDCOND find_cond;
    HB_NET_FINDDATA     find_data;
    int type, find_handle, index = 0, log_node = 0;

    memset(&find_cond, 0, sizeof(HB_NET_FILEFINDCOND));
    find_cond.dwSize = sizeof(HB_NET_FILEFINDCOND);
    find_cond.dwFileType = (HB_NET_RECTYPE_E)store_log->rec_type;
    find_cond.dwChannel  = channel;
    hbn_time_swap(&find_cond.struStartTime, &store_log->beg_time, SWAP_UNPACK);
    hbn_time_swap(&find_cond.struStopTime, &store_log->end_time, SWAP_UNPACK);

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
                hbn_time_swap(&find_data.struStartTime, 
                    &store_log->store[index].beg_time, SWAP_PACK);
                hbn_time_swap(&find_data.struStopTime, 
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
}



