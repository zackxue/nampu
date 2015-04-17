
#include "stream_api.h"
#include "rtsp-server.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_sdk.h"
#include "nmp_proxy_device.h"
#include "nmp_stream_operation.h"



static __inline__ unsigned long
swap_second_time(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
    unsigned int mon = mon0, year = year0;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int)(mon -= 2)) 
    {
        mon += 12; /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long)
        (year/4 - year/100 + year/400 + 367*mon/12 + day) +
            year*365 - 719499
            )*24 + hour /* now have hours */
         )*60 + min /* now have minutes */
    )*60 + sec; /* finally seconds */
}

static __inline__ time_t 
swap_local_time(JTime *ts)
{
    return swap_second_time(ts->year, ts->month, ts->date, 
            ts->hour, ts->minute, ts->second);
}

static __inline__ int 
get_private_time(const char *pri, const char *head, JTime *ts)
{
    char *ch;
    int y, m, d, h, n, s;
    NMP_ASSERT(pri);

    ch = strstr((char*)pri, head);
    if (ch)
    {
        ch += strlen(head);
        sscanf(ch, "%4d%2d%2d%2d%2d%2d", &y, &m, &d, &h, &n, &s);

        if (y > J_SDK_DEF_BASE_YEAR)
            y -= J_SDK_DEF_BASE_YEAR;

        ts->year   = y;
        ts->month  = m;
        ts->date   = d;
        ts->hour   = h;
        ts->minute = n;
        ts->second = s;

        return 0;
    }

    return -1;
}

int get_start_time(const char *pri, JTime *ts)
{
    return get_private_time(pri, (const char*)DEF_PRI_START_TIME, ts);
}

int get_end_time(const char *pri, JTime *ts)
{
    return get_private_time(pri, (const char*)DEF_PRI_END_TIME, ts);
}

double get_play_length(JTime *stop_ts, JTime *start_ts)
{
    time_t start, end;

    start = swap_local_time(start_ts);
    end   = swap_local_time(stop_ts);

    return end - start;
}


stream_list_t *
add_one_stream(stream_list_t *strm_list, stream_info_t *strm_info)
{
    NMP_ASSERT(strm_list && strm_info);

    //nmp_mutex_lock(strm_list->lock);
    strm_list->list = nmp_list_add_tail(strm_list->list, strm_info);
    //nmp_mutex_unlock(strm_list->lock);

    return strm_list;
}

stream_list_t *
remove_one_stream(stream_list_t *strm_list, stream_info_t *strm_info)
{
    NMP_ASSERT(strm_list && strm_info);

    //nmp_mutex_lock(strm_list->lock);
    strm_list->list = nmp_list_remove(strm_list->list, strm_info);
    //nmp_mutex_unlock(strm_list->lock);

    return strm_list;
}

void destory_stream_list(stream_list_t *strm_list, 
        stop_stream_cb stop_strm_cb, size_t size)
{
    nmp_list_t *list;
    stream_info_t *strm_info;

    NMP_ASSERT(strm_list && stop_strm_cb);

    //nmp_mutex_lock(strm_list->lock);
    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_info = (stream_info_t*)nmp_list_data(list);
        if (strm_info)
        {
            list = nmp_list_remove(list, strm_info);  //返回值已经是下一节点的地址了

            set_stream_user_data((gpointer)strm_info->opened, (void*)NULL);
            (*stop_strm_cb)(strm_info);

            memset(strm_info, 0, size);
            nmp_dealloc(strm_info, size);
        }
    }
    strm_list->list = NULL;
    //nmp_mutex_unlock(strm_list->lock);

    return ;
}


stream_info_t *
find_stream_by_handle(stream_list_t *strm_list, int handle)
{
    nmp_list_t *list;
    stream_info_t *strm_node;
    stream_info_t *strm_info = NULL;

    NMP_ASSERT(strm_list);

    //nmp_mutex_lock(strm_list->lock);
    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_node = (stream_info_t*)nmp_list_data(list);
        if (handle == strm_node->handle)
        {
            strm_info = strm_node;
            break;
        }

        list = nmp_list_next(list);
    }
    //nmp_mutex_unlock(strm_list->lock);

    return strm_info;
}

stream_info_t *
find_stream_by_opened(stream_list_t *strm_list, void *opened)
{
    nmp_list_t *list;
    stream_info_t *strm_node;
    stream_info_t *strm_info = NULL;

    NMP_ASSERT(strm_list);

    //nmp_mutex_lock(strm_list->lock);
    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_node = (stream_info_t*)nmp_list_data(list);
        if (opened == strm_node->opened)
        {
            strm_info = strm_node;
            break;
        }
        list = nmp_list_next(list);
    }
    //nmp_mutex_unlock(strm_list->lock);

    return strm_info;
}

stream_info_t *
find_stream_by_channel(stream_list_t *strm_list, int channel)
{
    nmp_list_t *list;
    stream_info_t *strm_node;
    stream_info_t *strm_info = NULL;

    NMP_ASSERT(strm_list);

    //nmp_mutex_lock(strm_list->lock);
    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_node = (stream_info_t*)nmp_list_data(list);
        if (channel == strm_node->channel)
        {
            strm_info = strm_node;
            break;
        }

        list = nmp_list_next(list);
    }
    //nmp_mutex_unlock(strm_list->lock);

    return strm_info;
}

stream_info_t *
find_stream_by_channel_and_level(stream_list_t *strm_list, 
    int channel, int level)
{
    nmp_list_t *list;
    stream_info_t *strm_node;
    stream_info_t *strm_info = NULL;

    NMP_ASSERT(strm_list);

    //nmp_mutex_lock(strm_list->lock);
    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_node = (stream_info_t*)nmp_list_data(list);
        if ((channel == strm_node->channel) && 
            (level == strm_node->level))
        {
            strm_info = strm_node;
            break;
        }

        list = nmp_list_next(list);
    }
    //nmp_mutex_unlock(strm_list->lock);

    return strm_info;
}

//////////////////////////////////////////////////////////////////////////////////////////
static gint 
stream_init(gchar *pu_id, gint type, gint channel, gint level, 
    gchar *pri, gchar **media_info, gsize *size, gdouble *length)
{
    show_debug("Enter stream_init--->pu_id: %s, channel: %d, level: %d, type: %d\n", 
        pu_id, channel, level, type);
    int ret = -1;
    int media_size;
    proxy_device_t *dev;
    stream_operation_t *strm_opt;
    stream_info_t strm_info;

    NMP_ASSERT(pu_id && pri && 0 <= channel);

    dev = proxy_find_device_by_pu_id((char*)pu_id);
    if (dev)
    {
        strm_opt = get_sdk_stream_opt(dev->sdk_srv);
        if (strm_opt)
        {
            memset(&strm_info, 0, sizeof(stream_info_t));
            strm_info.type     = type;
            strm_info.level    = level;
            strm_info.channel  = channel;
            strm_info.pri_data = pri;

            ret = (*strm_opt->stream_init)(dev->sdk_srv, &strm_info);
            if (!ret)
            {
                *size = strm_info.media_size;
                *media_info = (gchar*)g_new0(char, sizeof(int) + *size);

                media_size = htonl(strm_info.media_size);
                memcpy(*media_info, &media_size, sizeof(int));
                memcpy(*media_info + sizeof(int), strm_info.media_info, 
                    strm_info.media_size);

                *length = (gdouble)strm_info.length;
            }
        }

        proxy_device_unref(dev);
    }
    else
    {
        show_warn("device is not exist. \n");
    }

    show_debug("Left  stream_init--->pu_id: %s, type: %d, ret: %d\n", 
        pu_id, type, ret);
    return ret;
}
static gint 
stream_open(gpointer stream, gchar *pu_id, gint type, 
    gint channel, gint level, gchar *pri)
{
    show_debug("Enter stream_open--->stream: %p, channel: %d, type: %d\n", 
        stream, channel, type);
    int ret = -1;
    proxy_device_t *dev;
    stream_operation_t *strm_opt;
    stream_info_t strm_info;

    NMP_ASSERT(stream && 0 <= channel);

    dev = proxy_find_device_by_pu_id((char*)pu_id);
    if (dev)
    {
        strm_opt = get_sdk_stream_opt(dev->sdk_srv);
        if (strm_opt)
        {
            memset(&strm_info, 0, sizeof(stream_info_t));
            strm_info.type     = type;
            strm_info.level    = level;
            strm_info.channel  = channel;
            strm_info.pri_data = pri;
            strm_info.opened   = stream;

            ret = (*strm_opt->stream_open)(dev->sdk_srv, &strm_info);
        }
        proxy_device_unref(dev);
    }
    else
    {
        show_warn("device is not exist. \n");
    }

    show_debug("Left  stream_open--->stream: %p, type: %d, ret: %d\n", 
        stream, type, ret);
    return ret;
}
static void 
stream_play(gpointer stream)
{
    show_debug("Enter stream_play--->stream: %p\n", stream);
    int channel = -1;
    struct service *sdk_srv;
    stream_operation_t *strm_opt;
    stream_info_t *strm_info;

    NMP_ASSERT(stream);

    strm_info = (stream_info_t*)get_stream_user_data(stream);
    if (strm_info)
    {
        channel = strm_info->channel;
        sdk_srv = strm_info->sdk_srv;
        strm_opt = get_sdk_stream_opt(sdk_srv);
        if (strm_opt)
        {
            (*strm_opt->stream_play)(sdk_srv, strm_info);
        }
    }

    show_debug("Left  stream_play--->stream: %p, channel: %d\n", 
        stream, channel);
    return ;
}
static void 
stream_pause(gpointer stream)
{
    show_debug("Enter stream_pause--->stream: %p\n", stream);
    struct service *sdk_srv;
    stream_operation_t *strm_opt;
    stream_info_t *strm_info;

    NMP_ASSERT(stream);

    strm_info = (stream_info_t*)get_stream_user_data(stream);
    if (strm_info)
    {
        sdk_srv = strm_info->sdk_srv;
        strm_opt = get_sdk_stream_opt(sdk_srv);
        if (strm_opt)
        {
            (*strm_opt->stream_pause)(sdk_srv, strm_info);
        }
    }

    show_debug("Left  stream_pause--->stream: %p\n", stream);
    return ;
}
static void 
stream_close(gpointer stream)
{
    show_debug("Enter stream_close--->stream: %p\n", stream);
    int channel = -1;
    struct service *sdk_srv;
    stream_operation_t *strm_opt;
    stream_info_t *strm_info;

    NMP_ASSERT(stream);

    strm_info = (stream_info_t*)get_stream_user_data(stream);
    if (strm_info)
    {
        channel = strm_info->channel;
        sdk_srv = strm_info->sdk_srv;
        strm_opt = get_sdk_stream_opt(sdk_srv);
        if (strm_opt)
        {
            set_stream_user_data(stream, (void*)NULL);
            (*strm_opt->stream_close)(sdk_srv, strm_info);
        }
    }

    show_debug("Left  stream_close--->stream: %p, channel: %d\n", stream, channel);
    return ; 
}
static gint 
stream_seek(gpointer stream, gdouble ts)
{
    show_debug("Enter stream_seek--->stream: %p\n", stream);
    stream_info_t *strm_info;

    NMP_ASSERT(stream);

    strm_info = (stream_info_t*)get_stream_user_data(stream);
    if (strm_info)
    {
        strm_info->ts = (unsigned long)ts;
    }

    show_debug("Left  stream_seek--->stream: %p\n", stream);
    return 0;
}
static gint 
stream_ctrl(gchar *pu_id, gint channel, gint level, gint cmd, void *value)
{
    show_debug("stream_ctrl--->pu_id: %s\n", pu_id);
    int ret = -1;
    proxy_device_t *dev;
    stream_operation_t *strm_opt;

    NMP_ASSERT(pu_id && 0 <= channel);

    dev = proxy_find_device_by_pu_id((char*)pu_id);
    if (dev)
    {
        strm_opt = get_sdk_stream_opt(dev->sdk_srv);
        if (strm_opt)
        {
            ret = (*strm_opt->stream_ctrl)(dev->sdk_srv, 
                    channel, level, cmd, value);
        }
        proxy_device_unref(dev);
    }
    else
    {
        show_warn("device is not exist. \n");
    }

    show_debug("Left  stream_ctrl--->pu_id: %s, ret: %d\n", pu_id, ret);
    return ret;
}

Stream_operation strm_api =
{
    stream_init,
    stream_open,
    stream_play,
    stream_pause,
    stream_close, 
    stream_seek,
    stream_ctrl 
};

