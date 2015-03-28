#ifndef __NMP_STORAGE_H__
#define __NMP_STORAGE_H__

#include <glib.h>

#define STOR_LABEL_LEN					32
#define ATTR_LEN						4

#define DISK_GROUP_COMMON \
	gint	grp_no; \
	gint	grp_type; \
	gchar	grp_label[STOR_LABEL_LEN]; \
	gint	disk_count;

enum
{
	FS_NONE = 0,
	FS_EXT4 = 1,
	FS_EXT3 = 2,
	FS_EXT2 = 3,
	FS_FAT32 = 4,
	FS_FAT16 = 5
};


enum
{
	DS_IDLE,
	DS_FMT,
	DS_INUSE
};


enum
{
	GRP_MAJOR,
	GRP_MINOR
};

typedef struct _NmpStorage NmpStorage;
typedef struct _NmpStorageOps NmpStorageOps;

typedef struct _NmpDisk NmpDisk;
struct _NmpDisk
{
	gint	disk_no;
	gint	group;
	gint	total;		/* total size: MB */
	gint	used;		/* used size: MB */	
	gint	fs_type;	/* file system type */
	gint	health;		/* disk health status, bits set */
	gint	status;		/* disk status: 0-idle, 1-formatting, 2:inuse*/
	gchar	label[STOR_LABEL_LEN];	/* disk label */	
};


typedef struct _NmpDiskGrp NmpDiskGrp;
struct _NmpDiskGrp
{
	DISK_GROUP_COMMON
	NmpDisk	disks[0];
};


typedef struct _NmpAttri NmpAttri;
struct _NmpAttri
{
	guint	data[ATTR_LEN];
};


typedef struct _NmpDiskGrpStruct NmpDiskGrpStruct;
struct _NmpDiskGrpStruct
{
	DISK_GROUP_COMMON
	NmpStorage *storage;
	GList	*disk_list;		/* disk list */
};


struct _NmpStorage
{
	gint	grp_count;
	gint	dsk_count;

	gint	busy;			/* storage busy */
	NmpDiskGrpStruct *idles;

	NmpDisk *formatted;	/* last disk formatted */
	gint    formatting;	/* in progress */

	GList	*grp_list;		/* group list */
	GMutex	*mutex;			/* lock */

	NmpStorageOps	*ops;

	void *private_data;				/* storage private data */
	void *user_data;				/* data from user layer */
};


struct _NmpStorageOps
{
	gint (*init)(NmpStorage *s);
	void (*release)(NmpStorage *s);

	gint (*create_grp)(NmpStorage *s, NmpDiskGrpStruct *grp, const gchar *label);
	gint (*delete_grp)(NmpStorage *s, gint grp_no);

	gint (*add_grp_disk)(NmpDiskGrpStruct *grp, NmpDisk *disk);
	gint (*del_grp_disk)(NmpDiskGrpStruct *grp, NmpDisk *disk);

	gint (*flush_grp_info)(NmpStorage *s, NmpDiskGrpStruct *grp);

	gint (*format_grp)(NmpDiskGrpStruct *grp);
	gint (*format_disk)(NmpDisk *disk);

	gint (*get_disk_format_progress)(gint disk_no);
	gint (*get_grp_format_progress)(gint grp_no);

	gint (*writeable)(NmpStorage *s, void *user_data);

	gint (*set_grp_label)(NmpDiskGrpStruct *grp, const gchar *label);
	gint (*get_grp_label)(NmpDiskGrpStruct *grp, gchar label[], gsize size);
	gint (*set_disk_label)(NmpDisk *disk, const gchar *label);
	gint (*get_disk_label)(NmpDisk *disk, gchar label[], gsize size);

	gint (*set_grp_attr)(NmpDiskGrpStruct *grp, NmpAttri *attr);			/* group attributes */
	NmpAttri *(*get_grp_attr)(NmpDiskGrpStruct *grp, gint *err);

	gint (*set_disk_attr)(NmpDisk *disk, NmpAttri *attr);					/* disk attributes */
	NmpAttri *(*get_disk_attr)(NmpDisk *disk, gint *err);

	gchar *type;
};


gint nmp_register_storage_type(NmpStorageOps *ops);

NmpStorage *nmp_create_storage(const gchar *s_type);

NmpDiskGrp *nmp_get_disk_grps(NmpStorage *storage, gint *count);
void nmp_put_disk_grps(NmpDiskGrp *grps, gint count);

NmpDiskGrp *nmp_get_grp_info(NmpStorage *storage, gint grp_id);
void nmp_put_grp_info(NmpDiskGrp *grp);

NmpDiskGrp *nmp_get_idle_disks(NmpStorage *storage);
void nmp_put_idle_disks(NmpDiskGrp *idle);

NmpDisk *nmp_get_format_disk(NmpStorage *storage, gint *progress);
void nmp_put_disk(NmpDisk *disk);

gint nmp_create_grp(NmpStorage *storage, gchar *label);
gint nmp_delete_grp(NmpStorage *storage, gint grp_no);

gint nmp_grp_add_disk(NmpStorage *storage, gint grp_no, gint disk_no);
gint nmp_grp_del_disk(NmpStorage *storage, gint grp_no, gint disk_no);


NmpDisk *__nmp_disk_new( void );
void __nmp_disk_free(NmpDisk *disk);
NmpDiskGrpStruct *__nmp_grp_new( void );
void __nmp_grp_free(NmpDiskGrpStruct *grp);
void __nmp_grp_add_disk_safe(NmpDiskGrpStruct *orig, NmpDisk *disk);
void __nmp_storage_add_grp_safe(NmpStorage *storage, NmpDiskGrpStruct *grp);

void __nmp_grp_clear(NmpDiskGrpStruct *grp);

#endif	/* __NMP_STORAGE_H__ */
