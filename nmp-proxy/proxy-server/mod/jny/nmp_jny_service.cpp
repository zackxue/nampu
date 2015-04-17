#include <string.h>
#include <unistd.h>

#include "nmp_resolve_host.h"

#include "nmp_jny_swap.h"
#include "nmp_jny_channel.h"
#include "nmp_jny_srv_impl.h"
#include "nmp_jny_service.h"
#include "nmp_jny_proc_cfg.h"

static void 
jny_stop_real_stream(struct stream_info *strm_info)
{show_debug("%s()<------------------------\n", __FUNCTION__);
	NMP_ASSERT(strm_info);

	if (-1 != strm_info->handle)
    {
    	Remote_Camera2_Close(strm_info->handle);
        strm_info->handle = -1;
    }

	return ;
}


static void jny_stop_one_stream(void *orig, void *custom)
{
    stop_stream stop_func;
    struct stream_info *strm_info;

    NMP_ASSERT(orig && custom);

    stop_func = (stop_stream)custom;
    strm_info = (struct stream_info*)orig;

    (*stop_func)(strm_info);

    return ;
}

static __inline__ int jny_logout(long user_id)
{
	int ret;
	ret = Remote_Nvd_UnInit(user_id);

	if (ret != SN_SUCCESS)
	{
	    show_debug("jny_logout() successful, user_id: %d\n", (int)user_id);
	    return 0;
	}
	else
	{
	    return -1;
	}
}

static __inline__ void jny_login(jny_service *jny_srv)
{
    char addr[MAX_IP_LEN];
    int ret, state, error;
    int user_id = JNY_LOGOUT;
    proxy_sdk_t sdk_info;
	ST_DeviceInfo stDeviceInfo;

    NMP_ASSERT(jny_srv);

    if (proxy_get_device_private(jny_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    memset(addr, 0, sizeof(addr));
    if (proxy_resolve_host_immediate((const char*)sdk_info.dev_host, addr, sizeof(addr)))
    {
    	memset(&stDeviceInfo, 0, sizeof(ST_DeviceInfo));
		stDeviceInfo.nDeviceType = IPCAMERA;
		memcpy(stDeviceInfo.stInetAddr.szHostIP, 
				addr, strlen(addr));
		stDeviceInfo.stInetAddr.nPORT = sdk_info.dev_port;
		stDeviceInfo.stInetAddr.nIPProtoVer = IPPROTO_V4;
		memcpy(stDeviceInfo.szPassword, sdk_info.password, strlen(sdk_info.password));
		memcpy(stDeviceInfo.szUserID, sdk_info.username, strlen(sdk_info.username));
		
		ret = Remote_Nvd_Init((long *)&user_id, &stDeviceInfo, TCP);
		if(ret != SN_SUCCESS)
		{
			jny_print_error(ret);
			user_id = JNY_LOGOUT;
		}
    }
    else
    {
        show_warn("Device host '%s' unsolved.\n", sdk_info.dev_host);
    }

	if (JNY_LOGOUT != user_id)
    {
        state = JNY_LOGIN;
        error = 0;
        show_debug("jny_login() successful, user_id: %d\n", user_id);
    }
    else
    {
        state = JNY_LOGOUT;
        error = ret;

        if (strlen(addr))
        {
            show_warn("jny_login(deviceID:%d) failure[%s:%s @ %s:%d]. Error: %d\n", 
                jny_srv->owner->fastenings.dev_id, 
                sdk_info.username, sdk_info.password, 
                addr, sdk_info.dev_port, ret);
        }
    }

    jny_set_user_id(jny_srv, user_id);
    jny_set_state(jny_srv, (JNY_STATE_E)state, error);

    return ;
}

static int jny_init(service_template_t *self)
{
	int ret = 0;
	jny_conn_info *conn_info;
	jny_service_basic *jny_basic;

	NMP_ASSERT(self);

	jny_basic = (jny_service_basic*)self;
	conn_info = &jny_basic->conn_info;
	


	return ret;
}


static int jny_cleanup(service_template_t *self)
{
	//    jny_service_basic *jny_basic = (jny_service_basic_t*)self;
    NMP_ASSERT(self);

    //NET_DVR_Cleanup();

    return 0;
}


static struct service *jny_create_service(service_template_t *self, void *init_data)
{
    static int flag = DEFAULT_DISCONNECTED_SECS;
    proxy_device_t *prx_dev;
    jny_service_basic *jny_basic;
    jny_service *jny_srv = NULL;

    NMP_ASSERT(self && init_data);

    jny_basic = (jny_service_basic *)self;
    prx_dev = (proxy_device_t *)init_data;

    jny_srv = (jny_service *)nmp_new0(jny_service, 1);
    if (jny_srv)
    {
    	jny_basic->total_count += 1;

    	if (0 > flag--)
    		flag = DEFAULT_DISCONNECTED_SECS;

    	jny_srv->base.tm = self;
    	jny_srv->parm.lock = nmp_mutex_new();
    	jny_srv->parm.user_id = JNY_LOGOUT;
    	jny_srv->parm.state = JNY_LOGOUT;
    	jny_srv->parm.state_timer = flag;		//使得服务flag 秒后便进行第一次连接设备，而不是等待整个策略周期

    	//jny_srv->rtsp.lock = nmp_mutex_new();
    	//jny_srv->rtsp.real_strm_list.lock = nmp_mutex_new();
    	//jny_srv->rtsp.real_strm_list.list = NULL;
    	//jny_srv->rtsp.rec_strm_list.lock = nmp_mutex_new();
    	//jny_srv->rtsp.rec_strm_list.list = NULL;

    	jny_srv->owner = prx_dev;
    }

    return (struct service *)jny_srv;
}

static void jny_delete_service(service_template_t *self, struct service *srv)
{
    jny_service *jny_srv;
    jny_service_basic *jny_basic;

    NMP_ASSERT(self && srv);

    jny_srv = (jny_service*)srv;
    jny_basic = (jny_service_basic*)self;

    jny_basic->total_count -= 1;

    jny_set_user_id(jny_srv, JNY_LOGOUT);				//注销登录
    jny_set_state(jny_srv, JNY_LOGOUT, 0);

    /* 清理视频流链表 */
    jny_cleanup_stream_info(&jny_srv->rtsp);

    //if (jny_srv->rtsp.real_strm_list.lock)
    //    nmp_mutex_free(jny_srv->rtsp.real_strm_list.lock);
	
    //if (jny_srv->rtsp.rec_strm_list.lock)
    //    nmp_mutex_free(jny_srv->rtsp.rec_strm_list.lock);

    if (jny_srv->parm.lock)
        nmp_mutex_free(jny_srv->parm.lock);				//销毁相关锁

    //if (jny_srv->rtsp.lock)
    //    nmp_mutex_free(jny_srv->rtsp.lock);				//销毁相关锁

    memset(jny_srv, 0, sizeof(jny_service));			//内存块归零
    nmp_del(jny_srv, jny_service, 1);					//释放内存块
    return ;
}


static int jny_control_service(service_template_t *self, struct service *srv,
									int cmd, void *parm)
{
    int ret = -1;
    jny_service *jny_srv;
    jny_service_basic *jny_basic;

//	struct jny_format format;

    NMP_ASSERT(self && srv);

    jny_srv = (jny_service *)srv;
    jny_basic = (jny_service_basic *)self;

    switch (cmd)
    {
        case CTRL_CMD_LOGOUT:
            if (!jny_logout((long)jny_get_user_id(&jny_srv->parm)))
            {
                ret = 0;
                jny_set_user_id(jny_srv, JNY_LOGOUT);
                jny_set_state(jny_srv, JNY_LOGOUT, 0);
            }
            break;
        case CTRL_CMD_LOGIN:
            jny_login((jny_service*)jny_srv);
            ret = 0;
            break;
        case CTRL_CMD_RESET:
	
            break;
        case CTRL_CMD_REBOOT:

            break;
        case CTRL_CMD_SHUTDOWN:

            break;
        case CTRL_CMD_FRMTDISK:

            break;
        case CTRL_CMD_SUBMIT:

            break;

        case CTRL_CMD_COMPARE:

            break;

        case CTRL_CMD_OBTAIN_HANDLER:
            *(int*)parm = (int)&jny_handler_table;
			ret = 0;
            break;
        case CTRL_CMD_OBTAIN_STRM_OPT:
            *(int*)parm = (int)&jny_strm_opt;
            ret = 0;
            break;
        case CTRL_CMD_OBTAIN_TALK_OPT:
            break;

    	default:
    		break;
    }

    return ret;
}


static int jny_check_service(service_template_t *self, struct service *srv)
{
    int state_timer;
    proxy_ctrl_t *ctrl;
    proxy_task_t *task;
    jny_service *jny_srv;
	int ret;
	
    NMP_ASSERT(self && srv);

    jny_srv = (jny_service*)srv;

    switch (jny_get_state(&jny_srv->parm, &state_timer))
    {
        case JNY_LOGOUT:
            if (state_timer < DEFAULT_DISCONNECTED_SECS)
                return -1;

			ctrl = proxy_new_ctrl(srv, CTRL_CMD_LOGIN, NULL);
            if (!ctrl)
                ret = -1;
            else
            {
                task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                        proxy_free_ctrl, jny_srv->owner);
                if (task)
                {
                    jny_set_state(jny_srv, JNY_LOGING, 0);
                    proxy_thread_pool_push(task);
                }
                else
                {
                    proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                    ret = -2;
                }
            }
		break;

        case JNY_LOGING:
            break;

        case JNY_LOGIN:
            break;
    }

    return ret;
}



static int jny_get_device_config(struct service *srv, int parm_id, void *parm)
{
	int ret = -1, flag = -1;
	jny_service *jny_srv;
	proxy_sdk_t sdk_info;

    int user_id;

    NMP_ASSERT(srv && parm);

    jny_srv = (jny_service *)srv;
    user_id = jny_get_user_id(&jny_srv->parm);

	flag = Remote_System_Open(user_id);
	if(flag != SN_SUCCESS)
	{
		jny_print_error(flag);
		//user_id = JNY_LOGOUT;
		return -1;
	}


    if (proxy_get_device_private(jny_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    if (JNY_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

    switch (parm_id)
    {
        case GET_DEVICE_CONFIG:
			printf("************************GET_DEVICE_CONFIG\n");
			ret = jny_proc_get_device_cfg(user_id, parm, &sdk_info);
			printf("jny_proc_get_device_cfg***********************ret = %d\n", ret);
			break;
		
		case GET_SERIAL_CONFIG:
			ret = jny_proc_get_serial_cfg(user_id, parm, &sdk_info);

			break;

		case GET_DEVICE_TIME:
			ret = jny_proc_get_dev_time_cfg(user_id, parm, &sdk_info);

			break;

		case GET_NTP_CONFIG:
			ret = jny_proc_get_ntp_cfg(user_id, parm, &sdk_info);
			break;

		case GET_NETWORK_CONFIG:
			ret = jny_proc_get_network_cfg(user_id, parm, &sdk_info);
			break;

		case GET_PPPOE_CONFIG:
			ret = jny_proc_get_pppoe_cfg(user_id, parm, &sdk_info);
			break;

		case GET_FTP_CONFIG:
			ret = jny_proc_get_ftp_cfg(user_id, parm, &sdk_info);
			break;

		case GET_DDNS_CONFIG:
			ret = jny_proc_get_ddns_cfg(user_id, parm, &sdk_info);
			break;

		case GET_SMTP_CONFIG:
			ret = jny_proc_get_smtp_cfg(user_id, parm, &sdk_info);
			break;

		case GET_DISK_LIST:
			ret = jny_proc_get_disk_cfg(user_id, parm, &sdk_info);
			break;

		case GET_ENCODE_CONFIG:
			ret = jny_proc_get_encode_cfg(user_id, parm, &sdk_info);
			break;
			
		case GET_OSD_CONFIG:
			ret = jny_proc_get_osd_cfg(user_id, parm, &sdk_info);
			break;

		case GET_PTZ_CONFIG:
			ret = jny_proc_get_ptz_cfg(user_id, parm, &sdk_info);
			break;

		case GET_HIDE_CONFIG:
			ret = jny_proc_get_hide_cfg(user_id, parm, &sdk_info);
			break;

		case GET_MOTION_CONFIG:
			ret = jny_proc_get_move_alarm_cfg(user_id, parm, &sdk_info);
			break;

		case GET_CAPABILITY_SET:
			ret = jny_proc_get_capability_list(user_id, parm, &sdk_info);
			break;

		case GET_HIDE_ALARM_CONFIG:
			ret = jny_proc_get_hide_alarm_cfg(user_id, parm, &sdk_info);
			break;

		case GET_VIDEO_LOST_CONFIG:
			ret = jny_proc_get_video_lost_cfg(user_id, parm, &sdk_info);
			break;

		case GET_RECORD_CONFIG:
			ret = jny_proc_get_record_cfg(user_id, parm, &sdk_info);
			break;
			
		default:
			break;
	}
	flag = Remote_System_Close(user_id);
	if(flag != SN_SUCCESS)
	{
		jny_print_error(flag);
		//user_id = JNY_LOGOUT;
		return -1;
	}
	return ret;
}


static int jny_set_device_config(struct service *srv, int parm_id, void *parm)
{
	int ret, flag;
	jny_service *jny_srv;
	proxy_sdk_t sdk_info;

    int user_id;

    NMP_ASSERT(srv && parm);

    jny_srv = (jny_service *)srv;
    user_id = jny_get_user_id(&jny_srv->parm);


	flag = Remote_System_Open(user_id);
	if(flag != SN_SUCCESS)
	{
		jny_print_error(flag);
		//user_id = JNY_LOGOUT;
		return -1;
	}

    if (proxy_get_device_private(jny_srv->owner, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    if (JNY_LOGOUT == user_id)
    {
        show_debug("Proxy device logout!!!!!!!!!!!!!!\n");
        return -1;
    }

	switch(parm_id)
	{
		case SET_SERIAL_CONFIG:
			ret = jny_proc_set_serial_cfg(user_id, parm, &sdk_info);
			break;
			
		case SET_DEVICE_TIME:
			ret = jny_proc_set_dev_time_cfg(user_id, parm, &sdk_info);
			break;
			
		case SET_NTP_CONFIG:
			ret = jny_proc_set_ntp_cfg(user_id, parm, &sdk_info);
			break;

		case SET_NETWORK_CONFIG:
			ret = jny_proc_set_network_cfg(user_id, parm, &sdk_info);
			break;

		case SET_PPPOE_CONFIG:
			ret = jny_proc_set_pppoe_cfg(user_id, parm, &sdk_info);
			break;

		case SET_FTP_CONFIG:
			ret = jny_proc_set_ftp_cfg(user_id, parm, &sdk_info);
			break;

		case SET_DDNS_CONFIG:
			ret = jny_proc_set_ddns_cfg(user_id, parm, &sdk_info);
			break;

		case SET_SMTP_CONFIG:
			ret = jny_proc_set_smtp_cfg(user_id, parm, &sdk_info);
			break;
			
		case CONTROL_DEVICE_CMD:
			ret = jny_proc_ctrl_device(user_id, parm, &sdk_info);
			break;
			
		case SET_ENCODE_CONFIG:
			ret = jny_proc_set_encode_cfg(user_id, parm, &sdk_info);
			break;
			
		case SET_OSD_CONFIG:
			ret = jny_proc_set_osd_cfg(user_id, parm, &sdk_info);
			break;

		case SET_PTZ_CONFIG:
			ret = jny_proc_set_ptz_cfg(user_id, parm, &sdk_info);
			break;

		case SET_HIDE_CONFIG:
			ret = jny_proc_set_hide_cfg(user_id, parm, &sdk_info);
			break;

		case SET_MOTION_CONFIG:
			ret = jny_proc_set_move_alarm_cfg(user_id, parm, &sdk_info);
			break;

		case SET_HIDE_ALARM_CONFIG:
			ret = jny_proc_set_hide_alarm_cfg(user_id, parm, &sdk_info);
			break;

		case SET_VIDEO_LOST_CONFIG:
			ret = jny_proc_set_video_lost_cfg(user_id, parm, &sdk_info);
			break;

		case CONTROL_PTZ_CMD:
			ret = jny_proc_control_ptz(user_id, parm, &sdk_info);
			break;

		case SET_PRESET_CONFIG:
			ret = jny_proc_set_preset_point_cfg(user_id, parm, &sdk_info);
			break;

		case ADD_CRUISE_CONFIG:
			ret = jny_proc_add_cruise_cfg(user_id, parm, &sdk_info);
			break;

		case SET_RECORD_CONFIG:
			ret = jny_proc_set_record_cfg(user_id, parm, &sdk_info);
			break;
			
		default:
			break;
	}
	flag = Remote_System_Close(user_id);
	if(flag != SN_SUCCESS)
	{
		jny_print_error(flag);
		//user_id = JNY_LOGOUT;
		return -1;
	}
	return ret;
}


jny_service_basic jny_srv_basic = 
{
    {
        g_sdk_items[SDK_JNY].sdk_name,
        jny_init,
        jny_cleanup,
        jny_create_service,
        jny_delete_service,
        jny_control_service,
        jny_check_service,
    },
    {
        jny_get_device_config,
        jny_set_device_config,
    },
    0,
    {
        1*1000,		//毫秒,
        10,
        10*1000,	//毫秒,
        1,
    }
};

int jny_get_state(jny_parm *pm, int *state_timer)
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

int jny_set_state(jny_service *jny_srv, JNY_STATE_E state, int error)
{
    int flag, old;
    jny_parm *pm;
    proxy_state_t dev_state;

    NMP_ASSERT(jny_srv);

    old = jny_get_state(&jny_srv->parm);
    if (old == state && !error)
    {/* 如果新旧状态相同，而且错误码为0，
            则直接返回，不更新上层状态*/
        show_info("JNY State [%d:%d] No Update!!!!!!!!!!!!\n", old, state);
        goto done;
    }

    switch (state)
    {
        case JNY_LOGOUT:
            flag = STATE_LOGOUT;
            break;
        case JNY_LOGING:
            flag = STATE_LOGING;
            break;
        case JNY_LOGIN:
            flag = STATE_LOGIN;
            break;
        default:
            return JNY_UNKNOWN;
    }
    dev_state.state = flag;
    dev_state.error = error;

    if (flag == proxy_set_device_state(jny_srv->owner, &dev_state))
    {
        pm = &jny_srv->parm;
        nmp_mutex_lock(pm->lock);
        pm->state = state;
        pm->state_timer = 0;
        old = pm->state;
        nmp_mutex_unlock(pm->lock);
    }

done:
    return old;
}


int jny_get_user_id(jny_parm *pm)
{
    int id;
    NMP_ASSERT(pm);

    nmp_mutex_lock(pm->lock);
    id = pm->user_id;
    nmp_mutex_unlock(pm->lock);

    return id;
}


void jny_cleanup_stream_info(jny_rtsp *rm)
{
    //nmp_mutex_lock(rm->lock);

    /* 清理实时流链表 */
    destory_stream_list(&rm->real_strm_list, 
    	jny_stop_real_stream, sizeof(jny_real_strm_info));

    /* 清理历史流链表 */
    //destory_stream_list(&rm->rec_strm_list, 
    //	jny_stop_record_stream, sizeof(jny_rec_strm));

    //nmp_mutex_unlock(rm->lock);
    return ;
}

void jny_stop_all_stream(jny_rtsp *rm)
{
    //nmp_mutex_lock(rm->lock);

    nmp_list_foreach(nmp_list_first(rm->real_strm_list.list), 
        jny_stop_one_stream, (void*)jny_stop_real_stream);

    //nmp_list_foreach(nmp_list_first(rm->rec_strm_list.list), 
    //    jny_stop_one_stream, (void*)jny_stop_record_stream);

   // nmp_mutex_unlock(rm->lock);
    return ;
}


int jny_set_user_id(jny_service *jny_srv, int user_id)
{
    int ret, old;
    jny_parm *pm;

    NMP_ASSERT(jny_srv);

    pm = &jny_srv->parm;

    nmp_mutex_lock(pm->lock);
    old = pm->user_id;
    pm->user_id = user_id;
    nmp_mutex_unlock(pm->lock);

    if (JNY_LOGOUT != old)
    {
        	ret = Remote_Camera2_Close(old);
        
            show_debug("Remote_Camera2_Close: %s<-------------------------------\n", 
                (ret == SN_SUCCESS) ? "Success" : "Failure");
        jny_logout(old);
    }

    if (JNY_LOGOUT == user_id)
        jny_stop_all_stream(&jny_srv->rtsp);

    return user_id;
}



