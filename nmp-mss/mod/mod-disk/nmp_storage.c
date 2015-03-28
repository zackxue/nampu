#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_mods.h"
#include "nmp_storage.h"

//TODO:
static NmpStorageOps *registered_ops = NULL;

static __inline__ gint
nmp_mode_perm( void )
{
	gint mode;

	mode = nmp_mss_get_running_mode();
	if (mode == MSS_RUNNING_MODE_REC)
		return -EPERM;
	return 0;
}


gint
nmp_register_storage_type(NmpStorageOps *ops)
{
	G_ASSERT(ops != NULL);

	if (!ops->type)
		return -E_INVAL;

	registered_ops = ops;
	return 0;
}


static __inline__ NmpStorage*
__nmp_new_storage( void )
{
	NmpStorage *s;

	s = g_new0(NmpStorage, 1);
	s->idles = __nmp_grp_new();
	s->idles->storage = s;
	s->mutex = g_mutex_new();
	return s;
}


static __inline__ void
__nmp_delete_storage(NmpStorage *s)
{
	g_free(s);
}


static __inline__ NmpStorageOps*
__nmp_get_storage_type(const gchar *s_type)
{
	return registered_ops;
}


static __inline__ gint
__nmp_init_storage(NmpStorage *s, NmpStorageOps *ops)
{
	gint ret = -E_INVAL;

	if (ops->init)
	{
		s->ops = ops;
		ret = (*ops->init)(s);
	}

	return ret;
}


NmpStorage *
nmp_create_storage(const gchar *s_type)
{
	NmpStorageOps *ops;
	NmpStorage *s;

	ops = __nmp_get_storage_type(s_type);
	if (G_UNLIKELY(!ops))
		return NULL;

	s = __nmp_new_storage();
	if (G_LIKELY(s))
	{
		if (__nmp_init_storage(s, ops))
		{
			__nmp_delete_storage(s);
			return NULL;
		}
	}

	return s;
}


NmpDisk *
__nmp_disk_new( void )
{
	NmpDisk *disk;

	disk = g_new0(NmpDisk, 1);
	return disk;
}


void
__nmp_disk_free(NmpDisk *disk)
{
	g_free(disk);
}


void
__nmp_grp_clear(NmpDiskGrpStruct *grp)
{
	g_list_foreach(grp->disk_list, (GFunc)__nmp_disk_free, NULL);
	g_list_free(grp->disk_list);
	grp->disk_list = NULL;
	grp->disk_count = 0;
}


NmpDiskGrp *
__nmp_get_disk_grps(NmpStorage *storage, gint *count)
{
	NmpDiskGrp *arr;
	gint index;
	GList *list;
	NmpDiskGrp *orig;

	if (!storage->grp_count)
	{
		*count = 0;
		return NULL;
	}

	arr = g_new0(NmpDiskGrp, storage->grp_count);
	list = storage->grp_list;

	for (index = 0; index < storage->grp_count; ++index)
	{
		BUG_ON(!list);
		orig = (NmpDiskGrp*)list->data;
		memcpy(arr + index, orig, sizeof(NmpDiskGrp));

		list = g_list_next(list);
	}

	*count = storage->grp_count;
	return arr;
}


static gint
__nmp_disk_equal(gconstpointer a, gconstpointer b)
{
	NmpDisk *disk = (NmpDisk*)a;

	if (disk->disk_no == (gint)b)
		return 0;

	return 1;
}


static __inline__ NmpDisk *
__nmp_grp_remove_disk(NmpDiskGrpStruct *orig, gint disk_no)
{
	NmpDisk *disk = NULL;
	GList *list;

	G_ASSERT(orig != NULL);

	list = g_list_find_custom(orig->disk_list, (gconstpointer)disk_no,
		__nmp_disk_equal);
	if (list)
	{
		disk = (NmpDisk*)list->data;
		orig->disk_list = g_list_delete_link(orig->disk_list, list);
		--orig->disk_count;
	}

	return disk;
}


void
__nmp_grp_add_disk_safe(NmpDiskGrpStruct *orig, NmpDisk *disk)
{
	G_ASSERT(orig != NULL && disk != NULL);

	orig->disk_list = g_list_append(orig->disk_list, disk);
	++orig->disk_count;
}


static gint
__nmp_grp_equal(gconstpointer a, gconstpointer b)
{
	NmpDiskGrpStruct *grp = (NmpDiskGrpStruct*)a;

	if (grp->grp_no == (gint)b)
		return 0;

	return 1;
}


static __inline__ NmpDiskGrp *
__nmp_copy_grp_info(NmpDiskGrpStruct *orig)
{
	NmpDiskGrp *grp;
	NmpDisk *disk;
	gint index;
	GList *list;

	grp = g_malloc(sizeof(NmpDiskGrp) + (orig->disk_count * sizeof(NmpDisk)));

	memcpy(grp, orig, sizeof(NmpDiskGrp));
	list = orig->disk_list;

	for (index = 0; index < grp->disk_count; ++index)
	{
		BUG_ON(!list);
		disk = (NmpDisk*)list->data;
		memcpy(&grp->disks[index], disk, sizeof(NmpDisk));
		list = g_list_next(list);
	}

	return grp;	
}


static __inline__ NmpDiskGrpStruct*
__nmp_find_grp(NmpStorage *storage, gint grp_id)
{
	GList *list;

	if (!storage->grp_count)
		return NULL;

	list = g_list_find_custom(storage->grp_list, (gconstpointer)grp_id,
		__nmp_grp_equal);
	if (!list)
		return NULL;

	return (NmpDiskGrpStruct*)list->data;
}

                                                 
static __inline__ NmpDiskGrp *
__nmp_get_grp_info(NmpStorage *storage, gint grp_id)
{
	NmpDiskGrpStruct *orig;
	gint err;

	orig = __nmp_find_grp(storage, grp_id);
	if (G_UNLIKELY(!orig))
		return NULL;

	if (storage->ops->flush_grp_info)
	{
		err = (*storage->ops->flush_grp_info)(storage, orig);
		if (err)
			return NULL;
	}

	return __nmp_copy_grp_info(orig);
}


NmpDiskGrp *
nmp_get_disk_grps(NmpStorage *storage, gint *count)
{
	NmpDiskGrp *grps;

	if (G_UNLIKELY(!storage || !count))
		return NULL;

	g_mutex_lock(storage->mutex);
	grps = __nmp_get_disk_grps(storage, count);
	g_mutex_unlock(storage->mutex);

	return grps;
}


void
nmp_put_disk_grps(NmpDiskGrp *grps, gint count)
{
	g_free(grps);
}


NmpDiskGrp *
nmp_get_grp_info(NmpStorage *storage, gint grp_id)
{
	NmpDiskGrp *grps;

	if (G_UNLIKELY(!storage || !grp_id))
		return NULL;

	g_mutex_lock(storage->mutex);
	grps = __nmp_get_grp_info(storage, grp_id);
	g_mutex_unlock(storage->mutex);

	return grps;
}


void
nmp_put_grp_info(NmpDiskGrp *grp)
{
	g_free(grp);
}


NmpDiskGrp *
nmp_get_idle_disks(NmpStorage *storage)
{
	NmpDiskGrp *idle;

	if (G_UNLIKELY(!storage))
		return NULL;

	g_mutex_lock(storage->mutex);
	idle = 	__nmp_copy_grp_info(storage->idles);
	g_mutex_unlock(storage->mutex);

	return idle;
}


void
nmp_put_idle_disks(NmpDiskGrp *idle)
{
	nmp_put_grp_info(idle);
}


NmpDiskGrpStruct *
__nmp_grp_new( void )
{
	NmpDiskGrpStruct *grp;

	grp = g_new0(NmpDiskGrpStruct, 1);
	return grp;
}


void
__nmp_grp_free(NmpDiskGrpStruct *grp)
{
	g_free(grp);
}


void
__nmp_storage_add_grp_safe(NmpStorage *storage, NmpDiskGrpStruct *grp)
{
	storage->grp_list = g_list_prepend(storage->grp_list, grp);
	++storage->grp_count;
	grp->storage = storage;
}


static __inline__ gint
__nmp_add_grp_to_storage(NmpStorage *storage, NmpDiskGrpStruct *grp)
{
	NmpDiskGrpStruct *old;

	old =__nmp_find_grp(storage, grp->grp_no);
	if (old)
		return -EEXIST;

	__nmp_storage_add_grp_safe(storage, grp);

	return 0;
}


static __inline__ gint
__nmp_create_grp(NmpStorage *storage, gchar *label)
{
	static gint grp_no_generator = 1;
	NmpDiskGrpStruct *grp;
	gint grp_no, err = -E_INVAL;

	grp = __nmp_grp_new();
	if (grp)
	{
		if (storage->ops->create_grp)
			grp_no = (*storage->ops->create_grp)(storage, grp, label);
		else
		{
			grp_no = grp_no_generator++;
			strncpy(grp->grp_label, label, STOR_LABEL_LEN - 1);
			grp->grp_label[STOR_LABEL_LEN - 1] = 0;
		}
	}

	if (grp_no < 0)
	{
		__nmp_grp_free(grp);
		return grp_no;
	}

	grp->grp_no = grp_no;

	if ((err = __nmp_add_grp_to_storage(storage, grp)))
	{
		if (storage->ops->delete_grp)
			(*storage->ops->delete_grp)(storage, grp_no);

		__nmp_grp_free(grp);
		grp_no = err;
	}

	return grp_no;
}


gint
nmp_create_grp(NmpStorage *storage, gchar *label)
{
	gint ret;

	if (!storage || !storage->ops)
		return -E_INVAL;

	if (nmp_mode_perm())
		return -EPERM;

	g_mutex_lock(storage->mutex);
	ret = __nmp_create_grp(storage, label);
	g_mutex_unlock(storage->mutex);

	return ret;
}


static __inline__ void
__nmp_delete_grp_from_storage(NmpStorage *storage, NmpDiskGrpStruct *grp)
{
	GList *list;

	list = g_list_find(storage->grp_list, grp);
	if (!list)
		return;

	BUG_ON(grp->disk_list);
	storage->grp_list = g_list_delete_link(storage->grp_list, list);
	__nmp_grp_free(grp);
	--storage->grp_count;	
}


static __inline__ gint
__nmp_delete_grp(NmpStorage *storage, gint grp_no)
{
	NmpDiskGrpStruct *grp;
	gint err = 0;

	grp = __nmp_find_grp(storage, grp_no);
	if (!grp)
		return -E_INVAL;

	if (grp->disk_count > 0)
		return -EBUSY;

	if (storage->ops->delete_grp)
		err = (*storage->ops->delete_grp)(storage, grp_no);

	if (!err)
		__nmp_delete_grp_from_storage(storage, grp);

	return err;
}


gint
nmp_delete_grp(NmpStorage *storage, gint grp_no)
{
	gint err = 0;

	if (G_UNLIKELY(!storage || !storage->ops))
		return -E_INVAL;

	if (nmp_mode_perm())
		return -EPERM;

	g_mutex_lock(storage->mutex);
	err = __nmp_delete_grp(storage, grp_no);
	g_mutex_unlock(storage->mutex);

	return err;
}


static __inline__ gint
__nmp_grp_add_disk(NmpStorage *storage, gint grp_no, gint disk_no)
{
	NmpDiskGrpStruct *grp;
	NmpDisk *disk;
	gint err;

	if (storage->busy)
		return -E_AGAIN;

	if (!storage->ops || !storage->ops->add_grp_disk)
		return -E_INVAL;

	grp = __nmp_find_grp(storage, grp_no);
	if (!grp)
		return -E_INVAL;

	disk = __nmp_grp_remove_disk(storage->idles, disk_no);
	if (!disk)
		return -E_INVAL;

	err = (*storage->ops->add_grp_disk)(grp, disk);
	if (err)
	{
		__nmp_grp_add_disk_safe(storage->idles, disk);
		return err;
	}

	__nmp_grp_add_disk_safe(grp, disk);
	return 0;
}


gint
nmp_grp_add_disk(NmpStorage *storage, gint grp_no, gint disk_no)
{
	gint ret;

	if (G_UNLIKELY(!storage))
		return -E_INVAL;

	if (nmp_mode_perm())
		return -EPERM;

	g_mutex_lock(storage->mutex);
	ret = __nmp_grp_add_disk(storage, grp_no, disk_no);
	g_mutex_unlock(storage->mutex);

	return ret;
}


static __inline__ gint
__nmp_grp_del_disk(NmpStorage *storage, gint grp_no, gint disk_no)
{
	NmpDiskGrpStruct *grp;
	NmpDisk *disk;
	gint err;

	if (storage->busy)
		return -E_AGAIN;

	if (!storage->ops || !storage->ops->del_grp_disk)
		return -E_INVAL;

	grp = __nmp_find_grp(storage, grp_no);
	if (!grp)
		return -E_INVAL;

	disk = __nmp_grp_remove_disk(grp, disk_no);
	if (!disk)
		return -E_INVAL;

	err = (*storage->ops->del_grp_disk)(grp, disk);
	if (err)
	{
		__nmp_grp_add_disk_safe(grp, disk);
		return err;
	}

	__nmp_grp_add_disk_safe(storage->idles, disk);
	return 0;
}


gint
nmp_grp_del_disk(NmpStorage *storage, gint grp_no, gint disk_no)
{
	gint ret;

	if (G_UNLIKELY(!storage))
		return -E_INVAL;

	if (nmp_mode_perm())
		return -EPERM;

	g_mutex_lock(storage->mutex);
	ret = __nmp_grp_del_disk(storage, grp_no, disk_no);
	g_mutex_unlock(storage->mutex);

	return ret;
}


NmpDisk *
nmp_get_format_disk(NmpStorage *storage, gint *progress)
{
	NmpDisk *disk = NULL;
	*progress = 100;

	g_mutex_lock(storage->mutex);
	if (storage->formatted && storage->formatting)
	{
		disk = g_new0(NmpDisk, 1);
		memcpy(disk, storage->formatted, sizeof(*disk));

		if (storage->ops->get_disk_format_progress)
		{
			*progress = (*storage->ops->get_disk_format_progress)(disk->disk_no);
		}
	}
	g_mutex_unlock(storage->mutex);

	return disk;
}


void
nmp_put_disk(NmpDisk *disk)
{
	g_free(disk);
}


//:~ End
