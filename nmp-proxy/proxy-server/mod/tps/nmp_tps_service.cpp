#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_tps_channel.h"
#include "nmp_tps_talk.h"

#include "nmp_tps_swap.h"
#include "nmp_tps_handler.h"
#include "nmp_tps_srv_impl.h"
#include "nmp_tps_service.h"



static __inline__ void 
tps_submit_format_progress(tps_service_t *tps_srv, int disk_no);


/*static __inline__ void 
tps_time_swap(int flag, IP_NET_DVR_TIME *time, JTime *time0)
{
    switch(flag)
    {
        case SWAP_UNPACK:
            time->dwYear   = time0->year + 1900;
            time->dwMonth  = time0->month;
            time->dwDay    = time0->date;
            time->dwHour   = time0->hour;
            time->dwMinute = time0->minute;
            time->dwSecond = time0->second;
            break;
        case SWAP_PACK:
            time0->year    = time->dwYear - 1900;
            time0->month   = time->dwMonth;
            time0->date    = time->dwDay;
            time0->hour    = time->dwHour;
            time0->minute  = time->dwMinute;
            time0->second  = time->dwSecond;
            break;

        default:
            break;
    }
}*/

static void tps_free_ctrl_user(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static void tps_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int tps_logout(long user_id)
{
    if (!IP_NET_DVR_Logout(user_id))
    {
        show_debug("tps_logout() successful, user_id: %d\n", (int)user_id);
        return 0;
    }
    else
        return -1;
}

static __inline__ void tps_login(tps_service_t *tps_srv)
{
    char addr[MAX_IP_LEN];
    //int state, error = 0;
    int user_id = TPS_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(tps_srv);

    if (proxy_get_device_private(tps_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, 
        addr, sizeof(addr)))
    {
        user_id = IP_NET_DVR_Login(addr, sdk_info.dev_port, 
                    sdk_info.username, sdk_info.password, 
                    &tps_srv->tps_info);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (TPS_LOGOUT != user_id)
    {
        IP_NET_DVR_SetAutoReconnect((LONG)user_id, 0);
        //IP_NET_DVR_StartListen(user_id);

        //state = TPS_LOGIN;
        //error = 0;
        show_debug("tps_login() successful, user_id: %d\n", user_id);
    }
    else
    {
        //state = TPS_LOGOUT;
        if (strlen(addr))
        {
            show_warn("tps_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                tps_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port/*, error*/);
        }
    }

    tps_set_user_id(tps_srv, user_id);
    //tps_set_state(tps_srv, (TPS_STATE_E)state, error);
    return ;
}

static int tps_init(service_template_t *self)
{
    int ret = 0;
    tps_conn_t *conn_info;
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(self);

    tps_basic = (tps_service_basic_t*)self;
    conn_info = &tps_basic->conn_info;

    if (!IP_NET_DVR_Init())
    {
        IP_NET_DVR_SetStatusEventCallBack(tps_status_event_call_back, (void*)tps_basic);
        //IP_NET_DVR_SetConnectTime(conn_info->conn_wait_time, conn_info->conn_try_times);
        //IP_NET_DVR_SetExceptionCallBack(tps_message_call_back, (unsigned long)tps_basic);
    }
    else
    {
        ret = -1;
        show_warn("NET_DVR_Init() failure!\n");
    }

    return ret;
}

static int tps_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    IP_NET_DVR_Cleanup();
    return 0;
}

static struct service *
tps_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    tps_service_t *tps_srv;
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(self && init_data);

    tps_basic = (tps_service_basic_t*)self;
    prx_dev  = (proxy_device_t*)init_data;

    tps_srv = (tps_service_t*)nmp_new0(tps_service_t, 1);
    if (tps_srv)
    {
        tps_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        tps_srv->base.tm = self;
        tps_srv->parm.lock = nmp_mutex_new();
        tps_srv->parm.user_id = TPS_LOGOUT;
        tps_srv->parm.state = TPS_LOGOUT;
        tps_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

        tps_srv->rtsp.lock = nmp_mutex_new();
        tps_srv->rtsp.real_strm_list.list = NULL;
        tps_srv->rtsp.rec_strm_list.list = NULL;

        tps_srv->owner = prx_dev;
    }

    return (struct service*)tps_srv;
}

static void 
tps_delete_service(service_template_t *self, struct service *srv)
{
    tps_service_t *tps_srv;
    tps_service_basic_t *tps_basic;

    NMP_ASSERT(self && srv);

    tps_srv = (tps_service_t*)srv;
    tps_basic = (tps_service_basic_t*)self;

    tps_basic->total_count -= 1;

    tps_set_user_id(tps_srv, TPS_LOGOUT);             //注销登录
    tps_set_state(tps_srv, TPS_LOGOUT, 0);

    /* 清理视频流链表 */
    tps_cleanup_stream_info(&tps_srv->rtsp);

    if (tps_srv->parm.lock)
        nmp_mutex_free(tps_srv->parm.lock);
    if (tps_srv->rtsp.lock)
        nmp_mutex_free(tps_srv->rtsp.lock);             //销毁相关锁

    memset(tps_srv, 0, sizeof(tps_service_t));        //内存块归零
    nmp_del(tps_srv, tps_service_t, 1);                 //释放内存块
    return ;
}

static int 
tps_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, ret = -1;
    tps_service_t *tps_srv;
    tps_service_basic_t *tps_basic;
    //SDK_NetKeyBoardData key_data;

    NMP_ASSERT(self && srv);

    tps_srv = (tps_service_t*)srv;
    tps_basic = (tps_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!tps_logout((long)tps_get_user_id(&tps_srv->parm)))
            {
                ret = 0;
                tps_set_user_id(tps_srv, TPS_LOGOUT);
                tps_set_state(tps_srv, TPS_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            tps_login(tps_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            break;
        case CTRL_CMD_REBOOT:
            /*if (IP_NET_DVR_ControlDVR((long)tps_get_user_id(&tps_srv->parm), 0))
            {
                ret = 0;
                tps_set_user_id(tps_srv, TPS_LOGOUT);
                tps_set_state(tps_srv, TPS_LOGOUT, 0);
            }
            else
                show_warn("IP_NET_DVR_ControlDVR fail, err: %d\n", 
                    (int)IP_NET_DVR_GetLastError());*/
            break;
        case CTRL_CMD_SHUTDOWN:
            /*key_data.iValue = SDK_NET_KEY_SHUT;
            key_data.iState = SDK_NET_KEYBOARD_KEYDOWN;
            if (IP_NET_DVR_ClickKey((long)tps_get_user_id(&tps_srv->parm), &key_data))
            {
                ret = 0;
                tps_set_user_id(tps_srv, TPS_LOGOUT);
                tps_set_state(tps_srv, TPS_LOGOUT, 0);
            }
            else
                show_warn("IP_NET_DVR_ClickKey fail, err: %d\n", 
                    (int)IP_NET_DVR_GetLastError());*/
            break;

        case CTRL_CMD_FRMTDISK:
            tps_submit_format_progress(tps_srv, *(int*)parm);
            ret = 0;
            break;

        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == tps_get_user_id(&tps_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&tps_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&tps_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            //*(int*)parm = (int)&tps_talk_opt;
            ret = -1;
            break;

        default:
            break;
    }

    return ret;
}
static int 
tps_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    tps_service_t *tps_srv;

    NMP_ASSERT(self && srv);

    tps_srv = (tps_service_t*)srv;

    switch (tps_get_state(&tps_srv->parm, &state_timer))
    {
        case TPS_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, tps_srv->owner);
                    if (task)
                    {
                        tps_set_state(tps_srv, TPS_LOGING, 0);
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

        case TPS_LOGING:
            if (state_timer > DEFAULT_CONNECTING_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGOUT, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, tps_srv->owner);
                    if (task)
                    {
                        tps_set_state(tps_srv, TPS_OUTING, 0);
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

        case TPS_LOGIN:
            break;

        case TPS_OUTING:
            ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGOUT, NULL);
            if (!ctrl)
                ret = -1;
            else
            {
                task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                        proxy_free_ctrl, tps_srv->owner);
                if (task)
                {
                    tps_set_state(tps_srv, TPS_LOGOUT, 0);
                    proxy_thread_pool_push(task);
                }
                else
                {
                    proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                    ret = -2;
                }
            }
            break;
    }

    return ret;
}

static int 
tps_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int user_id, ret = FALSE;

    tps_service_t *tps_srv;
    tps_config_t  *tps_cfg;

    JDevCap *dev_cap;

    NMP_ASSERT(srv && parm);

    tps_query_t *query_file;
    //struct IP_NET_DVR_FINDINFO find_info;

    tps_srv = (tps_service_t*)srv;
    user_id = tps_get_user_id(&tps_srv->parm);

    if (TPS_LOGOUT == user_id)
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
            /*query_file = (tps_query_t*)parm;
            find_info.nChannelN0 = query_file->channel;
            find_info.nFileType = query_file->file_type;
            memcpy(&find_info.startTime, &query_file->start, sizeof(IP_NET_DVR_TIME));
            memcpy(&find_info.endTime, &query_file->end, sizeof(IP_NET_DVR_TIME));
            ret = IP_NET_DVR_FindFile((long)user_id, &find_info, 
                    (struct IP_NET_DVR_FILE_DATA*)query_file->buffer, 
                    query_file->max_count, &query_file->filecount);*/
            break;

        case GET_CRUISE_CONFIG:
            break;

        case GET_CAPABILITY_SET:
            dev_cap = (JDevCap*)parm;
            dev_cap->cap0 = DEV_CAP_VIDEO_IN | DEV_CAP_AUDIO | DEV_CAP_IRIS | DEV_CAP_PTZ | 
                DEV_CAP_ALARM_IN | DEV_CAP_ALARM_OUT | DEV_CAP_STORAGE | DEV_CAP_WEB | DEV_CAP_PLATFORM | 
                DEV_CAP_INTELLIGENT_ANALYSIS | DEV_CAP_UPDATE | DEV_CAP_VIDEO_OUT;// | DEV_CAP_IR;
            dev_cap->ftp_enable = 1;
            dev_cap->upnp_enable = 1;
            dev_cap->chn_cap.size = sizeof(JChnCap);
            dev_cap->chn_cap.encode = VIDEO_ENCODE_H264_E/*/ | VIDEO_ENCODE_MJPEG_E | 
                VIDEO_ENCODE_JPEG_E | VIDEO_ENCODE_MPEG4_E*/;
            dev_cap->chn_cap.supp_mask = 0;
            dev_cap->chn_cap.mask_count = 0;
            dev_cap->chn_cap.supp_hide_alarm = 1;
            dev_cap->chn_cap.hide_alarm_count = 1;
            dev_cap->chn_cap.supp_move_alarm = 1;
            dev_cap->chn_cap.move_alarm_count = 1;
            dev_cap->chn_cap.supp_video_lost_alarm = 1;
            dev_cap->chn_cap.osd_count = 0;
            dev_cap->chn_cap.stream_count = 2;
            dev_cap->chn_cap.stream_supp_resolution[0] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
                CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
            dev_cap->chn_cap.stream_supp_resolution[1] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
                CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
            dev_cap->chn_cap.stream_supp_resolution[2] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
                CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
            dev_cap->chn_cap.stream_supp_resolution[3] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
                CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
            dev_cap->chn_cap.stream_max_frate_rate[0] = 25;
            dev_cap->chn_cap.img_cap = IMA_BRIGHTNESS | IMA_CONTRAST | IMA_SATURATION | IMA_HUE | IMA_SHARPNESS;
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
    tps_cfg = (tps_config_t*)parm;
    /*ret = IP_NET_DVR_GetDevConfig((long)user_id, 
            (unsigned long ) tps_cfg->command, 
            (int           ) tps_cfg->channel, 
            (char*         ) tps_cfg->buffer, 
            (unsigned long ) tps_cfg->b_size, 
            (unsigned long*)&tps_cfg->returns, 
            (int           ) tps_cfg->waittime);*/
    show_debug("IP_NET_DVR_GetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        //show_debug("ERROR: %d<-------------------------------\n", 
          //  (int)IP_NET_DVR_GetLastError());
        return -1;
    }
}
static int 
tps_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;//, err = 0;
    int user_id, cmd;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;

    tps_service_t *tps_srv;
    tps_config_t  *tps_cfg;

    NMP_ASSERT(srv && parm_id && parm);

    tps_srv = (tps_service_t*)srv;
    user_id = tps_get_user_id(&tps_srv->parm);

    if (TPS_LOGOUT == user_id)
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
            /*ret = IP_NET_DVR_StorageManage((long)user_id, 
                    (SDK_StorageDeviceControl*)parm);
            if (0 < ret)
            {
                int *idx = (int*)nmp_alloc(sizeof(int));
                *idx = ((SDK_StorageDeviceControl*)parm)->iSerialNo;
                ctrl = proxy_new_ctrl_2(srv, CTRL_CMD_FRMTDISK, 
                        (void*)idx, sizeof(int), tps_free_ctrl_user);
                if (!ctrl)
                    ret = FALSE;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, tps_srv->owner);
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
            else*/
                ret = FALSE;
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
                            proxy_free_ctrl, tps_srv->owner);
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
            /*ret = IP_NET_DVR_PTZControl((long)user_id, 
                    (long)((tps_ptz_ctrl_t*)parm)->ptz_cmd,
                    (int )((tps_ptz_ctrl_t*)parm)->);*/
            break;

        case SET_CRUISE_CONFIG:
        case ADD_CRUISE_CONFIG:
        case MDF_CRUISE_CONFIG:
            ret = FALSE;
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
    tps_cfg = (tps_config_t*)parm;
    /*ret = IP_NET_DVR_SetDevConfig((long         )user_id, 
                                (unsigned long)tps_cfg->command, 
                                (int          )tps_cfg->channel, 
                                (char*        )tps_cfg->buffer, 
                                (unsigned long)tps_cfg->b_size, 
                                (int          )tps_cfg->waittime);*/
    show_debug("IP_NET_DVR_SetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");

    if (TRUE == ret)
        return 0;
    else
    {
        //show_debug("ERROR: %d<-------------------------------\n", 
          //  (int)IP_NET_DVR_GetLastError());
        return -1;
    }
}

tps_service_basic_t tps_srv_basic = 
{
    {
        g_sdk_items[SDK_TPS].sdk_name,
        tps_init,
        tps_cleanup,
        tps_create_service,
        tps_delete_service,
        tps_control_service,
        tps_check_service,
    },
    {
        tps_get_device_config,
        tps_set_device_config,
    },
    0,
    {
        5000,
        10,
        tps_disconnet_call_back,
        tps_reconnect_call_back,
    }
};

//////////////////////////////////////////////////////////////////////////
int tps_get_state(tps_parm_t *pm, int *state_timer)
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
int tps_set_state(tps_service_t *tps_srv, TPS_STATE_E state, int error)
{
    int flag, old;
    tps_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(tps_srv);

    old = tps_get_state(&tps_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("TPS State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case TPS_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case TPS_LOGING:
            flag = STATE_LOGING;
            break;
        case TPS_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return TPS_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(tps_srv->owner, &dev_state))
    {
        pm = &tps_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }

done:
    return old;
}
int tps_get_user_id(tps_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int tps_set_user_id(tps_service_t *tps_srv, int user_id)
{
    int old;
    tps_parm_t *pm;

    NMP_ASSERT(tps_srv);

    pm = &tps_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (TPS_LOGOUT != old)
        tps_logout(old);

    if (TPS_LOGOUT == user_id)
        tps_stop_all_stream(&tps_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
tps_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (TPS_INVALID_HANDLE != strm_info->handle)
    {
        IP_NET_DVR_StopRealPlay((long)strm_info->handle);
        strm_info->handle = TPS_INVALID_HANDLE;
    }

    return ;
}
static void 
tps_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (TPS_INVALID_HANDLE != strm_info->handle)
    {
        //IP_NET_DVR_StopPlayBack((long)strm_info->handle);
        strm_info->handle = TPS_INVALID_HANDLE;
    }

    return ;
}

static void tps_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void tps_stop_all_stream(struct tps_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        tps_stop_one_stream, (void*)tps_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        tps_stop_one_stream, (void*)tps_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
tps_cleanup_stream_info(struct tps_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        tps_stop_real_stream, sizeof(tps_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        tps_stop_record_stream, sizeof(tps_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}

static __inline__ void 
tps_submit_format_progress(tps_service_t *tps_srv, int disk_no)
{
    msg_t *msg;
    proxy_device_t *dev = tps_srv->owner;
    FormatProgressPacket *packet;

    packet = (FormatProgressPacket*)nmp_alloc0(sizeof(FormatProgressPacket));
    msg = alloc_new_msg_2(SUBMIT_FORMAT_PROGRESS_ID, packet, 
            sizeof(FormatProgressPacket), 0, tps_free_msg_data);

    usleep(0.2*1000*1000);
    packet->disk_no = disk_no;
    packet->progress = 5;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    usleep(0.2*1000*1000);
    packet->progress = 25;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    usleep(0.2*1000*1000);
    packet->progress = 50;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    usleep(0.2*1000*1000);
    packet->progress = 75;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    usleep(0.2*1000*1000);
    packet->progress = 90;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    usleep(0.2*1000*1000);
    packet->progress = 100;
    (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

    free_msg(msg);
}




//~End!

