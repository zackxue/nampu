#include <string.h>
#include "ldl_src.h"
#include "alloc.h"
#include "tr_log.h"

extern ls_avs_ops *lso;
static void ldl_op_fuck(void *data, void *user_data);
static JThreadPool *ls_tp = NULL;      /* For sleepable callback() */

enum
{
	INIT, OPENED, CLOSED
};

enum
{
	ACTION_PROBE, ACTION_OPEN, ACTION_PLAY, ACTION_CTRL, ACTION_CLOSE
};


typedef struct __tp_block tp_block;
struct __tp_block
{
    ldl_src *ls;
    uint32_t action;
    media_uri mrl;
};


int32_t
ldl_facility_init(int32_t tp_threads)
{
    if (ls_tp)
        return -EEXIST;

    ls_tp = j_thread_pool_new(ldl_op_fuck, NULL, tp_threads, NULL);
    BUG_ON(!ls_tp);
    return 0;
}


static __inline__ int32_t
ldl_push_op(ldl_src *ls, uint32_t action, media_uri *mrl)
{
    int32_t err = -ENOMEM;
    tp_block *tb;

    tb = tr_alloc(sizeof(*tb));
    if (tb)
    {
        tb->ls = ls;
        media_src_ref((media_src*)ls);

        tb->action = action;
        if (mrl)
        {
            memcpy(&tb->mrl, mrl, sizeof(*mrl));
        }

        j_thread_pool_push(ls_tp, tb);
        err = 0;
    }

    return err;
}


static int32_t
ldl_src_init(media_src *src)
{
	ld_src *ld = (ld_src*)src;
	ldl_src *ldl = (ldl_src*)ld;

	ld->idx[ST_VIDEO] = ST_MAX;
	ld->idx[ST_AUDIO] = ST_MAX;
	ld->ldl = 1;
	ld->break_off = 0;
	ld->u = NULL;

	ldl->state = INIT;
	ldl->ldl_lock = LOCK_NEW();
	return 0;
}


static void
ldl_src_finalize(media_src *src)
{
	ldl_src *ldl = (ldl_src*)src;
	LOCK_DEL(ldl->ldl_lock);
}


static void
ldl_src_kill(media_src *src)
{
	ldl_push_op((ldl_src*)src, ACTION_CLOSE, NULL);
}


static void
__ldl_src_kill(ldl_src *ldl)
{
	if (ldl->state == OPENED)
	{
		if (lso && lso->close)
		{
			LOG_I("__ldl_src_kill(): avs_media '%p'.", ldl);
			(*lso->close)((avs_media*)ldl);
			LOG_I("__ldl_src_kill(): avs_media '%p' ok.", ldl);
		}
	}

	ldl->state = CLOSED;
}


static __inline__ int32_t
__parse_mrl(media_uri *mrl, int32_t *channel, int32_t *level)
{
	*channel = mrl->mrl_ind1;
	*level = mrl->mrl_ind2;
	return 0;
}


static int32_t
ldl_src_ctrl(media_src *src, int32_t cmd, void *data)
{
	int32_t err;

	err = ldl_push_op((ldl_src*)src, ACTION_CTRL, NULL);
	return err;
}


static int32_t
__ldl_src_ctrl(ldl_src *ldl, int32_t cmd, void *data)
{
	int32_t err = -EPERM;

	if (!lso || !lso->ctrl)
		return err;

	if (ldl->state != OPENED)
		return err;

	LOG_I(
		"__ldl_src_ctrl(): avs_media '%p'", ldl
	);

	err = (*lso->ctrl)((avs_media*)ldl, cmd, data);
	if (err)
	{
		LOG_W(
			"__ldl_src_ctrl()->(*lso->ctrl)() failed, err:'%d'.",
			err
		);
		return err;
	}
	else
	{
		LOG_I(
			"__ldl_src_ctrl(): avs_media '%p' ok.", ldl
		);
	}

	return 0;
}


static int32_t
ldl_src_probe(media_src *src, media_uri *mrl, media_info *msi)
{
	int32_t err;

	err = ldl_push_op((ldl_src*)src, ACTION_PROBE, mrl);
	if (!err)
	{
		err = -EAGAIN;
	}

	return err;
}


static __inline__ int32_t
__ldl_src_probe(ldl_src *ls, media_uri *mrl, media_info *msi)
{
	int32_t ch, level, err = -EPERM;
	media_info_t mi;

	if (!lso || !lso->probe)
		return err;

	err = __parse_mrl(mrl, &ch, &level);
	if (err)
	{
		LOG_W(
			"__ldl_src_probe()->__parse_mrl(%s) failed, err:'%d'.",
			__str(mrl), err
		);
		return err;
	}

	memset(&mi, 0, sizeof(mi));

	LOG_I(
		"__ldl_src_probe(): mrl '%s'.", __str(mrl->mrl)
	);

	err = (*lso->probe)(ch, level, &mi);
	if (err)
	{
		LOG_W(
			"__ldl_src_probe()->(*lso->probe)(%s) failed, err:'%d'.",
			__str(mrl->mrl), err
		);
		return -ENOENT;
	}
	else
	{
		LOG_I(
			"__ldl_src_probe(): mrl '%s' ok.", __str(mrl->mrl)
		);
	}

	__fill_media_info((ld_src*)ls, msi, &mi);
	return 0;
}


static int32_t
ldl_src_open(media_src *src, media_uri *mrl)
{
	int32_t err;

	err = ldl_push_op((ldl_src*)src, ACTION_OPEN, mrl);
	if (!err)
	{
		err = -EAGAIN;
	}

	return err;
}


static int32_t
__ldl_src_open(ldl_src *ldl, media_uri *mrl)
{
	int32_t err = -EPERM, ch, level;

	if (!lso || !lso->open)
		return err;

	if (ldl->state != INIT)
		return err;

	err = __parse_mrl(mrl, &ch, &level);
	if (err)
	{
		LOG_W(
			"__ldl_src_open()->__parse_mrl(%s) failed, err:'%d'.",
			__str(mrl), err
		);
		return err;
	}

	LOG_I(
		"__ldl_src_open(): mrl '%s'.", __str(mrl->mrl)
	);

	err = (*lso->open)((avs_media*)ldl, ch, level);
	if (err)
	{
		LOG_W(
			"__ldl_src_open()->(*lso->open)() failed, err:'%d'.",
			err
		);
		return err;	
	}
	else
	{
		LOG_I(
			"__ldl_src_open(): mrl '%s' ok.", __str(mrl->mrl)
		);
	}

	ldl->state = OPENED;
	return 0;
}


static int32_t
ldl_src_play(media_src *src)
{
	int32_t err;

	err = ldl_push_op((ldl_src*)src, ACTION_PLAY, NULL);
	return err;
}


static int32_t
__ldl_src_play(ldl_src *ldl)
{
	int32_t err = -EPERM;

	if (!lso || !lso->play)
		return err;

	if (ldl->state != OPENED)
		return err;

	LOG_I(
		"__ldl_src_play(): avs_media '%p'.", ldl
	);

	err = (*lso->play)((avs_media*)ldl);
	if (err)
	{
		LOG_W(
			"__ldl_src_play()->(*lso->play)() failed, err:'%d'.",
			err
		);
		return err;
	}
	else
	{
		LOG_I(
			"__ldl_src_play(): avs_media '%p' ok.", ldl
		);
	}

	return 0;
}


static int32_t
ldl_src_pause(media_src *src)
{
	return -EPERM;
}


static int32_t
ldl_src_lseek(media_src *src, uint32_t ts)
{
	return -EPERM;
}


static void
ldl_op_fuck(void *data, void *user_data)
{
	int32_t err;
	media_info msi;
	tp_block *tb = (tp_block*)data;
	ldl_src *ldl = (ldl_src*)tb->ls;

	switch (tb->action)
	{
	case ACTION_PROBE:
		media_info_init(&msi);
		err = __ldl_src_probe(ldl, &tb->mrl, &msi);
		media_src_fill_info((media_src*)ldl, &msi, err);
		media_info_clear(&msi);

#ifdef DEVICE_INTERFACE_TEST
		void ldl_interfaces_test(media_uri *mrl);
		ldl_interfaces_test(&tb->mrl);
#endif
		break;

	case ACTION_OPEN:
		AQUIRE_LOCK(ldl->ldl_lock);
		err = __ldl_src_open(ldl, &tb->mrl);
		RELEASE_LOCK(ldl->ldl_lock);
		media_src_open_end((media_src*)ldl, err);
		break;

	case ACTION_PLAY:
		AQUIRE_LOCK(ldl->ldl_lock);
		__ldl_src_play(ldl);
		RELEASE_LOCK(ldl->ldl_lock);
		break;

	case ACTION_CTRL:
		AQUIRE_LOCK(ldl->ldl_lock);
		__ldl_src_ctrl(ldl, 1, NULL);
		RELEASE_LOCK(ldl->ldl_lock);
		break;

	case ACTION_CLOSE:
		AQUIRE_LOCK(ldl->ldl_lock);
		__ldl_src_kill(ldl);
		RELEASE_LOCK(ldl->ldl_lock);
		break;

	default:
		break;
	}

    media_src_unref((media_src*)ldl);
    tr_free(tb, sizeof(*tb));
}


static media_src_ops ldl_src_ops =
{
	.init	= ldl_src_init,
	.fin	= ldl_src_finalize,
	.kill	= ldl_src_kill,
	.ctl    = ldl_src_ctrl, 
	.probe	= ldl_src_probe,
	.open	= ldl_src_open,
	.play	= ldl_src_play,
	.pause	= ldl_src_pause,
	.lseek	= ldl_src_lseek
};


ldl_src *ldl_src_alloc(void *u)
{
	return (ldl_src*)media_src_alloc(sizeof(ldl_src),
		&ldl_src_ops, u);
}


int32_t
ldl_src_consume(ldl_src *ldl, frame_t *frm)
{
	ld_src *lds = (ld_src*)ldl;
	int32_t type, stm_index;

	if (ldl->state != OPENED)
		return -EPERM;

	type = frm->hdr.type == FRAME_A ? ST_AUDIO : ST_VIDEO;
	stm_index = lds->idx[type];
	return media_src_produce((media_src *)ldl, stm_index, frm, 0);
}


#ifdef DEVICE_INTERFACE_TEST

void ldl_interfaces_test(media_uri *mrl)
{
	ldl_src *ldl;
	int32_t err;
	media_info msi;

	for (;;)
	{
		ldl = ldl_src_alloc(NULL);

		media_info_init(&msi);
		err = __ldl_src_probe(ldl, mrl, &msi);
		media_info_clear(&msi);

		if (err)
		{
			LOG_W(
				"ldl_interfaces_test()->__ldl_src_probe() failed, err:'%d'",
				err
			);
			goto test_failed;
		}

		err = __ldl_src_open(ldl, mrl);
		if (err)
		{
			LOG_W(
				"ldl_interfaces_test()->__ldl_src_open() failed, err:'%d'",
				err
			);
			goto test_failed;
		}

		__ldl_src_play(ldl);

		usleep(100*1000);
		__ldl_src_ctrl(ldl, 1, NULL);

		usleep(1000*1000);
		__ldl_src_kill(ldl);		

test_failed:
		media_src_kill_unref((media_src*)ldl);
		usleep(2000*1000);
	}
}


#endif

//:~ End
