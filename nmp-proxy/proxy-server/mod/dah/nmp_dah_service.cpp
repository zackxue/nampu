#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_dah_channel.h"
#include "nmp_dah_talk.h"

#include "nmp_dah_swap.h"
#include "nmp_dah_handler.h"
#include "nmp_dah_srv_impl.h"
#include "nmp_dah_service.h"

static __inline__ void 
dah_submit_format_progress(dah_service_t *dah_srv, int disk_no);


static __inline__ void 
dah_time_swap(int flag, NET_TIME *time, JTime *time0)
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

static void dah_free_ctrl_user(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static void dah_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int dah_logout(long user_id)
{
    CLIENT_StopListen(user_id);
    if (CLIENT_Logout(user_id))
    {
        show_debug("dah_logout() successful, user_id: %d\n", (int)user_id);
        return 0;
    }
    else
        return -1;
}

static __inline__ void dah_login(dah_service_t *dah_srv)
{
    char addr[MAX_IP_LEN];
    int state, error = 0;
    int user_id = DAH_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(dah_srv);

    if (proxy_get_device_private(dah_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        user_id = CLIENT_Login(addr, sdk_info.dev_port, 
                    sdk_info.username, sdk_info.password, 
                    &dah_srv->dah_info, &error);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (DAH_LOGOUT != user_id)
    {
        CLIENT_StartListen(user_id);

        state = DAH_LOGIN;
        error = 0;
        show_debug("dah_login() successful, user_id: %d\n", user_id);
    }
    else
    {
        state = DAH_LOGOUT;
        //error = ;

        if (strlen(addr))
        {
            show_warn("dah_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                dah_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    dah_set_user_id(dah_srv, user_id);
    dah_set_state(dah_srv, (DAH_STATE_E)state, error);

    return ;
}

static int dah_init(service_template_t *self)
{
    int ret = 0;
    dah_conn_t *conn_info;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(self);

    dah_basic = (dah_service_basic_t*)self;
    conn_info = &dah_basic->conn_info;

    if (!CLIENT_Init(conn_info->dis_conn, (DWORD)dah_basic))
    {
        ret = -1;
        show_warn("NET_DVR_Init() failure!\n");
    }
    else
    {
        CLIENT_SetConnectTime(conn_info->conn_wait_time, conn_info->conn_try_times);
        CLIENT_SetDVRMessCallBack(dah_message_call_back, (LDWORD)dah_basic);
    }

    return ret;
}

static int dah_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    CLIENT_Cleanup();
    return 0;
}

static struct service *
dah_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    dah_service_t *dah_srv;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(self && init_data);

    dah_basic = (dah_service_basic_t*)self;
    prx_dev   = (proxy_device_t*)init_data;

    dah_srv = (dah_service_t*)nmp_new0(dah_service_t, 1);
    if (dah_srv)
    {
        dah_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        dah_srv->base.tm = self;
        dah_srv->parm.lock = nmp_mutex_new();
        dah_srv->parm.user_id = DAH_LOGOUT;
        dah_srv->parm.state = DAH_LOGOUT;
        dah_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

        dah_srv->rtsp.lock = nmp_mutex_new();
        dah_srv->rtsp.real_strm_list.list = NULL;
        dah_srv->rtsp.rec_strm_list.list = NULL;

        dah_srv->owner = prx_dev;
    }

    return (struct service *)dah_srv;
}
static void 
dah_delete_service(service_template_t *self, struct service *srv)
{
    dah_service_t *dah_srv;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(self && srv);

    dah_srv = (dah_service_t*)srv;
    dah_basic = (dah_service_basic_t*)self;

    dah_basic->total_count -= 1;

    dah_set_user_id(dah_srv, DAH_LOGOUT);               //注销登录
    dah_set_state(dah_srv, DAH_LOGOUT, 0);

    /* 清理视频流链表 */
    dah_cleanup_stream_info(&dah_srv->rtsp);

    if (dah_srv->parm.lock)
        nmp_mutex_free(dah_srv->parm.lock);
    if (dah_srv->rtsp.lock)
        nmp_mutex_free(dah_srv->rtsp.lock);             //销毁相关锁

    memset(dah_srv, 0, sizeof(dah_service_t));          //内存块归零
    nmp_del(dah_srv, dah_service_t, 1);                   //释放内存块
    return ;
}

static int 
dah_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, ret = -1;
    dah_service_t *dah_srv;
    dah_service_basic_t *dah_basic;

    NMP_ASSERT(self && srv);

    dah_srv = (dah_service_t*)srv;
    dah_basic = (dah_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!dah_logout((LONG)dah_get_user_id(&dah_srv->parm)))
            {
                ret = 0;
                dah_set_user_id(dah_srv, DAH_LOGOUT);
                dah_set_state(dah_srv, DAH_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            dah_login(dah_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            break;
        case CTRL_CMD_REBOOT:
            sleep(2);
            if (CLIENT_RebootDev((LONG)dah_get_user_id(&dah_srv->parm)))
            {
                ret = 0;
                dah_set_user_id(dah_srv, DAH_LOGOUT);
                dah_set_state(dah_srv, DAH_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_SHUTDOWN:
            sleep(2);
            if (CLIENT_ShutDownDev((LONG)dah_get_user_id(&dah_srv->parm)))
            {
                ret = 0;
                dah_set_user_id(dah_srv, DAH_LOGOUT);
                dah_set_state(dah_srv, DAH_LOGOUT, 0);
            }
            break;

        case CTRL_CMD_FRMTDISK:
            CLIENT_ControlDevice(dah_get_user_id(&dah_srv->parm), 
                    (CtrlType)DH_CTRL_DISK, (void*)parm, (int)1000);
            show_debug("CLIENT_ControlDevice: %s, ERROR: 0x%x<-------------------------------\n", 
                (ret == TRUE) ? "Success" : "Failure", CLIENT_GetLastError());

            dah_submit_format_progress(dah_srv, ((DISKCTRL_PARAM*)parm)->nIndex);

            //Reboot
            dah_set_user_id(dah_srv, DAH_LOGOUT);
            dah_set_state(dah_srv, DAH_LOGOUT, 0);
            ret = 0;
            break;

        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == dah_get_user_id(&dah_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&dah_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&dah_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            *(int*)parm = (int)&dah_talk_opt;
            ret = 0;
            break;

        default:
            break;
    }

    return ret;
}
static int 
dah_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    dah_service_t *dah_srv;

    NMP_ASSERT(self && srv);

    dah_srv = (dah_service_t*)srv;

    switch (dah_get_state(&dah_srv->parm, &state_timer))
    {
        case DAH_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, dah_srv->owner);
                    if (task)
                    {
                        dah_set_state(dah_srv, DAH_LOGING, 0);
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

        case DAH_LOGING:
            break;

        case DAH_LOGIN:
            break;
    }

    return ret;
}

static int 
dah_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int user_id, ret = FALSE, err = 0;

    JDevCap *dev_cap;

    dah_service_t *dah_srv;
    dah_config_t  *dah_cfg;
    dah_new_config_t *dah_new_cfg;
    dah_query_t *query;

    NMP_ASSERT(srv && parm);

    dah_srv = (dah_service_t*)srv;
    user_id = dah_get_user_id(&dah_srv->parm);

    if (DAH_LOGOUT == user_id)
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
            goto GET_DEV_STATE;
        case GET_ENCODE_CONFIG:
            goto GET_CFG;
        case GET_DISPLAY_CONFIG:
            goto GET_CFG;
        case GET_OSD_CONFIG:
            goto GET_CFG;
        case GET_PTZ_CONFIG:
            goto GET_NEW_CFG;
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
            query = (dah_query_t*)parm;

            printf("user_id : %d\n", user_id);
            printf("channel  : %d\n", query->channel);
            printf("file_type: %d\n", query->file_type);
            if (CLIENT_QueryRecordFile((LLONG)user_id, 
                    query->channel,
                    query->file_type, 
                    &query->start, 
                    &query->end, 
                    query->card_id, 
                    (NET_RECORDFILE_INFO*)query->buffer, 
                    query->buf_size, 
                    &query->filecount,
                    query->waittime, FALSE))
            {
                show_info("count: %d<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", query->filecount);
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
                show_debug("ERROR: 0x%x<-------------------------------\n", 
                    CLIENT_GetLastError());
            }
            show_debug("CLIENT_QueryRecordFile: %s<-----------------\n", 
                (ret == TRUE) ? "Success" : "Failure");
            break;

        case GET_CRUISE_CONFIG:
            /*cruise = (dah_cruise_t*)parm;
            show_debug("crz_no: %d<--------------------\n", cruise->crz_no);
            ret = NET_DVR_GetPTZCruise(user_id, 
                    (LONG)cruise->channel, 
                    (LONG)cruise->crz_no, 
                    (NET_DVR_CRUISE_RET*)cruise->input);
            show_debug("NET_DVR_GetDVRConfig: %s<-------------------------------\n", 
                (ret == TRUE) ? "Success" : "Failure");
            if (FALSE == ret)
                show_debug("ERROR: 0x%x<-------------------------------\n", CLIENT_GetLastError());*/
            break;

        case GET_CAPABILITY_SET:
            dev_cap = (JDevCap*)parm;
            dev_cap->cap0 = DEV_CAP_VIDEO_IN | DEV_CAP_AUDIO | DEV_CAP_IRIS | DEV_CAP_PTZ | 
                DEV_CAP_ALARM_IN | DEV_CAP_ALARM_OUT | DEV_CAP_STORAGE | DEV_CAP_WEB | DEV_CAP_PLATFORM | 
                DEV_CAP_INTELLIGENT_ANALYSIS | DEV_CAP_UPDATE | DEV_CAP_VIDEO_OUT;// | DEV_CAP_IR;
            dev_cap->ftp_enable = 1;
            dev_cap->upnp_enable = 0;
            dev_cap->chn_cap.size = sizeof(JChnCap);
            dev_cap->chn_cap.encode = VIDEO_ENCODE_H264_E | VIDEO_ENCODE_MJPEG_E | 
                VIDEO_ENCODE_JPEG_E | VIDEO_ENCODE_MPEG4_E;
            dev_cap->chn_cap.supp_mask = 1;
            dev_cap->chn_cap.mask_count = 4;
            dev_cap->chn_cap.supp_hide_alarm = 1;
            dev_cap->chn_cap.hide_alarm_count = 1;
            dev_cap->chn_cap.supp_move_alarm = 1;
            dev_cap->chn_cap.move_alarm_count = 1;
            dev_cap->chn_cap.supp_video_lost_alarm = 1;
            dev_cap->chn_cap.osd_count = 1;
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
    dah_cfg = (dah_config_t*)parm;
    ret = CLIENT_GetDevConfig((LLONG)user_id, 
            (DWORD  ) dah_cfg->command, 
            (LONG   ) dah_cfg->channel, 
            (LPVOID ) dah_cfg->buffer, 
            (DWORD  ) dah_cfg->b_size, 
            (LPDWORD)&dah_cfg->returns, 
            (int    ) dah_cfg->waittime);
    show_debug("CLIENT_GetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: 0x%x<-------------------------------\n", CLIENT_GetLastError());
        return -1;
    }

GET_NEW_CFG:
    dah_new_cfg = (dah_new_config_t*)parm;
    ret = CLIENT_GetNewDevConfig((LLONG)user_id, 
            dah_new_cfg->command, 
            dah_new_cfg->channel, 
            dah_new_cfg->out_json, 
            DEF_OUT_JSON_SIZE, &err, 
            dah_new_cfg->waittime);

    show_debug("CLIENT_GetNewDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");

    if (TRUE == ret)
    {
        CLIENT_ParseData(
            dah_new_cfg->command, 
            dah_new_cfg->out_json, 
            dah_new_cfg->buffer, 
            dah_new_cfg->b_size, NULL);
        return 0;
    }
    else
    {
        show_debug("ERROR: 0x%x, err: 0x%x<-------------------------------\n", 
            CLIENT_GetLastError(), err);
        return -1;
    }

GET_DEV_STATE:
    dah_cfg = (dah_config_t*)parm;
    ret = CLIENT_QueryDevState(user_id, 
            (DWORD) dah_cfg->command, 
            (char*) dah_cfg->buffer, 
            (DWORD) dah_cfg->b_size, 
            (int* )&dah_cfg->returns, 
            (int  ) dah_cfg->waittime);

    show_debug("CLIENT_QueryDevState: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");

    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: 0x%x, err: %d<-------------------------------\n", 
            CLIENT_GetLastError(), err);
        return -1;
    }
}
static int 
dah_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE, err = 0;
    int user_id, cmd;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;

    dah_service_t *dah_srv;
    dah_config_t  *dah_cfg;
    dah_new_config_t *dah_new_cfg;

    DISKCTRL_PARAM *disk_ctrl;

    NMP_ASSERT(srv && parm_id && parm);

    dah_srv = (dah_service_t*)srv;
    user_id = dah_get_user_id(&dah_srv->parm);

    if (DAH_LOGOUT == user_id)
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
            disk_ctrl = (DISKCTRL_PARAM*)nmp_new(DISKCTRL_PARAM, 1);
            memcpy(disk_ctrl, ((dah_config_t*)parm)->buffer, sizeof(DISKCTRL_PARAM));
            ctrl = proxy_new_ctrl_2(srv, CTRL_CMD_FRMTDISK, disk_ctrl, 
                    sizeof(DISKCTRL_PARAM), dah_free_ctrl_user);
            if (!ctrl)
                ret = FALSE;
            else
            {
                task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                        proxy_free_ctrl, dah_srv->owner);
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
                            proxy_free_ctrl, dah_srv->owner);
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
            goto SET_NEW_CFG;
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
            ret = CLIENT_PTZControl(user_id, 
                    (int  )((dah_ptz_ctrl_t*)parm)->channel, 
                    (DWORD)((dah_ptz_ctrl_t*)parm)->ptz_cmd, 
                    (DWORD)((dah_ptz_ctrl_t*)parm)->step, 
                    (BOOL )((dah_ptz_ctrl_t*)parm)->stop);
            break;

        case SET_PRESET_CONFIG:
            /*strm_info = find_stream_by_channel(&dah_srv->rtsp.real_strm_list, 
                            ((struct dah_preset*)parm)->channel);
            if (strm_info && DAH_INVALID_HANDLE != strm_info->handle)
            {
                handle = strm_info->handle;
            }

            if (DAH_INVALID_HANDLE != handle)
            {
                ret = NET_DVR_PTZPreset((LONG)handle, 
                    (DWORD)((struct dah_preset*)parm)->preset_cmd, 
                    (DWORD)((struct dah_preset*)parm)->preset_no);
show_debug("NET_DVR_PTZPreset: %s<-------------------------------\n", (ret == TRUE) ? "Success" : "Failure");
            }*/
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
    dah_cfg = (dah_config_t*)parm;
    ret = CLIENT_SetDevConfig(user_id, 
            (DWORD  )dah_cfg->command, 
            (LONG   )dah_cfg->channel, 
            (LPVOID )dah_cfg->buffer, 
            (DWORD  )dah_cfg->b_size, 
            (int    )dah_cfg->waittime);
    show_debug("CLIENT_SetDevConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");

    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: 0x%x<-------------------------------\n", 
            CLIENT_GetLastError());
        return -1;
    }

SET_NEW_CFG:
    dah_new_cfg = (dah_new_config_t*)parm;
    ret = CLIENT_PacketData(
            dah_new_cfg->command, 
            dah_new_cfg->buffer, 
            dah_new_cfg->b_size, 
            dah_new_cfg->out_json, 
            DEF_OUT_JSON_SIZE);
    if (ret)
    {
        ret = CLIENT_SetNewDevConfig(user_id, 
                dah_new_cfg->command, 
                dah_new_cfg->channel, 
                dah_new_cfg->out_json, 
                DEF_OUT_JSON_SIZE, 
                &err, FALSE, dah_new_cfg->waittime);

        show_debug("CLIENT_SetNewDevConfig: %s<-------------------------------\n", 
            (ret == TRUE) ? "Success" : "Failure");
    }

    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: 0x%x, err: %d<-------------------------------\n", 
            CLIENT_GetLastError(), err);
        return -1;
    }
}

dah_service_basic_t dah_srv_basic = 
{
    {
        g_sdk_items[SDK_DAH].sdk_name,
        dah_init,
        dah_cleanup,
        dah_create_service,
        dah_delete_service,
        dah_control_service,
        dah_check_service,
    },
    {
        dah_get_device_config,
        dah_set_device_config,
    },
    0,
    {
        5000,
        10,
        dah_disconnet_call_back,
        have_reconnect_call_back,
    }
};

//////////////////////////////////////////////////////////////////////////
int dah_get_state(dah_parm_t *pm, int *state_timer)
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
int dah_set_state(dah_service_t *dah_srv, DAH_STATE_E state, int error)
{
    int flag, old;
    dah_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(dah_srv);

    old = dah_get_state(&dah_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("DAH State No Update!!!!!!!!!!!!\n");
        goto done;
    }

    switch (state)
    {
        case DAH_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case DAH_LOGING:
            flag = STATE_LOGING;
            break;
        case DAH_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return DAH_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(dah_srv->owner, &dev_state))
    {
        pm = &dah_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }

done:
    return old;
}
int dah_get_user_id(dah_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int dah_set_user_id(dah_service_t *dah_srv, int user_id)
{
    int old;
    dah_parm_t *pm;

    NMP_ASSERT(dah_srv);

    pm = &dah_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (DAH_LOGOUT != old)
        dah_logout(old);

    if (DAH_LOGOUT == user_id)
        dah_stop_all_stream(&dah_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
dah_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (DAH_INVALID_HANDLE != strm_info->handle)
    {
        CLIENT_StopRealPlay((LONG)strm_info->handle);
        strm_info->handle = DAH_INVALID_HANDLE;
    }

    return ;
}
static void 
dah_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (DAH_INVALID_HANDLE != strm_info->handle)
    {
        CLIENT_StopPlayBack((LONG)strm_info->handle);
        strm_info->handle = DAH_INVALID_HANDLE;
    }

    return ;
}

static void dah_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void dah_stop_all_stream(struct dah_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        dah_stop_one_stream, (void*)dah_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        dah_stop_one_stream, (void*)dah_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
dah_cleanup_stream_info(struct dah_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        dah_stop_real_stream, sizeof(dah_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        dah_stop_record_stream, sizeof(dah_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}

static __inline__ void 
dah_submit_format_progress(dah_service_t *dah_srv, int disk_no)
{
    msg_t *msg;
    proxy_device_t *dev = dah_srv->owner;
    FormatProgressPacket *packet;

    packet = (FormatProgressPacket*)nmp_alloc0(sizeof(FormatProgressPacket));
    msg = alloc_new_msg_2(SUBMIT_FORMAT_PROGRESS_ID, packet, 
            sizeof(FormatProgressPacket), 0, dah_free_msg_data);

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



