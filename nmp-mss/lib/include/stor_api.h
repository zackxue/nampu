#ifndef __stor_api__
#define __stor_api__ 




#define STOR_BLOCK_ALIGN_SIZE   (2*1024)

#define STOR_REC_GROUP_NUM   (16)    

#define STOR_DISK_MAX_PART_NUM  (4)

#define STOR_DISK_MAX_CH_NUM (1000)   /* 磁盘最大允许存储通道数 */
#define STOR_CH_NAME_LEN     (64)
#define STOR_DIR_NAME_LEN    (128)
#define STOR_DISK_USE_INFO_LEN  (256)


#define STOR_DISK_NOPART     (-1)  /* part_no */

#define STOR_CB_EXIT         (1)
#define STOR_CB_CONTINUE     (0)   

#define STOR_SUCCESS           (0)
#define STOR_ERR_BASE               (0x80000000+0x1000000)

#define STOR_PREA_BASE              (STOR_ERR_BASE + 0x100000)
#define STOR_ERR_PREA_READ_EOF           (STOR_PREA_BASE+1) /* 读到文件段结尾 */
#define STOR_ERR_PREA_WRITE_NO_CACHE     (STOR_PREA_BASE+2) /* write cache 空间不足 */
#define STOR_ERR_PREA_WRITE_DIS_OVER     (STOR_PREA_BASE+3) /* write 禁止覆盖 */
#define STOR_ERR_PREA_WRITE_ARG_ERR     (STOR_PREA_BASE+4) /* write 禁止覆盖 */


#define STOR_COMM_BASE              (STOR_ERR_BASE + 0x200000)
#define STOR_ERR_PARAM              (STOR_COMM_BASE+1)
#define STOR_ERR_DISK_NODE_READD    (STOR_COMM_BASE+2)
#define STOR_ERR_DISK_NO_FIND       (STOR_COMM_BASE+3)
#define STOR_ERR_DISK_BUSY          (STOR_COMM_BASE+4)
#define STOR_ERR_DISK_ORDER         (STOR_COMM_BASE+5)
#define STOR_ERR_DISK_INDX_IO       (STOR_COMM_BASE+6)
#define STOR_ERR_DISK_LOG_IO        (STOR_COMM_BASE+7)
#define STOR_ERR_DISK_DATA_IO       (STOR_COMM_BASE+8)
#define STOR_ERR_ALLOC_CH           (STOR_COMM_BASE+9)
#define STOR_ERR_ALL_DISK_ERR       (STOR_COMM_BASE+10)
#define STOR_ERR_NO_FS_TYPE         (STOR_COMM_BASE+11)

#define STOR_ERR_ARG_ERR            (STOR_COMM_BASE+100)                


#define STOR_GROUP_BASE                (STOR_ERR_BASE + 0x200000)
#define STOR_ERR_GROUP_DISK_NO_FORMAT  (STOR_GROUP_BASE+1)
#define STOR_ERR_GROUP_DISK_BUSY       (STOR_GROUP_BASE+2)
#define STOR_ERR_GROUP_NO_CUR_DISK     (STOR_GROUP_BASE+3)
#define STOR_GROUP_DISK_NO_GROUP         (2) 

#define STOR_SYS_IO_SCHEDULER       "noop"  /* noop anticipatory deadline cfq */

#define STOR_FILE_SYS_VER        "SDK_FSV002"


typedef enum _STOR_INFO_E
{
    STOR_INFO_ALARM_GROUP_SPACE = 1
   , STOR_INFO_ALARM_GROUP_OVER = 2
}STOR_INFO_E;


typedef struct _stor_info_alarm_group_space_s
{
    int group_id;
    long long total_size;
    long long diff_end;
}stor_info_alarm_group_space_t;

typedef struct _stor_info_alarm_group_over_s
{  
    int group_id;
    int is_over;
    long long total_size;
    long long diff_end;
}stor_info_alarm_group_over_t;


/* -----------------磁盘组ID-------------------------- */
typedef enum _STOR_GROUP_ID {
      STOR_GROUP_ID_NONE    = 0         /* 空闲磁盘组 */
    , STOR_GROUP_ID_REC_1   = 1         /* 录像组 */
    , STOR_GROUP_ID_BAK_1   = 16       /* 录像备份组 */
    , STOR_GROUP_ID_BAD     = 20
    , STOR_GROUP_ID_ALL     = 255
}STOR_GROUP_ID_E;

    /* 磁盘总线类型*/
typedef enum _STOR_DISK_BUS_CLASS {     
    DISK_BUS_CLASS_UNKNOW = 0
    , DISK_BUS_CLASS_PCI  = 1
    , DISK_BUS_CLASS_USB  = 2
    , DISK_BUS_CLASS_IPSAN = 3
    , DISK_BUS_CLASS_MMC  = 4
    , DISK_BUS_CLASS_NFS  = 5
}STOR_DISK_BUS_CLASS_E;


typedef struct _stor_scandisk_info_s
{
    char *dev_name;
    STOR_DISK_BUS_CLASS_E bus_class;
    int hostno;
    int disk_no;
    int ma;
    int mi;
    int part_num;
    char *ptarget;
}stor_scandisk_info_t;


typedef struct _stor_scandisk_op_s
{
    char **ppdisk_feature;
    void *user_arg;
    int (*scan_disk_out_cb)(void *user_arg, stor_scandisk_info_t *pscandisk_info);
    //char *dev_name, STOR_DISK_BUS_CLASS_E bus_class
            //, int hostno, int disk_no, int ma, int mi, int part_num, char *ptarget);
    //int (*user_filter_disk_cb)(char *dev_name, STOR_DISK_BUS_CLASS_E bus_class);
}stor_scandisk_op_t;


/* 磁盘格式类型 */
typedef enum _STOR_FS_TYPE {
       STOR_FS_NONE    = 0
     , STOR_FS_EXT4    = 1
     , STOR_FS_FAT32   = 2
     , STOR_FS_FILE    = 3
     , STOR_FS_BLOCK   = 4
}STOR_FS_TYPE_E;


/* ------------------------------------------- */
/*  存储帧类型 */
typedef enum _STOR_FRAME_TYPE {
      STOR_FRAME_VIDEO_I = 1
    , STOR_FRAME_MSG_EV1 = 100
}STOR_FRAME_TYPE_E;


typedef struct _stor_frame_info_s {
    char type;              /* 帧类型 : STOR_FRAME_TYPE_E*/
    char is_windex;         /* 写关键帧索引 */
    char res[2];
    int  time;              /* 帧时间 */
    int  tags;              /* 用户标注 */
    int  size;              /* 写数据尺寸 */
}stor_frame_info_t;

typedef void stor_fwrite_handle_t;

/* ----------存储数据帧索引格式--------------------------- */
typedef struct _stor_frame_sindex_s
{
    char type;
    char res[3];
    int  time;
    int  tags;   /* 用户标注 */
    int  offset; /* 文件内偏移 */   
    int  size;
    int  res2[3];
}stor_frame_sindex_t;


/* ------------------------------------------- */
//init
#define STOR_MIN_RWBLOCK_SIZE   (256*1024)  
#define STOR_MAX_RWBLOCK_SIZE   (1*1024*1024) /* 4k align */
#define SOTR_RWBLOCK_ALIGN_SIZE (4*1024)
typedef struct _stor_init_cfg_s
{
    int max_ch;     /* 最大磁盘通道数 */
    int group_num;  /* 同时写盘磁盘数 刷盘线程数 */
    int ch_wblock_num;  /* 通道写块数 3-10 0.5Mblk */
//    int rwblock_size;   /* 读写块尺寸 目前默认512k */
    char *pdev_id;
    char *pmount_point;
    void *info_arg; 
    int (*stor_info_cb)(void *info_arg, int type, void *info);  /* 存储信息回调 如 磁盘告警错误等 */
    void *free_blk_user_arg;    
    int (*stor_free_blk_cb)(void *free_blk_user_arg);
}stor_init_cfg_t;
int stor_init(stor_init_cfg_t *pinit_cfg);
int stor_start();

struct _stor_disk_use_info_s;
/* 磁盘格式化 */
int stor_format(int group_id, int add_group_id, int disk_no
        , int fs_type, struct _stor_disk_use_info_s *pdisk_use_info);

int stor_get_format_progress(int disk_no);
int stor_set_format_progress(int format_progress);
int stor_move_group(int old_group_id, int add_group_id, int disk_no);  /* free ->use */

/* 设置用户的日志信息输出 */
int stor_log_set(int level, void (*stor_log_fn)(char *pstr));

/* 组磁盘查询返回信息 */
typedef struct _stor_disk_info_s {
   int disk_no;                 /* 磁盘号       : */
   int part_no;                 /* 分区号       : 无分区-1 STOR_DISK_NOPART */ 
   int group_id;                /* 组Id         :STOR_GROUP_ID_E */
   int is_format;
   int disk_id;
   int bus_class;               /* 磁盘总线类型 :STOR_DISK_BUS_CLASS_E*/
   int disk_err;
   //int host_no;                 /* 磁盘主机站号 驱动初始时决定设备确定后磁盘基本确定可依据此信息来判断磁盘位置 此位站位可挂多磁盘进一部提供信息 */
   char target[8];                /* 磁盘主机站号 驱动初始时决定设备确定后磁盘基本确定可依据此信息来判断磁盘物理位置 */  
   char dev_name[16];
   int fs_type;                 /* 格式类型     :STOR_FS_TYPE_E*/
   int use_count;
   int over_times;     
   long long free_size;         /* 自由空间     :*/
   long long total_size;        /* 总空间       :*/
}stor_disk_info_t;



/* 磁盘组查询 */
typedef struct _stor_group_query_disk_s {
    int  group_id;     /* 查询组id : :STOR_GROUP_ID_E*/
    void *user_arg;
    int  (*callback)(struct _stor_group_query_disk_s *handle, stor_disk_info_t *disk);
}stor_group_query_disk_t;

int stor_group_query_disk(stor_group_query_disk_t *query);        /* for each disk */

typedef struct _stor_group_info_s
{
    int group_id;
    int disk_num;
    long long total_size;
    long long diff_end;
}stor_group_info_t;

typedef struct _stor_group_query_s {
    void *user_arg;
    int  (*callback)(struct _stor_group_query_s *handle, stor_group_info_t *group);
}stor_group_query_t;

int stor_group_query(stor_group_query_t *query);    
/* 磁盘组状态查询暂未实现 */
int stor_get_group_status(int group_id);


typedef enum _STOR_MSG_ID_E
{
    STOR_MSG_SET_GROUP_CFG = 0
    , STOR_MSG_GET_GROUP_CFG = 1    
    , STOR_MSG_GET_DISK_USE_INFO = 2
    , STOR_MSG_BOTTOM
}STOR_MSG_ID_E;


typedef struct _stor_group_cfg_s
{
    int is_disover;
//    long long alarm_space;
    long long alarm_diff_end;
}stor_group_cfg_t;

typedef struct _stor_group_cfg_op_s
{
    int group_id;
    stor_group_cfg_t cfg;
}stor_group_cfg_op_t;


int stor_set_group_cfg(int group_id, void *arg);
int stor_get_group_cfg(int group_id, void *arg);

typedef struct _stor_disk_use_info_s
{
    int len;
    char info[STOR_DISK_USE_INFO_LEN];
}stor_disk_use_info_t;

typedef struct _stor_disk_use_info_op_s
{
    int group_id;
    int disk_no;
    stor_disk_use_info_t disk_use_info;
}stor_disk_use_info_op_t;

int stor_msg(int type, void *pmsg);

/* 磁盘打开写关闭接口 */
//file write (group_id, ch_name);
stor_fwrite_handle_t *stor_file_openW(int group_id, char *ch_name, int time, int *err_no); //time open ~0
int stor_file_write(stor_fwrite_handle_t *handle, char *frame_buf, stor_frame_info_t *frame_info); 
//int stor_file_sync(stor_fwrite_handle_t *handle);
int stor_file_closeW(stor_fwrite_handle_t *handle, int time);

/* 磁盘查询返回段信息 */
typedef struct _stor_fragment_s {
    /* public  */ 
    /* private */
    int group_id;   
    int disk_no; 
    int part_no;   /* no part FF */
    //char res[1];
    int file_no;   /* frag no */     

    int tags;
    int start_time;
    int end_time;

    int data_start;
    int data_end;
    int index_start;
    int index_end;
    char *pdev_name;
}stor_fragment_t;


#define STOR_FILE_QUERY_MOD_FRAG    (0)
#define STOR_FILE_QUERY_MOD_FILE    (1)


//file query;
typedef struct _stor_file_query_s {
    int mod; /* 0 query frag 1 file */
    char ch_name[STOR_CH_NAME_LEN];
    int group_id;   /* 没分组默认时组1 */
    int  start_time;
    int  end_time;
    int  tags;              /* 用户标注 */

    int frag_num;

    void *user_arg;
    int (*callback)(struct _stor_file_query_s *handle, stor_fragment_t *fragment);
}stor_file_query_t;


int stor_file_query(stor_file_query_t *query); 

typedef void stor_fread_handle_t;
stor_fread_handle_t *stor_file_openR(stor_fragment_t *fragment, int *err_no);
int stor_file_read(stor_fread_handle_t *handle, char *buf, int len);
int stor_file_closeR(stor_fread_handle_t *handle);

typedef enum _STOR_EACH_SINDEX_DIR_E
{
    STOR_EACH_SINDEX_DIR_PREV =  1      /* 遍历帧索引向前 */
        , STOR_EACH_SINDEX_DIR_BACK = 2 /* 向后遍历帧索引 */
        , STOR_EACH_SINDEX_DIR_RESTART = 3  /* 从头开始遍历 */
}STOR_EACH_SINDEX_DIR_E;

typedef enum _STOR_EACH_SINDEX_CBRET_E
{
    STOR_EACH_SINDEX_CBRET_EXIT   =        -1   /* 遍历退出 */
        , STOR_EACH_SINDEX_CBRET_CONTINUE =      0   /* 继续遍历 */
        , STOR_EACH_SINDEX_CBRET_SEEKTO    =     1   /* 读位置定位到当前位置 */
}STOR_EACH_SINDEX_CBRET_E;

typedef struct _stor_each_sindex_s
{
    int dir;
    void *user_arg;
    STOR_EACH_SINDEX_CBRET_E (*callback)(struct _stor_each_sindex_s *handle, stor_frame_sindex_t *psindx);
           // , int time, int fream_type, int data_type);
}stor_each_sindex_t;
int stor_file_lseek(stor_fread_handle_t *handle, stor_each_sindex_t *peach);

int stor_mount(const char *source, const char *target);
int stor_unmount(const char *target);
int stor_comm_format(char *pdev_name);
//int stor_mount(int disk_no);
//int stor_umount(int disk_no);

int stor_scan_disk(stor_scandisk_op_t *pscandisk);
int stor_add_disk(char *mnt_dir, long long disk_size
        , STOR_FS_TYPE_E fs_type, stor_scandisk_info_t *pscandisk);

#endif //__stor_api__
