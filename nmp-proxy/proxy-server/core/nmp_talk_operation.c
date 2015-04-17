
#include "nmp_proxy_log.h"
#include "nmp_proxy_sdk.h"
#include "nmp_proxy_device.h"
#include "nmp_talk_operation.h"


static int free_talk_info(talk_info_t *talk_info)
{
    if(talk_info == NULL)
        return 1;
    if(talk_info->frm)
		nmp_dealloc(talk_info->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);

	nmp_dealloc(talk_info, sizeof(talk_info_t));

    return 0;
}

static int talk_open(talk_handle_t *hdl, const char* device, const int channel, media_info_t *info)
{
    show_debug("enter talk_open--->pu_id: %s,------>channel: %d\n", device, channel);
    int ret = -1;
    proxy_device_t *dev;
    talk_opt_t *talk_opt;
    talk_info_t *talk_info = NULL;

    NMP_ASSERT(device && 0 <= channel);

    talk_info = (talk_info_t*)nmp_alloc0(sizeof(talk_info_t));
    if(talk_info == NULL)
    {
        return -1;
    }

    dev = proxy_find_device_by_pu_id((char*)device);
    if (dev)
    {
        talk_info->channel  = channel;
        talk_info->frm = NULL;
        talk_info->hdl = hdl;
        talk_info->sdk_srv = dev->sdk_srv;
        memcpy(talk_info->media_info, (char*)info, sizeof(media_info_t));

        talk_opt = get_sdk_talk_opt(talk_info->sdk_srv);
        if (talk_opt)
        {
            ret = talk_opt->talk_open(talk_info->sdk_srv, talk_info, hdl, info);
            if (ret < 0)
            {   
                free_talk_info(talk_info);
                show_warn("talk open err");
            }
            else
            {
                set_user_data(hdl, (void*)talk_info);
            }
        }

        proxy_device_unref(dev);
    }
    else
    {
        free_talk_info(talk_info);
        show_warn("device is not exist. \n");
    }

    return ret;
}

static int talk_close(talk_handle_t *hdl)
{show_debug("enter ---------->talk_close\n");
    int ret = -1;
    talk_opt_t *talk_opt;
    talk_info_t *talk_info = NULL;

    NMP_ASSERT(hdl);

    talk_info = (talk_info_t*)get_user_data(hdl);
    if(talk_info == NULL)
    {
        show_warn("get_user_data\n");
        return -1;
    }

    if (talk_info->sdk_srv)
    {
        talk_opt = get_sdk_talk_opt(talk_info->sdk_srv);
        if (talk_opt)
        {
            set_user_data(hdl, (void*)NULL);
            ret = (*talk_opt->talk_close)(talk_info->sdk_srv, talk_info);   
            if (ret < 0)
            {
                show_warn("talk close error\n");
            }
        }
    }
    else
    {
        show_warn("device is not exist. \n");
    }
    free_talk_info(talk_info);

    return ret;
}

static int talk_recv(talk_handle_t *hdl, char *data, int len)
{
    int ret = -1;
    media_info_t *info;
    talk_opt_t *talk_opt;
    talk_info_t *talk_info;

    NMP_ASSERT(hdl && data);

    talk_info = (talk_info_t*)get_user_data(hdl);
    if(talk_info == NULL)
    {
        show_warn("get_user_data\n");
        return -1;
    }

    info = (media_info_t*)talk_info->media_info;
    if (talk_info->sdk_srv)
    {
        talk_opt = get_sdk_talk_opt(talk_info->sdk_srv);
        if (talk_opt)
        {
            ret = (*talk_opt->talk_recv)(talk_info->sdk_srv, 
                                         talk_info, data, len, info);
            if (ret < 0)
            {
                show_warn("talk recv error\n");
            }
        }
    }
    else
    {
        show_warn("device is not exist. \n");
    }

    return ret;
}

struct _talk_ops talk_api = 
{
    talk_open,
    talk_close,
    talk_recv
};


