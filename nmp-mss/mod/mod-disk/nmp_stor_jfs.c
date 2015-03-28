#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "nmp_storage.h"
#include "stor_api.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"
#include "nmp_sysctl.h"


#define MAX_JFS_GROUPS				4
#define USELESS_GROUPID				0
#define STRING_LEN					512

typedef struct _NmpJfsGroup NmpJfsGroup;
struct _NmpJfsGroup
{
	NmpDiskGrpStruct *grp;
	gint used;
	gint disk_num;
};


typedef struct _NmpStorJfsPrivate NmpStorJfsPrivate;
struct _NmpStorJfsPrivate
{
	gint groups_count;

	NmpJfsGroup groups[MAX_JFS_GROUPS];
};


typedef struct _NmpJfsSysCtl NmpJfsSysCtl;
struct _NmpJfsSysCtl
{
	gint max_channels;
	gint flush_threads;
	gint channel_blocks;
	gint block_size;
	gchar mount_points_root[PATH_MAX];
};


typedef struct _NmpJfsFormatData NmpJfsFormatData;
struct _NmpJfsFormatData
{
	NmpStorage *s;
	gint disk_no;
	NmpDiskGrpStruct *from_grp;
	NmpDiskGrpStruct *to_grp;
};


static NmpStorJfsPrivate jfs_storage_private = {0};
static GThreadPool *format_threads_pool = NULL;
static NmpJfsSysCtl nmp_jfs_sysctl_parm =
{
	.max_channels = 256,
	.flush_threads = 4,
	.channel_blocks = 5,
	.block_size = 512 << 10,
	.mount_points_root = "/mnt/record"
};

#define GET_SYSCTL_PARM(name) (nmp_jfs_sysctl_parm.name)


static __inline__ off64_t
nmp_get_disk_size(const gchar *dev_name)
{/* dev_name: /dev/sda */
	off64_t size;
	gint fd;

	fd = open(dev_name, O_RDONLY);
	if (fd < 0)
	{
		return -errno;
	}

	size = lseek64(fd, 0, SEEK_END);
	if (size < 0)
	{
		size = -errno;
	}

	close(fd);
	return size;
}


static void
nmp_set_mss_disk_mark(gchar *dev_name)
{
	gchar query_buf[STRING_LEN] = {0};

	snprintf(query_buf, STRING_LEN - 1, 
		"system_generic_config set_mss_disk_mark %s", dev_name);

	nmp_print("%s", query_buf);
	system(query_buf);
}


static void
nmp_clear_mss_disk_mark()
{
	gchar query_buf[STRING_LEN] = {0};

	snprintf(query_buf, STRING_LEN - 1, 
		"system_generic_config clear_mss_disk_mark");

	nmp_print("%s", query_buf);
	system(query_buf);
}


static int
nmp_print_disks_info(void *user_arg, stor_scandisk_info_t *pscandisk_info)
{
    nmp_print("ma %d mi %d disk no %d target %s dev_name %s"
            , pscandisk_info->ma, pscandisk_info->mi
            , pscandisk_info->disk_no, pscandisk_info->ptarget
            , pscandisk_info->dev_name);

	nmp_set_mss_disk_mark(pscandisk_info->dev_name);
    stor_add_disk(NULL, 0 , STOR_FS_NONE, pscandisk_info);
	nmp_clear_mss_disk_mark();

    return STOR_SUCCESS;
}


static __inline__ void
nmp_init_jfs_lib( void )
{
    stor_init_cfg_t init_cfg;
    gint err;
    stor_scandisk_op_t scan_disk;

    memset(&init_cfg, 0, sizeof(init_cfg));
    init_cfg.max_ch = GET_SYSCTL_PARM(max_channels);
    init_cfg.group_num = GET_SYSCTL_PARM(flush_threads);
    init_cfg.ch_wblock_num = GET_SYSCTL_PARM(channel_blocks);
	init_cfg.pmount_point = GET_SYSCTL_PARM(mount_points_root);

	err = nmp_make_abs_path(GET_SYSCTL_PARM(mount_points_root));
	if (err)
	{
		nmp_warning(
    		"JFS mkdir mount-dir failed, err.");
    	FATAL_ERROR_EXIT;
	}

    err = stor_init(&init_cfg);
    if (err)
    {
    	nmp_warning(
    		"JFS stor_init() failed, err:%d.", err);
    	FATAL_ERROR_EXIT;
    }

	memset(&scan_disk, 0, sizeof(scan_disk));
    scan_disk.scan_disk_out_cb = nmp_print_disks_info;
    err = stor_scan_disk(&scan_disk);
    if (err)
    {
     	nmp_warning(
    		"JFS stor_scan_disk() failed, err:%d.", err);
    	FATAL_ERROR_EXIT;
    }

    err = stor_start();
    if (err)
    {
     	nmp_warning(
    		"JFS stor_start() failed, err:%d.", err);
    	FATAL_ERROR_EXIT;
    }
}


static gint
nmp_stor_jfs_foreach_idle_disk(struct _stor_group_query_disk_s *handle,
	stor_disk_info_t *disk_info)
{
	NmpStorage *s = (NmpStorage*)handle->user_arg;
	NmpStorJfsPrivate *jfs_pri = s->private_data;
	NmpDisk *disk;
	guint64 disk_size;

	if (disk_info->group_id)
		return 0;

	jfs_pri->groups[0].used = 1;
	++jfs_pri->groups[0].disk_num;

	disk = __nmp_disk_new();
	disk->disk_no = disk_info->disk_no;
	disk->total = disk_info->total_size >> 20;
	disk->used = (disk_info->total_size - disk_info->free_size) >> 20;
	disk->fs_type = STOR_FS_NONE;
	disk->status = DS_IDLE;

	if (!disk->total && (disk_size = nmp_get_disk_size(disk_info->dev_name)) > 0)
	{
		disk->total = disk_size >> 20;
	}

	strncpy(disk->label, disk_info->dev_name, STOR_LABEL_LEN - 1);
	__nmp_grp_add_disk_safe(s->idles, disk);

	nmp_print(
		"    disk:0x%x, %s, total:%dMB, used:%dMB.",
		disk->disk_no, disk->label, disk->total, disk->used);

	return 0;
}


static gint
nmp_stor_jfs_foreach_grps_disk(struct _stor_group_query_disk_s *handle,
	stor_disk_info_t *disk_info)
{
	NmpStorage *s = (NmpStorage*)handle->user_arg;
	NmpStorJfsPrivate *jfs_pri = s->private_data;
	NmpDisk *disk;
	NmpDiskGrpStruct *grp;

	if (!disk_info->group_id || disk_info->group_id >= MAX_JFS_GROUPS)
		return 0;

	if (!jfs_pri->groups[disk_info->group_id].used)
	{
		grp = __nmp_grp_new();
		grp->grp_no = disk_info->group_id;
		snprintf(grp->grp_label, STOR_LABEL_LEN, "group-%d", grp->grp_no);
		__nmp_storage_add_grp_safe(s, grp);
		jfs_pri->groups[disk_info->group_id].grp = grp;
		jfs_pri->groups[disk_info->group_id].used = 1;	
	}

	++jfs_pri->groups[disk_info->group_id].disk_num;
	grp = jfs_pri->groups[disk_info->group_id].grp;
	disk = __nmp_disk_new();
	disk->disk_no = disk_info->disk_no;
	disk->total = disk_info->total_size >> 20;
	disk->used = (disk_info->total_size - disk_info->free_size) >> 20;
	disk->fs_type = FS_EXT4;
	disk->status = DS_INUSE;

	strncpy(disk->label, disk_info->dev_name, STOR_LABEL_LEN - 1);
	__nmp_grp_add_disk_safe(grp, disk);

	nmp_print(
		"    group:%d, disk:0x%x, %s, total:%dMB, used:%dMB.",
		disk_info->group_id, disk->disk_no, disk->label, disk->total, disk->used);

	return 0;
}


static gint
nmp_stor_jfs_foreach_grp_disk(struct _stor_group_query_disk_s *handle,
	stor_disk_info_t *disk_info)
{
	NmpDiskGrpStruct *grp = (NmpDiskGrpStruct*)handle->user_arg;
	NmpStorage *s = grp->storage;
	NmpStorJfsPrivate *jfs_pri = s->private_data;
	NmpDisk *disk;
	guint64 disk_size;

	if (disk_info->group_id != grp->grp_no)
		return 0;

	++jfs_pri->groups[disk_info->group_id].disk_num;
	disk = __nmp_disk_new();
	disk->disk_no = disk_info->disk_no;
	disk->group = disk_info->group_id;
	disk->total = disk_info->total_size >> 20;
	disk->used = (disk_info->total_size - disk_info->free_size) >> 20;
	disk->fs_type = FS_EXT4;
	disk->status = disk_info->group_id ? DS_INUSE : DS_IDLE;

	if (!disk->total && (disk_size = nmp_get_disk_size(disk_info->dev_name)) > 0)
	{
		disk->total = disk_size >> 20;
	}

	strncpy(disk->label, disk_info->dev_name, STOR_LABEL_LEN - 1);
	__nmp_grp_add_disk_safe(grp, disk);

	nmp_print("    group:%d, disk:0x%x, %s, total:%dMB, used:%dMB.",
		disk_info->group_id, disk->disk_no, disk->label, disk->total, disk->used);

	return 0;
}


static __inline__ gint
nmp_stor_jfs_init_idle_disks(NmpStorage *s)
{
	stor_group_query_disk_t qd;
	gint err;

	qd.group_id = USELESS_GROUPID;
	qd.user_arg = s;
	qd.callback = nmp_stor_jfs_foreach_idle_disk;

	nmp_print("  Idle-group:");

	err = stor_group_query_disk(&qd);
	if (err)
	{
		nmp_warning("stor_group_query_disk() failed, err:%d!!", err);
		return -err;
	};

	return 0;
}


static __inline__ gint
nmp_stor_jfs_init_groups(NmpStorage *s)
{
	stor_group_query_disk_t qd;
	gint err;

	qd.group_id = USELESS_GROUPID;
	qd.user_arg = s;
	qd.callback = nmp_stor_jfs_foreach_grps_disk;

	nmp_print("  Using-disks:");

	err = stor_group_query_disk(&qd);
	if (err)
	{
		nmp_warning("stor_group_query_disk() failed, err:%d!!", err);
		return -err;
	};

	return 0;
}


static __inline__ gint
nmp_stor_jfs_refill_group(NmpStorage *s, NmpDiskGrpStruct *grp)
{
	stor_group_query_disk_t qd;
	gint err;

	qd.group_id = USELESS_GROUPID;
	qd.user_arg = grp;
	qd.callback = nmp_stor_jfs_foreach_grp_disk;

	nmp_print("  Refill-group %d:", grp->grp_no);

	err = stor_group_query_disk(&qd);
	if (err)
	{
		nmp_warning("stor_group_query_disk() failed, err:%d!!", err);
		return -err;
	};

	return 0;
}


static gint
nmp_stor_jfs_init(NmpStorage *s)
{
	gint err;

	s->private_data = &jfs_storage_private;
	nmp_init_jfs_lib();

	nmp_print(
		"Jfs-storage report:"
	);

	err = nmp_stor_jfs_init_idle_disks(s);
	if (G_UNLIKELY(err))
		return err;

	err = nmp_stor_jfs_init_groups(s);
	if (G_UNLIKELY(err))
		return err;

	return 0;
}


static void
nmp_stor_jfs_release(NmpStorage *s)
{//TODO
}


static gint
nmp_stor_jfs_create_group(NmpStorage *s, NmpDiskGrpStruct *grp,
	const gchar *label)
{
	NmpStorJfsPrivate *jfs_pri = s->private_data;
	gint index;

	G_ASSERT(s != NULL);

	for (index = 1; index < MAX_JFS_GROUPS; ++index)
	{
		if (!jfs_pri->groups[index].used)
		{
			++jfs_pri->groups_count;
			jfs_pri->groups[index].used = 1;
			jfs_pri->groups[index].disk_num = 0;
			return index;
		}
	}

	return -ENOMEM;
}


static gint
nmp_stor_jfs_delete_group(NmpStorage *s, gint grp_no)
{
	NmpStorJfsPrivate *jfs_pri = s->private_data;
	G_ASSERT(s != NULL);

	if (grp_no >= MAX_JFS_GROUPS || grp_no <= 0)
	{
		return -EINVAL;
	}

	if (!jfs_pri->groups[grp_no].used)
	{
		BUG();
	}

	if (jfs_pri->groups[grp_no].disk_num > 0)
	{
		return -EINVAL;
	}

	jfs_pri->groups[grp_no].used = 0;
	--jfs_pri->groups_count;
	return 0;
}


static void
nmp_stor_jfs_formatting_task_func(gpointer data, gpointer user_data)
{
	NmpJfsFormatData *fdata = (NmpJfsFormatData*)data;
	gint err;
	NmpStorage *s;
	NmpDiskGrpStruct *from, *to;
	NmpStorJfsPrivate *jfs_pri;

	s = fdata->s;
	jfs_pri = s->private_data;
	from = fdata->from_grp;
	to = fdata->to_grp;

	err = stor_format(from->grp_no, to->grp_no, fdata->disk_no,
		STOR_FS_EXT4, NULL);
	if (err)
	{
		nmp_warning(
			"Format disk:%d grp:%d failed, err:%d",
			fdata->disk_no,
			to->grp_no,
			err
		);
	}
	else
	{
		nmp_print(
			"Format disk:%d grp:%d ok.",
			fdata->disk_no,
			to->grp_no
		);
	}

	g_mutex_lock(s->mutex);
	__nmp_grp_clear(to);
	nmp_stor_jfs_refill_group(s, to);
	jfs_pri->groups[to->grp_no].disk_num = to->disk_count;

	__nmp_grp_clear(from);
	nmp_stor_jfs_refill_group(s, from);
	jfs_pri->groups[from->grp_no].disk_num = from->disk_count;

	s->busy = FALSE;
	s->formatting = 0;
	g_mutex_unlock(s->mutex);

	g_free(fdata);
}


static __inline__ void
nmp_stor_jfs_formatting_prepare( void )
{
	static gsize init_format_threads = FALSE;

	if (g_once_init_enter(&init_format_threads))
	{
		format_threads_pool = g_thread_pool_new(
			nmp_stor_jfs_formatting_task_func,
			NULL,
			-1,
			FALSE,
			NULL);

		g_once_init_leave(&init_format_threads, TRUE);
	}
}


static gint
nmp_stor_jfs_add_grp_disk(NmpDiskGrpStruct *grp, NmpDisk *disk)
{
	NmpStorJfsPrivate *nmp_priv;
	NmpJfsFormatData *fdata;
	NmpStorage *s;
	nmp_stor_jfs_formatting_prepare();

	s = grp->storage;
	BUG_ON(!s);
	s->busy = TRUE;

	fdata = g_new0(NmpJfsFormatData, 1);
	fdata->s = s;
	fdata->disk_no = disk->disk_no;
	fdata->from_grp = s->idles;
	fdata->to_grp = grp;
	disk->group = grp->grp_no;
	disk->status = DS_FMT;

	nmp_priv = (NmpStorJfsPrivate*)s->private_data;
	BUG_ON(grp->grp_no >= MAX_JFS_GROUPS);

	if (s->formatted)
		g_free(s->formatted);
	s->formatted = g_new0(NmpDisk, 1);
	s->formatting = 1;
	memcpy(s->formatted, disk, sizeof(*disk));

	g_thread_pool_push(format_threads_pool, fdata, NULL);

	return 0;
}


static gint
nmp_stor_jfs_del_grp_disk(NmpDiskGrpStruct *grp, NmpDisk *disk)
{
	NmpStorJfsPrivate *nmp_priv;
	NmpJfsFormatData *fdata;
	NmpStorage *s;
	nmp_stor_jfs_formatting_prepare();

	s = grp->storage;
	BUG_ON(!s);
	s->busy = TRUE;

	fdata = g_new0(NmpJfsFormatData, 1);
	fdata->s = s;
	fdata->disk_no = disk->disk_no;
	fdata->from_grp = grp;
	fdata->to_grp = s->idles;
	disk->group = 0;
	disk->status = DS_FMT;

	nmp_priv = (NmpStorJfsPrivate*)s->private_data;
	BUG_ON(grp->grp_no >= MAX_JFS_GROUPS);

	if (s->formatted)
		g_free(s->formatted);
	s->formatted = g_new0(NmpDisk, 1);
	s->formatting = 1;
	memcpy(s->formatted, disk, sizeof(*disk));

	g_thread_pool_push(format_threads_pool, fdata, NULL);

	return 0;
}


static gint
nmp_stor_jfs_flush_grp_info(NmpStorage *s, NmpDiskGrpStruct *grp)
{
	NmpStorJfsPrivate *jfs_pri = s->private_data;

	__nmp_grp_clear(grp);
	jfs_pri->groups[grp->grp_no].disk_num = 0;
	nmp_stor_jfs_refill_group(s, grp);
	return 0;
}


static gint
nmp_stor_jfs_format_progress(gint disk_no)
{
	return stor_get_format_progress(disk_no);
}


static NmpStorageOps nmp_stor_jfs_ops =
{
	.init			= nmp_stor_jfs_init,
	.release		= nmp_stor_jfs_release,
	.create_grp     = nmp_stor_jfs_create_group,
	.delete_grp     = nmp_stor_jfs_delete_group,
	.add_grp_disk	= nmp_stor_jfs_add_grp_disk,
	.del_grp_disk	= nmp_stor_jfs_del_grp_disk,
	.flush_grp_info = nmp_stor_jfs_flush_grp_info,
	.get_disk_format_progress = nmp_stor_jfs_format_progress,
	.type			= "jfs-storage"
};


void
nmp_register_jfs_storage( void )
{
	nmp_register_storage_type(&nmp_stor_jfs_ops);
}

//:~ End
