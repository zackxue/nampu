
#include "nmp_proxy_log.h"
#include "nmp_proxy_sdk.h"


#define DEF_HIK_SDK_SERVICE_NAME        "Hik Service SDK"
#define DEF_DAH_SDK_SERVICE_NAME        "Dah Service SDK"
#define DEF_HBN_SDK_SERVICE_NAME        "Hbn Service SDK"
#define DEF_BSM_SDK_SERVICE_NAME        "Bsm Service SDK"
#define DEF_JNY_SDK_SERVICE_NAME        "Jny Service SDK"
#define DEF_HIE_SDK_SERVICE_NAME        "Hie Service SDK"
#define DEF_XMT_SDK_SERVICE_NAME        "XMT Service SDK"
#define DEF_TPS_SDK_SERVICE_NAME        "TPS Service SDK"

#define DEF_HIK_FACTORY_NAME            "Hikvision"
#define DEF_DAH_FACTORY_NAME            "Dahua"
#define DEF_HBN_FACTORY_NAME            "Hbgk"
#define DEF_BSM_FACTORY_NAME            "Bosiming"
#define DEF_JNY_FACTORY_NAME            "Sunell"
#define DEF_HIE_FACTORY_NAME            "HieYi"
#define DEF_XMT_FACTORY_NAME            "XiongMai"
#define DEF_TPS_FACTORY_NAME            "TopSee"



sdk_item_t g_sdk_items[MAX_SDK] = 
{
    { SDK_HIK, DEF_HIK_SDK_SERVICE_NAME, DEF_HIK_FACTORY_NAME },
    { SDK_DAH, DEF_DAH_SDK_SERVICE_NAME, DEF_DAH_FACTORY_NAME },
    { SDK_HBN, DEF_HBN_SDK_SERVICE_NAME, DEF_HBN_FACTORY_NAME },
    { SDK_BSM, DEF_BSM_SDK_SERVICE_NAME, DEF_BSM_FACTORY_NAME },
    { SDK_JNY, DEF_JNY_SDK_SERVICE_NAME, DEF_JNY_FACTORY_NAME },
    { SDK_HIE, DEF_HIE_SDK_SERVICE_NAME, DEF_HIE_FACTORY_NAME },
    { SDK_XMT, DEF_XMT_SDK_SERVICE_NAME, DEF_XMT_FACTORY_NAME },
    { SDK_TPS, DEF_TPS_SDK_SERVICE_NAME, DEF_TPS_FACTORY_NAME },
};


handler_table_t *
get_sdk_msg_handler(struct service *srv)
{
    int handler = 0;
    NMP_ASSERT(srv);

    (*srv->tm->control_service)(srv->tm, srv, CTRL_CMD_OBTAIN_HANDLER, &handler);

    return (handler_table_t*)handler;
}

stream_operation_t *
get_sdk_stream_opt(struct service *srv)
{
    int opt_addr = 0;
    NMP_ASSERT(srv);

    (*srv->tm->control_service)(srv->tm, srv, CTRL_CMD_OBTAIN_STRM_OPT, &opt_addr);

    return (stream_operation_t*)opt_addr;
}

talk_opt_t *
get_sdk_talk_opt(struct service *srv)
{
    int opt_addr = 0;
    NMP_ASSERT(srv);

    (*srv->tm->control_service)(srv->tm, srv, CTRL_CMD_OBTAIN_TALK_OPT, &opt_addr);

    return (talk_opt_t*)opt_addr;
}

