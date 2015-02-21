#include <unistd.h>
#include "rtsp_impl.h"
#include "tr_log.h"
#include "rtsp_def.h"
#include "rtsp_parse.h"
#include "rtsp_parser.h"
#include "alloc.h"
#include "media.h"
#include "version.h"
#include "rtsp_url2.h"
#include "rtsp_methods.h"

#define MAX_RTSP_BUF_SIZE       4096
#define MAX_SDP_SIZE            4096
#define DEFAULT_TTL             32
#define MAX_ATTR_LEN            1024

#define TRACK_INDICATOR         "trackID="
#define TRACK_SUFFIX            "/"TRACK_INDICATOR              


static int32_t
rtsp_impl_session_init(session *s, void *u)
{
    uint32_t media_seq = (uint32_t)u;

    snprintf(s->id, SESS_MAX_LEN, "%08d", media_seq);
    return 0;
}


static session_ops rtsp_impl_session_ops =
{
    .init = rtsp_impl_session_init
};


static session *
rtsp_impl_create_session(rtsp_client *rc, void *p)
{
    return session_alloc(sizeof(session), &rtsp_impl_session_ops, p);
}


static __inline__ rtsp_message *
msg_to_rtsp_message(msg *m)
{
    rtsp_message *rm;
    msg *_m = GET_MSG_ADDR(m);
    BUG_ON(_m == m);
    rm = (rtsp_message*)_m;
    return rm;
}


static uint32_t
rtsp_impl_recognize(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;
    RTSP_MSG_TYPE mt;
    RTSP_METHOD method;
    uint32_t rtsp = TR_RTSP_UNKNOWN;

    rm = msg_to_rtsp_message(req);
    mt = rtsp_message_get_type(rm);

    if (mt == RTSP_MESSAGE_DATA)
        return TR_RTSP_DATA;

    if (mt != RTSP_MESSAGE_REQUEST)
        return TR_RTSP_RESPONSE;

    rtsp_message_parse_request(rm, &method, NULL, NULL, NULL);
    switch (method)
    {
    case RTSP_DESCRIBE:
        rtsp = TR_RTSP_DESC;
        break;

    case RTSP_OPTIONS:
        rtsp = TR_RTSP_OPTION;
        break;

    case RTSP_SETUP:
        rtsp = TR_RTSP_SETUP;
        break;
    
    case RTSP_PLAY:
        rtsp = TR_RTSP_PLAY;
        break;

    case RTSP_PAUSE:
        rtsp = TR_RTSP_PAUSE;
        break;
    
    case RTSP_TEARDOWN:
        rtsp = TR_RTSP_TEARDOWN;
        break;

    case RTSP_SET_PARAMETER:
        rtsp = TR_RTSP_SETPARM;
        break;

    case RTSP_GET_PARAMETER:
        rtsp = TR_RTSP_GETPARM;
        break;

    default:
        break;
    }
    return rtsp;
}


static __inline__ msg *
rtsp_message_to_msg(rtsp_message *rm)
{
    return SET_MSG_TYPE(rm, RTSP_MSG_MT);
}


rtsp_wait_block *
rtsp_wait_block_new( void )
{
    return tr_alloc0(sizeof(rtsp_wait_block));
}


void
rtsp_wait_block_free(rtsp_wait_block *rwb)
{
    if (rwb->rc)
    {
        client_unref((client*)rwb->rc);
    }

    if (rwb->orig_media)
    {
        media_unref(rwb->orig_media);
    }

    if (rwb->req_msg)
    {
        rtsp_message_free(rwb->req_msg);
    }

    tr_free(rwb, sizeof(*rwb));
}


static int32_t
ls_url_equal(media_uri *url_1, media_uri *url_2)
{//url_1: &media->uri
    return url_1->mrl_type == url_2->mrl_type &&
        url_1->mrl_ind1 == url_2->mrl_ind1 &&
        url_1->mrl_ind2 == url_2->mrl_ind2;
}


static int32_t
hs_url_equal(media_uri *url_1, media_uri *url_2)
{//url_1: &media->uri
    if (url_1->mrl_type != url_2->mrl_type)
        return 0;
    if (url_1->mrl_ind3 != url_2->mrl_ind3)
        return 0;
    if (url_1->mrl_ind4 != url_2->mrl_ind4)
        return 0;
    return 1;
}


static __inline__ int32_t
rtsp_impl_special_parse(rtsp_client *rc, media_uri *mrl, int32_t *track,
    int32_t media_seq)
{//简单分析下, media_seq用于回放或下载时查找media.
    char *ptr;
    int32_t m_type, channel, level;
    static int32_t hs_seq = 0;

    ptr = strstr(__str(mrl->mrl), "/media=");
    if (!ptr)
    {
        return -EINVAL;
    }

    if (sscanf(ptr, "/media=%d/channel=%d&level=%d",
        &m_type, &channel, &level) != 3)
    {
        return -EINVAL;
    }

    mrl->mrl_type = m_type;
    mrl->mrl_ind1 = channel;
    mrl->mrl_ind2 = level;
    mrl->mrl_ind3 = (int32_t)rc;
    mrl->mrl_ind4 = media_seq;

    if (m_type == MS_LIVE)
    {
        mrl->equal = ls_url_equal;
        return 0;
    }

    if (m_type == MS_VOD)
    {
        mrl->equal = hs_url_equal;
        return 0;
    }

    if (m_type == MS_DOWNLD)
    {
        mrl->equal = hs_url_equal;
        return 0;
    }

    return -EINVAL;
}


static __inline__ int32_t
rtsp_impl_parse_media_seq(rtsp_message *req)
{
    char *sid = NULL;
    int32_t media_seq = 0;

    rtsp_message_get_header(req, RTSP_HDR_SESSION, &sid, 0);
    if (sid)
    {
        sscanf(sid, "%d", &media_seq);
    }
    return media_seq;
}


int32_t
rtsp_impl_parse_url(rtsp_client *rc, const char *url, media_uri *mrl,
    int32_t *track, int32_t media_seq)
{
    char *track_suffix;
    RTSP_Url *r;
    int32_t err = 0;

    r = RTSP_url_parse((char*)url);
    if (!r || !r->path)
    {
        LOG_W(
            "rtsp_impl_parse_url()->RTSP_url_parse() failed."
        );
        return -EINVAL;
    }

    strncpy(__str(mrl->mrl), r->path, MAX_URL_LEN - 1);
    mrl->mrl[MAX_URL_LEN - 1] = 0;

    if (track)
    {
        track_suffix = strstr(__str(mrl->mrl), TRACK_SUFFIX);   //@{Fix Me}
        if (track_suffix)
        {
            *track_suffix = 0;
            sscanf(track_suffix + 1, TRACK_INDICATOR"%d", track);
        }
        else
        {
            err = -EINVAL;
        }
    }

    if (!err && rtsp_impl_special_parse(rc, mrl, track, media_seq))
    {
        err = -EINVAL;
    }

    RTSP_Url_destroy(r);
    return err;
}


RTSP_STATUS_CODE
rtsp_impl_trans_status_code(int32_t request, int32_t err)
{
    RTSP_STATUS_CODE rsc;

    switch (err)
    {
    case -EINVAL:
        rsc = RTSP_STS_BAD_REQUEST;
        break;

    case -ENOMEM:
        rsc = RTSP_STS_INTERNAL_SERVER_ERROR;
        break;

    case -EKILLED:
        rsc = RTSP_STS_CONTINUE;
        break;

    default:
        rsc = RTSP_STS_BAD_REQUEST;
        break;
    }

    return rsc;
}


rtsp_message *
rtsp_impl_new_generic_response(rtsp_message *req, RTSP_STATUS_CODE code)
{
    rtsp_message *res = NULL;

    rtsp_message_new_response(&res, code, NULL, req);
    BUG_ON(!res);
    rtsp_message_add_header(res, RTSP_HDR_SERVER, TR_SERVER_BANNER);
    return res;
}


int32_t
rtsp_impl_send_message(rtsp_client *rc, rtsp_message *message)
{
    return rtsp_client_send_msg(rc, rtsp_message_to_msg(message), 0);
}


static __inline__ int32_t
rtsp_impl_media_wait(media *media, rtsp_wait_block *rwb, int32_t wt, 
    pq_fun fun)
{
    return media_wait(media, wt, rwb, fun);
}


int32_t
rtsp_impl_queue_request(rtsp_client *rc, rtsp_message *req, int32_t *track,
    int32_t wt, pq_fun fun)
{
    rtsp_wait_block *rwb;
    int32_t err, media_seq;
    media *media;
    media_uri mrl;
    const char *url = NULL;

    rtsp_message_parse_request(req, NULL, &url, NULL, NULL);
    if (!url)
    {
        LOG_W(
            "rtsp_impl_on_setup()->rtsp_message_parse_request() failed."
        );
        return -EINVAL;
    }

    rwb = rtsp_wait_block_new();
    media_seq = rtsp_impl_parse_media_seq(req);
    if (rtsp_impl_parse_url(rc, url, &mrl, track, media_seq))
    {
        LOG_I(
            "Invalid request url '%s'.", url
        );
        rtsp_wait_block_free(rwb);
        return -EINVAL;
    }

    media = get_media(&mrl);
    if (!media)
    {
        LOG_W(
            "Get media object failed, url '%s'.",url
        );
        rtsp_wait_block_free(rwb);
        return -ENOMEM;
    }

    LOG_I(
        "Client '%p' got media '%p'.", rc, media
    );

    rwb->rc = (rtsp_client*)client_ref((client*)rc);
    rwb->orig_media = media_ref(media);
    rwb->req_msg = rtsp_message_dup_request(req);
    if (track)
    {
        rwb->i_track = *track;
    }

    err = rtsp_impl_media_wait(media, rwb, wt, fun);
    if (err)
    {
        LOG_W(
            "Stop waiting, err:'%d'.", err
        );
        media_unref(media);
        rtsp_wait_block_free(rwb);
        return err;
    }

    media_unref(media);
    return 0;
}


static int32_t
rtsp_impl_on_option(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_option(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_desc(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_desc(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_setup(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_setup(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_play(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_play(rc, rm);
    return 0;
}

static int32_t
rtsp_impl_on_pause(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_pause(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_teardown(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_teardown(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_setparm(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_setparm(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_getparm(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_getparm(rc, rm);
    return 0;
}


static int32_t
rtsp_impl_on_data(rtsp_client *rc, msg *req)
{
    rtsp_message *rm;

    rm = msg_to_rtsp_message(req);
    rtsp_method_on_data(rc, rm);
    return 0;
}


static rtsp_client_ops rtsp_impl_client_ops = 
{
    .create_session         = rtsp_impl_create_session,
    .recognize              = rtsp_impl_recognize,
    .on_option              = rtsp_impl_on_option,
    .on_desc                = rtsp_impl_on_desc,
    .on_setup               = rtsp_impl_on_setup,
    .on_play                = rtsp_impl_on_play,
    .on_pause               = rtsp_impl_on_pause,
    .on_teardown            = rtsp_impl_on_teardown,
    .on_setparm             = rtsp_impl_on_setparm,
    .on_getparm             = rtsp_impl_on_getparm,
    .on_data                = rtsp_impl_on_data
};


static int32_t
rtsp_impl_ts_init(rtsp_client *rc)
{
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    rtc->killed = 0;
    rtc->lock = LOCK_NEW();
    return 0;
}


static void
rtsp_impl_ts_fin(rtsp_client *rc)
{
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;
    LOCK_DEL(rtc->lock);
}


static void
rtsp_impl_ts_kill(rtsp_client *rc)
{
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    rtc->killed = 1;
    RELEASE_LOCK(rtc->lock);
}


int32_t
rtsp_impl_ts_client_killed(rtsp_ts_client *rtc)
{
    int32_t killed;

    AQUIRE_LOCK(rtc->lock);
    killed = rtc->killed;
    RELEASE_LOCK(rtc->lock);

    return killed;
}


static int32_t
rtsp_impl_ts_on_desc(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_desc(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static int32_t
rtsp_impl_ts_on_setup(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_setup(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static int32_t
rtsp_impl_ts_on_play(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_play(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static int32_t
rtsp_impl_ts_on_teardown(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_teardown(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static int32_t
rtsp_impl_ts_on_setparm(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_setparm(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static int32_t
rtsp_impl_ts_on_getparm(rtsp_client *rc, msg *req)
{
    int32_t err = -EKILLED;
    rtsp_ts_client *rtc = (rtsp_ts_client*)rc;

    AQUIRE_LOCK(rtc->lock);
    if (!rtc->killed)
    {
        err = rtsp_impl_on_getparm(rc, req);
    }
    RELEASE_LOCK(rtc->lock);

    return err;
}


static rtsp_client_ops rtsp_impl_ts_client_ops = 
{
    .init                   = rtsp_impl_ts_init,
    .fin                    = rtsp_impl_ts_fin,
    .kill                   = rtsp_impl_ts_kill,
    .create_session         = rtsp_impl_create_session,
    .recognize              = rtsp_impl_recognize,
    .on_option              = rtsp_impl_on_option,
    .on_desc                = rtsp_impl_ts_on_desc,
    .on_setup               = rtsp_impl_ts_on_setup,
    .on_play                = rtsp_impl_ts_on_play,
    .on_teardown            = rtsp_impl_ts_on_teardown,
    .on_setparm             = rtsp_impl_ts_on_setparm,
    .on_getparm             = rtsp_impl_ts_on_getparm,
    .on_data                = rtsp_impl_on_data
};


client *
rtsp_impl_client_new(uint32_t factory, void *sock)
{
    return (client*)rtsp_client_new(sizeof(rtsp_client),
        &rtsp_impl_client_ops, factory, sock);
}


client *
rtsp_impl_ts_client_new(uint32_t factory, void *sock)
{
    return (client*)rtsp_client_new(sizeof(rtsp_ts_client),
        &rtsp_impl_ts_client_ops, factory, sock);
}


//:~ End
