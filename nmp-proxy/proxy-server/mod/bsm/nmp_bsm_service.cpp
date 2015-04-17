#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_bsm_swap.h"
#include "nmp_bsm_channel.h"
#include "nmp_bsm_srv_impl.h"
#include "nmp_bsm_service.h"


static __inline__ int bsm_logout(int user_id)
{
    if (HI_NET_DEV_Logout((HI_U32)user_id))
    {
        show_debug("bsm_logout() successful, user_id: %d\n", user_id);
        return 0;
    }
    else
        return -1;
}
static __inline__ void bsm_login(bsm_service_t *bsm_srv)
{
    char addr[MAX_IP_LEN];
    int state, error = 0;
    int user_id = BSM_LOGOUT;
    proxy_sdk_t sdk_info;

    NMP_ASSERT(bsm_srv);

    if (proxy_get_device_private(bsm_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
        error = HI_NET_DEV_Login((HI_U32*)&user_id, 
            sdk_info.username, sdk_info.password, 
            addr, sdk_info.dev_port);
    }
    else
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);

    if (BSM_LOGOUT != user_id)
    {
        state = BSM_LOGIN;
        error = 0;
        show_debug("bsm_login() successful, user_id: %d\n", user_id);
    }
    else
    {
        state = BSM_LOGOUT;
        //error = ;

        if (strlen(addr))
        {
            show_warn("bsm_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                bsm_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, error);
        }
    }

    bsm_set_user_id(bsm_srv, user_id);
    bsm_set_state(bsm_srv, (BSM_STATE_E)state, error);

    return ;
}

static int bsm_init(service_template_t *self)
{
    int ret = 0;
    bsm_service_basic_t *bsm_basic;

    NMP_ASSERT(self);

    bsm_basic = (bsm_service_basic_t*)self;

    if (HI_SUCCESS != HI_NET_DEV_Init())
    {
        ret = -1;
        show_warn("HI_NET_DEV_Init() failure!\n");
    }
    else
    {
    }

    return ret;
}
static int bsm_cleanup(service_template_t *self)
{
    NMP_ASSERT(self);
    HI_NET_DEV_DeInit();
    return 0;
}

static struct service *
bsm_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    bsm_service_t *bsm_srv;
    bsm_service_basic_t *bsm_basic;

    NMP_ASSERT(self && init_data);

    bsm_basic = (bsm_service_basic_t*)self;
    prx_dev   = (proxy_device_t*)init_data;

    bsm_srv = (bsm_service_t*)nmp_new0(bsm_service_t, 1);
    if (bsm_srv)
    {
        bsm_basic->total_count += 1;

        if (0 > flag--)
            flag = DEFAULT_DISCONNECTED_SECS;

        bsm_srv->base.tm = self;
        bsm_srv->parm.lock = nmp_mutex_new();
        bsm_srv->parm.user_id = BSM_LOGOUT;
        bsm_srv->parm.state = BSM_LOGOUT;
        bsm_srv->parm.state_timer = flag;       //使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

        bsm_srv->rtsp.lock = nmp_mutex_new();
        bsm_srv->rtsp.real_strm_list.list = NULL;
        bsm_srv->rtsp.rec_strm_list.list = NULL;

        bsm_srv->owner = prx_dev;
    }

    return (struct service *)bsm_srv;
}
static void 
bsm_delete_service(service_template_t *self, struct service *srv)
{
    bsm_service_t *bsm_srv;
    bsm_service_basic_t *bsm_basic;

    NMP_ASSERT(self && srv);

    bsm_srv = (bsm_service_t*)srv;
    bsm_basic = (bsm_service_basic_t*)self;

    bsm_basic->total_count -= 1;

    bsm_set_user_id(bsm_srv, BSM_LOGOUT);               //注销登录
    bsm_set_state(bsm_srv, BSM_LOGOUT, 0);

    /* 清理视频流链表 */
    bsm_cleanup_stream_info(&bsm_srv->rtsp);

    if (bsm_srv->parm.lock)
        nmp_mutex_free(bsm_srv->parm.lock);
    if (bsm_srv->rtsp.lock)
        nmp_mutex_free(bsm_srv->rtsp.lock);             //销毁相关锁

    memset(bsm_srv, 0, sizeof(bsm_service_t));          //内存块归零
    nmp_del(bsm_srv, bsm_service_t, 1);                   //释放内存块
    return ;
}

static int 
bsm_control_service(service_template_t *self, struct service *srv, int cmd, void *parm)
{
    int user_id, ret = -1;
    bsm_service_t *bsm_srv;
    bsm_service_basic_t *bsm_basic;

//  struct bsm_format format;

    NMP_ASSERT(self && srv);

    bsm_srv = (bsm_service_t*)srv;
    bsm_basic = (bsm_service_basic_t*)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!bsm_logout(bsm_get_user_id(&bsm_srv->parm)))
            {
                ret = 0;
                bsm_set_user_id(bsm_srv, BSM_LOGOUT);
                bsm_set_state(bsm_srv, BSM_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            bsm_login(bsm_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
            sleep(2);
            ret = HI_NET_DEV_SetConfig((HI_U32)bsm_get_user_id(&bsm_srv->parm), 
                    (HI_U32)HI_NET_DEV_CMD_RESET, (HI_VOID*)NULL, (HI_U32)0);
            show_debug("HI_NET_DEV_SetConfig: %s<-------------------------------\n", 
                (ret == HI_SUCCESS) ? "Success" : "Failure");
            if (HI_SUCCESS == ret)
            {
                ret = 0;
                bsm_set_user_id(bsm_srv, BSM_LOGOUT);
                bsm_set_state(bsm_srv, BSM_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_REBOOT:
            sleep(2);
            ret = HI_NET_DEV_SetConfig((HI_U32)bsm_get_user_id(&bsm_srv->parm), 
                    (HI_U32)HI_NET_DEV_CMD_REBOOT, (HI_VOID*)NULL, (HI_U32)0);
            show_debug("HI_NET_DEV_SetConfig: %s<-------------------------------\n", 
                (ret == HI_SUCCESS) ? "Success" : "Failure");
            if (HI_SUCCESS == ret)
            {
                ret = 0;
                bsm_set_user_id(bsm_srv, BSM_LOGOUT);
                bsm_set_state(bsm_srv, BSM_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_SHUTDOWN:
            break;
        case CTRL_CMD_FRMTDISK:
//            memcpy((void*)&format.handle, (void*)&parm, sizeof(format.handle));
//            bsm_submit_format_progress((proxy_device_t*)bsm_srv->owner, &format);
            break;
        case CTRL_CMD_SUBMIT:
            break;

        case CTRL_CMD_COMPARE:
            user_id = (int)parm;
            if (user_id == bsm_get_user_id(&bsm_srv->parm))
                ret = 0;
            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&bsm_handler_table;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&bsm_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            break;

        default:
            break;
    }

    return ret;
}
static int 
bsm_check_service(service_template_t *self, struct service *srv)
{
    int ret = 0;
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    bsm_service_t *bsm_srv;

    NMP_ASSERT(self && srv);

    bsm_srv = (bsm_service_t*)srv;

    switch (bsm_get_state(&bsm_srv->parm, &state_timer))
    {
        case BSM_LOGOUT:
            if (state_timer > DEFAULT_DISCONNECTED_SECS)
            {
                ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
                if (!ctrl)
                    ret = -1;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, bsm_srv->owner);
                    if (task)
                    {
                        bsm_set_state(bsm_srv, BSM_LOGING, 0);
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

        case BSM_LOGING:
            break;

        case BSM_LOGIN:
            break;
    }

    return ret;
}

static int 
bsm_get_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = HI_FAILURE;
    int user_id;

    JDevCap *dev_cap;
    bsm_config_t *config;
    bsm_service_t*bsm_srv;

    NMP_ASSERT(srv && parm);

    bsm_srv = (bsm_service_t*)srv;
    user_id = bsm_get_user_id(&bsm_srv->parm);

    if (BSM_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    switch (parm_id)
    {
        case GET_DEVICE_CONFIG:
            goto GET_CFG;
        case GET_NETWORK_CONFIG:
            goto GET_CFG;
        case GET_FTP_CONFIG:
            goto GET_CFG;
        case GET_SMTP_CONFIG:
            goto GET_CFG;
        case GET_DDNS_CONFIG:
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
        case GET_MOTION_CONFIG:
            goto GET_CFG;

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
            dev_cap->chn_cap.stream_supp_resolution[0] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_720P;
            dev_cap->chn_cap.stream_supp_resolution[1] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_720P;
            dev_cap->chn_cap.stream_supp_resolution[2] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_720P;
            dev_cap->chn_cap.stream_supp_resolution[3] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | 
                CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_720P;
            dev_cap->chn_cap.stream_max_frate_rate[0] = 25;
            dev_cap->chn_cap.img_cap = IMA_BRIGHTNESS | IMA_CONTRAST | IMA_SATURATION | IMA_HUE;
            ret = HI_SUCCESS;
            break;

        default:
            ret = HI_FAILURE;
            show_warn("parm_id Invalid!!!!!\n");
            break;
    }

    if (HI_SUCCESS == ret)
        return 0;
    else
        return -1;

GET_CFG:
    config = (bsm_config_t*)parm;
    ret = HI_NET_DEV_GetConfig(
            (HI_U32)  user_id, 
            (HI_U32)  config->command, 
            (HI_VOID*)config->buffer, 
            (HI_U32)  config->b_size);
    show_debug("HI_NET_DEV_GetConfig: %s<-------------------------------\n", 
        (ret == HI_SUCCESS) ? "Success" : "Failure");
    if (HI_SUCCESS == ret)
        return 0;
    else
        return -1;
}
static int 
bsm_set_device_config(struct service *srv, int parm_id, void *parm)
{
    int ret = HI_FAILURE;
    int user_id, cmd;

    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    bsm_config_t *config;
    bsm_service_t*bsm_srv;

    NMP_ASSERT(srv && parm_id && parm);

    bsm_srv = (bsm_service_t*)srv;
    user_id = bsm_get_user_id(&bsm_srv->parm);

    if (BSM_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    switch (parm_id)
    {
        case SET_NETWORK_CONFIG:
            goto SET_CFG;
        case SET_FTP_CONFIG:
            goto SET_CFG;
        case SET_SMTP_CONFIG:
            goto SET_CFG;
        case SET_DDNS_CONFIG:
            goto SET_CFG;
        case SET_DISK_FORMAT:
            goto SET_CFG;

        case CONTROL_DEVICE_CMD:
            switch (((JControlDevice*)parm)->command)
            {
                case SHUTDOWN_DEVICE:
                    ret = HI_SUCCESS;
                    cmd = CTRL_CMD_SHUTDOWN;
                    break;
                case RESTART_DEVICE:
                    ret = HI_SUCCESS;
                    cmd = CTRL_CMD_REBOOT;
                    break;
                case RESTORE_DEFAULT:
                    ret = HI_SUCCESS;
                    cmd = CTRL_CMD_RESET;
                    break;

                case DETECT_DEAL_PIX:
                    ret = HI_FAILURE;
                    break;
                case DETECT_IRIS:
                    ret = HI_FAILURE;
                    break;

                default:
                    ret = HI_FAILURE;
                    break;
            }

            if (HI_SUCCESS == ret)
            {
                ctrl = proxy_new_ctrl(srv, cmd, NULL);
                if (!ctrl)
                    ret = HI_FAILURE;
                else
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, bsm_srv->owner);
                    if (task)
                    {
                        proxy_thread_pool_push(task);
                        ret = HI_SUCCESS;
                    }
                    else
                    {
                        proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                        ret = HI_FAILURE;
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
            config = (bsm_config_t*)parm;
            switch (config->command)
            {
                case BSM_PTZ_CTRL_STD:
                    HI_NET_DEV_PTZ_Ctrl_Standard((HI_U32)user_id, 
                        (HI_U32)((bsm_ptz_ctrl_std_t*)config->buffer)->cmd, 
                        (HI_U32)((bsm_ptz_ctrl_std_t*)config->buffer)->speed);
                    break;
                case BSM_PTZ_CTRL_PRS:
                    HI_NET_DEV_PTZ_Ctrl_Preset((HI_U32)user_id, 
                        (HI_U32)((bsm_ptz_ctrl_prs_t*)config->buffer)->cmd, 
                        (HI_U32)((bsm_ptz_ctrl_prs_t*)config->buffer)->preset);
                    break;
                case BSM_PTZ_CTRL_EXT:
                    HI_NET_DEV_PTZ_Ctrl_Extend((HI_U32)user_id, 
                        (HI_U32)((bsm_ptz_ctrl_ext_t*)config->buffer)->cmd);
                    break;
            }
            break;
        case SET_MOTION_CONFIG:
            goto SET_CFG;

        default:
            ret = HI_FAILURE;
            break;
    }

    if (HI_SUCCESS == ret)
        return 0;
    else
        return -1;

SET_CFG:
    config = (bsm_config_t*)parm;
    ret = HI_NET_DEV_SetConfig(
            (HI_U32)  user_id, 
            (HI_U32)  config->command, 
            (HI_VOID*)config->buffer, 
            (HI_U32)  config->b_size);
    show_debug("HI_NET_DEV_SetConfig: %s<-------------------------------\n", 
        (ret == HI_SUCCESS) ? "Success" : "Failure");
    if (HI_SUCCESS == ret)
        return 0;
    else
        return -1;
}

bsm_service_basic_t bsm_srv_basic = 
{
    {
        g_sdk_items[SDK_BSM].sdk_name,
        bsm_init,
        bsm_cleanup,
        bsm_create_service,
        bsm_delete_service,
        bsm_control_service,
        bsm_check_service,
    },
    {
        bsm_get_device_config,
        bsm_set_device_config,
    },
    0
};

//////////////////////////////////////////////////////////////////////////
int bsm_get_state(bsm_parm_t *pm, int *state_timer)
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
int bsm_set_state(bsm_service_t *bsm_srv, BSM_STATE_E state, int error)
{
    int flag, old;
    bsm_parm_t *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(bsm_srv);

    old = bsm_get_state(&bsm_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("BSM State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case BSM_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case BSM_LOGING:
            flag = STATE_LOGING;
            break;
        case BSM_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return BSM_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(bsm_srv->owner, &dev_state))
    {
        pm = &bsm_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }
    
done:
    return old;
}
int bsm_get_user_id(bsm_parm_t *pm)
{
    int user_id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    user_id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return user_id;
}
int bsm_set_user_id(bsm_service_t *bsm_srv, int user_id)
{
    int old;
    bsm_parm_t *pm;

    NMP_ASSERT(bsm_srv);

    pm = &bsm_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (BSM_LOGOUT != old)
        bsm_logout(old);

    if (BSM_LOGOUT == user_id)
        bsm_stop_all_stream(&bsm_srv->rtsp);

    return user_id;
}

////////////////////////////////////////////////////////////////////////////
static void 
bsm_stop_real_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (BSM_INVALID_HANDLE != strm_info->handle)
    {
        HI_NET_DEV_StopStream((HI_U32)strm_info->handle);
        strm_info->handle = BSM_INVALID_HANDLE;
    }

    return ;
}
static void 
bsm_stop_record_stream(stream_info_t *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
    NMP_ASSERT(strm_info);

    if (BSM_INVALID_HANDLE != strm_info->handle)
    {
        //
        strm_info->handle = BSM_INVALID_HANDLE;
    }

    return ;
}

static void bsm_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    stream_info_t *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (stream_info_t*)orig;
    (*stop_func)(strm_info);

    return ;
}

void bsm_stop_all_stream(struct bsm_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        bsm_stop_one_stream, (void*)bsm_stop_real_stream);

    nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
        bsm_stop_one_stream, (void*)bsm_stop_record_stream);
    nmp_mutex_unlock(rm->lock);
}

void 
bsm_cleanup_stream_info(struct bsm_rtsp *rm)
{
    nmp_mutex_lock(rm->lock);
    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
        bsm_stop_real_stream, sizeof(bsm_real_strm_t));

    /* 清理历史流链表 */
    destory_stream_list(&rm->rec_strm_list, 
        bsm_stop_record_stream, sizeof(bsm_rec_strm_t));
    nmp_mutex_unlock(rm->lock);
}




