
#include "nmp_net_impl.h"
#include "nmp_proxy_sdk.h"
#include "nmp_proto_medium.h"

#include "nmp_platform_service.h"
#include "nmp_plt_parm_impl.h"
#include "nmp_plt_rtsp_impl.h"


extern Stream_operation strm_api;
extern struct _talk_ops talk_api;


static int establish_io(nmpio_t *io, void *init_data)
{//Never
    return 0;
}

static void finalize_io(nmpio_t *io, int err, void *init_data)
{
    platform_service_t *plt_srv = (platform_service_t*)init_data;
    finalize_parm_io(plt_srv, io, err);
}

static __inline__ void
register_all_msg_handlers(msg_engine_t *me)
{
    register_msg_handler(me, GET_DEVICE_INFO_ID, GET_DEVICE_CONFIG, get_device_info);
    register_msg_handler(me, GET_SERIAL_PARAMETER_ID, GET_SERIAL_CONFIG, get_serial_info);
    register_msg_handler(me, SET_SERIAL_PARAMETER_ID, SET_SERIAL_CONFIG, set_serial_info);
    register_msg_handler(me, GET_DEVICE_TIME_ID, GET_DEVICE_TIME, get_device_time);
    register_msg_handler(me, SET_DEVICE_TIME_ID, SET_DEVICE_TIME, set_device_time);
    register_msg_handler(me, GET_DEVICE_NTP_INFO_ID, GET_NTP_CONFIG, get_ntp_info);
    register_msg_handler(me, SET_DEVICE_NTP_INFO_ID, SET_NTP_CONFIG, set_ntp_info);
    register_msg_handler(me, GET_NETWORK_INFO_ID, GET_NETWORK_CONFIG, get_network_info);
    register_msg_handler(me, SET_NETWORK_INFO_ID, SET_NETWORK_CONFIG, set_network_info);
    register_msg_handler(me, GET_PPPOE_INFO_ID, GET_PPPOE_CONFIG, get_pppoe_info);
    register_msg_handler(me, SET_PPPOE_INFO_ID, SET_PPPOE_CONFIG, set_pppoe_info);
    register_msg_handler(me, GET_FTP_PARAMETER_ID, GET_FTP_CONFIG, get_ftp_info);
    register_msg_handler(me, SET_FTP_PARAMETER_ID, SET_FTP_CONFIG, set_ftp_info);
    register_msg_handler(me, GET_SMTP_PARAMETER_ID, GET_SMTP_CONFIG, get_smtp_info);
    register_msg_handler(me, SET_SMTP_PARAMETER_ID, SET_SMTP_CONFIG, set_smtp_info);
    register_msg_handler(me, GET_DDNS_CONFIG_REQUEST_ID, GET_DDNS_CONFIG, get_ddns_info);
    register_msg_handler(me, SET_DDNS_CONFIG_REQUEST_ID, SET_DDNS_CONFIG, set_ddns_info);
    register_msg_handler(me, GET_UPNP_PARAMETER_ID, GET_UPNP_CONFIG, get_upnp_info);
    register_msg_handler(me, SET_UPNP_PARAMETER_ID, SET_UPNP_CONFIG, set_upnp_info);
    register_msg_handler(me, GET_DEVICE_DISK_INFO_ID, GET_DISK_LIST, get_disk_list);
    register_msg_handler(me, FORMAT_DISK_REQUEST_ID, SET_DISK_FORMAT, format_disk);
    register_msg_handler(me, CONTROL_DEVICE_REQUEST_ID, CONTROL_DEVICE_CMD, control_device);

    register_msg_handler(me, GET_ENCODE_PARAMETER_ID, GET_ENCODE_CONFIG, get_encode_info);
    register_msg_handler(me, SET_ENCODE_PARAMETER_ID, SET_ENCODE_CONFIG, set_encode_info);
    register_msg_handler(me, GET_DISPLAY_PARAMETER_ID, GET_DISPLAY_CONFIG, get_display_info);
    register_msg_handler(me, SET_DISPLAY_PARAMETER_ID, SET_DISPLAY_CONFIG, set_display_info);
    register_msg_handler(me, GET_OSD_PARAMETER_ID, GET_OSD_CONFIG, get_osd_info);
    register_msg_handler(me, SET_OSD_PARAMETER_ID, SET_OSD_CONFIG, set_osd_info);
    register_msg_handler(me, GET_PTZ_PARAMETER_ID, GET_PTZ_CONFIG, get_ptz_info);
    register_msg_handler(me, SET_PTZ_PARAMETER_ID, SET_PTZ_CONFIG, set_ptz_info);
    register_msg_handler(me, GET_RECORD_PARAMETER_ID, GET_RECORD_CONFIG, get_record_info);
    register_msg_handler(me, SET_RECORD_PARAMETER_ID, SET_RECORD_CONFIG, set_record_info);
    register_msg_handler(me, GET_HIDE_PARAMETER_ID, GET_HIDE_CONFIG, get_hide_info);
    register_msg_handler(me, SET_HIDE_PARAMETER_ID, SET_HIDE_CONFIG, set_hide_info);
    register_msg_handler(me, GET_MOVE_ALARM_INFO_ID, GET_MOTION_CONFIG, get_move_alarm_info);
    register_msg_handler(me, SET_MOVE_ALARM_INFO_ID, SET_MOTION_CONFIG, set_move_alarm_info);
    register_msg_handler(me, GET_LOST_ALARM_INFO_ID, GET_VIDEO_LOST_CONFIG, get_video_lost_info);
    register_msg_handler(me, SET_LOST_ALARM_INFO_ID, SET_VIDEO_LOST_CONFIG, set_video_lost_info);
    register_msg_handler(me, GET_HIDE_ALARM_INFO_ID, GET_HIDE_ALARM_CONFIG, get_hide_alarm_info);
    register_msg_handler(me, SET_HIDE_ALARM_INFO_ID, SET_HIDE_ALARM_CONFIG, set_hide_alarm_info);
    register_msg_handler(me, GET_IO_ALARM_INFO_ID, GET_IO_ALARM_CONFIG, get_io_alarm_info);
    register_msg_handler(me, SET_IO_ALARM_INFO_ID, SET_IO_ALARM_CONFIG, set_io_alarm_info);

    register_msg_handler(me, GET_STORE_LOG_REQUEST_ID, GET_STORE_LOG, get_store_log);

    register_msg_handler(me, CONTROL_PTZ_COMMAND_ID, CONTROL_PTZ_CMD, ptz_control);
    register_msg_handler(me, SET_PRESET_POINT_REQUEST_ID, SET_PRESET_CONFIG, set_preset_point);
    register_msg_handler(me, GET_CRUISE_WAY_REQUEST_ID, GET_CRUISE_CONFIG, get_cruise_way);
    register_msg_handler(me, SET_CRUISE_WAY_REQUEST_ID, SET_CRUISE_CONFIG, set_cruise_way);
    register_msg_handler(me, ADD_CRUISE_WAY_REQUEST_ID, ADD_CRUISE_CONFIG, add_cruise_way);
    register_msg_handler(me, MODIFY_CRUISE_WAY_REQUEST_ID, MDF_CRUISE_CONFIG, modify_cruise_way);

    register_msg_handler(me, GET_TRANSPARENTPARAM_REQUEST_ID, GET_CAPABILITY_SET, transparent_get_param);

    register_msg_handler(me, GET_PLATFORM_INFO_ID, -1, get_platform_info);
    register_msg_handler(me, SET_PLATFORM_INFO_ID, -2, set_platform_info);
    register_msg_handler(me, GET_MEDIA_URL_REQUEST_ID, -3, get_media_url);
    register_msg_handler(me, GET_PRESET_POINT_SET_REQUEST_ID, -4, get_preset_set);
    register_msg_handler(me, GET_CRUISE_WAY_SET_REQUEST_ID, -5, get_cruise_set);

    return ;
}

static int 
platform_init_service(struct service_template *self)
{
    proxy_config_t cfg;
    plt_service_basic_t *plt_basic;
    NMP_ASSERT(self);

    plt_basic = (plt_service_basic_t*)self;

    plt_basic->_2lp_net = create_2lp_net_object(establish_io, finalize_io);
    if (!plt_basic->_2lp_net)
        goto NET_ERROR;

    plt_basic->pack_opt = nmp_new(packet_opt_t, 1);
    if (plt_basic->pack_opt)
    {
        memcpy((void*)plt_basic->pack_opt, &pkt_opt, sizeof(pkt_opt));
        plt_basic->pack_opt->proto.init_data = (void*)plt_basic;//Here, it's for establish_io, but never dispatch.
    }
    else
        goto PACKE_ERROR;

    plt_basic->rtsp_srv = rtsp_server_new();
    if (plt_basic->rtsp_srv)
    {
        get_proxy_config(&cfg);
        register_stream_operations(&strm_api);
        rtsp_server_set_port(plt_basic->rtsp_srv, (gint)cfg.rtsp_port);

        if (rtsp_server_bind_port(plt_basic->rtsp_srv))
        {
            show_warn("rtsp_server_bind_port fail!\n");
            rtsp_server_free(plt_basic->rtsp_srv);
            goto RTSP_ERROR;
        }
    }
    else
        goto RTSP_ERROR;

    plt_basic->talk_srv = talk_server_new();
    if(plt_basic->talk_srv)
    {
        register_talk_ops(&talk_api);
        if (0 > talk_server_start(plt_basic->talk_srv, TALK_PORT))
        {
            show_warn("talk_server_start fail!\n");
            talk_server_free(plt_basic->talk_srv);
            goto TALK_ERROR;
        }
    }
    else
        goto TALK_ERROR;

    plt_basic->plt_me = create_msg_engine(FALSE, 16);
    register_all_msg_handlers(plt_basic->plt_me);
    return 0;

NET_ERROR:
    return -1;

PACKE_ERROR:
    destory_2lp_net_object(plt_basic->_2lp_net);
    return -1;

RTSP_ERROR:
    destory_2lp_net_object(plt_basic->_2lp_net);
    nmp_del(plt_basic->pack_opt, packet_opt_t, 1);
    plt_basic->pack_opt = NULL;
    return -1;

TALK_ERROR:
    destory_2lp_net_object(plt_basic->_2lp_net);
    nmp_del(plt_basic->pack_opt, packet_opt_t, 1);
    plt_basic->pack_opt = NULL;
    rtsp_server_free(plt_basic->rtsp_srv);
    return -1;
}

static int 
platform_cleanup_service(struct service_template *self)
{
    plt_service_basic_t *plt_basic;
    NMP_ASSERT(self);

    plt_basic = (plt_service_basic_t*)self;
    destory_2lp_net_object(plt_basic->_2lp_net);
    nmp_del(plt_basic->pack_opt, packet_opt_t, 1);
    plt_basic->pack_opt = NULL;
    rtsp_server_free(plt_basic->rtsp_srv);
    talk_server_free(plt_basic->talk_srv);
    return 0;
}

static struct service *
platform_create_service(struct service_template *self, void *init_data)
{
    platform_service_t *plt_srv;
    plt_service_basic_t *plt_basic;
    NMP_ASSERT(self && init_data);

    plt_basic = (plt_service_basic_t*)self;
    plt_srv = (platform_service_t*)nmp_new0(platform_service_t, 1);
    plt_srv->base.tm = self;
    plt_srv->owner = (proxy_device_t*)init_data;

    init_parm_module(plt_srv, &plt_srv->parm);
    init_rtsp_module(plt_srv, &plt_srv->rtsp);

    memcpy(&plt_srv->proto, &plt_basic->pack_opt->proto, sizeof(nmp_2proto_t));
    plt_srv->proto.init_data = (void*)plt_srv;//Here, it's for finalize_io.

    return (struct service*)plt_srv;
}

static void 
platform_delete_service(struct service_template *self, struct service *srv)
{
    platform_service_t *plt_srv;
    NMP_ASSERT(NULL != self && NULL != srv);

    plt_srv = (platform_service_t*)srv;

    cleanup_parm_module(plt_srv, &plt_srv->parm);
    cleanup_rtsp_module(plt_srv, &plt_srv->rtsp);

    memset(plt_srv, 0, sizeof(platform_service_t));
    nmp_del(plt_srv, platform_service_t, 1);

    return ;
}

static int 
platform_control_service(struct service_template *self, struct service *srv, int cmd, void *user)
{
    platform_service_t *plt_srv;
    NMP_ASSERT(NULL != self && NULL != srv);

    plt_srv = (platform_service_t*)srv;
    control_parm_module(plt_srv, cmd, user);
    control_rtsp_module(plt_srv, cmd, user);
    return 0;
}
static int 
platform_check_service(struct service_template *self, struct service *srv)
{//show_debug("Enter %s()\n", __FUNCTION__);
    proxy_plt_t plt_info;
    platform_service_t *plt_srv = (platform_service_t*)srv;

    if (proxy_get_device_private(plt_srv->owner, DEV_PRI_PLT_SRV, &plt_info))
    {
        BUG();
    }

    check_parm_module(plt_srv, &plt_info);
    check_rtsp_module(plt_srv, &plt_info);
    return 0;
}

plt_service_basic_t plt_srv_basic = 
{
    {
        DEF_PLATFORM_SERVICE_NAME,
        platform_init_service,
        platform_cleanup_service,
        platform_create_service,
        platform_delete_service,
        platform_control_service,
        platform_check_service,
    },
    NULL,
    NULL,
    NULL,
};


