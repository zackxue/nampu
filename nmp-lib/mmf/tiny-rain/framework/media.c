#include <stdio.h>
#include <string.h>
#include "media.h"
#include "alloc.h"
#include "media_sinker.h"
#include "tr_factory.h"
#include "tr_log.h"
#include "timer.h"

#define HASH_BUCKETS            8
#define MEDIA_KEEP_TIMER        8000        /* 8 secs */
#define MAX_PULL_COUNT          256     

#define MFLGS_HASHED          0x01
#define MFLGS_STARTED         0x02
#define MFLGS_RELIABLE        0x04

enum
{
    STAT_INIT,
    STAT_OPENING,
    STAT_READY,
    STAT_KILLED
};

typedef struct __wait_info wait_info;
struct __wait_info
{
    media *m;
    void *parm;
    pq_fun fun;
};

typedef struct __hash_table hash_table;
struct __hash_table
{
    struct list_head buckets[HASH_BUCKETS];
    LOCK_T lock;
};

static hash_table media_table;
static atomic_t media_objs = ATOMIC_INIT;
static atomic_t media_no = ATOMIC_INIT;

void init_hash_table(hash_table *t)
{
    int32_t i;

    for (i = 0; i < HASH_BUCKETS; ++i)
    {
        INIT_LIST_HEAD(&t->buckets[i]);
    }

    t->lock = LOCK_NEW();
}


void init_media_facility( void )
{
    init_hash_table(&media_table);
}


static __inline__ int32_t
__media_uri_equal(media_uri *mrl_1, media_uri *mrl_2)
{
    if (mrl_1->equal == mrl_2->equal && mrl_1->equal)
    {
        return (*mrl_1->equal)(mrl_1, mrl_2);
    }
    return 0;
}


static __inline__ int32_t
media_uri_hash(media_uri *mrl)
{
    return 0;
}


static __inline__ void
media_info_free(media_info *info)
{
    media_info_clear(info);
    tr_free(info, sizeof(*info));
}


int32_t
media_seq(media *m)
{
    return m->no;
}


media *media_ref(media *m)
{
    if (m)
    {
        obj_ref(m);
    }
    return m;
}


void media_unref(media *m)
{
    if (m)
    {
        obj_unref(m);
    }
}


static __inline__ media *
__find_media_by_url(hash_table *t, media_uri *mrl)
{
    media *m;
    struct list_head *l;
    int32_t hash_key = media_uri_hash(mrl);

    list_for_each(l, &t->buckets[hash_key])
    {
        m = list_entry(l, media, hash_list);
        if (__media_uri_equal(&m->mrl, mrl))
            return m;
    }

    return NULL;
}


static __inline__ media *
__find_media(hash_table *t, media *m)
{
    media *_m;
    struct list_head *l;
    int32_t hash_key = media_uri_hash(&m->mrl);

    list_for_each(l, &t->buckets[hash_key])
    {
        _m = list_entry(l, media, hash_list);
        if (_m == m)
            return m;
    }

    return NULL;
}


static __inline__ media *
__find_and_get_media(hash_table *t, media_uri *mrl)
{
    media *m;

    m = __find_media_by_url(t, mrl);
    if (m)
    {
        media_ref(m);
    }

    return m;
}


static __inline__ media *
_find_and_get_media(hash_table *t, media_uri *mrl)
{
    media *m;

    AQUIRE_LOCK(t->lock);
    m = __find_and_get_media(t, mrl);
    RELEASE_LOCK(t->lock);

    return m;
}


static __inline__ media *
find_and_get_media(media_uri *mrl)
{
    return _find_and_get_media(&media_table, mrl);
}


static __inline__ void
__add_media(hash_table *t, media *m)
{
    int32_t hash_key = media_uri_hash(&m->mrl);
    m->flags |= MFLGS_HASHED;
    list_add(&m->hash_list, &t->buckets[hash_key]);
}


static __inline__ int32_t
__add_media_safe(hash_table *t, media *m)
{
    media *old;
    int32_t err = -EEXIST;

    old = __find_media_by_url(t, &m->mrl);
    if (!old)
    {
        err = 0;
        __add_media(t, m);
        media_ref(m);
    }

    return err;
}


static __inline__ int32_t
_add_media_safe(hash_table *t, media *m)
{
    int32_t err;

    AQUIRE_LOCK(t->lock);
    err = __add_media_safe(t, m);
    RELEASE_LOCK(t->lock);

    return err;
}


static __inline__ int32_t
__media_remove(hash_table *t, media *m)
{
    if (!__find_media(t, m))
    {
        return -ENOENT;
    }

    list_del(&m->hash_list);
    m->flags &= ~MFLGS_HASHED;
    media_unref(m);

    return 0;
}


static __inline__ int32_t
_media_remove(hash_table *t, media *m)
{
    int32_t err;

    AQUIRE_LOCK(t->lock);
    err = __media_remove(t, m);
    RELEASE_LOCK(t->lock);

    return err;
}


static __inline__ int32_t
add_media_safe(media *m)
{
    return _add_media_safe(&media_table, m);
}


static __inline__ void
media_remove(media *m)
{
    if (m->flags & MFLGS_HASHED)
    {
        _media_remove(&media_table, m);
    }
}


static __inline__ media_filter *
media_create_sfilter(media *m)
{
   tr_factory *factory;
   media_filter *mf = NULL;

    factory = get_tr_factory(0);
    if (factory)
    {
        mf = factory_create_msfilter(factory);
    }

    if (!mf)
    {
        LOG_W(
            "media_create_sfilter() failed."
        );
    }

    return mf;  
}


static __inline__ void
media_delete_sfilter(media_filter *mf)
{
    media_filter_release(mf);
}


static int32_t
media_timer(Timer *self, void *data)
{
    media *m = (media*)data;

    if (!(m->flags & MFLGS_STARTED))
    {
        media_ref(m);
        media_kill_unref(m);
    }

    return TIMER_EXIT;
}


static void
media_timer_del(void *data)
{
    media *m = (media*)data;
    media_unref(m);
}


static __inline__ void
init_media_obj(media *m)
{
    INIT_LIST_HEAD(&m->hash_list);
    m->no = atomic_add(&media_no, 1);
    m->state = STAT_INIT;
    m->flags = 0;
    m->seq_generator = 0;
    memset(&m->mrl, 0, sizeof(media_uri));
    m->info = NULL;
    m->filter_src = NULL;
    m->src = NULL;
    INIT_LIST_HEAD(&m->sinkers);
    pq_init(&m->waiting_for_info);
    pq_init(&m->waiting_for_stms);
    m->wfi_err = 0;
    m->wfs_err = 0;
    m->lock = LOCK_NEW();

    media_ref(m);
    m->timer = set_timer(MEDIA_KEEP_TIMER,
                           media_timer,
                           media_timer_del,
                           m);
    BUG_ON(!m->timer);
    atomic_inc(&media_objs);
}


static __inline__ void
fin_media_obj(media *m)
{
    BUG_ON(!media_killed(m));
    LOCK_DEL(m->lock);
    media_delete_sfilter(m->filter_src);

    if (m->info)
    {
        media_info_free(m->info);
    }

    atomic_dec(&media_objs);
    LOG_V("Media objs count: %d.",
        atomic_get(&media_objs));
}


static __inline__ void
on_obj_fin(obj *p)
{
    fin_media_obj((media*)p);
}


static __inline__ media *
new_media(media_uri *mrl)
{
    media *m;
    media_filter *filter_src;

    filter_src = media_create_sfilter(NULL);
    if (!filter_src)
        return NULL;

    m = (media*)obj_new(sizeof(*m), on_obj_fin);
    if (m)
    {
        init_media_obj(m);

        m->filter_src = filter_src;
        mrl->mrl_ind4 = m->no;
        memcpy(&m->mrl, mrl, sizeof(*mrl));
    }
    else
    {
        media_delete_sfilter(filter_src);
    }

    return m;
}


static __inline__ media_src *
create_media_src(media *m, media_uri *mrl)
{
    int32_t err;
    media_src *msrc;
    tr_factory *factory;

    factory = get_tr_factory(0);
    if (factory)
    {
        msrc = factory_create_msrc(factory, mrl->mrl_type);
        if (msrc)
        {
            err = media_attach_source(m, msrc);
            if (err)
            {
                media_src_kill_unref(msrc);
                return NULL;
            }
            media_src_unref(msrc);
        }
        return msrc;
    }

    return NULL;
}


media *get_media(media_uri *mrl)
{
    int32_t races = 3;
    media *m = NULL;
    media_src *msrc;

    while (!m)
    {
        m = find_and_get_media(mrl);
        if (!m)
        {
            m = new_media(mrl);
            if (!m)
            {
                break;
            }

            msrc = create_media_src(m, mrl);
            if (!msrc)
            {
                media_unref(m);
                m = NULL;
                break;
            }

            add_media_safe(m);
            media_unref(m);
            m = NULL;

            if (--races <= 0)
            {
                break;
            }
        }
    }

    return m;
}


static __inline__ int32_t
__media_killed(media *m)
{
    return m->state == STAT_KILLED;
}


int32_t
media_killed(media *m)
{
    int32_t ret;

    AQUIRE_LOCK(m->lock);
    ret = __media_killed(m);
    RELEASE_LOCK(m->lock);

    return ret;
}


int32_t
media_ready_to_start(media *m)
{
    int32_t err = -EKILLED;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = -EPERM;
        if (m->state == STAT_READY)
        {
            if (!(m->flags & MFLGS_STARTED))
            {
                m->flags |= MFLGS_STARTED;
                if (m->src)
                {
                    err = media_src_play(m->src);
                }
            }
            else
            {
                if (m->src)
                {
                    err = media_src_ctl(m->src, MEDIA_FORCE_IFRAME, NULL);
                }
            }
        }
    }
    RELEASE_LOCK(m->lock);

    return err;
}


static __inline__ uint32_t
media_cal_real_size(media *m, int32_t stm_idx,
    void *data, uint32_t size)
{
    uint32_t real_size = 0;

    if (m->filter_src)
    {
        real_size = media_filter_cal_size(
            m->filter_src, 0, data, size);
    }

    return real_size;
}


int32_t
media_would_block(media *m, int32_t stm_idx, void *data,
    uint32_t size)
{
    int32_t ret = -EKILLED;
    media_sinker *sinker;
    uint32_t real_size;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        ret = 0;
        if (m->flags & MFLGS_RELIABLE)
        {
            if (!list_empty(&m->sinkers))
            {
                sinker = list_of_sinker(m->sinkers.next);
                real_size = media_cal_real_size(m,
                    stm_idx, data, size);
                ret = media_sinker_consumable(sinker,
                    stm_idx, real_size);
            }
        }
    }
    RELEASE_LOCK(m->lock);

    return ret;
}


static __inline__ void
media_unref_all_sinkers(media *m)
{
    struct list_head *l;
    media_sinker *sinker;

    while (!list_empty(&m->sinkers))
    {
        l = m->sinkers.next;
        list_del(l);
        RELEASE_LOCK(m->lock);

        sinker = list_of_sinker(l);
        media_sinker_unref(sinker);

        AQUIRE_LOCK(m->lock);
    }
}


static __inline__ void
media_call_pq(media *m)
{
    pq pq_i, pq_stm;

    pq_init(&pq_i);
    pq_init(&pq_stm);
    pq_graft(&m->waiting_for_info, &pq_i);
    pq_graft(&m->waiting_for_stms, &pq_stm);
    RELEASE_LOCK(m->lock);

    pq_call_and_clear(&pq_i);
    pq_call_and_clear(&pq_stm);

    AQUIRE_LOCK(m->lock);
}


static __inline__ void
__media_kill(media *m)
{
    if (__media_killed(m))
        return;

    m->state = STAT_KILLED;
    del_timer(m->timer);
    media_unref_all_sinkers(m);
    media_src_kill_unref(m->src);
    m->src = NULL;
    media_call_pq(m);
}


static __inline__ void
media_kill(media *m)
{
    AQUIRE_LOCK(m->lock);
    __media_kill(m);
    RELEASE_LOCK(m->lock);
}


void
media_kill_unref(media *m)
{
    media_remove(m);
    media_kill(m);
    media_unref(m);
}


static __inline__ int32_t
__media_attach_source(media *m, media_src *src)
{
    if (m->src)
        return -EEXIST;

    src->medium = media_ref(m);
    m->src = media_src_ref(src);

    return 0;
}


int32_t
media_attach_source(media *m, media_src *src)
{
    int32_t err = -EKILLED;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = __media_attach_source(m, src);
    }
    RELEASE_LOCK(m->lock);

    return err;
}


static __inline__ int32_t
__media_insert_sinker(media *m, media_sinker *sinker)
{
    if (!list_empty(&sinker->list))
        return -EEXIST;

    list_add(&sinker->list, &m->sinkers);
    sinker->medium = media_ref(m);
    media_sinker_ref(sinker);

    return 0;
}


int32_t
media_insert_sinker(media *m, struct list_head *snk)
{
    int32_t err = -EKILLED;
    media_sinker *sinker;

    sinker = list_of_sinker(snk);
    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = __media_insert_sinker(m, sinker);
    }
    RELEASE_LOCK(m->lock);

    return err;
}


static __inline__ int32_t
__media_sinker_detach(media *m, media_sinker *sinker,
    int32_t *last)
{
    int32_t err = -ENOENT;
    struct list_head *l;

    list_for_each(l, &m->sinkers)
    {
        if (l == &sinker->list)
        {
            list_del_init(&sinker->list);
            err = 0;
            break;
        }
    }

    *last = 0;
    if (!err && list_empty(&m->sinkers))
    {
        *last = 1;
    }

    return err;
}


int32_t
media_remove_sinker(media *m, struct list_head *snk)
{
    int32_t err = -EKILLED, last;
    media_sinker *sinker;

    sinker = list_of_sinker(snk);
    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = __media_sinker_detach(m, sinker, &last);
    }
    RELEASE_LOCK(m->lock);

    if (!err)
    {
         media_sinker_unref(sinker);
         if (last)
         {
            media_ref(m);
            media_kill_unref(m);
         }
    }

    return err; 
}


static __inline__ int32_t
__media_fill_all_sinkers(media *medium, int32_t stm_i, msg *m,
    uint32_t seq)
{
    struct list_head *l;
    media_sinker *sinker;

    if (medium->flags & MFLGS_RELIABLE)
    {
        if (list_empty(&medium->sinkers))
            return -ENOENT;

        l = medium->sinkers.next;
        sinker = list_of_sinker(l);
        return media_sinker_fill(sinker, stm_i, m, seq);
    }

    list_for_each(l, &medium->sinkers)
    {
        sinker = list_of_sinker(l);
        media_sinker_fill(sinker, stm_i, m, seq);
    }

    return 0;
}


int32_t
media_ctl(media *m, int32_t cmd, void *data)
{
    int32_t err = -EINVAL;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        if (m->src)
        {
            err = media_src_ctl(m->src, cmd, data);
        }
    }
    RELEASE_LOCK(m->lock);

    return err;
}


int32_t
media_play_ctl(media *m, int32_t cmd, void *data)
{
	int32_t err = -EINVAL;

	AQUIRE_LOCK(m->lock);
	if (!__media_killed(m))
	{
		if (m->src)
		{
            switch(cmd)
            {
                case MEDIA_PLAY_SEEK:
			        err = media_src_lseek(m->src, (uint32_t)data);
                    break;
                case MEDIA_PLAY_PAUSE:
			        err = media_src_pause(m->src);
                    break;
                default:
                    err = media_src_ctl(m->src, cmd, data);
                    break;
            }
		}
	}
	RELEASE_LOCK(m->lock);

	return err;
}



static __inline__ int32_t
media_get_stm_type(media *m, int32_t stm_i)
{
    int32_t type = -EINVAL;

    if (m->info)
    {
        if (stm_i >= 0 && stm_i < m->info->n_stms)
            type = m->info->stms[stm_i].stm_type;
    }

    return type;
}


static __inline__ int32_t
__media_fill_stream_from_source(media *m, int32_t stm_i,
    void *data, uint32_t size)
{
    msg *new_data = NULL;
    int32_t err, type, pull;

    type = media_get_stm_type(m, stm_i);
    if (type < 0)
    {//@{drop un-probed stream data}
        return 0;
    }

    err = media_filter_fill(m->filter_src, type, data, size);
    if (!err)
    {
        pull = MAX_PULL_COUNT;
        ++m->seq_generator;

        for (;;)
        {
            err = media_filter_pull(m->filter_src, type, &new_data);
            if (!err)
            {
                BUG_ON(!new_data);
                err = __media_fill_all_sinkers(m, stm_i, new_data,
                    m->seq_generator);
                msg_unref(new_data);

                if (err)
                {
                    BUG_ON(err == -EAGAIN);
                    break;
                }
            }
            else
            {
                if (err == -EAGAIN)
                    err = 0;
                break;
            }

            if (--pull <= 0)
            {
                LOG_W(
                    "media '%p' pulling reach max count '%d'.",
                    m, MAX_PULL_COUNT
                );
                break;
            }
        }
    }

    return err; 
}


int32_t
media_fill_stream_from_source(media *m, int32_t stm_i, void *data,
    uint32_t size)
{
    int32_t err = -EKILLED;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        if (m->filter_src)
        {
            err = __media_fill_stream_from_source(m, stm_i, data, size);
        }
    }
    RELEASE_LOCK(m->lock);

    return err;
}


static __inline__ void
media_call_queue(media *m, wait_type wt)
{
    pq pq_i;

    pq_init(&pq_i);
    if (wt == W_MEDIA_INFO)
    {
        pq_graft(&m->waiting_for_info, &pq_i);
    }
    else
    {
        pq_graft(&m->waiting_for_stms, &pq_i);
    }

    RELEASE_LOCK(m->lock);
    pq_call_and_clear(&pq_i);
    AQUIRE_LOCK(m->lock);
}


int32_t
media_info_fill(media *m, media_info *msi, int32_t err)
{
    int32_t error = -EKILLED;
    pq pq_i;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        m->wfi_err = err;

        if (m->info)
        {
            media_info_free(m->info);
            m->info = NULL;
        }

        if (!err && msi)
        {
            m->info = tr_alloc0(sizeof(*msi));
            media_info_dup(m->info, msi);
        }

        pq_init(&pq_i);
        pq_graft(&m->waiting_for_info, &pq_i);
        RELEASE_LOCK(m->lock);

        pq_call_and_clear(&pq_i);
        return 0;
    }
    RELEASE_LOCK(m->lock);

    return error;   
}


int32_t
media_info_get(media *m, media_info *msi)
{
    int32_t err = -EKILLED;

    media_info_init(msi);
    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = m->wfi_err;
        if (!err && m->info)
        {
            media_info_dup(msi, m->info);
        }
        else
        {
            err = -ENOENT;
        }
    }
    RELEASE_LOCK(m->lock);

    return err; 
}


int32_t
media_open_end(media *m, int32_t err)
{
    int32_t _err = -EKILLED;
    pq pq_stm;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        m->state = STAT_READY;
        m->wfs_err = err;
        pq_init(&pq_stm);
        pq_graft(&m->waiting_for_stms, &pq_stm);
        RELEASE_LOCK(m->lock);

        pq_call_and_clear(&pq_stm);
        return 0;
    }
    RELEASE_LOCK(m->lock);

    return _err;
}


static __inline__ int32_t
__media_wait_needed(media *m, wait_type wt, int32_t *first)
{
    switch (wt)
    {
    case W_MEDIA_INFO:
        *first = 1;
        return 1;

    case W_MEDIA_STM:
        if (m->state >= STAT_READY)
        {
            return 0;
        }
        if (m->state == STAT_INIT)
        {
            *first = 1;
            m->state = STAT_OPENING;
        }
        else
        {
            *first = 0;
        }
        return 1;

    default:
        break;
    }

    return 0;
}


static __inline__ int32_t
__media_wait_info(media *m, void *parm, pq_fun fun)
{
    int32_t err, first;
    media_src *src;
    media_info msi;

    err = pq_pend(&m->waiting_for_info, parm, NULL, NULL, fun);
    if (err)
    {
        return err;
    }

    if (__media_wait_needed(m, W_MEDIA_INFO, &first))
    {
        if (first)
        {
            src = media_src_ref(m->src);
            RELEASE_LOCK(m->lock);

            media_info_init(&msi);
            err = media_src_probe(src, &m->mrl, &msi);
            media_src_unref(src);

            if (err != -EAGAIN)
            {
                media_info_fill(m, &msi, err);
            }
            media_info_clear(&msi);

            AQUIRE_LOCK(m->lock);
        }
    }
    else
    {
        media_call_queue(m, W_MEDIA_INFO);
    }

    return 0;
}


static __inline__ int32_t
__media_wait_stm(media *m, void *parm, pq_fun fun)
{
    int32_t err, first;
    media_src *src;

    err = pq_pend(&m->waiting_for_stms, parm, NULL, NULL, fun);
    if (err)
    {
        return err;
    }

    if (__media_wait_needed(m, W_MEDIA_STM, &first))
    {
        if (first)
        {
            src = media_src_ref(m->src);
            RELEASE_LOCK(m->lock);

            err = media_src_open(src, &m->mrl);
            media_src_unref(src);
            if (err != -EAGAIN)
            {
                media_open_end(m, err);
            }

            AQUIRE_LOCK(m->lock);
        }
    }
    else
    {
        media_call_queue(m, W_MEDIA_STM);
    }

    return 0;
}


static void
__media_pre_open(void *parm1, void *parm2, void *parm3)
{
    int32_t err, done = 0;
    wait_info *pwi = (wait_info *)parm1;
    media *m = pwi->m;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        if (m->src)
        {
            err = __media_wait_stm(pwi->m, pwi->parm, pwi->fun);
            if (!err)
            {
                done = 1;
            }
        }
    }
    RELEASE_LOCK(m->lock);

    if (!done)
    {
        (*pwi->fun)(pwi->parm, NULL, NULL);
    }

    media_unref(m);
    tr_free(pwi, sizeof(*pwi));
}


static __inline__ int32_t
__media_open_wait(media *m, void *parm, pq_fun fun)
{
    int32_t err;
    wait_info *pwi;

    pwi = tr_alloc0(sizeof(*pwi));
    pwi->m = media_ref(m);
    pwi->parm = parm;
    pwi->fun = fun;

    err = __media_wait_info(m, pwi, __media_pre_open);
    if (err)
    {
        media_unref(pwi->m);
        tr_free(pwi, sizeof(*pwi));
    }
    return err;
}


static __inline__ int32_t
__media_wait(media *m, wait_type wt, void *parm, pq_fun fun)
{
    int32_t err;

    switch (wt)
    {
    case W_MEDIA_INFO:
        err = __media_wait_info(m, parm, fun);
        break;

    case W_MEDIA_STM:
        err = m->wfi_err;
        if (!err)
        {
            if (!m->info)
            {
                err = __media_open_wait(m, parm, fun);
            }
            else
            {
                err = __media_wait_stm(m, parm, fun);
            }
        }
        break;

    default:
        err = -EINVAL;
        break;
    }

    return err;
}


int32_t
media_wait(media *m, wait_type wt, void *parm, pq_fun fun)
{
    int32_t err = -EKILLED;

    AQUIRE_LOCK(m->lock);
    if (!__media_killed(m))
    {
        err = -ENOENT;
        if (m->src)
        {
            err = __media_wait(m, wt, parm, fun);
        }
    }
    RELEASE_LOCK(m->lock);

    return err;
}


//:~ End
