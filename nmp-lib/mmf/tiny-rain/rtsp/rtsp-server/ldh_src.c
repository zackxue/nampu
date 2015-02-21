#include <string.h>
#include "ldh_src.h"
#include "alloc.h"
#include "tr_log.h"

#define DEFAULT_INTERVAL            10
#define MAX_INTERVAL                200
#define MIN_INTERVAL                5

#ifdef _HI3516_IPC_
# define DL_SPEED_RATIO             4
#else
# define DL_SPEED_RATIO             2
#endif

#define FRAME_RATE_RATIO            1
#define PREREAD_BATCH               4
#define MIN_CACHED                  1
#define MAX_CACHED                  64
#define MAX_NALD_SIZE               16
#define TIME_STR_LEN                14
#define TIME_BUF_LEN                16
#define PROPERTY_BUF_LEN            128
#define IS_VIDEO_FRAME(frm)         ((frm)->hdr.type != FRAME_A)

#define LDH_FLGS_EOF                0x00000001
#define LDH_FLGS_READING            0x00000010
#define LDH_FLGS_DOWNLD             0x00000100

extern hs_avs_ops *hso;
static void ldh_op_fuck(void *data, void *user_data);
static JThreadPool *disk_io_tp = NULL;      /* For disk I/O */

enum
{
    PROBE, OPEN, PLAY, PAUSE, SEEK, KILL, READ
};

enum
{
    INIT, OPENED, PLAYED, PAUSED, CLOSED
};

typedef struct __prr_frame prr_frame;
struct __prr_frame
{
    struct list_head list;
    uint32_t data_size; /* save original */
    uint32_t nal_count;
    frame_t frame;
};

typedef struct __tp_block tp_block;
struct __tp_block
{
    ldh_src *hs;
    uint32_t action;
    uint32_t ts;
    media_uri mrl;
};


int32_t
ldh_facility_init(int32_t tp_threads)
{
    if (disk_io_tp)
        return -EEXIST;

    disk_io_tp = j_thread_pool_new(ldh_op_fuck, NULL, tp_threads, NULL);
    BUG_ON(!disk_io_tp);
    return 0;
}


static __inline__ int32_t
ldh_push_op(ldh_src *hs, uint32_t action, uint32_t ts, media_uri *mrl)
{
    int32_t err = -ENOMEM;
    tp_block *tb;

    tb = tr_alloc(sizeof(*tb));
    if (tb)
    {
        tb->hs = hs;
        media_src_ref((media_src*)hs);

        tb->action = action;
        tb->ts = ts;

        if (mrl)
        {
            memcpy(&tb->mrl, mrl, sizeof(*mrl));
        }

        if (action == READ)
        {
            hs->flags |= LDH_FLGS_READING;
        }

        j_thread_pool_push(disk_io_tp, tb);
        err = 0;
    }

    return err;
}



static __inline__ void
ldh_fill_frm(ldh_src *ldh, frame_t *frm)
{
    prr_frame *pf = container_of(frm, prr_frame, frame);
    list_add_tail(&pf->list, &ldh->frm_list);
    ++ldh->frm_count;

    if (frm->hdr.type == FRAME_EOF)
    {
        ldh->flags |= LDH_FLGS_EOF;
    }
}


static __inline__ prr_frame *
ldh_src_get_frm(ldh_src *ldh)
{
    prr_frame *pf = NULL;
    struct list_head *l;

    AQUIRE_LOCK(ldh->ldh_lock);
    if (!list_empty(&ldh->frm_list))
    {
        l = ldh->frm_list.next;
        list_del(l);
        pf = list_entry(l, prr_frame, list);
        --ldh->frm_count;
    }
    RELEASE_LOCK(ldh->ldh_lock);

    return pf;
}


static __inline__ int32_t
ldh_might_wait(ldh_src *ldh)
{
    prr_frame *pf;
    ld_src *lds = (ld_src*)ldh;
    int32_t type, stm_index;

    if (!ldh->next_frame)
    {
        ldh->next_frame = ldh_src_get_frm(ldh);
        if (!ldh->next_frame)
            return 1;
    }

    pf = (prr_frame*)ldh->next_frame;
    type = IS_VIDEO_FRAME(&pf->frame) ? ST_VIDEO : ST_AUDIO;
    stm_index = lds->idx[type];

    return media_src_would_block((media_src*)ldh, stm_index, &pf->frame, 0);
}


static __inline__ void
__ldh_preread_frames(ldh_src *ldh)
{
    if (ldh->state != PLAYED)
        return;

    if (ldh->flags & (LDH_FLGS_READING|LDH_FLGS_EOF))
        return;

    if (ldh->frm_count >= MIN_CACHED)
        return;

    ldh_push_op(ldh, READ, 0, NULL);
}


static __inline__ void
ldh_preread_frames(ldh_src *ldh)
{
    AQUIRE_LOCK(ldh->ldh_lock);
    __ldh_preread_frames(ldh);
    RELEASE_LOCK(ldh->ldh_lock);
}


static __inline__ uint32_t
ldh_state(ldh_src *ldh)
{
    uint32_t state;

    AQUIRE_LOCK(ldh->ldh_lock);
    state = ldh->state;
    RELEASE_LOCK(ldh->ldh_lock);

    return state;
}


static __inline__ prr_frame *
__ldh_alloc_frame(uint32_t data_size, uint32_t nal_count)
{
    prr_frame *pf;
    frame_t *f;
    uint32_t nd_size;

    data_size = data_size > 0 ? data_size : 4;
    nd_size = sizeof(nal_desc_t) + nal_count * 8;

    pf = tr_alloc(sizeof(*pf));
    if (pf)
    {
        f = &pf->frame;
        pf->data_size = data_size;
        pf->nal_count = nal_count;

        f->data = tr_alloc(data_size);
        if (f->data)
        {
            f->nal_desc = tr_alloc(nd_size);
            if (f->nal_desc)
            {
                f->nal_desc->nal_num = nal_count;
                f->hdr.size = data_size;
            }
            else
            {
                tr_free(f->data, data_size);
                tr_free(pf, sizeof(*pf));
                pf = NULL;
            }
        }
        else
        {
            tr_free(pf, sizeof(*pf));
            pf = NULL;
        }
    }

    return pf;
}


static __inline__ void
__ldh_free_frame(prr_frame *pf)
{
    int32_t nd_size;

    nd_size = sizeof(nal_desc_t) + pf->nal_count*8;
    tr_free(pf->frame.nal_desc, nd_size);
    tr_free(pf->frame.data, pf->data_size);
    tr_free(pf, sizeof(*pf));
}


static __inline__ void
__ldh_src_read_ahead(ldh_src *ldh)
{
    int32_t err, vfrm_count = 0;
    frame_t *frm;

    if (!hso || !hso->pull)
    {
        LOG_W(
            "__ldh_src_read_ahead()->hso->pull is NULL."
        );
        return;
    }

    for (;;)
    {
        if (ldh->state != PLAYED || (ldh->flags & LDH_FLGS_EOF))   
            break;

        if (ldh->frm_count >= MAX_CACHED)
            break;

        if (vfrm_count >= PREREAD_BATCH)
            break;

        frm = NULL;
        err = (*hso->pull)((avs_media*)ldh, &frm);
        if (err)
        {
            LOG_W(
                "__ldh_src_read_ahead()->(*hso->pull) failed, err:'%d'",
                err
            );
            return;
        }

        if (frm)
        {
            ldh_fill_frm(ldh, frm);
            if (IS_VIDEO_FRAME(frm))
            {
                ++vfrm_count;
            }
        }
        else
        {
            LOG_W(
                "__ldh_src_read_ahead()->(*hso->pull) got 'NULL' frame"
            );
        }
    }

    ldh->flags &= ~LDH_FLGS_READING;
}


static __inline__ int32_t
ldh_src_consume(ldh_src *ldh, frame_t *frm, int32_t vframe)
{
    ld_src *lds = (ld_src*)ldh;
    int32_t type, stm_index;

    if (!ldh->err)
    {
        type = vframe ? ST_VIDEO : ST_AUDIO;
        stm_index = lds->idx[type];
        return media_src_produce((media_src *)ldh, stm_index, frm, 0);      
    }

    return ldh->err;
}


static int32_t
ldh_stm_timer(Timer *timer, void *data)
{
    int32_t err, interval = DEFAULT_INTERVAL;
    prr_frame *pf;
    ldh_src *ldh = (ldh_src*)data;
    uint32_t ts, vframe;

    for (;;)
    {
        if (ldh_state(ldh) != PLAYED)
            break;

        if (ldh_might_wait(ldh))
        {
            break;
        }

        pf = (prr_frame*)ldh->next_frame;
        ldh->next_frame = NULL;

        ts = pf->frame.hdr.timestamp;
        vframe = IS_VIDEO_FRAME(&pf->frame);
        err = ldh_src_consume(ldh, &pf->frame, vframe);
        __ldh_free_frame(pf);

        if (err)
        {
            LOG_W(
                "ldh_stm_timer()->ldh_src_consume() failed, err:'%d'.",
                err
            );
            return TIMER_EXIT;
        }

        if (!vframe)
        {//@{fix me)
            continue;
        }

        if (ldh->last_ts)
        {
            interval = ts - ldh->last_ts;
#if 0
            interval *= FRAME_RATE_RATIO;
#else
            /* only support forward 1, 2/3  */
            if(ldh->scale != 1)
            {
                interval = (interval*2)/3.0;
            }
#endif
            if (ldh->flags & LDH_FLGS_DOWNLD)
                interval /= DL_SPEED_RATIO;

            if (interval > MAX_INTERVAL)
                interval = MAX_INTERVAL;
            else if (interval < MIN_INTERVAL)
                interval = MIN_INTERVAL;
        }

        ldh->last_ts = ts;
        break;
    }

    ldh_preread_frames(ldh);
    mod_timer(timer, interval);
    return TIMER_CONT;
}


static void
ldh_stm_timer_del(void *data)
{
    ldh_src *ldh = (ldh_src*)data;
    media_src *src = (media_src*)ldh;

    media_src_unref(src);
}


static int32_t
ldh_src_init(media_src *src)
{
    ld_src *ld = (ld_src*)src;
    ldh_src *ldh = (ldh_src*)ld;

    ld->idx[ST_VIDEO] = ST_MAX;
    ld->idx[ST_AUDIO] = ST_MAX;
    ld->ldl = 0;
    ld->break_off = 0;
    ld->u = NULL;

    ldh->state = INIT;
    ldh->flags = 0;
    INIT_LIST_HEAD(&ldh->frm_list);
    ldh->frm_count = 0;
    ldh->ldh_lock = LOCK_NEW();
    ldh->next_frame = NULL;
    ldh->err = 0;
    ldh->last_ts = 0;
    ldh->scale = 1;
    ldh->timer = set_timer(10, ldh_stm_timer, ldh_stm_timer_del, ldh);
    if (!ldh->timer)
    {
        LOCK_DEL(ldh->ldh_lock);
        return -ENOMEM;
    }

    media_src_ref(src);
    return 0;
}


static __inline__ void
ldh_free_preread_cache(ldh_src *ldh)
{
    struct list_head *l;
    prr_frame *pf;

    while (!list_empty(&ldh->frm_list))
    {
        l = ldh->frm_list.next;
        list_del(l);
        pf = list_entry(l, prr_frame, list);
        __ldh_free_frame(pf);
    }
}


static void
ldh_src_finalize(media_src *src)
{
    ld_src *ld = (ld_src*)src;
    ldh_src *ldh = (ldh_src*)ld;

    if (ldh->next_frame)
        __ldh_free_frame((prr_frame*)ldh->next_frame);
    LOCK_DEL(ldh->ldh_lock);
    ldh_free_preread_cache(ldh);
}


static __inline__ void
__ldh_src_kill(ldh_src *ldh)
{
    uint32_t state;

    state = ldh->state;
    if (state == CLOSED)
        return;

    ldh->state = CLOSED;
    if (ldh->timer)
    {
        del_timer(ldh->timer);
        ldh->timer = NULL;
    }

    if (state != INIT)
    {
        if (hso && hso->close)
            (*hso->close)((avs_media*)ldh);
    }
}


static void
ldh_src_kill(media_src *src)
{
    ldh_src *hs = (ldh_src*)src;

    ldh_push_op(hs, KILL, 0, NULL);
}


static __inline__ int32_t
__parse_mrl(media_uri *mrl, int32_t *channel, int32_t *level,
    int32_t *type, uint8_t *start_time, uint8_t *end_time, uint8_t *property,
    int32_t *downld)
{
    char url[MAX_URL_LEN];
    char *rt, *start, *end, *pro;

    strcpy(url, __str(mrl->mrl));
    rt = strstr(url, "recordType=");
    if (!rt)
        return -EINVAL;

    start = strstr(url, "startTime=");
    if (!start)
        return -EINVAL;

    end = strstr(url, "endTime=");
    if (!end)
        return -EINVAL;

    pro = strstr(url, "property=");
    if (pro)
    {
        strncpy(__str(property), pro, PROPERTY_BUF_LEN - 1);
        property[PROPERTY_BUF_LEN - 1] = 0;
    }

    strtok(rt, "&");
    strtok(rt, "/");
    strtok(start, "/");
    strtok(end, "/");

    if (sscanf(rt, "recordType=%d", type) != 1)
        return -EINVAL;

    start += strlen("startTime=");
    strncpy(__str(start_time), start, TIME_STR_LEN);
    start_time[TIME_STR_LEN] = 0;
    end += strlen("endTime=");
    strncpy(__str(end_time), end, TIME_STR_LEN);
    end_time[TIME_STR_LEN] = 0;

    *channel = mrl->mrl_ind1;
    *level = mrl->mrl_ind2;
    if (downld)
        *downld = (mrl->mrl_type == 3);
    return 0;
}


static __inline__ int32_t
__ldh_src_probe(ldh_src *src, media_uri *mrl, media_info *msi)
{
    int32_t ch, level, type, downld, err = -EPERM;
    uint8_t prop[PROPERTY_BUF_LEN], start_time[TIME_BUF_LEN], end_time[TIME_BUF_LEN];
    media_info_t mi;

    if (!hso || !hso->probe)
        return err;

    err = __parse_mrl(mrl, &ch, &level, &type, start_time, end_time,
        prop, &downld);
    if (err)
    {
        LOG_W(
            "__ldh_src_probe()->__parse_mrl(%s) failed, err:'%d'.",
            __str(mrl), err
        );
        return err;
    }
   
    memset(&mi, 0, sizeof(mi));
    err = (*hso->probe)(ch, level, type, start_time, end_time, prop, &mi);
    if (err)
    {
        LOG_W(
            "__ldh_src_probe()->(*hso->probe)(%s) failed, err:'%d'.",
            __str(mrl), err
        );
        return -ENOENT;
    }

    if (downld)
        src->flags |= LDH_FLGS_DOWNLD;
    __fill_media_info((ld_src*)src, msi, &mi);
    return 0;
}


static int32_t
__ldh_src_ratio_set(ldh_src *ldh, int32_t scale)
{
    int32_t err = 0;

    if (ldh->state != INIT && ldh->state != CLOSED)
    {
        ldh->scale = scale;
    }

    return err;
}


static int32_t
ldh_src_ctl(media_src *src, int32_t cmd, void *data)
{

    int32_t err;
    ldh_src *hs = (ldh_src*)src;

    switch(cmd)
    {
        case MEDIA_PLAY_SCALE:
            LOG_W( "__ldh_src_set_ratio()-> MEDIA_PLAY_SCALE %d .", (int)data);
            __ldh_src_ratio_set(hs, (int)data);
            break;
        default:
            break;
    }

    return 0;
}


static int32_t
ldh_src_probe(media_src *src, media_uri *mrl, media_info *msi)
{
    int32_t err;
    ldh_src *hs = (ldh_src*)src;

    err = ldh_push_op(hs, PROBE, 0, mrl);
    if (!err)
    {
        return -EAGAIN;
    }
    return err;
}


static __inline__ int32_t
__ldh_src_open(ldh_src *ldh, media_uri *mrl)
{
    int32_t err = -EPERM, ch, level, type;
    uint8_t prop[PROPERTY_BUF_LEN], start_time[TIME_BUF_LEN],end_time[TIME_BUF_LEN];

    if (!hso || !hso->open)
        return err;

    err = __parse_mrl(mrl, &ch, &level, &type, start_time,
        end_time, prop, NULL);
    if (err)
    {
        LOG_W(
            "__ldh_src_open()->__parse_mrl(%s) failed, err:'%d'.",
            __str(mrl), err
        );
        return err;
    }

    err = (*hso->open)((avs_media*)ldh, ch, level, type,
        start_time, end_time, prop);
    if (err)
    {
        LOG_W(
            "__ldh_src_open()->(*hso->open)(%s) failed, err:'%d'.",
            __str(mrl), err
        );
        return err;
    }

    ldh->state = OPENED;
    return 0;
}


static int32_t
ldh_src_open(media_src *src, media_uri *mrl)
{
    int32_t err = -EEXIST;
    ldh_src *ldh = (ldh_src*)src;

    err = ldh_push_op(ldh, OPEN, 0, mrl);
    if (!err)
    {
        err = -EAGAIN;
    }

    return err; 
}


static __inline__ int32_t
__ldh_src_play(ldh_src *ldh)
{
    int32_t err;

    if (ldh->state == CLOSED || ldh->state == PLAYED)
        return -EINVAL;

    if (!hso || !hso->play)
    {
        ldh->state = PLAYED;
        return 0;
    }

    err = (*hso->play)((avs_media*)ldh);
    if (err)
    {
        LOG_W(
            "__ldh_src_play()->(*hso->play)() failed, err:'%d'.",
            err
        );
        return err; 
    }

    ldh->state = PLAYED;
    return 0;
}


static int32_t
ldh_src_play(media_src *src)
{
    int32_t err;
    ldh_src *hs = (ldh_src*)src;

    err = ldh_push_op(hs, PLAY, 0, NULL);
    if (!err)
    {
        return -EAGAIN;
    }

    return err;     
}


static int32_t
__ldh_src_pause(ldh_src *ldh)
{
    int32_t err;

    if (ldh->state == OPENED || ldh->state == PLAYED)
    {
        if (!hso || !hso->pause)
        {
            ldh->state = PAUSED;
            return 0;
        }

        err = (*hso->pause)((avs_media*)ldh);
        if (err)
        {
            LOG_W(
                "__ldh_src_pause()->(*hso->pause)() failed, err:'%d'.",
                err
            );
        }
        else
        {
            ldh->state = PAUSED;
        }
        return err;
    }

    return 0;
}


static int32_t
ldh_src_pause(media_src *src)
{
    int32_t err;
    ldh_src *hs = (ldh_src*)src;

    err = ldh_push_op(hs, PAUSE, 0, NULL);
    if (!err)
    {
        return -EAGAIN;
    }
    return err;
}


static int32_t
__ldh_src_lseek(ldh_src *ldh, uint32_t ts)
{
    int32_t err;

    if (!hso || !hso->lseek)
        return -EPERM;

    if (ldh->state != INIT && ldh->state != CLOSED)
    {
        err = (*hso->lseek)((avs_media*)ldh, ts);
        if (err)
        {
            LOG_W(
                "__ldh_src_lseek()->(*hso->lseek)() failed, err:'%d'.",
                err
            );
        }
    }

    return err;
}


static int32_t
ldh_src_lseek(media_src *src, uint32_t ts)
{
    int32_t err;
    ldh_src *hs = (ldh_src*)src;

    err = ldh_push_op(hs, SEEK, ts, NULL);
    if (!err)
    {
        return -EAGAIN;
    }
    return err;
}


static void
ldh_op_fuck(void *data, void *user_data)
{
    tp_block *tb = (tp_block*)data;
    int32_t err;
    media_info msi;

    ldh_src *ldh = (ldh_src*)tb->hs;
    BUG_ON(!ldh);

    switch (tb->action)
    {
    case PROBE:
        media_info_init(&msi);
        err = __ldh_src_probe(ldh, &tb->mrl, &msi);
        media_src_fill_info((media_src*)ldh, &msi, err);
        media_info_clear(&msi);
        break;

    case OPEN:
        AQUIRE_LOCK(ldh->ldh_lock);
        err = __ldh_src_open(ldh, &tb->mrl);
        RELEASE_LOCK(ldh->ldh_lock);
        media_src_open_end((media_src*)ldh, err);
        break;

    case PLAY:
        AQUIRE_LOCK(ldh->ldh_lock);
        __ldh_src_play(ldh);
        RELEASE_LOCK(ldh->ldh_lock);
        break;

    case PAUSE:
        AQUIRE_LOCK(ldh->ldh_lock);
        __ldh_src_pause(ldh);
        RELEASE_LOCK(ldh->ldh_lock);
        break;

    case SEEK:
        AQUIRE_LOCK(ldh->ldh_lock);
        __ldh_src_lseek(ldh, tb->ts);
        RELEASE_LOCK(ldh->ldh_lock);
        break;

    case KILL:
        AQUIRE_LOCK(ldh->ldh_lock);
        __ldh_src_kill(ldh);
        RELEASE_LOCK(ldh->ldh_lock);
        break;

    case READ:
        AQUIRE_LOCK(ldh->ldh_lock);
        __ldh_src_read_ahead(ldh);
        RELEASE_LOCK(ldh->ldh_lock);
        break;
    }

    media_src_unref((media_src*)ldh);
    tr_free(tb, sizeof(*tb));
}


static media_src_ops ldh_src_ops =
{
    .init   = ldh_src_init,
    .fin    = ldh_src_finalize,
    .kill   = ldh_src_kill,
	.ctl    = ldh_src_ctl,
    .probe  = ldh_src_probe,
    .open   = ldh_src_open,
    .play   = ldh_src_play,
    .pause  = ldh_src_pause,
    .lseek  = ldh_src_lseek
};


ldh_src *ldh_src_alloc(void *u)
{
    return (ldh_src*)media_src_alloc(sizeof(ldh_src),
        &ldh_src_ops, u);
}


void
ldh_src_set_download_mode(ldh_src *ldh)
{
    ldh->flags |= LDH_FLGS_DOWNLD;
}


frame_t *
ldh_alloc_frame(uint32_t data_size, uint32_t nal_count)
{
    prr_frame *pf;

    pf = __ldh_alloc_frame(data_size, nal_count);
    return pf ? &pf->frame : NULL;
}


void
ldh_free_frame(frame_t *frame)
{
    prr_frame *pf;

    if (frame)
    {
        pf = container_of(frame, prr_frame, frame);
        __ldh_free_frame(pf);
    }
}

//:~ End
