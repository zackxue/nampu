#include <string.h>
#include "nmp_storage.h"

static gchar *fake_disk_name[] =
{
	"sda",
	"sdb",
	"sdc",
	"sdd",
	"sde",
	NULL
};

#define DISK_COUNT() (sizeof(fake_disk_name)/sizeof(gchar*) - 1)


typedef struct _NmpStorFakePrivate NmpStorFakePrivate;
struct _NmpStorFakePrivate
{
	gint	disk_mapping;
};


static NmpStorFakePrivate fake_storage_private = {0};

static gint
nmp_stor_fake_init(NmpStorage *s)
{
	NmpDisk *disk;
	gint index;

	s->dsk_count = DISK_COUNT();
	s->private_data = &fake_storage_private;

	for (index = 0; index < s->dsk_count; ++index)
	{
		disk = __nmp_disk_new();
		disk->disk_no = index;
		disk->total = 128*1024;
		disk->used = 0;
		disk->fs_type = FS_EXT3;

		strncpy(disk->label, fake_disk_name[index], STOR_LABEL_LEN - 1);
		__nmp_grp_add_disk_safe(s->idles, disk);
	}

	return 0;
}


static void
nmp_stor_fake_release(NmpStorage *s)
{//TODO
}


static gint
nmp_stor_fake_add_grp_disk(NmpDiskGrpStruct *grp, NmpDisk *disk)
{
	disk->group = grp->grp_no;
	return 0;
}


static gint
nmp_stor_fake_del_grp_disk(NmpDiskGrpStruct *grp, NmpDisk *disk)
{
	disk->group = 0;
	return 0;
}


static gint
nmp_stor_fake_format_progress(gint disk_no)
{
	static int progress;
	return (progress += 10);
}


static NmpStorageOps nmp_stor_fake_ops =
{
	.init			= nmp_stor_fake_init,
	.release		= nmp_stor_fake_release,
	.add_grp_disk	= nmp_stor_fake_add_grp_disk,
	.del_grp_disk	= nmp_stor_fake_del_grp_disk,
	.get_disk_format_progress = nmp_stor_fake_format_progress,
	.type			= "fake-storage"
};


void
nmp_register_fake_storage( void )
{
	nmp_register_storage_type(&nmp_stor_fake_ops);
}


//:~ End
