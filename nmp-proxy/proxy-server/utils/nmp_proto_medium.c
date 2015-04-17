
#include "nmp_base64.h"
#include "nmp_transform.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_sdk.h"
#include "nmp_resolve_host.h"
#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"
#include "nmp_proto_medium.h"




static __inline__ int 
get_alarm_type(const char *gu_id)
{
    int flag = 0;
    char *p, *pvalue;
    char type[3];

    NMP_ASSERT(gu_id);

    p = pvalue = (char*)strchr(gu_id, '-');
    while (p)
    {
        if (3 == flag)
        {
            type[0] = *pvalue;
            type[1] = *(++pvalue);
            type[2] = '\0';
            show_info("type: %s\n", type);
            break;
        }
        else
        {
            pvalue = p+1;
            p = (char*)strchr(pvalue, '-');
            flag++;
        }
    }

    if (3 == flag)
    {
        if (!strcmp(type, "AI"))
            return ALARM_IN;
        if (!strcmp(type, "AO"))
            return ALARM_OUT;
    }

    return -1;
}

static __inline__ int 
get_channel(const char *gu_id)
{
    int channel = 0;
    char *p, *pvalue;

    NMP_ASSERT(gu_id);

    p = pvalue = (char*)strchr(gu_id, '-');
    while (p)
    {
        pvalue = p+1;
        p = (char*)strchr(pvalue, '-');
    }

    if (pvalue)
    {
        sscanf(pvalue, "%d", &channel);
    }

    return channel;
}
static __inline__ int 
get_level(const char *gu_id)
{
    int level = 0;
    char *p, *pvalue, *old;

    char buffer[J_SDK_MAX_ID_LEN];
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, gu_id);

    p = pvalue = strchr(buffer, '-');
    while (p)
    {
        old = pvalue;
        pvalue = p+1;
        p = strchr(pvalue, '-');
    }

    if (pvalue)
    {
        pvalue = strtok(old, "-");
        if (pvalue)
            sscanf(pvalue, "%d", &level);
    }

    return level;
}
static __inline__ char *
get_pu_id(const char *gu_id, char *pu_id)
{
    int index = 0;
    while (*gu_id != '\0')
    {
        if (*gu_id == '-')
        {
            if (++index>2)
            {
                *pu_id = '\0';
                break;
            }
        }
        *pu_id++ = *gu_id++;
    }

    return pu_id;
}

int get_device_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request          *request = NULL;
    DeviceInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DEVICE_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DeviceInfoPacket*)nmp_alloc0(sizeof(DeviceInfoPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_device_info(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, DEVICE_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DeviceInfoPacket), dealloc_msg_data);

    return RET_BACK;
}

int get_platform_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret;
    proxy_plt_t plt_info;

    Request            *request = NULL;
    PlatformInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_PLATFORM_INFO_ID != MSG_ID(msg));

    request = (Request*)MSG_DATA(msg);
    reponse = (PlatformInfoPacket*)nmp_alloc0(sizeof(PlatformInfoPacket));

    ret = proxy_get_device_private((proxy_device_t*)init_data, DEV_PRI_PLT_SRV, &plt_info);
    if (!ret)
    {
        strcpy(reponse->cms_ip, plt_info.cms_host);
        strcpy(reponse->mds_ip, plt_info.mds_host);
        reponse->cms_port = plt_info.cms_port;
        reponse->mds_port = plt_info.mds_port;
        reponse->protocol = plt_info.protocol;
    }

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, PLATFORM_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(PlatformInfoPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_platform_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, proto;
    proxy_plt_t plt_info;

    Result             *reponse = NULL;
    PlatformInfoPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_PLATFORM_INFO_ID != MSG_ID(msg));

    request = (PlatformInfoPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));

    ret = proxy_get_device_private((proxy_device_t*)init_data, DEV_PRI_PLT_SRV, &plt_info);
    if (!ret)
    {
        proto = plt_info.protocol;
        strcpy(plt_info.cms_host, request->cms_ip);
        strcpy(plt_info.mds_host, request->mds_ip);
        plt_info.cms_port = request->cms_port;
        plt_info.mds_port = request->mds_port;
        plt_info.protocol = request->protocol;
        
        ret = proxy_set_device_private((proxy_device_t*)init_data, 
                DEV_PRI_PLT_SRV, sizeof(plt_info), &plt_info);
        
        if (!ret && proto != request->protocol)
            ;//ÖØÆôÆ½Ì¨·þÎñ ----->todo
    }

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, PLATFORM_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_serial_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JSerialParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request               *request = NULL;
    SerialParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_SERIAL_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (SerialParameterPacket*)nmp_alloc0(sizeof(SerialParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);
    pvalue.serial_no = request->reserve;

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_serial_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, SERIAL_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(SerialParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_serial_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JSerialParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result                *reponse = NULL;
    SerialParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_SERIAL_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (SerialParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_serial_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    reponse->reserve = request->serial_no;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, SERIAL_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_device_time(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceTime pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request          *request = NULL;
    DeviceTimePacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DEVICE_TIME_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DeviceTimePacket*)nmp_alloc0(sizeof(DeviceTimePacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_device_time(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, DEVICE_TIME_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DeviceTimePacket), dealloc_msg_data);

    return RET_BACK;
}
int set_device_time(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceTime pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result           *reponse = NULL;
    DeviceTimePacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_DEVICE_TIME_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (DeviceTimePacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_device_time(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, DEVICE_TIME_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_ntp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceNTPInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request             *request = NULL;
    DeviceNTPInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DEVICE_NTP_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DeviceNTPInfoPacket*)nmp_alloc0(sizeof(DeviceNTPInfoPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_device_ntp_info(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, DEVICE_NTP_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DeviceNTPInfoPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_ntp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceNTPInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result              *reponse = NULL;
    DeviceNTPInfoPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_DEVICE_NTP_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (DeviceNTPInfoPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_device_ntp_info(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, DEVICE_NTP_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_network_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JNetworkInfo pvalue;
    proxy_config_t cfg;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request           *request = NULL;
    NetworkInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_NETWORK_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    memset(&cfg, 0, sizeof(proxy_config_t));
    
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (NetworkInfoPacket*)nmp_alloc0(sizeof(NetworkInfoPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_network_info(reponse, &pvalue, ACTION_PACK);

    get_proxy_config(&cfg);
    reponse->data_port = cfg.rtsp_port;

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, NETWORK_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(NetworkInfoPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_network_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JNetworkInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result            *reponse = NULL;
    NetworkInfoPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_NETWORK_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (NetworkInfoPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_network_info(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, NETWORK_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_pppoe_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JPPPOEInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request         *request = NULL;
    PPPOEInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_PPPOE_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (PPPOEInfoPacket*)nmp_alloc0(sizeof(PPPOEInfoPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_pppoe_info(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, PPPOE_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(PPPOEInfoPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_pppoe_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JPPPOEInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    PPPOEInfoPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_PPPOE_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (PPPOEInfoPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_pppoe_info(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, PPPOE_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_ftp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JFTPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request            *request = NULL;
    FTPParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_FTP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (FTPParameterPacket*)nmp_alloc0(sizeof(FTPParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_ftp_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, FTP_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(FTPParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_ftp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JFTPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result             *reponse = NULL;
    FTPParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_FTP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (FTPParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_ftp_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, FTP_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_smtp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JSMTPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request             *request = NULL;
    SMTPParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_SMTP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (SMTPParameterPacket*)nmp_alloc0(sizeof(SMTPParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_smtp_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, SMTP_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(SMTPParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_smtp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JSMTPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result              *reponse = NULL;
    SMTPParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_SMTP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (SMTPParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_smtp_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, SMTP_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_ddns_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDdnsConfig pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request          *request = NULL;
    DdnsConfigPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DDNS_CONFIG_REQUEST_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DdnsConfigPacket*)nmp_alloc0(sizeof(DdnsConfigPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_ddns_config(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, GET_DDNS_CONFIG_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DdnsConfigPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_ddns_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDdnsConfig pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result           *reponse = NULL;
    DdnsConfigPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_DDNS_CONFIG_REQUEST_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (DdnsConfigPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_ddns_config(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, SET_DDNS_CONFIG_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}
int get_upnp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JUPNPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request             *request = NULL;
    UPNPParameterPacket *reponse = NULL;

    BUG_ON(GET_UPNP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (UPNPParameterPacket*)nmp_alloc0(sizeof(UPNPParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_upnp_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, UPNP_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(UPNPParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_upnp_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JUPNPParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result              *reponse = NULL;
    UPNPParameterPacket *request = NULL;

    memset(&pvalue, 0, sizeof(JUPNPParameter));

    BUG_ON(SET_UPNP_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (UPNPParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_upnp_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, UPNP_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_disk_list(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDeviceDiskInfo pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request              *request = NULL;
    DeviceDiskInfoPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DEVICE_DISK_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DeviceDiskInfoPacket*)nmp_alloc0(sizeof(DeviceDiskInfoPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_device_disk_info(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info0(reponse, request);

    set_msg_id(msg, DEVICE_DISK_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DeviceDiskInfoPacket), dealloc_msg_data);

    return RET_BACK;
}
int format_disk(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JFormatDisk pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result           *reponse = NULL;
    FormatDiskPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(FORMAT_DISK_REQUEST_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (FormatDiskPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_format_disk(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, FORMAT_DISK_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int control_device(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JControlDevice pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result              *reponse = NULL;
    ControlDevicePacket *request = NULL;

    BUG_ON(CONTROL_DEVICE_REQUEST_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (ControlDevicePacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_control_device(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info1(reponse, request);

    set_msg_id(msg, CONTROL_DEVICE_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_encode_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JEncodeParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request               *request = NULL;
    EncodeParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_ENCODE_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (EncodeParameterPacket*)nmp_alloc0(sizeof(EncodeParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_encode_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, ENCODE_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(EncodeParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_encode_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JEncodeParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result                *reponse = NULL;
    EncodeParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_ENCODE_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (EncodeParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_encode_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, ENCODE_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_display_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDisplayParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request                *request = NULL;
    DisplayParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DISPLAY_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (DisplayParameterPacket*)nmp_alloc0(sizeof(DisplayParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_display_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, DISPLAY_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(DisplayParameterPacket), dealloc_msg_data);

    return RET_BACK;
}

int set_display_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JDisplayParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result                 *reponse = NULL;
    DisplayParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_DISPLAY_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (DisplayParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_display_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, DISPLAY_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_osd_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JOSDParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request            *request = NULL;
    OSDParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_OSD_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (OSDParameterPacket*)nmp_alloc0(sizeof(OSDParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_osd_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, OSD_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(OSDParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_osd_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JOSDParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result             *reponse = NULL;
    OSDParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_OSD_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (OSDParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_osd_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, OSD_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_ptz_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JPTZParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request            *request = NULL;
    PTZParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_PTZ_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (PTZParameterPacket*)nmp_alloc0(sizeof(PTZParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_ptz_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, PTZ_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(PTZParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_ptz_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JPTZParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result             *reponse = NULL;
    PTZParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_PTZ_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (PTZParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_ptz_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, PTZ_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_record_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JRecordParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request               *request = NULL;
    RecordParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_RECORD_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (RecordParameterPacket*)nmp_alloc0(sizeof(RecordParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_record_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, RECORD_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(RecordParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_record_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JRecordParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result                *reponse = NULL;
    RecordParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_RECORD_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (RecordParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_record_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, RECORD_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_hide_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JHideParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request             *request = NULL;
    HideParameterPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_HIDE_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (HideParameterPacket*)nmp_alloc0(sizeof(HideParameterPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_hide_parameter(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, HIDE_PARAMETER_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(HideParameterPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_hide_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JHideParameter pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result              *reponse = NULL;
    HideParameterPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_HIDE_PARAMETER_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (HideParameterPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_hide_parameter(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, HIDE_PARAMETER_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_move_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JMoveAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request         *request = NULL;
    MoveAlarmPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_MOVE_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (MoveAlarmPacket*)nmp_alloc0(sizeof(MoveAlarmPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_move_alarm(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, MOVE_ALARM_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(MoveAlarmPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_move_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JMoveAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    MoveAlarmPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_MOVE_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (MoveAlarmPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_move_alarm(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, MOVE_ALARM_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_video_lost_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JLostAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request         *request = NULL;
    LostAlarmPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_LOST_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (LostAlarmPacket*)nmp_alloc0(sizeof(LostAlarmPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_lost_alarm(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, LOST_ALARM_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(LostAlarmPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_video_lost_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JLostAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    LostAlarmPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_LOST_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (LostAlarmPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_lost_alarm(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, LOST_ALARM_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_hide_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JHideAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request         *request = NULL;
    HideAlarmPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_HIDE_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (HideAlarmPacket*)nmp_alloc0(sizeof(HideAlarmPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_hide_alarm(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, HIDE_ALARM_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(HideAlarmPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_hide_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JHideAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    HideAlarmPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_HIDE_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (HideAlarmPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_hide_alarm(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, HIDE_ALARM_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_io_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, type, channel;
    JIoAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request       *request = NULL;
    IoAlarmPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_IO_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (IoAlarmPacket*)nmp_alloc0(sizeof(IoAlarmPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    type = get_alarm_type(request->pu_or_gu_id);
    if (-1 == type)
        return RET_DROP;

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, type, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_io_alarm(reponse, &pvalue, ACTION_PACK);

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, IO_ALARM_INFO_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(IoAlarmPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_io_alarm_info(void *init_data, msg_t *msg, int parm_id)
{
    int ret, type, channel;
    JIoAlarm pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result        *reponse = NULL;
    IoAlarmPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_IO_ALARM_INFO_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (IoAlarmPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    type = get_alarm_type(request->gu_id);
    if (-1 == type)
        return RET_DROP;

    process_io_alarm(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, type, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, IO_ALARM_INFO_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_media_url(void *init_data, msg_t *msg, int parm_id)
{  
    int level, channel;
    char pu_id[J_SDK_MAX_ID_LEN];
    proxy_sdk_t sdk_info;

    MediaUrlPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_MEDIA_URL_REQUEST_ID != MSG_ID(msg));

    request = (MediaUrlPacket*)MSG_DATA(msg);

    if (*(request->gu_id) == '#' && *(request->gu_id+1) == '@')
        ;
    else
        get_pu_id(request->gu_id, pu_id);

    level = get_level(request->gu_id);
    channel = get_channel(request->gu_id);

    if (proxy_get_device_private((proxy_device_t*)init_data, DEV_PRI_PRX_SDK, &sdk_info))
    {
        BUG();
    }

    //rtsp£º//Éè±¸IP:Éè±¸¶Ë¿Ú/dev=Éè±¸ID(¼´PUID)/media=Ã½ÌåÀàÐÍ/channel=Í¨µÀºÅ&level=ÂëÁ÷ÀàÐÍ
    //"rtsp://192.168.1.217:7554/dev=JXJ-IPC-11111122/media=0/channel=0&level=0/recordType=1&startTime=123456789&endTime=987654321";

    local_ifs_t ifs;
    proxy_config_t cfg;
    char srv_addr[MAX_IP_LEN] = {0};

    get_proxy_config(&cfg);
    if (!strcmp(cfg.host, "127.0.0.1"))
    {
        int i, cu_ip = 0;
        int addr, flag = 0;
        proxy_get_local_ip(&ifs);

        inet_pton(AF_INET, request->cu_ip, &cu_ip);
        cu_ip = htonl(cu_ip);

        for (i=0; i<ifs.count; i++)
        {
            if ((ifs.ifa[i].addr & 0xff000000) == (cu_ip & 0xff000000))
            {
                ifs.ifa[i].match++;
                if ((ifs.ifa[i].addr & 0x00ff0000) == (cu_ip & 0x00ff0000))
                {
                    ifs.ifa[i].match++;
                    if ((ifs.ifa[i].addr & 0x0000ff00) == (cu_ip & 0x0000ff00))
                    {
                        ifs.ifa[i].match++;
                    }
                }
            }
        }

        for (i=0; i<ifs.count; i++)
        {
            if (ifs.ifa[i].match > flag)
            {
                flag = ifs.ifa[i].match;
                addr = ifs.ifa[i].addr;
            }
        }

        addr = htonl(addr);
        inet_ntop(AF_INET, (void*)&addr, srv_addr, sizeof(srv_addr));
    }
    else
        strncpy(srv_addr, cfg.host, sizeof(srv_addr)-1);

    if (strlen(pu_id))
    {
        snprintf(request->url, sizeof(request->url), 
            "rtsp://%s:%d/dev=%s/media=%d/channel=%d&level=%d", 
            srv_addr, cfg.rtsp_port, 
            pu_id, request->media_type, channel, level);
    }
    else
    {
        snprintf(request->url, sizeof(request->url), 
            "rtsp://%s:%d/dev=%s/media=%d/channel=%d&level=%d", 
            srv_addr, cfg.rtsp_port, 
            "xxxxxxxxxxxxx", request->media_type, channel, level);
    }
    printf("\nurl: %s\n\n", request->url);

    request->result.code = 0;
    set_msg_id(msg, GET_MEDIA_URL_RESPONSE_ID);

    return RET_BACK;
}

int get_store_log(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JStoreLog pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    StoreLogPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_STORE_LOG_REQUEST_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (StoreLogPacket*)MSG_DATA(msg);
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_store_log(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_GET_SUCCESS;

    if (!ret)
        process_store_log(request, &pvalue, ACTION_PACK);

    request->result.code = ret;
    set_msg_id(msg, GET_STORE_LOG_RESPONSE_ID);

    return RET_BACK;
}

int ptz_control(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    JPTZControl pvalue;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result           *reponse = NULL;
    PTZControlPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(CONTROL_PTZ_COMMAND_ID != MSG_ID(msg));

    memset(&pvalue, 0, sizeof(pvalue));
    prx_dev = (proxy_device_t*)init_data;
    request = (PTZControlPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    process_control_ptz_cmd(request, &pvalue, ACTION_UNPACK);
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &pvalue);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, PTZ_COMMAND_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_preset_set(void *init_data, msg_t *msg, int parm_id)
{
    int i;
    JPresetPoint *pp;

    Request     *request = NULL;
    PPSetPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_PRESET_POINT_SET_REQUEST_ID != MSG_ID(msg));

    request = (Request*)MSG_DATA(msg);
    reponse = (PPSetPacket*)nmp_alloc0(sizeof(PPSetPacket));

    reponse->pp_set.pp_count = J_SDK_MAX_PRESET_PORT_SIZE;
    for (i=1; i<=J_SDK_MAX_PRESET_PORT_SIZE; i++)
    {
        pp = &reponse->pp_set.pp[i-1];
        sprintf((char*)pp->name, "é¢„ç½®ç‚¹ %d", i);
        pp->preset = i;
    }

    reponse->result.code = 0;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, GET_PRESET_POINT_SET_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(PPSetPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_preset_point(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result         *reponse = NULL;
    PPConfigPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_PRESET_POINT_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (PPConfigPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &request->pp_cfg);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, SET_PRESET_POINT_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}

int get_cruise_set(void *init_data, msg_t *msg, int parm_id)
{
    int i;
    JCruiseInfo *crz_info;

    Request            *request = NULL;
    CruiseWaySetPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_CRUISE_WAY_SET_REQUEST_ID != MSG_ID(msg));

    request = (Request*)MSG_DATA(msg);
    reponse = (CruiseWaySetPacket*)nmp_alloc0(sizeof(CruiseWaySetPacket));

    reponse->crz_set.crz_count = J_SDK_MAX_CRUISE_WAY_SIZE;
    for (i=1; i<=J_SDK_MAX_CRUISE_WAY_SIZE; i++)
    {
        crz_info = &reponse->crz_set.crz_info[i-1];
        sprintf((char*)crz_info->crz_name, "å·¡èˆªè·¯å¾„ %d", i);
        crz_info->crz_no = i;
    }

    reponse->result.code = 0;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, GET_CRUISE_WAY_SET_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(CruiseWaySetPacket), dealloc_msg_data);

    return RET_BACK;
}
int get_cruise_way(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Request         *request = NULL;
    CruiseWayPacket *reponse = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_CRUISE_WAY_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (Request*)MSG_DATA(msg);
    reponse = (CruiseWayPacket*)nmp_alloc0(sizeof(CruiseWayPacket));
    channel = get_channel(request->pu_or_gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    reponse->crz_way.crz_info.crz_no = request->reserve;
    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &reponse->crz_way);
    else
        ret = DEF_GET_SUCCESS;

    reponse->result.code = ret;
    strcpy_connect_info2(reponse, request);

    set_msg_id(msg, GET_CRUISE_WAY_RESPONSE_ID);
    set_msg_data_2(msg, reponse, sizeof(CruiseWayPacket), dealloc_msg_data);

    return RET_BACK;
}
int set_cruise_way(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result             *reponse = NULL;
    CruiseConfigPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_CRUISE_WAY_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (CruiseConfigPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &request->crz_cfg);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, SET_CRUISE_WAY_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}
int add_cruise_way(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    CruiseWayPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(ADD_CRUISE_WAY_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (CruiseWayPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &request->crz_way);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    reponse->reserve = request->crz_way.crz_info.crz_no;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, ADD_CRUISE_WAY_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}
int modify_cruise_way(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    Result          *reponse = NULL;
    CruiseWayPacket *request = NULL;

    NMP_ASSERT(init_data && msg);

    BUG_ON(MODIFY_CRUISE_WAY_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (CruiseWayPacket*)MSG_DATA(msg);
    reponse = (Result*)nmp_alloc0(sizeof(Result));
    channel = get_channel(request->gu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    if (handler[parm_id].handler)
        ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &request->crz_way);
    else
        ret = DEF_SET_FAILURE;

    reponse->result.code = ret;
    strcpy_connect_info3(reponse, request);

    set_msg_id(msg, MODIFY_CRUISE_WAY_RESULT_ID);
    set_msg_data_2(msg, reponse, sizeof(Result), dealloc_msg_data);

    return RET_BACK;
}










//###########################################################################
int transparent_get_param(void *init_data, msg_t *msg, int parm_id)
{
    int ret, channel;
    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    int esize=0, dsize=0;
    int e_len, d_len = 0;
    void *ecode = NULL;
    void *decode = NULL;

    void              *reponse = NULL;
    TransparentPacket *request = NULL;

    BUG_ON(GET_TRANSPARENTPARAM_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (TransparentPacket*)MSG_DATA(msg);
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    printf("parm_id: %d\n", parm_id);
    printf("handler.id: %d\n", handler[parm_id].id);

    BUG_ON(handler[parm_id].id != parm_id);

    printf("type   : %d\n", request->type);
    printf("channel: %d\n", request->channel);
    printf("length : %d\n", request->length);
    printf("data   : %s\n", (char*)request->data);

    if (request->data)
    {
        d_len = base64_decode((unsigned char*)request->data, 
                (unsigned int)request->length, 
                (unsigned char**)&decode, 
                (unsigned int*)&dsize);
    printf("base64_decode size     : %d<--------------000000000\n", dsize);
    }

    if (0 < d_len)
    {
        JSDKType sdk_type;
        memset(&sdk_type, 0, sizeof(JSDKType));
        
        if (dsize > (int)sizeof(JSDKType))
            memcpy(&sdk_type, decode, sizeof(JSDKType));
        else
            memcpy(&sdk_type, decode, dsize);
        
show_debug( "start get_device_parm(type:%d, channel:%d, size:%d)\n", request->type, request->channel, dsize);
        if (handler[parm_id].handler)
            ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &sdk_type);
        else
            ret = DEF_GET_SUCCESS;
show_debug( "get reponse: %d\n", ret);
        if (!ret)
        {
            if (dsize > (int)sizeof(JSDKType))
                memcpy(decode, &sdk_type, sizeof(JSDKType));
            else
                memcpy(decode, &sdk_type, dsize);

            e_len = base64_encode((unsigned char*)decode, 
                    (unsigned int)dsize, 
                    (unsigned char**)&ecode, 
                    (unsigned int*)&esize);
            if (0 < e_len)
            {
    printf("\n--->length : %d\n", esize);
    printf("--->data   : %s\n", (char*)ecode);
                reponse = nmp_alloc0(sizeof(TransparentPacket)+(esize+1));
                
                memcpy(reponse, request, sizeof(TransparentPacket));
                ((TransparentPacket*)reponse)->data = (char*)reponse+sizeof(TransparentPacket);
                memcpy((char*)reponse+sizeof(TransparentPacket), ecode, esize);
                
                set_msg_data_2(msg, reponse, sizeof(TransparentPacket)+(esize+1), dealloc_msg_data);
                
                base64_free(ecode, e_len);
            }
            else
            {
                show_debug("base64_encode failure!!!!!!!!\n");
            }
        }
        
        base64_free(decode, d_len);
    }
    else
    {
        request->type = -1;
        request->channel = -1;
    }

    set_msg_id(msg, GET_TRANSPARENTPARAM_RESPONSE_ID);

    return RET_BACK;
}
int transparent_set_param(void *init_data, msg_t *msg, int parm_id)
{
return RET_DROP;//Nonsupport!!!
    int channel, len = 0, ret = -1;
    int dsize = 0;
    void *decode = NULL;

    proxy_device_t  *prx_dev;
    handler_table_t *handler;

    TransparentPacket *request = NULL;

    BUG_ON(SET_TRANSPARENTPARAM_REQUEST_ID != MSG_ID(msg));

    prx_dev = (proxy_device_t*)init_data;
    request = (TransparentPacket*)MSG_DATA(msg);
    channel = get_channel(request->pu_id);
    handler = get_sdk_msg_handler(prx_dev->sdk_srv);

    BUG_ON(handler[parm_id].id != parm_id);

    printf("type   : %d\n", request->type);
    printf("channel: %d\n", request->channel);
    printf("length : %d\n", request->length);
    printf("data   : %p\n", (char*)request->data);

    if (request->data)
    {
        len = base64_decode((unsigned char*)request->data, 
                (unsigned int)request->length, 
                (unsigned char**)&decode, 
                (unsigned int*)&dsize);
    }

    printf("\n----->length : %d\n", dsize);
    printf("----->data   : %p\n", (char*)decode);

    if (0 < len && decode && 0 < dsize)
    {
        JSDKType sdk_type;
        memset(&sdk_type, 0, sizeof(JSDKType));

        /*ÉèÖÃÖ®Ç°ÏÈ»ñµÃÉè±¸Ô­ÓÐµÄÊý¾Ý£¬±ÜÃâÎ²°Í¹éÁã*/
    //          proxy_get_device_parm((proxy_device_t*)init_data, parm_id, &sdk_type);

        if (handler[parm_id].handler)
            ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &sdk_type);
        else
            ret = DEF_GET_SUCCESS;

        if (dsize > (int)sizeof(JSDKType))
            memcpy(&sdk_type, decode, sizeof(JSDKType));
        else
            memcpy(&sdk_type, decode, dsize);

show_debug( "start set_device_parm(type:%d, channel:%d, size:%d)", request->type, request->channel, dsize);
        if (handler[parm_id].handler)
            ret = (*handler[parm_id].handler)(prx_dev->sdk_srv, channel, parm_id, &sdk_type);
        else
            ret = DEF_SET_FAILURE;
show_debug( "set reponse: %d", ret);

        base64_free(decode, len);
    }

    set_msg_id(msg, SET_TRANSPARENTPARAM_RESPONSE_ID);
    set_msg_data(msg, &ret, sizeof(int));

    return RET_BACK;
}


