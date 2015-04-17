#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_xmt_channel.h"
#include "nmp_xmt_talk.h"

#include "nmp_xmt_swap.h"
#include "nmp_xmt_handler.h"
#include "nmp_xmt_srv_impl.h"
#include "nmp_xmt_service.h"



static __inline__ void 
xmt_submit_format_progress(xmt_service_t *xmt_srv, int disk_no);


static __inline__ void 
xmt_time_swap(int flag, H264_DVR_TIME *time, JTime *time0)
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
}

static void xmt_free_ctrl_user(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static void xmt_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int xmt_logout(long user_id)
{
    if (H264_DVR_Logout(user_id))
    {
        show_debug("xmt_logout() successful, user_id: %d\n", (int)user_id);
        return 0;
    }
    else
        return -1;
}

static __inline__ void xmt_login(xmt_service_t *xmt_srv)
{
    char addr[MAX_IP_LEN];
    int state, error = 0;
    int user_id = XMT_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(xmt_srv);

    if (proxy_get_device_private(xmt_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        user_id = H264_DVR_Login(addr, sdk_info.dev_port, 
                    sdk_info.username, sdk_info.password, 
                    &xmt_srv->xmt_info, &error);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (XMT_LOGOUT != user_id)
    {
        //H264_DVR_StartListen(user_id);

        state = XMT_LOGIN;
        error = 0;
        show_debug("xmt_login() successful, user_id: %d\n", user_id);
    }
    else
    {
        state = XMT_LOGOUT;
        if (strlen(addr))
        {
            show_warn("xmt_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                xmt_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    xmt_set_user_id(xmt_srv, user_id);
    xmt_set_state(xmt_srv, (XMT_STATE_E)state, error);

    return ;
}

static int xmt_init(service_template_t *self)
{
    int ret = 0;
    xmt_conn_t *conn_info;
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(self);

    xmt_basic = (xmt_service_basic_t*)self;
    conn_info = &xmt_basic->conn_info;

    if (!H264_DVR_Init(conn_info->dis_conn, (unsigned long)xmt_basic))
    {
        ret = -1;
        show_warn("NET_DVR_Init() failure!\n");
    }
    else
    {
        H264_DVR_SetConnectTime(conn_info->conn_wait_time, conn_info->conn_try_times);
        H264_DVR_SetDVRMessCallBack(xmt_message_call_back, (unsigned long)xmt_basic);
    }

    return ret;
}

static int xmt_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    H264_DVR_Cleanup();
    return 0;
}

static struct service *
xmt_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    xmt_service_t *xmt_srv;
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(self && init_data);

    xmt_basic = (xmt_service_basic_t*)self;
    prx_dev  = (proxy_device_t*)init_data;

    xmt_srv = (xmt_service_t*)nmp_new0(xmt_service_t, 1);
    if (xmt_srv)
    {
        xmt_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        xmt_srv->base.tm = self;
        xmt_srv->parm.lock = nmp_mutex_new();
        xmt_srv->parm.user_id = XMT_LOGOUT;
        xmt_srv->parm.state = XMT_LOGOUT;
        xmt_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

        xmt_srv->rtsp.lock = nmp_mutex_new();
        xmt_srv->rtsp.real_strm_list.list = NULL;
        xmt_srv->rtsp.rec_strm_list.list = NULL;

        xmt_srv->owner = prx_dev;
    }

    return (struct service*)xmt_srv;
}

static void 
xmt_delete_service(service_template_t *self, struct service *srv)
{
    xmt_service_t *xmt_srv;
    xmt_service_basic_t *xmt_basic;

    NMP_ASSERT(self && srv);

    xmt_srv = (xmt_service_t*)srv;
    xmt_basic = (xmt_service_basic_t*)self;

    xmt_basic->total_count -= 1;

    xmt_set_user_id(xmt_srv, XMT_LOGOUT);             //注销登录
    xmt_set_state(xmt_srv, XMT_LOGOUT, 0);

    /* 清理视频流链表 */
    xmt_cleanup_stream_info(&xmt_srv->rtsp);

    if (xmt_srv->parm.lock)
        nmp_mutex_free(xmt_srv->parm.lock);
    if (xmt_srv->rtsp.lock)
        nmp_mutex_free(xmt_srv->rtsp.lock);             //销毁相关锁

    memset(xmt_srv, 0, sizeof(xmt_service_t));        //内存块归零
    nmp_del(xmt_srv, xmt_service_t, 1);                 //释放内存块
    return ;
}

static int 
xmt_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, ret = -1;
    xmt_service_t *xmt_srv;
    xmt_service_basic_t *xmt_basic;
    SDK_NetKeyBoardData key_data;

    NMP_ASSERT(self && srv);

    xmt_srv = (xmt_service_t*)srv;
    xmt_basic = (xmt_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!xmt_logout((long)xmt_get_user_id(&xmt_srv->parm)))
            {
                ret = 0;
                xmt_set_user_id(xmt_srv, XMT_LOGOUT);
                xmt_set_state(xmt_srv, XMT_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            xmt_login(xmt_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            break;
        case CTRL_CMD_REBOOT:
            if (H264_DVR_ControlDVR((long)xmt_get_user_id(&xmt_srv->parm), 0))
            {
                ret = 0;
                xmt_set_user_id(xmt_srv, XMT_LOGOUT);
                xmt_set_state(xmt_srv, XMT_LOGOUT, 0);
            }
            else
                show_warn("H264_DVR_ControlDVR fail, err: %d\n", 
                    (int)H264_DVR_GetLastError());
            break;
        case CTRL_CMD_SHUTDOWN:
            key_data.iValue = SDK_NET_KEY_SHUT;
            key_data.iState = SDK_NET_KEYBOARD_KEYDOWN;
            if (H264_DVR_ClickKey((long)xmt_get_user_id(&xmt_srv->parm), &key_data))
            {
                ret = 0;
                xmt_set_user_id(xmt_srv, XMT_LOGOUT);
                xmt_set_state(xmt_srv, XMT_LOGOUT, 0);
            }
            else
                show_warn("H264_DVR_ClickKey fail, err: %d\n", 
                    (int)H264_DVR_GetLastError());
            break;

        case CTRL_CMD_FRMTDISK:
            xmt_submit_format_progress(xmt_srv, *(int*)parm);
            ret = 0;
            break;

        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == xmt_get_user_id(&xmt_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&xmt_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&xmt_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            //*(int*)parm = (int)&xmt_talk_opt;
            ret = -1;
            break;

        default:
            break;
    }

    return ret;
}
static int 
xmt_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    xmt_service_t *xmt_srv;

    NMP_ASSERT(self && srv);

    xmt_srv = (xmt_service_t*)srv;

    switch (xmt_get_state(&xmt_srv->parm, &state_timer))
    {
        case XMT_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, xmt_srv->owner);
                    if (task)
                    {
                        xmt_set_state(xmt_srv, XMT_LOGING, 0);
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

        case XMT_LOGING:
            if (state_timer > DEFAULT_CONNECTING_SECS)
            {
            }
            break;

        case XMT_LOGIN:
            break;
    }

    return ret;
}

static int 
xmt_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int user_id, ret = FALSE;

    xmt_service_t *xmt_srv;
    xmt_config_t  *xmt_cfg;

    JDevCap *dev_cap;

    NMP_ASSERT(srv && parm);

    xmt_query_t *query_file;
    struct H264_DVR_FINDINFO find_info;

    xmt_srv = (xmt_service_t*)srv;
    user_id = xmt_get_user_id(&xmt_srv->parm);

    if (XMT_LOGOUT == user_id)
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
            query_file = (xmt_query_t*)parm;
            find_info.nChannelN0 = query_file->channel;
            find_info.nFileType = query_file->file_type;
            memcpy(&find_info.startTime, &query_file->start, sizeof(H264_DVR_TIME));
            memcpy(&find_info.endTime, &query_file->end, sizeof(H264_DVR_TIME));
            ret = H264_DVR_FindFile((long)user_id, &find_info, 
                    (struct H264_DVR_FILE_DATA*)query_file->buffer, 
                    query_file->max_count, &query_file->filecount);
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
    xmt_cfg = (xmt_config_t*)parm;
    ret = H264_DVR_GetDevConfig((long)user_id, 
            (unsigned long ) xmt_cfg->command, 
            (int           ) xmt_cfg->channel, 
            (char*         ) xmt_cfg->buffer, 
            (unsigned long ) xmt_cfg->b_size, 
            (unsigned long*)&xmt_cfg->returns, 
            (int           ) xmt_cfg->waittime);
    show_debug("H264_DVR_GetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", 
            (int)H264_DVR_GetLastError());
        return -1;
    }
}
static int 
xmt_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;//, err = 0;
    int user_id, cmd;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;

    xmt_service_t *xmt_srv;
    xmt_config_t  *xmt_cfg;

    NMP_ASSERT(srv && parm_id && parm);

    xmt_srv = (xmt_service_t*)srv;
    user_id = xmt_get_user_id(&xmt_srv->parm);

    if (XMT_LOGOUT == user_id)
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
            ret = H264_DVR_StorageManage((long)user_id, 
                    (SDK_StorageDeviceControl*)parm);
            if (0 < ret)
            {
                int *idx = (int*)nmp_alloc(sizeof(int));
                *idx = ((SDK_StorageDeviceControl*)parm)->iSerialNo;
                ctrl = proxy_new_ctrl_2(srv, CTRL_CMD_FRMTDISK, 
                        (void*)idx, sizeof(int), xmt_free_ctrl_user);
                if (!ctrl)
                    ret = FALSE;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, xmt_srv->owner);
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
            else
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
                            proxy_free_ctrl, xmt_srv->owner);
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
            ret = H264_DVR_PTZControl((long)user_id, 
                    (int )((xmt_ptz_ctrl_t*)parm)->channel, 
                    (long)((xmt_ptz_ctrl_t*)parm)->ptz_cmd);
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
    xmt_cfg = (xmt_config_t*)parm;
    ret = H264_DVR_SetDevConfig((long         )user_id, 
                                (unsigned long)xmt_cfg->command, 
                                (int          )xmt_cfg->channel, 
                                (char*        )xmt_cfg->buffer, 
                                (unsigned long)xmt_cfg->b_size, 
                                (int          )xmt_cfg->waittime);
    show_debug("H264_DVR_SetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");

    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", 
            (int)H264_DVR_GetLastError());
        return -1;
    }
}

xmt_service_basic_t xmt_srv_basic = 
{
    {
        g_sdk_items[SDK_XMT].sdk_name,
        xmt_init,
        xmt_cleanup,
        xmt_create_service,
        xmt_delete_service,
        xmt_control_service,
        xmt_check_service,
    },
    {
        xmt_get_device_config,
        xmt_set_device_config,
    },
    0,
    {
        5000,
        10,
        xmt_disconnet_call_back,
        xmt_reconnect_call_back,
    }
};

//////////////////////////////////////////////////////////////////////////
int xmt_get_state(xmt_parm_t *pm, int *state_timer)
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
int xmt_set_state(xmt_service_t *xmt_srv, XMT_STATE_E state, int error)
{
    int flag, old;
    xmt_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(xmt_srv);

    old = xmt_get_state(&xmt_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("XMT State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case XMT_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case XMT_LOGING:
            flag = STATE_LOGING;
            break;
        case XMT_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return XMT_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(xmt_srv->owner, &dev_state))
    {
        pm = &xmt_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }

done:
    return old;
}
int xmt_get_user_id(xmt_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int xmt_set_user_id(xmt_service_t *xmt_srv, int user_id)
{
    int old;
    xmt_parm_t *pm;

    NMP_ASSERT(xmt_srv);

    pm = &xmt_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (XMT_LOGOUT != old)
        xmt_logout(old);

    if (XMT_LOGOUT == user_id)
        xmt_stop_all_stream(&xmt_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
xmt_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (XMT_INVALID_HANDLE != strm_info->handle)
    {
        H264_DVR_StopRealPlay((long)strm_info->handle);
        strm_info->handle = XMT_INVALID_HANDLE;
    }

    return ;
}
static void 
xmt_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (XMT_INVALID_HANDLE != strm_info->handle)
    {
        H264_DVR_StopPlayBack((long)strm_info->handle);
        strm_info->handle = XMT_INVALID_HANDLE;
    }

    return ;
}

static void xmt_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void xmt_stop_all_stream(struct xmt_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        xmt_stop_one_stream, (void*)xmt_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        xmt_stop_one_stream, (void*)xmt_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
xmt_cleanup_stream_info(struct xmt_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        xmt_stop_real_stream, sizeof(xmt_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        xmt_stop_record_stream, sizeof(xmt_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}

static __inline__ void 
xmt_submit_format_progress(xmt_service_t *xmt_srv, int disk_no)
{
    msg_t *msg;
    proxy_device_t *dev = xmt_srv->owner;
    FormatProgressPacket *packet;

    packet = (FormatProgressPacket*)nmp_alloc0(sizeof(FormatProgressPacket));
    msg = alloc_new_msg_2(SUBMIT_FORMAT_PROGRESS_ID, packet, 
            sizeof(FormatProgressPacket), 0, xmt_free_msg_data);

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
