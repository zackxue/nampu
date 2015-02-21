#include "alloc.h"
#include "media_filter.h"


media_filter *
media_filter_new( void )
{
	media_filter *mf;

	mf = tr_alloc0(sizeof(*mf));
	return mf;
}


static __inline__ void
fin_all_filters(media_filter *mf)
{
	int32_t index;
	filter *f;

	for (index = 0; index < FILTER_MAX; ++index)
	{
		f = mf->flts[index];
		if (f)
		{
			obj_unref(f);
		}
	}
}


void
media_filter_release(media_filter *mf)
{
	fin_all_filters(mf);
	tr_free(mf, sizeof(*mf));
}


static __inline__ void
fin_self(filter *f){}


static __inline__ void
on_obj_fin(obj *p)
{
	filter *f = (filter*)p;

	if (f->ops && f->ops->fin)
	{
		(*f->ops->fin)(f);
	}
	fin_self(f);
}


static __inline__ int32_t
filter_cal_size(filter *f, void *data, uint32_t size)
{
	int32_t real_size = MAX_FRAME_SIZE;

	if (f->ops && f->ops->cal)
	{
		real_size = (*f->ops->cal)(f, data, size);
	}
	return real_size;
}


static __inline__ int32_t
filter_fill(filter *f, void *data, uint32_t size)
{
	int32_t err = -EOPNOTSUPP;

	if (f->ops && f->ops->fill)
	{
		err = (*f->ops->fill)(f, data, size);
	}
	return err;
}


static __inline__ int32_t
filter_pull(filter *f, msg **o)
{
	int32_t err = -EOPNOTSUPP;

	if (f->ops && f->ops->pull)
	{
		err = (*f->ops->pull)(f, o);
	}
	return err;
}


static __inline__ filter *
media_filter_find(media_filter *mf, int32_t id)
{
	filter *f = NULL;

	if (id >= 0 && id < FILTER_MAX)
	{
		f = mf->flts[id];
	}

	return f;
}


int32_t
media_filter_register(media_filter *mf, int32_t id,
	filter_ops *ops)
{
	filter *f;
	int32_t err;

	if (!ops || ops->filter_size < sizeof(*f))
		return -EINVAL;

	if (id < 0 || id >= FILTER_MAX)
		return -EINVAL;

	if (media_filter_find(mf, id))
		return -EEXIST;

	f = (filter*)obj_new(ops->filter_size, on_obj_fin);
	if (f)
	{
		f->id = id;
		f->ops = ops;

		if (ops->init)
		{
			err = (*ops->init)(f);
			if (err)
			{
				f->ops = NULL;
				obj_unref(f);
				return err;
			}	
		}

		mf->flts[id] = f;
	}

	return 0;
}


int32_t
media_filter_cal_size(media_filter *mf, int32_t id,
	void *data, uint32_t size)
{
	filter *f = media_filter_find(mf, id);
	if (f)
	{
		return filter_cal_size(f, data, size);
	}
	return -EINVAL;	
}


int32_t
media_filter_fill(media_filter *mf, int32_t id,
	void *data, uint32_t size)
{
	filter *f = media_filter_find(mf, id);
	if (f)
	{
		return filter_fill(f, data, size);
	}
	return -EINVAL;
}


int32_t
media_filter_pull(media_filter *mf, int32_t id, msg **o)
{
	filter *f = media_filter_find(mf, id);
	if (f)
	{
		return filter_pull(f, o);
	}
	return -EINVAL;
}


//:~ End
