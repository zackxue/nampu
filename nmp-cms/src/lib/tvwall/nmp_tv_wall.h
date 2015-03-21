/*
 *	author:	zyt
 *	time:	begin in 2012/9/10
 */
#ifndef __NMP_TV_WALL_H__
#define __NMP_TV_WALL_H__

#include <time.h>
#include <pthread.h>

#include "nmp_tw_interface.h"


#define	MAX_SCREENS		(65)		//不可扩展，最多只能用64个屏，另1用于记录空屏
#define	MAX_GROUPS		(32)		//可扩展
#define	INCREASE			(8)

#define	DIS_SCREEN_REG_MODE	(0x01)
#define	DIS_SCREEN_STOP		(0x02)
#define	DIS_SCREEN_COMPLITE	(0x04)

#define	GPF_WAITING			(0x01)
#define	GPF_STOP				(0x02)
#define	GPF_FIRST_TIME_RUN	(0x04)	//[告警联动添加]

#define	LOOKUP_DEL			(0x01)	//查找并删除
#define	LOOKUP_UNQ		(0x02)	//查找

#define	MAX_OPERATE_NUM	(1000)


typedef enum
{
	TW_STOP_TYPE_GPT,						//停止分割所在的GPT
	TW_STOP_TYPE_DIVISION,				//停止分割
	TW_STOP_TYPE_SCREEN_BY_DIVISION_ID	//设置分屏模式时，如果分屏模式不一样，则清屏
} TW_STOP_TYPE;



#define g_log_num (0xff)
/*
#define error_msg(fmt, args ...) do {	\
	if (g_log_num & 0x01)	\
		printf("<jtw,error_msg>   _________________________________________________________________ [error] %s[%d]:" \
		fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)
*/
#define nmp_tw_log_msg(fmt, args ...) do {	\
	if (g_log_num & 0x02)	\
		printf("<jtw,nmp_tw_log_msg> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define log_2(fmt, args ...) do {	\
	if (g_log_num & 0x04)	\
		printf("<jtw,log_2> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define log_3(fmt, args ...) do {	\
	if (g_log_num & 0x08)	\
		printf("<jtw,log_3> %s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define func_begin(fmt, args ...) do {	\
	if (g_log_num & 0x10)	\
		printf("<jtw,b> %s[%d]__ _"fmt, __FUNCTION__, __LINE__, ##args);		\
} while (0)


typedef struct
{
	unsigned int		seq;
	int				keep_other;
} action_special_t;


typedef struct
{
	int		is_action;
	time_t	action_time;
	int		division_id;
	int		division_num;
} action_step_t;


/**************************************************************
 *				电视墙逻辑处理数据结构
 **************************************************************/

typedef struct
{
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_num;				//分割号
	int		division_id;					//分屏模式，设置分屏模式的时候判断
} division_pos_t;

typedef struct
{
	char		ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	char		ec_domain_id[TW_ID_LEN];		//编码器所在的域id
	char		ec_guid[TW_ID_LEN];				//编码器guid
	char		ec_url[TW_MAX_URL_LEN];			//编码器对应的转发单元地址
	char		ec_dec_plug[TW_ID_LEN];
} encoder_t;

typedef encoder_t *division_t;

typedef struct
{
	int			division_id;				//分屏模式
	division_t		divisions[TW_MAX_DIVISIONS];	//分割分布情况及内容
} division_mode_t;

typedef struct
{
	int				step_num;			//步序号
	int				step_ttl;				//步长
	division_mode_t	division_mode;		//屏幕情况
} step_t;

struct __group_t;

typedef struct
{
	unsigned int		seq;					//序列号
	char				dis_guid[TW_ID_LEN];		//显示屏guid
	char				dis_domain_id[TW_ID_LEN];	//显示屏域id
	int				dis_flags;				//@{PAUSE,STOP,...}
	int				dis_steps;				//并不一定是总步数，严格讲应该是最大步号

	int				screen_id;				//显示屏号
	division_mode_t	dis_division_mode;		//@{valid when REG_MODE}

	step_t			*dis_pstep;
	int				steps_capacity;

	action_special_t	action_special;			//[告警联动添加]
	action_step_t		action;					//[告警联动添加]

	struct __group_t	*dis_group;				//@{this channel belongs to}

	step_t			steps_init[TW_MAX_STEPS_DEF];
} dec_screen_t;		//一屏中包括多步

typedef struct __group_t
{
	int				gp_id;					//@{巡回，群组，或单步ID,单步在实现时会赋一个负数ID}
	TW_GPT_TYPE	gp_type;				//@{类型：GPT_TOUR-巡回 GPT_GROUP-群组 GPT_STEP-单步}
	int				gp_flags;				//控制标记(Group Flags)取值：GPF_WAITING/GPF_STOP/GPF_FIRST_TIME_RUN

	int				gp_tw_id;				//@{对应的电视墙ID}

	int				gp_num;				//@{群组/巡回编号（多用于健盘）}

	dec_screen_t		*gp_dis_screens;		//解码器集
	int				gp_screens;				//解码通道数（以显示GUID区分），即显示屏数

	int				gp_auto_jump;			//巡回步无效时自动跳转
	int				gp_max_steps;			//最大步数
	int				gp_now_step;			//现在正在执行的步

	time_t			gp_switch_time;			//下一次步切换时间
	time_t			gp_alive;				//使用时间，用于删除过时的结点

	int				gp_screens_capacity;	//gp_dis_screens指向的空间大小
	char				gp_name[TW_MAX_VALUE_LEN];
	char				gp_session_id[TW_SESSION_ID_LEN];	//cu会话ID
	unsigned int		gp_cu_seq;					//记录cu seq

	dec_screen_t		dis_screen_init[MAX_SCREENS];	//在显示通道数不超过MAXCHANS的情况下gp_dis_screens指向这里
} group_t;

typedef struct
{
	group_t			**gv_parr;

	int				gv_size;					//gv中group_t对象（代表巡回，群组，或单步）的数目
	int				gv_capacity;				//gv_parr所指向的空间大小

	pthread_mutex_t	gv_mutex;
	group_t			*groups_init[MAX_GROUPS];
} group_vec_t;

typedef struct
{
	group_vec_t		tw_group_vec_ok;		//具有完整信息的group_t对象（代表巡回，群组，或单步）
	group_vec_t		tw_group_vec_wait;		//信息不完整，还在向DBS请求信息
} tv_wall_t;



typedef struct
{
	unsigned int		seq;					//发向dec内部维护的seq
	char				session_id[TW_SESSION_ID_LEN];		//cu会话id
	unsigned int		cu_seq;					//cu操作对应seq
	int				tw_id;					//电视墙id
	int				screen_id;				//显示屏id
	int				division_id;				//分屏模式
	int				division_num;			//分割号
} tw_operate_unit;

typedef struct
{
	pthread_mutex_t	operate_mutex;
	tw_operate_unit	unit[MAX_OPERATE_NUM];
} tw_operate_info;


#endif

