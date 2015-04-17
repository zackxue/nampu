#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_hik_swap.h"
#include "nmp_hik_channel.h"
#include "nmp_hik_handler.h"
#include "nmp_hik_srv_impl.h"
#include "nmp_hik_service.h"
#include "nmp_hik_talk.h"

static __inline__ void 
hik_time_swap(int flag, NET_DVR_TIME *time, JTime *time0)
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

static void hik_free_ctrl_user(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static void hik_free_msg_data(void *data, size_t size)
{
    nmp_dealloc(data, size);
}

static __inline__ int hik_logout(long user_id)
{
    if (NET_DVR_Logout(user_id))
    {
        show_debug("hik_logout() successful, user_id: %d\n", (int)user_id);
        return 0;
    }
    else
        return -1;
}
static __inline__ void hik_login(hik_service_t *hik_srv)
{
    char addr[MAX_IP_LEN];
    int state, error;
    int handle, user_id = HIK_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(hik_srv);

    if (proxy_get_device_private(hik_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        user_id = NET_DVR_Login_V30(addr, 
                    sdk_info.dev_port, sdk_info.username, 
                    sdk_info.password, &hik_srv->hik_info);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (HIK_LOGOUT != user_id)
    {
        state = HIK_LOGIN;
        error = 0;
        show_debug("hik_login() successful, user_id: %d\n", user_id);

        handle = NET_DVR_StartListen(NULL, 7000);//搭配NET_DVR_SetDVRMessCallBack使用
        if (HIK_INVALID_HANDLE != handle)
        {
            hik_srv->listen = handle;
        }
        show_debug("NET_DVR_StartListen: %s<-------------------------------\n", 
            (HIK_INVALID_HANDLE != handle) ? "Success" : "Failure");
    }
    else
    {
#if 0
    static int allow = 0;
    if (allow > 500)
    {
#endif
        state = HIK_LOGOUT;
        error = NET_DVR_GetLastError();
#if 0
    }
    else
    {
        user_id = allow++ + 500;
        state = HIK_LOGIN;
        error = 0;
    }
#endif

        if (strlen(addr))
        {
            show_warn("hik_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                hik_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    hik_set_user_id(hik_srv, user_id);
    hik_set_state(hik_srv, (HIK_STATE_E)state, error);

    return ;
}

static __inline__ void 
hik_submit_format_progress(hik_service_t *hik_srv, hik_format_t *hik_fmt)
{
    BOOL ret;
    msg_t *msg;
    proxy_device_t *dev;
    FormatProgressPacket *packet;

    dev = hik_srv->owner;
    packet = (FormatProgressPacket*)nmp_alloc0(sizeof(FormatProgressPacket));

    msg = alloc_new_msg_2(SUBMIT_FORMAT_PROGRESS_ID, packet, 
            sizeof(FormatProgressPacket), 0, hik_free_msg_data);
    do
    {
        sleep(1);
        ret = NET_DVR_GetFormatProgress(
                (LONG ) hik_fmt->handle,
                (LONG*)&hik_fmt->disk_no,
                (LONG*)&hik_fmt->disk_pos,
                (LONG*)&hik_fmt->state);
        if (TRUE == ret)
        {
            packet->disk_no = hik_fmt->disk_no;
            switch (hik_fmt->state)
            {
                case 0://正在格式化
                    packet->progress = hik_fmt->disk_pos;
                    break;
                case 1://硬盘全部格式化完成
                    packet->progress = 100;
                    break;
                case 2://格式化当前硬盘出错，不能继续格式化此硬盘
                    packet->progress = 0xFFFFFFFF;
                    break;
                case 3://由于网络异常造成网络硬盘丢失而不能开始格式化当前硬盘
                    packet->progress = 0xFFFFFFFF;
                    break;
            }

            (*dev->plt_srv->tm->control_service)(dev->plt_srv->tm, 
                dev->plt_srv, CTRL_CMD_SUBMIT, msg);
        }
        else
            show_warn("NET_DVR_GetFormatProgress(handle: %d) fail! Error: %d\n", 
                hik_fmt->handle, NET_DVR_GetLastError());
    }while (!hik_fmt->state?TRUE:FALSE);

    free_msg(msg);
}

static int hik_init(service_template_t *self)
{
    int ret = 0;
    hik_conn_t *conn_info;
    hik_service_basic_t *hik_basic;
    NMP_ASSERT(self);

    hik_basic = (hik_service_basic_t*)self;
    conn_info = &hik_basic->conn_info;

    if (!NET_DVR_Init())
    {
        ret = -1;
        show_warn("NET_DVR_Init() failure!\n");
    }
    if (!NET_DVR_SetConnectTime(conn_info->conn_wait_time, 
            conn_info->conn_try_times))
    {
        ret = -1;
        show_warn("NET_DVR_SetConnectTime() failure!\n");
    }
    if (!NET_DVR_SetReconnect(conn_info->reconn_interval, 
            conn_info->reconn_enable))
    {
        ret = -1;
        show_warn("NET_DVR_SetReconnect() failure!\n");
    }

    if (!ret)
    {
        NET_DVR_SetDVRMessCallBack(conn_info->ms_cb);
        NET_DVR_SetExceptionCallBack_V30(0, NULL, conn_info->ex_cb, NULL);
    }

    return ret;
}
static int hik_cleanup(service_template_t *self)
{
    hik_service_basic_t *hik_basic;
    NMP_ASSERT(self);

    hik_basic = (hik_service_basic_t*)self;
    NET_DVR_Cleanup();
    return 0;
}

static struct service *
hik_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    hik_service_t *hik_srv;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(self && init_data);

    hik_basic = (hik_service_basic_t*)self;
    prx_dev   = (proxy_device_t*)init_data;

    hik_srv = (hik_service_t*)nmp_new0(hik_service_t, 1);
    if (hik_srv)
    {
        hik_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        hik_srv->base.tm = self;
        hik_srv->parm.lock = nmp_mutex_new();
        hik_srv->parm.user_id = HIK_LOGOUT;
        hik_srv->parm.state = HIK_LOGOUT;
        hik_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

        hik_srv->rtsp.lock = nmp_mutex_new();
        hik_srv->rtsp.real_strm_list.list = NULL;
        hik_srv->rtsp.rec_strm_list.list = NULL;

        hik_srv->owner = prx_dev;
    }

    return (struct service *)hik_srv;
}
static void 
hik_delete_service(service_template_t *self, struct service *srv)
{
    hik_service_t *hik_srv;
    hik_service_basic_t *hik_basic;

    NMP_ASSERT(self && srv);

    hik_srv = (hik_service_t*)srv;
    hik_basic = (hik_service_basic_t*)self;

    hik_basic->total_count -= 1;

    hik_set_user_id(hik_srv, HIK_LOGOUT);               //注销登录
    hik_set_state(hik_srv, HIK_LOGOUT, 0);

    /* 清理视频流链表 */
    hik_cleanup_stream_info(&hik_srv->rtsp);

    if (hik_srv->parm.lock)
        nmp_mutex_free(hik_srv->parm.lock);               //销毁相关锁
    if (hik_srv->rtsp.lock)
        nmp_mutex_free(hik_srv->rtsp.lock);             //销毁相关锁

    memset(hik_srv, 0, sizeof(hik_service_t));          //内存块归零
    nmp_del(hik_srv, hik_service_t, 1);                   //释放内存块
    return ;
}

static int 
hik_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, ret = -1;
    hik_service_t *hik_srv;
    hik_service_basic_t *hik_basic;

    hik_format_t hik_fmt;

    NMP_ASSERT(self && srv);

    hik_srv = (hik_service_t*)srv;
    hik_basic = (hik_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!hik_logout((LONG)hik_get_user_id(&hik_srv->parm)))
            {
                ret = 0;
                hik_set_user_id(hik_srv, HIK_LOGOUT);
                hik_set_state(hik_srv, HIK_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            hik_login(hik_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            sleep(2);
            if (NET_DVR_RestoreConfig((LONG)hik_get_user_id(&hik_srv->parm)))
            {
                ret = 0;
                hik_set_user_id(hik_srv, HIK_LOGOUT);
                hik_set_state(hik_srv, HIK_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_REBOOT:
            sleep(2);
            if (NET_DVR_RebootDVR((LONG)hik_get_user_id(&hik_srv->parm)))
            {
                ret = 0;
                hik_set_user_id(hik_srv, HIK_LOGOUT);
                hik_set_state(hik_srv, HIK_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_SHUTDOWN:
            sleep(2);
            if (NET_DVR_ShutDownDVR((LONG)hik_get_user_id(&hik_srv->parm)))
            {
                ret = 0;
                hik_set_user_id(hik_srv, HIK_LOGOUT);
                hik_set_state(hik_srv, HIK_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_FRMTDISK:
            hik_fmt.handle = (int)parm;
            hik_submit_format_progress(hik_srv, &hik_fmt);
            ret = 0;
            break;
        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == hik_get_user_id(&hik_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&hik_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&hik_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            *(int*)parm = (int)&hik_talk_opt;
			ret = 0;
            break;

        default:
            break;
    }

    return ret;
}
static int 
hik_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hik_service_t *hik_srv;

    NMP_ASSERT(self && srv);

    hik_srv = (hik_service_t*)srv;

    switch (hik_get_state(&hik_srv->parm, &state_timer))
    {
        case HIK_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hik_srv->owner);
                    if (task)
                    {
                        hik_set_state(hik_srv, HIK_LOGING, 0);
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

        case HIK_LOGING:
            break;

        case HIK_LOGIN:
            break;
    }

    return ret;
}

static int 
hik_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;
    int offset = 0;
    int user_id, handle, log_node, index;

    JDevCap *dev_cap;
    JStoreLog *store_log;

    hik_service_t *hik_srv;
    hik_config_t  *config;
    hik_cruise_t  *cruise;

    LPNET_DVR_FILECOND   find_cond;
    NET_DVR_FINDDATA_V30 find_data;

    NMP_ASSERT(srv && parm);

    hik_srv = (hik_service_t*)srv;
    user_id = hik_get_user_id(&hik_srv->parm);

    if (HIK_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
        offset = HIK_NVR_CHANNEL_OFFSET;

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
            index = 0;
            log_node = 0;

            config = (hik_config_t*)parm;
            store_log = (JStoreLog*)config->buffer;

            find_cond = (LPNET_DVR_FILECOND)nmp_alloc0(sizeof(NET_DVR_FILECOND));
            find_cond->dwFileType = store_log->rec_type;
            find_cond->lChannel   = config->channel + offset;
            hik_time_swap(SWAP_UNPACK, &find_cond->struStartTime, &store_log->beg_time);
            hik_time_swap(SWAP_UNPACK, &find_cond->struStopTime, &store_log->end_time);

            if (find_cond->struStopTime.dwYear > 2037)
                find_cond->struStopTime.dwYear = 2037;
            
            handle = NET_DVR_FindFile_V30((LONG)user_id, find_cond);
            if(HIK_INVALID_HANDLE == handle)
            {
                show_warn("NET_DVR_FindFile_V30(%d), FileType: %d\n\n", 
                    NET_DVR_GetLastError(), find_cond->dwFileType);
                return -1;
            }
            
            while(TRUE)
            {
                ret = NET_DVR_FindNextFile_V30(handle, &find_data);
                if(ret == NET_DVR_ISFINDING)
                {
                    continue;
                }
                else if(ret == NET_DVR_FILE_SUCCESS)
                {
                    if (index >= J_SDK_MAX_STORE_LOG_SIZE)
                    {
                        ret = TRUE;
                        break;
                    }

                    if ((int)store_log->beg_node <= log_node && (int)store_log->end_node >= log_node)
                    {
                        store_log->store[index].rec_type = find_data.byFileType;
                        store_log->store[index].file_size = find_data.dwFileSize;
                        hik_time_swap(SWAP_PACK, &find_data.struStartTime, 
                            &store_log->store[index].beg_time);
                        hik_time_swap(SWAP_PACK, &find_data.struStopTime, 
                            &store_log->store[index].end_time);

                        index++;
                    }

                    log_node++;
                }
                else if(ret == NET_DVR_FILE_NOFIND || ret == NET_DVR_NOMOREFILE)
                {
                    show_warn("NET_DVR_FILE_NOFIND or NET_DVR_NOMOREFILE\n");
                    store_log->node_count = index;
                    store_log->total_count = log_node;

                    ret = TRUE;
                    break;
                }
                else
                {
                    show_warn("find file fail for illegal get file state (%d)\n", 
                        NET_DVR_GetLastError());
                    store_log->node_count = index;
                    store_log->total_count = log_node;

                    ret = TRUE;
                    break;
                }
            }

            nmp_dealloc(find_cond, sizeof(NET_DVR_FILECOND));            
            NET_DVR_FindClose_V30(handle);
            break;

        case GET_CRUISE_CONFIG:
            cruise = (hik_cruise_t*)parm;
            show_debug("crz_no: %d<--------------------\n", cruise->crz_no);
            ret = NET_DVR_GetPTZCruise(user_id, 
                    (LONG)cruise->channel + offset, 
                    (LONG)cruise->crz_no, 
                    (NET_DVR_CRUISE_RET*)cruise->input);
            show_debug("NET_DVR_GetDVRConfig: %s<-------------------------------\n", 
                (ret == TRUE) ? "Success" : "Failure");
            if (FALSE == ret)
                show_debug("ERROR: %d<-------------------------------\n", NET_DVR_GetLastError());
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
            dev_cap->chn_cap.stream_max_frate_rate[0] = 30;
            dev_cap->chn_cap.img_cap = IMA_BRIGHTNESS | IMA_CONTRAST | IMA_SATURATION | IMA_HUE | IMA_GAMMA | IMA_SHARPNESS;
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
    config = (hik_config_t*)parm;
    ret = NET_DVR_GetDVRConfig(user_id, 
            (DWORD  ) config->command, 
            (LONG   ) config->channel + offset, 
            (LPVOID ) config->buffer, 
            (DWORD  ) config->b_size, 
            (LPDWORD)&config->returns);
    show_debug("NET_DVR_GetDVRConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", NET_DVR_GetLastError());
        return -1;
    }
}
static int 
hik_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = FALSE;
    int user_id, cmd;
    int offset = 0, handle = HIK_INVALID_HANDLE;

    hik_service_t *hik_srv;
    stream_info_t *strm_info;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    hik_config_t *config;

    NMP_ASSERT(srv && parm_id && parm);

    hik_srv = (hik_service_t*)srv;
    user_id = hik_get_user_id(&hik_srv->parm);

    if (HIK_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
        offset = HIK_NVR_CHANNEL_OFFSET;

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
            handle = NET_DVR_FormatDisk((LONG)user_id, (LONG)*((int*)parm));
            show_debug("NET_DVR_FormatDisk: [no:%d, %s], error: %d<-------------------------------\n", 
                *((int*)parm), (HIK_INVALID_HANDLE != handle) ? "Success" : "Failure", NET_DVR_GetLastError());
            if (HIK_INVALID_HANDLE != handle)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_FRMTDISK, (void*)handle);
                if (!ctrl)
                    ret = FALSE;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hik_srv->owner);
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
                            proxy_free_ctrl, hik_srv->owner);
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
            nmp_mutex_lock(hik_srv->rtsp.lock);
            strm_info = find_stream_by_channel(&hik_srv->rtsp.real_strm_list, 
                            ((hik_ptz_ctrl_t*)parm)->channel);
            if (strm_info && HIK_INVALID_HANDLE != strm_info->handle)
            {
                handle = strm_info->handle;
            }
            nmp_mutex_unlock(hik_srv->rtsp.lock);

            if (HIK_INVALID_HANDLE != handle)
            {
                ret = NET_DVR_PTZControl((LONG)handle, 
                    (DWORD)((hik_ptz_ctrl_t*)parm)->ptz_cmd, 
                    (DWORD)((hik_ptz_ctrl_t*)parm)->stop);
                show_debug("NET_DVR_PTZControl: %s<-------------------------------\n", 
                    (ret == TRUE) ? "Success" : "Failure");
            }
            else
                show_debug("handle: %d<-------------------------------\n", handle);
            break;

        case SET_PRESET_CONFIG:
            nmp_mutex_lock(hik_srv->rtsp.lock);
            strm_info = find_stream_by_channel(&hik_srv->rtsp.real_strm_list, 
                            ((struct hik_preset*)parm)->channel);
            if (strm_info && HIK_INVALID_HANDLE != strm_info->handle)
            {
                handle = strm_info->handle;
            }
            nmp_mutex_unlock(hik_srv->rtsp.lock);

            if (HIK_INVALID_HANDLE != handle)
            {
                ret = NET_DVR_PTZPreset((LONG)handle, 
                    (DWORD)((struct hik_preset*)parm)->preset_cmd, 
                    (DWORD)((struct hik_preset*)parm)->preset_no);
show_debug("NET_DVR_PTZPreset: %s<-------------------------------\n", (ret == TRUE) ? "Success" : "Failure");
            }
            break;

        case SET_CRUISE_CONFIG:
        case ADD_CRUISE_CONFIG:
        case MDF_CRUISE_CONFIG:
            nmp_mutex_lock(hik_srv->rtsp.lock);
            strm_info = find_stream_by_channel(&hik_srv->rtsp.real_strm_list, 
                            ((hik_cruise_t*)parm)->channel);
            if (strm_info && HIK_INVALID_HANDLE != strm_info->handle)
            {
                handle = strm_info->handle;
            }
            nmp_mutex_unlock(hik_srv->rtsp.lock);

            if (HIK_INVALID_HANDLE != handle)
            {
                ret = NET_DVR_PTZCruise((LONG)handle, 
                    (DWORD)((hik_cruise_t*)parm)->crz_cmd, 
                    (BYTE)((hik_cruise_t*)parm)->crz_no, 
                    (BYTE)0, 
                    (WORD)((hik_cruise_t*)parm)->input);
                show_debug("NET_DVR_PTZCruise: %s<-------------------------------\n", 
                    (ret == TRUE) ? "Success" : "Failure");
            }
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
    config = (hik_config_t*)parm;
    ret = NET_DVR_SetDVRConfig(user_id, 
            (DWORD )config->command, 
            (LONG  )config->channel + offset, 
            (LPVOID)config->buffer, 
            (DWORD )config->b_size);

    show_debug("NET_DVR_SetDVRConfig: %s<-------------------------------\n", 
        (ret == TRUE) ? "Success" : "Failure");
    if (TRUE == ret)
        return 0;
    else
    {
        show_debug("ERROR: %d<-------------------------------\n", 
            NET_DVR_GetLastError());
        return -1;
    }
}

hik_service_basic_t hik_srv_basic = 
{
    {
        g_sdk_items[SDK_HIK].sdk_name,
        hik_init,
        hik_cleanup,
        hik_create_service,
        hik_delete_service,
        hik_control_service,
        hik_check_service,
    },
    {
        hik_get_device_config,
        hik_set_device_config,
    },
    0,
    {
        1*1000,     //毫秒,
        10,
        10*1000,    //毫秒,
        1,
        hik_exception_call_back,
        hik_message_call_back,
    }
};

//////////////////////////////////////////////////////////////////////////
int hik_get_state(hik_parm_t *pm, int *state_timer)
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
int hik_set_state(hik_service_t *hik_srv, HIK_STATE_E state, int error)
{
    int flag, old;
    hik_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(hik_srv);

    old = hik_get_state(&hik_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("HIK State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case HIK_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case HIK_LOGING:
            flag = STATE_LOGING;
            break;
        case HIK_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return HIK_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(hik_srv->owner, &dev_state))
    {
        pm = &hik_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }

done:
    return old;
}
int hik_get_user_id(hik_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int hik_set_user_id(hik_service_t *hik_srv, int user_id)
{
    int ret, old;
    hik_parm_t *pm;

    NMP_ASSERT(hik_srv);

    pm = &hik_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (HIK_LOGOUT != old)
    {
        if (HIK_INVALID_HANDLE != hik_srv->listen)
        {
            ret = NET_DVR_StopListen_V30(hik_srv->listen);
            hik_srv->listen = HIK_INVALID_HANDLE;
            show_debug("NET_DVR_StopListen_V30: %s<-------------------------------\n", 
                ret ? "Success" : "Failure");
        }
        hik_logout(old);
    }

    if (HIK_LOGOUT == user_id)
        hik_stop_all_stream(&hik_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
hik_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (HIK_INVALID_HANDLE != strm_info->handle)
    {
        NET_DVR_StopRealPlay((LONG)strm_info->handle);
        strm_info->handle = HIK_INVALID_HANDLE;
    }

    return ;
}
static void 
hik_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    hik_rec_strm_t *rec_strm;

    NMP_ASSERT(strm_info);

    rec_strm = (hik_rec_strm_t*)strm_info;

    if (rec_strm->pos_timer)
    {
        nmp_del_timer(rec_strm->pos_timer);
        rec_strm->pos_timer = NULL;
    }

    if (HIK_INVALID_HANDLE != strm_info->handle)
    {
        NET_DVR_StopPlayBack((LONG)strm_info->handle);
        strm_info->handle = HIK_INVALID_HANDLE;
    }

    return ;
}

static void hik_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);
    return ;
}

void hik_stop_all_stream(hik_rtsp_t *rm)
{
    nmp_mutex_lock(rm->lock);

    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        hik_stop_one_stream, (void*)hik_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        hik_stop_one_stream, (void*)hik_stop_record_stream);

    nmp_mutex_unlock(rm->lock);
    return ;
}

void 
hik_cleanup_stream_info(hik_rtsp_t *rm)
{
    nmp_mutex_lock(rm->lock);

    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        hik_stop_real_stream, sizeof(hik_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        hik_stop_record_stream, sizeof(hik_rec_strm_t));

    nmp_mutex_unlock(rm->lock);
    return ;
}


