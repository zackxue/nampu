
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "nmp_tv_wall.h"
#include "nmp_tw_interface.h"


#define EMPTY_SCREEN_ID	(0x7fffffff)
#define LEVEL_POS			(20)

static tv_wall_t g_tv_wall_object;
static int	if_init_tv_wall_object = 0;

static tw_operate_info g_tw_operate_info;
static int if_init_operate_info = 0;

static event_handler_t tw_info_hook = NULL;
static unsigned int g_tw_dec_seq = 1;
static unsigned int g_tw_dec_operate_seq = 2;

#define QUERY_SQL_INTERVAL		(30)	//访问数据库时间间隔


//#define TW_TEST
#define TW_TEST2


static int nmp_tw_get_dis_guid(int *tw_id, int *screen_id,
	tw_general_guid *dis_guid);
static int nmp_tw_get_ec_url(char dis_guid[], char dis_domain_id[],
	char ec_guid[], char ec_domain_id[], char ec_url[], char ec_dec_plug[]);
static void nmp_tw_init_tw_operate_info();
static int nmp_tw_deal_stop_res(tw_operate_with_seq *msg_with_seq);
static void
nmp_tw_set_action_sign_no_lock(group_vec_t *p_gv, char *dis_guid,
	int enable_action, int division_id, int division_num, int *keep_other);


static char tw_log_tmp[1024];
static char g_tw_log_path[1024];
#define MAX_TW_LOG_SIZE	(1 << 23)	//8M

#define my_printf(args...) do {	\
	snprintf(tw_log_tmp, sizeof(tw_log_tmp), ##args);	\
	tw_write_log(tw_log_tmp, strlen(tw_log_tmp));	\
} while (0)

#define error_msg(fmt, args ...) do {	\
	if (g_log_num & 0x01)	\
		my_printf("<jtw,error_msg> _________ [error] %s[%d]:" \
		fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define log_msg(fmt, args ...) do {	\
	if (g_log_num & 0x01)	\
		my_printf("<jtw,log_msg> ____ %s[%d]:" \
		fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define PU_DEF_MAX_ACTION_NUM	(1000)

typedef struct _TvWallActionInfo TvWallActionInfo;
struct _TvWallActionInfo
{
	unsigned int		seq;
	int				tw_id;
	int				screen_id;
	int				keep_other;
};

typedef struct _TvWallActionPool TvWallActionPool;
struct _TvWallActionPool
{
	TvWallActionInfo	info[PU_DEF_MAX_ACTION_NUM];
	pthread_mutex_t	mutex;
};
static TvWallActionPool g_tw_action_pool;



void nmp_tw_init_log_path(const char *folder_path, const char *name)
{
	snprintf(g_tw_log_path, sizeof(g_tw_log_path), "%s/%s", folder_path, name);
}

static long get_file_size(const char *path)
{
	long filesize = -1;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	} else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

static void tw_write_log(char *str, int len)
{
	if (get_file_size(g_tw_log_path) > MAX_TW_LOG_SIZE)
		return ;

	int fd = open(g_tw_log_path, O_RDWR | O_CREAT | O_APPEND,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd <= 0)
	{
		printf("[tw_write_log] fd=%d, error:%s", fd, strerror(errno));
		return ;
	}
	//assert(fd > 0);

	if (write(fd, str, len) != len)
	{
		printf("[tw_write_log] write error\n");
		//assert(0);
	}
	close(fd);
}


#define st_free(ptr)	do {free(ptr); ptr = NULL;} while (0)

static void *st_malloc(unsigned int size)
{
	void *ptr = malloc(size);
	if (ptr == NULL)
		return ptr;

	memset(ptr, 0, size);
	return ptr;
}

static char *str_dup(const char *str)
{
	char *new_str;
	unsigned int length;

	if (str)
	{
		length = strlen(str) + 1;
		new_str = (char *)st_malloc(length);
		memcpy(new_str, str, length);
	}
	else
		new_str = NULL;

	return new_str;
}


int nmp_tw_get_channel_from_guid(char *guid, int *channel)
{
	assert(guid != NULL);

	char temp[TW_ID_LEN] = {0};

	return sscanf(guid, "%22s%2d", temp, channel) != 1;
}

static int get_gp_type_num(TW_GPT_TYPE type)
{
	if (type == GPT_NONE)
		return 0;
	if (type == GPT_STEP)
		return 1;
	if (type == GPT_TOUR)
		return 2;
	if (type == GPT_GROUP)
		return 3;
	return -1;
}

static char *get_operate_type(TW_INFO_TYPE type)
{
	switch (type)
	{
	case TW_CLEAR_TO_DEC:
		return ("* tw clear *");
	case TW_CHANGE_DIVISION_MODE_TO_DEC:
		return ("* change division mode *");
	case TW_FULL_SCREEN_TO_DEC:
		return ("* full screen *");
	case TW_EXIT_FULL_SCREEN_TO_DEC:
		return ("* exit full screen *");
	default:
		return ("* operate type error *");
	}
}

static unsigned int get_tw_dec_seq()
{
	g_tw_dec_seq += 2;
	if (g_tw_dec_seq < 2)
		g_tw_dec_seq = 1;

	return g_tw_dec_seq;
}

static unsigned int get_tw_dec_operate_seq()
{
	g_tw_dec_operate_seq += 2;
	if (g_tw_dec_operate_seq < 2)
		g_tw_dec_operate_seq = 2;

	return g_tw_dec_operate_seq;
}


#ifdef TW_TEST
static void print_operate_info_no_lock()
{
	int i;

	for (i = 0; i < MAX_OPERATE_NUM; i++)
	{
		tw_operate_unit *unit = &g_tw_operate_info.unit[i];
		if (unit->seq != 0)
		{
			log_3("<i=%d, seq=%d, session_id:%s>\n", i, unit->seq, unit->session_id);
		}
	}
}
#endif


static void
__nmp_tw_action_pool_add_info(TvWallActionPool *action_pool, int seq,
	int tw_id, int screen_id, int keep_other)
{
	TvWallActionInfo *action_info;
	int min_seq, min_num;
	int i;

	min_seq = action_pool->info[0].seq;
	min_num = 0;
	for (i = 0; i < PU_DEF_MAX_ACTION_NUM; i++)
	{
		action_info = &action_pool->info[i];
		if (action_info->seq == 0)
		{
			min_num = i;
			break;
		}

		if (action_info->seq < min_seq)
		{
			min_seq = action_info->seq;
			min_num = i;
		}
	}

	action_info = &action_pool->info[min_seq];
	action_info->seq = seq;
	action_info->tw_id = tw_id;
	action_info->screen_id = screen_id;
	action_info->keep_other = keep_other;
}

static void
nmp_tw_action_pool_add_info(int seq, int tw_id, int screen_id, int keep_other)
{
	pthread_mutex_lock(&g_tw_action_pool.mutex);
	__nmp_tw_action_pool_add_info(&g_tw_action_pool, seq, tw_id, screen_id,
		keep_other);
	pthread_mutex_unlock(&g_tw_action_pool.mutex);
}


static int
__nmp_tw_action_pool_get_info(TvWallActionPool *action_pool, int seq,
	int *tw_id, int *screen_id, int *keep_other)
{
	TvWallActionInfo *action_info;
	int i;

	for (i = 0; i < PU_DEF_MAX_ACTION_NUM; i++)
	{
		action_info = &action_pool->info[i];
		if (action_info->seq == seq)
		{
			*tw_id = action_info->tw_id;
			*screen_id = action_info->screen_id;
			*keep_other = action_info->keep_other;

			action_info->seq = 0;
			return 0;
		}
	}

	return -1;
}

static int
nmp_tw_action_pool_get_info(int seq, int *tw_id, int *screen_id, int *keep_other)
{
	int ret;

	pthread_mutex_lock(&g_tw_action_pool.mutex);
	ret = __nmp_tw_action_pool_get_info(&g_tw_action_pool, seq, tw_id, screen_id,
		keep_other);
	pthread_mutex_unlock(&g_tw_action_pool.mutex);

	return ret;
}


static inline void save_dec_operate_unit(tw_operate_unit *unit,
	tw_operate_with_seq *msg_with_seq, unsigned int seq)
{
	tw_operate *msg = &msg_with_seq->operate;

	unit->seq = seq;
	unit->session_id[TW_SESSION_ID_LEN - 1] = '\0';
	strncpy(unit->session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	unit->cu_seq = msg_with_seq->seq;
	unit->tw_id = msg->tw_id;
	unit->screen_id = msg->screen_id;
	unit->division_id = msg->division_id;
	unit->division_num = msg->division_num;
}

static void nmp_tw_save_dec_operate_info(tw_operate_with_seq *msg_with_seq,
	unsigned int seq)
{
	int i;

	assert(seq != 0);

	if (!if_init_operate_info)
	{
		nmp_tw_init_tw_operate_info();
		if_init_operate_info = 1;
	}

	pthread_mutex_lock(&g_tw_operate_info.operate_mutex);
	for (i = 0; i < MAX_OPERATE_NUM; i++)
	{
		tw_operate_unit *unit = &g_tw_operate_info.unit[i];
		if (seq > unit->seq)
		{
			if (unit->seq == 0 || seq - unit->seq >= (MAX_OPERATE_NUM << 1))
			{
				save_dec_operate_unit(unit, msg_with_seq, seq);
				break;
			}
		}
		else
		{
			memset(&g_tw_operate_info.unit, 0, sizeof(tw_operate_unit) *
				MAX_OPERATE_NUM);
		}
	}
	pthread_mutex_unlock(&g_tw_operate_info.operate_mutex);
}


static inline void get_operate_info_from_unit(tw_operate_result_to_cu_with_seq *to_cu_with_seq,
	tw_operate_unit *unit)
{
	tw_operate_result_to_cu *to_cu = &to_cu_with_seq->to_cu;

	to_cu->session_id[TW_SESSION_ID_LEN - 1] = '\0';
	strncpy(to_cu->session_id, unit->session_id, TW_SESSION_ID_LEN - 1);
	to_cu->tw_id = unit->tw_id;
	to_cu->screen_id = unit->screen_id;
	to_cu->division_id = unit->division_id;
	to_cu->division_num = unit->division_num;

	to_cu_with_seq->seq = unit->cu_seq;
}

static int nmp_tw_get_operate_info(tw_operate_result_to_cu_with_seq *to_cu_with_seq,
	unsigned int seq)
{
	int i, found = 0;
	assert(seq != 0);

	pthread_mutex_lock(&g_tw_operate_info.operate_mutex);
#ifdef TW_TEST
	print_operate_info_no_lock();
#endif
	for (i = 0; i < MAX_OPERATE_NUM; i++)
	{
		tw_operate_unit *unit = &g_tw_operate_info.unit[i];
		if (seq == unit->seq)
		{
			get_operate_info_from_unit(to_cu_with_seq, unit);
			memset(unit, 0, sizeof(tw_operate_unit));
			found = 1;
			break;
		}
	}
	pthread_mutex_unlock(&g_tw_operate_info.operate_mutex);

	assert(found);
	return found;
}


static int get_operate_info_to_dec(tw_operate_to_decoder *dst, tw_operate *src)
{
	dst->division_id = src->division_id;
	dst->division_num = src->division_num;
	tw_general_guid dis_guid;

	if (nmp_tw_get_dis_guid(&src->tw_id, &src->screen_id, &dis_guid) < 0)
		return -1;

	dst->dis_guid[TW_ID_LEN - 1] = '\0';
	strncpy(dst->dis_guid, dis_guid.guid, TW_ID_LEN - 1);

	return 0;
}


int nmp_tw_init_group_vec(group_vec_t *p_gv)
{
	assert(p_gv != NULL);

	memset(p_gv, 0, sizeof(*p_gv));
	p_gv->gv_parr = p_gv->groups_init;
	p_gv->gv_capacity = MAX_GROUPS;
	if (pthread_mutex_init(&p_gv->gv_mutex, NULL))
		return -1;
	return 0;
}

void nmp_tw_destroy_group_vec(group_vec_t *p_gv)
{
	if (p_gv)
	{
		pthread_mutex_destroy(&p_gv->gv_mutex);
		if (p_gv->gv_parr != p_gv->groups_init)
			st_free(p_gv->gv_parr);
	}
}

int nmp_tw_init_tvwall(tv_wall_t *p_tw)
{
	assert(p_tw != NULL);

	if (nmp_tw_init_group_vec(&p_tw->tw_group_vec_ok) < 0)
	{
		error_msg("init tv wall group vec\n");
		return -1;
	}

	if (nmp_tw_init_group_vec(&p_tw->tw_group_vec_wait) < 0)
	{
		nmp_tw_destroy_group_vec(&p_tw->tw_group_vec_ok);
		error_msg("init tv wall wait group vec\n");
		return -1;
	}

	return 0;
}


int nmp_tw_init_action_pool(TvWallActionPool *action_pool)
{
	assert(action_pool != NULL);
	memset(action_pool, 0, sizeof(TvWallActionPool));

	if (pthread_mutex_init(&action_pool->mutex, NULL))
		return -1;

	return 0;
}


void nmp_tw_init_tvwall_module()
{
	if (nmp_tw_init_tvwall(&g_tv_wall_object) < 0)
	{
		error_msg("init tv wall\n");
		exit(1);
	}

	if (nmp_tw_init_action_pool(&g_tw_action_pool) < 0)
	{
		error_msg("init g_tw_action_pool failed\n");
		exit(1);
	}
}

static void nmp_tw_init_tw_operate_info()
{
	memset(&g_tw_operate_info, 0, sizeof(g_tw_operate_info));
	if (pthread_mutex_init(&g_tw_operate_info.operate_mutex, NULL))
	{
		error_msg("pthread_mutex_init failed\n");
		exit(1);
	}
}

tv_wall_t *nmp_tw_get_tv_wall_p()
{
	if (!if_init_tv_wall_object)
	{
		nmp_tw_init_tvwall_module();
		if_init_tv_wall_object = 1;
	}
	return (&g_tv_wall_object);
}


void nmp_tw_set_event_handler(event_handler_t hook)
{
	tw_info_hook = hook;
}

int nmp_tw_info_handle(TW_INFO_TYPE cmd, void *in_parm, void *out_parm)
{
	if (!tw_info_hook)
	{
		error_msg("tw_info_hook = NULL\n");
		return -1;
	}

	return (*tw_info_hook)(cmd, in_parm, out_parm);
}


void nmp_tw_set_group_tw_id(group_t *p_gp, int tw_id)
{
	assert(p_gp);

	p_gp->gp_tw_id = tw_id;
}

void nmp_tw_set_group_num(group_t *p_gp, int num)
{
	assert(p_gp);

	p_gp->gp_num = num;
}

void nmp_tw_set_auto_jump(group_t *p_gp, int auto_jump)
{
	assert(p_gp);

	p_gp->gp_auto_jump = auto_jump;
}

void nmp_tw_set_group_name(group_t *p_gp, char *p_name)
{
	assert(p_gp && p_name);

	p_gp->gp_name[TW_MAX_VALUE_LEN - 1] = '\0';
	strncpy(p_gp->gp_name, p_name, TW_MAX_VALUE_LEN - 1);
}

void nmp_tw_set_step_div_id(step_t *p_step, int div_id)	//div_id为0时代表空屏
{
	assert(p_step);

	p_step->division_mode.division_id = div_id;
}

step_t *nmp_tw_set_screen_step(dec_screen_t *p_dis_screen, int step,
	int intr)
{
	assert(p_dis_screen && step > 0);

	step_t *p_step;
	if (step > TW_MAX_STEPS_TOP)
	{
		error_msg("step num over [MAX_STEPS_TOP:%d]\n", TW_MAX_STEPS_TOP);
		return NULL;
	}

	while (step > p_dis_screen->steps_capacity)	//zyt,一屏中的总步数可扩展
	{
		p_step = (step_t *)st_malloc(sizeof(step_t) *
			(p_dis_screen->steps_capacity << 1));
		if (!p_step)
		{
			error_msg("no memory\n");
			return NULL;
		}
		memset(p_step, 0, sizeof(step_t) * (p_dis_screen->steps_capacity << 1));
		memcpy(p_step, p_dis_screen->dis_pstep, sizeof(step_t) *
			p_dis_screen->steps_capacity);

		if (p_dis_screen->dis_pstep != p_dis_screen->steps_init)
			st_free(p_dis_screen->dis_pstep);
		p_dis_screen->dis_pstep = p_step;
		p_dis_screen->steps_capacity <<= 1;
	}

	p_step = &p_dis_screen->dis_pstep[step - 1];
	if (p_step->step_num && p_step->step_num != step)	//zyt这情况不存在吧
	{
		error_msg("dup step\n");
		return NULL;
	}

	p_step->step_num = step;
	p_step->step_ttl = intr;
	if (step > p_dis_screen->dis_steps)
		p_dis_screen->dis_steps = step;
	if (step > p_dis_screen->dis_group->gp_max_steps)
		p_dis_screen->dis_group->gp_max_steps = step;

	if (p_dis_screen->dis_flags & DIS_SCREEN_REG_MODE)
	{
		p_step->division_mode.division_id =
			p_dis_screen->dis_division_mode.division_id;
	}

	return p_step;
}


int nmp_tw_set_focus_screen_div(dec_screen_t *p_screen,
	int div_id, int div_num)
{
	assert(p_screen);

	if (div_num < 0 || div_num >= TW_MAX_DIVISIONS)
		return -1;
	p_screen->dis_flags |= DIS_SCREEN_REG_MODE;
	p_screen->dis_division_mode.division_id = div_id;
	p_screen->dis_division_mode.divisions[div_num] = (division_t)1;

	return 0;
}


int nmp_tw_set_step_encoder(step_t *p_step, int div_num, encoder_t *p_encoder)
{
	assert(p_step && p_encoder);

	if (div_num < 0 || div_num > TW_MAX_DIVISIONS)
	{
		nmp_tw_log_msg("bad division num:%d\n", div_num);
		return -1;
	}
	if (p_step->division_mode.divisions[div_num])
	{
		nmp_tw_log_msg("dup division num:%d\n", div_num);
		return -1;
	}

	encoder_t *p_en = (encoder_t *)st_malloc(sizeof(encoder_t));
	if (!p_en)
	{
		error_msg("set step encoder, no memory\n");
		return -1;
	}

	*p_en = *p_encoder;	//拷贝一份编码器的信息
	p_step->division_mode.divisions[div_num] = p_en;
	log_3("p_step->division_mode.divisions[div_num]->ec_name:%s\n",
		p_step->division_mode.divisions[div_num]->ec_name);

	return 0;
}


void nmp_tw_destroy_step(step_t *p_step)
{
	assert(p_step);

	int div_i = 0;
	for (; div_i < TW_MAX_DIVISIONS; ++div_i)
		if (p_step->division_mode.divisions[div_i])
			st_free(p_step->division_mode.divisions[div_i]);
}

void nmp_tw_init_dis_screen(dec_screen_t *p_dis_screen, group_t *p_gp)
{
	assert(p_dis_screen && p_gp);

	memset(p_dis_screen, 0, sizeof(dec_screen_t));
	p_dis_screen->dis_pstep = &p_dis_screen->steps_init[0];
	p_dis_screen->steps_capacity = TW_MAX_STEPS_DEF;
	p_dis_screen->dis_group = p_gp;
}

void nmp_tw_destroy_dis_screen(dec_screen_t *p_dis_screen)
{
	assert(p_dis_screen != NULL);

	int step_i = p_dis_screen->dis_steps;
	for (; step_i > 0; --step_i)
		nmp_tw_destroy_step(&p_dis_screen->dis_pstep[step_i - 1]);
	if (p_dis_screen->dis_pstep != p_dis_screen->steps_init)
		st_free(p_dis_screen->dis_pstep);
}


void nmp_tw_init_group(group_t *p_group, int gp_id, TW_GPT_TYPE gp_type,
	char *session_id, unsigned int cu_seq)
{
	assert(p_group != NULL);

	int screen_i = 0;
	memset(p_group, 0, sizeof(*p_group));
	p_group->gp_id = gp_id;
	p_group->gp_type = gp_type;
	p_group->gp_flags = GPF_FIRST_TIME_RUN;
	p_group->gp_screens_capacity = MAX_SCREENS;
	p_group->gp_switch_time = time(NULL);
	p_group->gp_alive = time(NULL);
	p_group->gp_dis_screens = &p_group->dis_screen_init[0];
	strncpy(p_group->gp_session_id, session_id, TW_SESSION_ID_LEN - 1);
	p_group->gp_cu_seq = cu_seq;

	for (; screen_i < p_group->gp_screens_capacity; ++screen_i)
		nmp_tw_init_dis_screen(&p_group->dis_screen_init[screen_i], p_group);
}


void nmp_tw_destroy_group(group_t *p_group)
{
	assert(p_group != NULL);

	int screen_i;
	for (screen_i = p_group->gp_screens_capacity - 1; screen_i >= 0; --screen_i)
		nmp_tw_destroy_dis_screen(&p_group->gp_dis_screens[screen_i]);
	if (p_group->gp_dis_screens != p_group->dis_screen_init)
		st_free(p_group->gp_dis_screens);
}

group_t *nmp_tw_new_group(int gp_id, TW_GPT_TYPE gp_type, char *session_id,
	unsigned int cu_seq)
{
	group_t *p_gp = (group_t *)st_malloc(sizeof(group_t));
	if (p_gp)
		nmp_tw_init_group(p_gp, gp_id, gp_type, session_id, cu_seq);
	return p_gp;
}

void nmp_tw_delete_group(group_t *p_gp)
{
	func_begin("\n");
	if (p_gp)
	{
		nmp_tw_destroy_group(p_gp);
		st_free(p_gp);
		printf("a group destroyed!!!\n");
	}
}


dec_screen_t *nmp_tw_add_dis_screen(group_t *p_group, char *dis_guid,
	char *dis_domain_id, int screen_id)
{
	assert(p_group && dis_guid && dis_domain_id);

	dec_screen_t *p_screen;
	int index = 0;

	for (; index < p_group->gp_screens; ++index)
	{
		if (!strcmp(p_group->gp_dis_screens[index].dis_guid, dis_guid) &&
			!strcmp(p_group->gp_dis_screens[index].dis_domain_id, dis_domain_id))
		{
			error_msg("add dis screen twice\n");
			return NULL;
		}
	}
	if (p_group->gp_screens >= p_group->gp_screens_capacity)
	{
		error_msg("reach max screens\n");
		return NULL;
	}

	p_screen = &p_group->gp_dis_screens[p_group->gp_screens++];
	p_screen->dis_guid[TW_ID_LEN - 1] = '\0';
	strncpy(p_screen->dis_guid, dis_guid, TW_ID_LEN - 1);
	p_screen->dis_domain_id[TW_ID_LEN - 1] = '\0';
	strncpy(p_screen->dis_domain_id, dis_domain_id, TW_ID_LEN - 1);
	p_screen->screen_id = screen_id;
	if (p_group->gp_type == GPT_TOUR ||	//单步，巡回 有reg_mode属性
		p_group->gp_type == GPT_STEP)
	{
		p_screen->dis_flags |= DIS_SCREEN_REG_MODE;
	}

	return p_screen;
}


dec_screen_t *nmp_tw_get_tour_dis_screen(group_t *p_group)
{
	assert(p_group);

	dec_screen_t *p_screen = &p_group->gp_dis_screens[0];
	if (!p_screen)	//注意判断!!!
	{
		nmp_tw_log_msg("get tour dis screen error, screen[0] not set\n");
		return NULL;
	}

	return p_screen;
}


static int get_empty_screen_division_id(dec_screen_t *p_dis_screen)
{
	int step_i;
	int div_id;

	for (step_i = 0; step_i < p_dis_screen->dis_group->gp_max_steps; step_i++)
	{
		div_id = p_dis_screen->dis_pstep[step_i].division_mode.division_id;
		if (div_id != 0)
			return div_id;
	}

	return 1;
}

int nmp_tw_verify_dis_screen(dec_screen_t *p_dis_screen)
{
	assert(p_dis_screen);

	int step_i = 0, div_i = 0, reg_mode = 1, cmp_pos = -1;
	TW_GPT_TYPE gp_type = p_dis_screen->dis_group->gp_type;
	if (!p_dis_screen->dis_steps)
	{
		p_dis_screen->dis_flags |= DIS_SCREEN_STOP;
		return 1;
	}

	if (gp_type == GPT_TOUR || gp_type == GPT_STEP)
		return 1;

	for (; step_i < p_dis_screen->dis_group->gp_max_steps; ++step_i)
	{
		/* 空屏情况处理 */
		if (p_dis_screen->dis_pstep[step_i].step_num == 0)	//zyt此屏步未设置
		{
			step_t *p_step = nmp_tw_set_screen_step(p_dis_screen,
				step_i + 1, -1);	//步长-1，标识为非合法值
			if (!p_step)
			{
				nmp_tw_log_msg("verify screen:%s, add step:%d error\n",
					p_dis_screen->dis_guid, step_i + 1);
				return 0;
			}

			nmp_tw_set_step_div_id(p_step,
				get_empty_screen_division_id(p_dis_screen));
			continue;
		}

		if (p_dis_screen->dis_pstep[step_i].step_num &&
			p_dis_screen->dis_pstep[step_i].step_num != step_i + 1)
		{
			nmp_tw_log_msg("verify screen, bad step position\n");
			return 0;
		}

		if (reg_mode && p_dis_screen->dis_pstep[step_i].step_num)
		{
			if (cmp_pos < 0)
				cmp_pos = step_i;

			if (p_dis_screen->dis_pstep[cmp_pos].division_mode.division_id !=
				p_dis_screen->dis_pstep[step_i].division_mode.division_id)
				reg_mode = 0;
		}

		p_dis_screen->dis_pstep[step_i].step_num = step_i + 1;
	}

	if (reg_mode)
	{
		p_dis_screen->dis_flags |= DIS_SCREEN_REG_MODE;

		if (cmp_pos >= 0)
		{
			p_dis_screen->dis_division_mode.division_id =
				p_dis_screen->dis_pstep[cmp_pos].division_mode.division_id;
		}
	}

	nmp_tw_log_msg("group %d screen %s is %sreg-mode\n",
		p_dis_screen->dis_group->gp_id, p_dis_screen->dis_guid,
		reg_mode ? "" : "not ");

	if (step_i && (p_dis_screen->dis_flags & DIS_SCREEN_REG_MODE))	//zyt?如果step_i为0则代表只有一步，待重看
	{
		for (; div_i < TW_MAX_DIVISIONS; ++div_i)
		{
			if (!p_dis_screen->dis_division_mode.divisions[div_i])
			{
				for (step_i = 0; step_i < p_dis_screen->dis_steps; ++step_i)
				{
					if ((p_dis_screen->dis_division_mode.divisions[div_i] =
						p_dis_screen->dis_pstep[step_i].division_mode.divisions[div_i]))
						break;
				}
			}
		}
	}

	return p_dis_screen->dis_steps;
}


int nmp_tw_verify_group(group_t *p_group)	//检查group_t的正确性
{
	assert(p_group);
	func_begin("\n");

	int screen_i = 0;
	if (p_group->gp_flags & GPF_WAITING)
		return 1;

	for (; screen_i < p_group->gp_screens; ++screen_i)
	{
		if (!nmp_tw_verify_dis_screen(&p_group->gp_dis_screens[screen_i]))
		{
			nmp_tw_log_msg("verify gp %d screen %s error\n",
				p_group->gp_id, p_group->gp_dis_screens[screen_i].dis_guid);
			return 0;
		}
	}

	return 1;
}

/*
 *	判断是否为同一个巡回且在同一个分割中显示
 */
int nmp_tw_check_unique_tour(group_t *p_gp_1, group_t *p_gp_2)
{
	assert(p_gp_1 && p_gp_2);

	dec_screen_t *screen_1 = &p_gp_1->gp_dis_screens[0];
	dec_screen_t *screen_2 = &p_gp_2->gp_dis_screens[0];

	if (strcmp(screen_1->dis_guid, screen_2->dis_guid))
	{
		return 0;
	}

	if (screen_1->dis_division_mode.division_id !=
		screen_2->dis_division_mode.division_id)
	{
		return 0;
	}

	int div_i = 0;
	for (; div_i < TW_MAX_DIVISIONS; ++div_i)	//在同一个分割中显示
	{
		if (screen_1->dis_division_mode.divisions[div_i] &&
			screen_2->dis_division_mode.divisions[div_i])
			return 1;
	}

	return 0;
}


int nmp_tw_group_lookup(group_vec_t *p_gv, group_t *p_group,
	int gp_id, TW_GPT_TYPE type, int flags)
{
	assert(p_gv);
	func_begin("\n");

	int group_i = 0, rc = 0;

	if ((flags & LOOKUP_UNQ) && !p_group)
	{
		nmp_tw_log_msg("invalid lookup() args\n");
		return -1;
	}

	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp)
		{
			if (p_gp->gp_id == gp_id && p_gp->gp_type == type)
			{
				if (flags & LOOKUP_DEL)
				{
					p_gv->gv_parr[group_i] = NULL;
					--p_gv->gv_size;

					nmp_tw_log_msg("%s %d stopped.\n",
						p_gp->gp_type == GPT_TOUR ? "tour" : "group",
						p_gp->gp_id);
					nmp_tw_delete_group(p_gp);
					++rc;
					continue;
				}

				if (p_gp->gp_type == GPT_TOUR)
				{
					if (flags & LOOKUP_UNQ)
					{
						if (nmp_tw_check_unique_tour(p_gp, p_group))
						{
							++rc;
							break;
						}
					}
					else
					{
						++rc;
						break;
					}
				}
				else
				{
					++rc;
					break;
				}
			}
		}
	}
	return rc;
}

/*
 *	检查两个屏是否冲突，冲突将替换成新屏数据
 */
void nmp_tw_check_dc_conflict(dec_screen_t *p_screen,
	dec_screen_t *p_screen_new)
{
	assert(p_screen && p_screen_new);

	if (strcmp(p_screen->dis_guid, p_screen_new->dis_guid))	//非同一个屏(还可能来自不同墙)
		return ;

	if (p_screen->dis_flags & DIS_SCREEN_STOP)	//前屏已停止
		return ;

	if (!(p_screen->dis_flags & p_screen_new->dis_flags & DIS_SCREEN_REG_MODE))	//变屏
	{
		p_screen->dis_flags |= DIS_SCREEN_STOP;

		nmp_tw_log_msg("screen:%d-%s interrupted by gp %d\n",
			p_screen->dis_group->gp_id, p_screen->dis_guid,
			p_screen->dis_group->gp_id);
		return ;
	}

	/* zyt?这里处理与原来有小的差异，此处division_id不可能为0 */
	if (p_screen->dis_division_mode.division_id != 		//分屏模式不一样
		p_screen_new->dis_division_mode.division_id)
	{
		p_screen->dis_flags |= DIS_SCREEN_STOP;
		nmp_tw_log_msg("screen:%d-%s interrupted by gp %d, div_id:%d, " \
			"new div_id:%d\n",
			p_screen->dis_group->gp_id, p_screen->dis_guid,
			p_screen_new->dis_group->gp_id,
			p_screen->dis_division_mode.division_id,
			p_screen_new->dis_division_mode.division_id);
		return ;
	}

	//比较分割的冲突
	int div_i = 0;
	for (; div_i < TW_MAX_DIVISIONS; ++div_i)
	{
		if (p_screen->dis_division_mode.divisions[div_i] &&
			p_screen_new->dis_division_mode.divisions[div_i])
		{
			p_screen->dis_division_mode.divisions[div_i] = NULL;	//标记上一屏的分割停止，只是指针，其指向内容不由自己维护

			nmp_tw_log_msg("screen:%d-%s division %d interrupted by gp %d\n",
				p_screen->dis_group->gp_id, p_screen->dis_guid, div_i + 1,
				p_screen_new->dis_group->gp_id);
		}
	}
}

/*
 *	检查两个group冲突情况并处理
 */
void nmp_tw_check_gp_conflict(group_t *p_gp, group_t *p_gp_new)
{
	assert(p_gp && p_gp_new);

	int screen_i = 0, screen_j = 0;
	if (p_gp->gp_flags & GPF_STOP)
		return ;

	for (; screen_i < p_gp_new->gp_screens; ++screen_i)
	{
		for (screen_j = 0; screen_j < p_gp->gp_screens; ++screen_j)
		{
			nmp_tw_check_dc_conflict(&p_gp->gp_dis_screens[screen_j],
				&p_gp_new->gp_dis_screens[screen_i]);
		}
	}
}

int nmp_tw_check_conflict(group_vec_t *p_gv, group_t *p_group)
{
	assert(p_gv && p_group);
	func_begin("\n");

	int group_i = 0;
/*
	cfl = nmp_tw_group_lookup(p_gv, p_group, p_group->gp_id,
		p_group->gp_type, LOOKUP_UNQ);
	if (cfl > 0)
	{
		const char *p_type = p_group->gp_type == GPT_TOUR ?
			"tour" : "group";
		nmp_tw_log_msg("%s %d run again\n", p_type, p_group->gp_id);
		nmp_tw_delete_group(p_group);
		return 1;
	}
*/
	for (group_i = 0; group_i < p_gv->gv_capacity; ++group_i)
	{
		if (p_gv->gv_parr[group_i])
			nmp_tw_check_gp_conflict(p_gv->gv_parr[group_i], p_group);
	}

	return 0;
}


int nmp_tw_is_invalid_screen(dec_screen_t *p_dis_screen)
{
	int div_i = 0, flags = p_dis_screen->dis_flags;

	if (flags & DIS_SCREEN_STOP)
		return 1;

	if (!(flags & DIS_SCREEN_REG_MODE))	//如果为变屏，则返回0
		return 0;

	for (; div_i < TW_MAX_DIVISIONS; ++div_i)
	{
		if (p_dis_screen->dis_division_mode.divisions[div_i])	//有一个分割显示，则为真
			return 0;
	}

	return 1;
}


int nmp_tw_is_invalid_group(group_t *p_group)
{
	assert(p_group);

	int screen_i = 0;
	if (p_group->gp_flags & GPF_STOP)	//group已经停止，则删除
		return 1;

	for (; screen_i < p_group->gp_screens; ++screen_i)
	{
		if (!nmp_tw_is_invalid_screen(&p_group->gp_dis_screens[screen_i]))
			return 0;	//只要有一个屏为真，则group_t则有效
	}
	if (p_group->gp_flags & GPF_WAITING)
		return 0;

	return 1;
}


void nmp_tw_del_invalid_group(group_vec_t *p_gv)
{
	assert(p_gv);
	func_begin("\n");

	int group_i = 0;
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp)
		{
			if (nmp_tw_is_invalid_group(p_gp))
			{
				p_gv->gv_parr[group_i] = NULL;
				--p_gv->gv_size;
				nmp_tw_delete_group(p_gp);
			}
		}
	}
}

int nmp_tw_add_group_no_lock(group_vec_t *p_gv, group_t *p_group)
{
	assert(p_gv && p_group);

	int group_i = 0;
	if (!nmp_tw_verify_group(p_group))
	{
		nmp_tw_log_msg("new group verify failed\n");
		nmp_tw_delete_group(p_group);
		return -1;
	}

	if (nmp_tw_check_conflict(p_gv, p_group))	//zyt?如果此处加入的是单步，不特殊处理?
		return 0;

	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		if (!p_gv->gv_parr[group_i])
		{
			p_gv->gv_parr[group_i] = p_group;
			++p_gv->gv_size;
			return 0;
		}
	}

	group_t **p_gp = (group_t **)st_malloc(sizeof(group_t *) *
		(INCREASE + p_gv->gv_capacity));
	if (p_gp)
	{
		memset(p_gp, 0, sizeof(group_t *) * (INCREASE + p_gv->gv_capacity));
		memcpy(p_gp, p_gv->gv_parr, sizeof(group_t *) * p_gv->gv_capacity);
		p_gp[p_gv->gv_capacity] = p_group;
		++p_gv->gv_size;
		p_gv->gv_capacity += INCREASE;

		if (p_gv->gv_parr != p_gv->groups_init)
		{
			st_free(p_gv->gv_parr);
		}
		p_gv->gv_parr = p_gp;
		return 0;
	}

	nmp_tw_delete_group(p_group);
	return -1;
}

int nmp_tw_add_group(group_vec_t *p_gv, group_t *p_group)	//调用此函数后，p_group指向结构由此函数维护
{
	func_begin("\n");
	int ret;
	pthread_mutex_lock(&p_gv->gv_mutex);
	nmp_tw_del_invalid_group(p_gv);	//每次add group前会删除 invalid的group(不错的设计)
	ret = nmp_tw_add_group_no_lock(p_gv, p_group);
	pthread_mutex_unlock(&p_gv->gv_mutex);
	return ret;
}


int nmp_tw_send_cmd_res(char session_id[], unsigned int cu_seq,
	TW_CMD_TYPE type, int result)
{
	tw_run_res res;	//注意初始化
	memset(&res, 0, sizeof(tw_run_res));

	strncpy(res.session_id, session_id, TW_SESSION_ID_LEN - 1);
	res.cu_seq = cu_seq;
	res.tw_cmd_type = type;
	res.result = result;

	if (nmp_tw_info_handle(TW_INFO_SEND_CMD_RES_TO_CU, &res, NULL) < 0)
	{
		error_msg("send cmd res to cu failed\n");
		return -1;
	}
	nmp_tw_log_msg("send cmd res to cu success\n");

	return 0;
}


int nmp_tw_get_tw_info(tw_screen_to_cu *to_cu, unsigned int *cu_seq,
	int *tour_auto_jump, group_vec_t *p_gv, unsigned int seq)
{
	assert(p_gv && to_cu);

	int group_i = 0, screen_i = 0;
	dec_screen_t *p_screen = NULL;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (!p_gp)
			continue;

		for (screen_i = 0; screen_i < p_gp->gp_screens; ++screen_i)
		{
			p_screen = &p_gp->gp_dis_screens[screen_i];
			if (p_screen->seq == seq)
			{
				assert(p_gp->gp_type != GPT_NONE);

				to_cu->session_id[TW_SESSION_ID_LEN - 1] = '\0';
				strncpy(to_cu->session_id, p_gp->gp_session_id,
					TW_SESSION_ID_LEN - 1);
				to_cu->gp_type = p_gp->gp_type;
				to_cu->gp_name[TW_MAX_VALUE_LEN - 1] = '\0';
				strncpy(to_cu->gp_name, p_gp->gp_name, TW_MAX_VALUE_LEN - 1);
				to_cu->tw_id = p_gp->gp_tw_id;
				to_cu->screen_id = p_screen->screen_id;
				if (p_screen->seq == p_screen->action_special.seq)
				{
					to_cu->keep_other = p_screen->action_special.keep_other;
					p_screen->action_special.seq = 0;
				}
				else
					to_cu->keep_other = !!(p_screen->dis_flags & DIS_SCREEN_REG_MODE);

				*cu_seq = p_gp->gp_cu_seq;
				*tour_auto_jump = p_gp->gp_auto_jump;

				if (p_gp->gp_type == GPT_STEP)
					p_screen->dis_flags |= DIS_SCREEN_STOP;	//zyt需要设置，STEP执行完就相当于STOP状态，不会与其它gp冲突

				pthread_mutex_unlock(&p_gv->gv_mutex);
				return 0;
			}
		}
	}

	pthread_mutex_unlock(&p_gv->gv_mutex);
	return -1;
}


int nmp_tw_stop_group_by_div(group_t *p_gp, division_pos_t *div_pos,
	TW_STOP_TYPE stop_type)
{
	assert(p_gp && div_pos);

	int screen_i = 0;

	if (p_gp->gp_tw_id != div_pos->tw_id)
		return 0;

	if (p_gp->gp_type != GPT_TOUR &&
		p_gp->gp_type != GPT_GROUP)
		return 0;

	for (; screen_i < p_gp->gp_screens; ++screen_i)
	{
		dec_screen_t *p_screen = &p_gp->gp_dis_screens[screen_i];
		if (p_screen->screen_id == div_pos->screen_id)
		{
			if (p_screen->dis_flags & DIS_SCREEN_STOP)
				continue;

			if (stop_type == TW_STOP_TYPE_GPT)
			{
				if (!(p_screen->dis_flags & DIS_SCREEN_REG_MODE) ||
					p_screen->dis_division_mode.divisions[div_pos->division_num])
				{
					p_screen->dis_group->gp_flags |= GPF_STOP;
					return 1;
				}
				else
					return 0;
			}
			else if (stop_type == TW_STOP_TYPE_DIVISION)
			{
				log_msg("1223334********** stop,p_screen->screen_id:%d\n", p_screen->screen_id);
				if (div_pos->division_num == -1)	//清整屏
					p_screen->dis_flags |= DIS_SCREEN_STOP;
				else if (!(p_screen->dis_flags & DIS_SCREEN_REG_MODE))	//变屏
					p_screen->dis_flags |= DIS_SCREEN_STOP;
				else if (p_screen->dis_division_mode.divisions[div_pos->division_num])
					p_screen->dis_division_mode.divisions[div_pos->division_num] = NULL;	//清分割
				else
					return 0;
				return 1;
			}
			else if (stop_type == TW_STOP_TYPE_SCREEN_BY_DIVISION_ID)
			{
				if (!(p_screen->dis_flags & DIS_SCREEN_REG_MODE) ||
					p_screen->dis_division_mode.division_id != div_pos->division_id)	//变屏或者与新设置的分屏模式不同
					p_screen->dis_flags |= DIS_SCREEN_STOP;
				return 1;
			}
		}
	}
	return 0;
}

int nmp_tw_stop_run(group_vec_t *p_gv, TW_GPT_TYPE type, int id)
{
	assert(p_gv);

	int rc;
	pthread_mutex_lock(&p_gv->gv_mutex);
	rc = nmp_tw_group_lookup(p_gv, NULL, id, type, LOOKUP_DEL);
	pthread_mutex_unlock(&p_gv->gv_mutex);

	return TW_RES_OK;
}

int nmp_tw_query_state(group_vec_t *p_gv, TW_GPT_TYPE type, int id)
{
	assert(p_gv);

	int rc;
	pthread_mutex_lock(&p_gv->gv_mutex);
	nmp_tw_del_invalid_group(p_gv);
	rc = nmp_tw_group_lookup(p_gv, NULL, id, type, 0);
	pthread_mutex_unlock(&p_gv->gv_mutex);

	return (rc > 0) ? TW_RES_OK : TW_RES_ERROR;
}

int nmp_tw_stop_run_by_div(group_vec_t *p_gv, division_pos_t *div_pos,
	TW_STOP_TYPE stop_type)
{
	int group_i = 0, found = 0;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp && nmp_tw_stop_group_by_div(p_gp, div_pos, stop_type))
		{
			if (stop_type == TW_STOP_TYPE_GPT)
			{
				p_gv->gv_parr[group_i] = NULL;
				--p_gv->gv_size;
				nmp_tw_delete_group(p_gp);
				++found;
				break;
			}
			else
				++found;
		}
	}
	pthread_mutex_unlock(&p_gv->gv_mutex);
	return found;
}


int nmp_tw_do_step(tv_wall_t *p_tw, tw_general_guid *dis_guid, int tw_id, int screen_id,
	int div_num, int div_id, encoder_t *p_en, char *session_id, unsigned int cu_seq)
{
	func_begin("\n");
	static int step_id = 0;
	group_t *p_group = nmp_tw_new_group(--step_id, GPT_STEP, session_id, cu_seq);
	if (!p_group)
	{
		nmp_tw_log_msg("nmp_tw_new_group return NULL\n");
		return -1;
	}

	nmp_tw_set_group_tw_id(p_group, tw_id);

	dec_screen_t *p_screen = nmp_tw_add_dis_screen(p_group, dis_guid->guid,
		dis_guid->domain_id, screen_id);
	if (!p_screen)
	{
		nmp_tw_log_msg("nmp_tw_add_dis_screen failed\n");
		nmp_tw_delete_group(p_group);
		return -1;
	}

	if (nmp_tw_set_focus_screen_div(p_screen, div_id, div_num) < 0)
	{
		error_msg("run step:set screen div error, div_num = %d\n", div_num);
		nmp_tw_delete_group(p_group);
		return -1;
	}

	step_t *p_step = nmp_tw_set_screen_step(p_screen, 1, 1);	//zyt,设置第一步，步长为1(这里步长只要不为0即可)
	if (!p_step)
	{
		nmp_tw_log_msg("add screen step failed\n");
		nmp_tw_delete_group(p_group);
		return -1;
	}

	if (nmp_tw_set_step_encoder(p_step, div_num, p_en) < 0)
	{
		nmp_tw_log_msg("set encoder failed\n");
		nmp_tw_delete_group(p_group);
		return -1;
	}

	if (nmp_tw_add_group(&p_tw->tw_group_vec_ok, p_group) < 0)
	{
		nmp_tw_log_msg("add group to vector failed\n");
		//nmp_tw_delete_group(p_group);
		return -1;
	}

	return 0;
}

void nmp_tw_get_screen_to_dec(dec_screen_t *p_dis_screen, int step,
	tw_screen_to_decoder *to_dec)
{
	group_t *p_gp = p_dis_screen->dis_group;
	int regular = !!(p_dis_screen->dis_flags & DIS_SCREEN_REG_MODE);
	int div_i = 0, divs = 0;
	division_mode_t *div_mode = &p_dis_screen->dis_pstep[step - 1].division_mode;
	division_mode_t *div_or = &p_dis_screen->dis_division_mode;

	memset(to_dec, 0, sizeof(tw_screen_to_decoder));
	to_dec->gp_type = p_gp->gp_type;
	strncpy(to_dec->gp_name, p_gp->gp_name, TW_MAX_VALUE_LEN - 1);
	to_dec->step_num = step;
	strncpy(to_dec->dis_guid, p_dis_screen->dis_guid, TW_ID_LEN - 1);
	to_dec->division_id = div_mode->division_id;
	to_dec->keep_other = regular;

	for (; div_i < TW_MAX_DIVISIONS; ++div_i)
	{
		if (regular && !div_or->divisions[div_i])	//判断是否属于gp
			continue;

		tw_division_to_decoder *div_to_dec = &to_dec->divisions[divs];
		encoder_t *p_ec = div_mode->divisions[div_i];
		if (p_ec)
		{
			log_3("p_ec is true\n");
			div_to_dec->division_num = div_i;
			div_to_dec->ec_name[TW_MAX_VALUE_LEN - 1] = '\0';
			strncpy(div_to_dec->ec_name, p_ec->ec_name, TW_MAX_VALUE_LEN - 1);
			div_to_dec->ec_dec_plug[TW_ID_LEN - 1] = '\0';
			strncpy(div_to_dec->ec_dec_plug, p_ec->ec_dec_plug, TW_ID_LEN - 1);

			if (strlen(p_ec->ec_url))
				div_to_dec->ec_url = str_dup(p_ec->ec_url);
			else
				div_to_dec->ec_url = NULL;
			divs++;
		}
		else
		{
			log_3("p_ec is false\n");
			if (regular)	//zyt，不变屏情况填空，变屏情况无需处理
			{
				div_to_dec->division_num = div_i;
				strcpy(div_to_dec->ec_name, "");
				div_to_dec->ec_url = NULL;
				divs++;
			}
		}
		log_3("div_to_dec->ec_name:%s\n", div_to_dec->ec_name);
	}

	to_dec->div_sum = divs;
}


void nmp_tw_get_screen_to_cu(dec_screen_t *p_dis_screen, int step,
	tw_screen_to_cu *to_cu)
{
	group_t *p_gp = p_dis_screen->dis_group;
	int regular = !!(p_dis_screen->dis_flags & DIS_SCREEN_REG_MODE);
	int div_i = 0, divs = 0;
	division_mode_t *div_mode = &p_dis_screen->dis_pstep[step - 1].division_mode;
	division_mode_t *div_or = &p_dis_screen->dis_division_mode;

	memset(to_cu, 0, sizeof(tw_screen_to_cu));
	strncpy(to_cu->session_id, p_gp->gp_session_id, TW_SESSION_ID_LEN - 1);
	to_cu->gp_type = p_gp->gp_type;
	strncpy(to_cu->gp_name, p_gp->gp_name, TW_MAX_VALUE_LEN - 1);
	to_cu->tw_id = p_gp->gp_tw_id;
	to_cu->screen_id = p_dis_screen->screen_id;
	to_cu->division_id = div_mode->division_id;
	to_cu->keep_other = regular;

	for (; div_i < TW_MAX_DIVISIONS; ++div_i)
	{
		if (regular && !div_or->divisions[div_i])
			continue;

		tw_division_to_cu *div_to_cu = &to_cu->divisions[divs];
		encoder_t *p_ec = div_mode->divisions[div_i];
		if (p_ec)
		{
			div_to_cu->division_num = div_i;
			div_to_cu->ec_name[TW_MAX_VALUE_LEN - 1] = '\0';
			strncpy(div_to_cu->ec_name, p_ec->ec_name, TW_MAX_VALUE_LEN - 1);
			div_to_cu->level = (int)(p_ec->ec_guid[LEVEL_POS] - '0');
			nmp_tw_get_channel_from_guid(p_ec->ec_guid, &div_to_cu->ec_channel);
			div_to_cu->result = TW_RES_SENT_TO_DEC_FAILED;
			divs++;
		}
		else
		{
			if (regular)
			{
				div_to_cu->division_num = div_i;
				strcpy(div_to_cu->ec_name, "");
				div_to_cu->result = TW_RES_SENT_TO_DEC_FAILED;
				divs++;
			}
		}
	}

	to_cu->div_sum = divs;
}

tw_screen_to_decoder_with_seq *nmp_tw_new_screen_to_dec()
{
	tw_screen_to_decoder_with_seq *to_dec_with_seq = NULL;

	to_dec_with_seq = st_malloc(sizeof(tw_screen_to_decoder_with_seq));

	return to_dec_with_seq;
}

int nmp_tw_destroy_screen_to_dec(tw_screen_to_decoder_with_seq *to_dec_with_seq,
	int size)
{
	log_3("nmp_tw_destroy_screen_to_dec\n");
	assert(to_dec_with_seq);

	tw_screen_to_decoder *to_dec = &to_dec_with_seq->screen_to_dec;
	int div_i = 0, div_sum = to_dec->div_sum;

	for (; div_i < div_sum; ++div_i)
	{
		tw_division_to_decoder *div_to_dec = &to_dec->divisions[div_i];
		if (div_to_dec->ec_url)
		{
			log_3("free ec_url\n");
			st_free(div_to_dec->ec_url);
		}
	}

	st_free(to_dec_with_seq);

	return 0;
}

void nmp_tw_screen_work_failed(dec_screen_t *p_dis_screen)
{
	assert(p_dis_screen);
}

static void nmp_tw_screen_to_decoder_print(tw_screen_to_decoder *to_dec)
{
	time_t now = time(NULL);
	char *time_out = ctime(&now);
	my_printf("\n------------------------------------> now time = %s", time_out);
	int div_i = 0;

	my_printf("  gp_type:    '%d'\n", get_gp_type_num(to_dec->gp_type));
	my_printf("  gp_name:    '%s'\n", to_dec->gp_name);
	my_printf("  step_num:   '%d'\n", to_dec->step_num);
	my_printf("  dis_guid:   '%s'\n", to_dec->dis_guid);
	my_printf("  division_id:'%d'\n", to_dec->division_id);
	my_printf("  keep_other: '%d'\n", to_dec->keep_other);
	my_printf("  div_sum:    '%d'\n", to_dec->div_sum);

	for (; div_i < to_dec->div_sum; ++div_i)
	{
		tw_division_to_decoder *div_to_dec = &to_dec->divisions[div_i];
		my_printf("div_i = %d\n", div_i);
		my_printf("    division_num:'%d'\n", div_to_dec->division_num);
		my_printf("    ec_name:     '%s'\n", div_to_dec->ec_name);
		my_printf("    ec_url:      '%s'\n", div_to_dec->ec_url);
	}
}


int
nmp_tw_is_first_time_run_group(group_t *p_gp)
{
	return (p_gp->gp_flags & GPF_FIRST_TIME_RUN) ? 1 : 0;
}

int
nmp_tw_is_out_action_time(time_t action_time)
{
	time_t cur_time;

	cur_time = time(NULL);
	if (cur_time > action_time && cur_time - action_time <= TW_ACTION_TIME_LEN)
		return 0;
	return 1;
}


int
nmp_tw_action_division_conflict(tw_screen_to_decoder *to_dec, action_step_t *action)
{
	int i;

	if (to_dec->division_id != action->division_id)
		return 1;

	for (i = 0; i < to_dec->div_sum; i++)
	{
		if (to_dec->divisions[i].division_num == action->division_num)
			return 1;
	}

	return 0;
}


int nmp_tw_notify_decoder(dec_screen_t *p_dis_screen, int step)
{
	assert(p_dis_screen);
	func_begin("\n");

	int error = TW_RES_EINVAL;
	tw_screen_to_decoder_with_seq *to_dec_with_seq = NULL;
	unsigned int seq;
	int is_action = 0;

	to_dec_with_seq = nmp_tw_new_screen_to_dec();
	if (!to_dec_with_seq)
	{
		error_msg("nmp_tw_new_screen_to_dec failed, no memory!\n");
		return -1;
	}

	tw_screen_to_decoder *to_dec = &to_dec_with_seq->screen_to_dec;
	nmp_tw_get_screen_to_dec(p_dis_screen, step, to_dec);
	seq = get_tw_dec_seq();
	p_dis_screen->seq = to_dec_with_seq->seq = seq;	//设置seq

	if (p_dis_screen->action.is_action)
	{
		tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();

		if (nmp_tw_is_first_time_run_group(p_dis_screen->dis_group) && //第一次运行group
			to_dec->division_id != p_dis_screen->action.division_id)
		{
			nmp_tw_set_action_sign_no_lock(&p_tw->tw_group_vec_ok,
				to_dec->dis_guid, 0, 0, 0, NULL);

			to_dec->keep_other = 0;
			is_action = 0;
		}
		else if (nmp_tw_is_out_action_time(p_dis_screen->action.action_time))
		{
			if (to_dec->division_id != p_dis_screen->action.division_id)
			{
				to_dec->keep_other = 0;
				p_dis_screen->action_special.seq = seq;
				p_dis_screen->action_special.keep_other = to_dec->keep_other;
			}

			nmp_tw_set_action_sign_no_lock(&p_tw->tw_group_vec_ok,
				to_dec->dis_guid, 0, 0, 0, NULL);

			is_action = 0;
		}
		else
			is_action = 1;
	}

	if (is_action)
	{
		if (to_dec->keep_other == 0 ||
			nmp_tw_action_division_conflict(to_dec, &p_dis_screen->action))
		{
			nmp_tw_destroy_screen_to_dec(to_dec_with_seq,
				sizeof(tw_screen_to_decoder_with_seq));
			log_msg("<Tw> being action, stop send screen to dec!!!\n");
			return 0;
		}
	}

	log_msg("1223335********** work,p_screen->screen_id:%d\n", p_dis_screen->screen_id);

#ifdef TW_TEST2
	nmp_tw_screen_to_decoder_print(to_dec);
#endif

	if (nmp_tw_info_handle(TW_INFO_SEND_SCREEN_TO_DEC, to_dec_with_seq, NULL) != 0)
	{
		nmp_tw_destroy_screen_to_dec(to_dec_with_seq,
			sizeof(tw_screen_to_decoder_with_seq));
		if (p_dis_screen->dis_group->gp_type == GPT_STEP)
		{
			if (nmp_tw_send_cmd_res(p_dis_screen->dis_group->gp_session_id,
				p_dis_screen->dis_group->gp_cu_seq, TW_RUN_STEP, -1001) < 0)
			{
				error_msg("send cmd_res to cu failed\n");
			}
		}
		else
		{
			tw_screen_to_cu to_cu;
			memset(&to_cu, 0, sizeof(tw_screen_to_cu));
			nmp_tw_get_screen_to_cu(p_dis_screen, step, &to_cu);

			if (nmp_tw_info_handle(TW_INFO_SEND_SCREEN_TO_CU, &to_cu, NULL) < 0)
			{
				error_msg("send screen_err to cu failed\n");
			}
		}

		goto notify_dec_end;
	}
	error = TW_RES_OK;

notify_dec_end:
	return error;
}


int nmp_tw_query_if_update_url(dec_screen_t *p_dis_screen, int step)
{
	int div_i;
	int regular = !!(p_dis_screen->dis_flags & DIS_SCREEN_REG_MODE);
	division_mode_t *div_mode = &p_dis_screen->dis_pstep[step - 1].division_mode;
	division_mode_t *div_or = &p_dis_screen->dis_division_mode;
	tw_update_url update_url;
	update_url.ec_position.gp_id = p_dis_screen->dis_group->gp_id;
	update_url.ec_position.screen_id = p_dis_screen->screen_id;
	update_url.ec_position.step_num = step;

	for (div_i = 0; div_i < TW_MAX_DIVISIONS; ++div_i)
	{
		if (regular && !div_or->divisions[div_i])	//判断是否属于gp
			continue;

		encoder_t *p_ec = div_mode->divisions[div_i];

		if (p_ec)
		{
			update_url.ec_position.division_num = div_i;

			update_url.dis_guid[TW_ID_LEN - 1] = '\0';
			strncpy(update_url.dis_guid, p_dis_screen->dis_guid, TW_ID_LEN - 1);
			update_url.dis_domain_id[TW_ID_LEN - 1] = '\0';
			strncpy(update_url.dis_domain_id, p_dis_screen->dis_domain_id,
				TW_ID_LEN - 1);

			update_url.ec_guid[TW_ID_LEN - 1] = '\0';
			strncpy(update_url.ec_guid, p_ec->ec_guid, TW_ID_LEN - 1);
			update_url.ec_domain_id[TW_ID_LEN - 1] = '\0';
			strncpy(update_url.ec_domain_id, p_ec->ec_domain_id, TW_ID_LEN - 1);

			update_url.ec_url[TW_MAX_URL_LEN - 1] = '\0';
			strncpy(update_url.ec_url, p_ec->ec_url, TW_MAX_URL_LEN - 1);

			if (nmp_tw_info_handle(TW_INFO_QUERY_IF_UPDATE_URL, &update_url,
				NULL) < 0)
			{
				error_msg("update url failed\n");
				return -1;
			}
		}
	}

	return 0;
}


void nmp_tw_screen_work(dec_screen_t *p_dis_screen)
{
	assert(p_dis_screen);
	func_begin("\n");

	int step;
	if (nmp_tw_is_invalid_screen(p_dis_screen))
		return ;

	step = p_dis_screen->dis_group->gp_now_step;
	if (step > p_dis_screen->dis_steps || step < 1)
	{
		nmp_tw_log_msg("gp %d step %d screen:%s error\n",
			p_dis_screen->dis_group->gp_id, step, p_dis_screen->dis_guid);
		return ;
	}

	if (p_dis_screen->dis_group->gp_type == GPT_STEP)
	{
		if (p_dis_screen->dis_flags & DIS_SCREEN_COMPLITE)
			return ;
		p_dis_screen->dis_flags |= DIS_SCREEN_COMPLITE;
	}

	if (nmp_tw_notify_decoder(p_dis_screen, step) < 0)
		nmp_tw_screen_work_failed(p_dis_screen);


	int next_step = p_dis_screen->dis_group->gp_now_step + 1;
	if (next_step > p_dis_screen->dis_group->gp_max_steps)
		next_step = 1;
	if (next_step > p_dis_screen->dis_steps)
		return ;

	if (nmp_tw_query_if_update_url(p_dis_screen, next_step) < 0)
	{
		error_msg("nmp_tw_query_if_update_url failed\n");
	}
	else
	{
		log_msg("nmp_tw_query_if_update_url success\n");
	}
}


int nmp_tw_get_step_time(group_t *p_gp, int step)
{
	assert(p_gp);

	int screen_i = 0, step_time = 0;
	dec_screen_t *p_screen;

	if (step < 1 || step > p_gp->gp_max_steps)
	{
		nmp_tw_log_msg("gp %d get step time, invalid step:%d",
			p_gp->gp_id, step);
		step = p_gp->gp_max_steps;
	}

	for (; screen_i < p_gp->gp_screens; ++screen_i)
	{
		p_screen = &p_gp->gp_dis_screens[screen_i];
		if (step <= p_screen->dis_steps &&
			p_screen->dis_pstep[step - 1].step_num)
		{
			step_time = p_screen->dis_pstep[step - 1].step_ttl;
			if (step_time > 0)	//此屏步为空时，step_time = -1
				break;
		}
	}

	return step_time;
}


void nmp_tw_group_work(group_t *p_gp)
{
	assert(p_gp);
	//func_begin("\n");

	int next_step, screen_i = 0;
	time_t now = time(NULL);
	int step_time = 1;

	if (p_gp->gp_flags & GPF_STOP)
		return ;

	for (;;)
	{
		if (p_gp->gp_max_steps == 0)
			return ;

		if (now < p_gp->gp_switch_time)
			return ;

		next_step = p_gp->gp_now_step + 1;
		if (next_step > p_gp->gp_max_steps)
			next_step = 1;

		if (next_step == p_gp->gp_now_step)	//只有一步
			return ;

		p_gp->gp_now_step = next_step;
		step_time = nmp_tw_get_step_time(p_gp, next_step);
		if (step_time)
		{
			p_gp->gp_switch_time = now + step_time;
			break;
		}
	}

	for (; screen_i < p_gp->gp_screens; ++screen_i)
		nmp_tw_screen_work(&p_gp->gp_dis_screens[screen_i]);
}


int nmp_tw_groups_work(group_vec_t *p_gv)
{
	assert(p_gv);
	//func_begin("\n");

	int group_i = 0;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp)
		{
			nmp_tw_group_work(p_gp);
			if (p_gp->gp_flags & GPF_FIRST_TIME_RUN)
				p_gp->gp_flags &= (!GPF_FIRST_TIME_RUN);

		}
	}
	pthread_mutex_unlock(&p_gv->gv_mutex);

	return 0;
}


int nmp_tw_groups_clear(group_vec_t *p_gv)
{
	assert(p_gv);
	func_begin("\n");

	int group_i = 0;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp)
		{
			p_gv->gv_parr[group_i] = NULL;
			--p_gv->gv_size;
			nmp_tw_delete_group(p_gp);
		}
	}
	pthread_mutex_unlock(&p_gv->gv_mutex);

	return 0;
}


int nmp_tw_deal_tour_auto_jump(group_vec_t *p_gv, unsigned int seq)
{
	assert(p_gv);
	func_begin("\n");

	int group_i = 0, screen_i = 0;
	dec_screen_t *p_screen = NULL;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (!p_gp || p_gp->gp_type != GPT_TOUR)
			continue;

		for (screen_i = 0; screen_i < p_gp->gp_screens; ++screen_i)
		{
			p_screen = &p_gp->gp_dis_screens[screen_i];
			if (p_screen->seq == seq)
			{
				time_t now = time(NULL);
				int step_time = 1;
				int next_step = p_gp->gp_now_step + 1;

				if (next_step > p_gp->gp_max_steps)
					next_step = 1;

				if (next_step == p_gp->gp_now_step)	//只有一步，无需再跳转
				{
					nmp_tw_log_msg("the tour has only one step, not jump again\n");
					pthread_mutex_unlock(&p_gv->gv_mutex);
					return 0;
				}

				p_gp->gp_now_step = next_step;
				step_time = nmp_tw_get_step_time(p_gp, next_step);
				assert(step_time > 0);
				if (step_time <= 0)
					step_time = 1;
				p_gp->gp_switch_time = now + step_time;

				nmp_tw_screen_work(p_screen);
				pthread_mutex_unlock(&p_gv->gv_mutex);
				return 0;
			}
		}
	}

	pthread_mutex_unlock(&p_gv->gv_mutex);
	return -1;
}


static int nmp_tw_get_tour_msg(int tour_id, tw_tour_msg_response *tour_msg)
{
	func_begin("\n");
	tw_tour_msg_request req;

	req.tour_id = tour_id;

	if (nmp_tw_info_handle(TW_INFO_GET_TOUR, &req, tour_msg) < 0)
	{
		error_msg("get tour msg failed\n");
		return -1;
	}

	if (!tour_msg->steps)
		return -1;
	if (!tour_msg->step_count)
	{
		st_free(tour_msg->steps);
		return -1;
	}

	return 0;
}

static int nmp_tw_get_group_msg(int group_id, tw_group_msg_response *group_msg)
{
	tw_group_msg_request req;

	req.group_id = group_id;

	if (nmp_tw_info_handle(TW_INFO_GET_GROUP, &req, group_msg) < 0)
	{
		error_msg("get group msg failed\n");
		return -1;
	}

	return 0;
}

static int nmp_tw_get_group_step_n(int group_id, int step_num,
	tw_group_step_n_response *group_step)
{
	tw_group_step_n_request req;

	req.group_id = group_id;
	req.step_num = step_num;

	if (nmp_tw_info_handle(TW_INFO_GET_GROUP_STEP_N, &req, group_step) < 0)
	{
		error_msg("get group step_n failed\n");
		return -1;
	}
/*
	if (!group_step->screens)
		return -1;
	if (!group_step->screen_sum)
	{
		st_free(group_step->screens);
		return -1;
	}
*/
	return 0;
}

static int nmp_tw_get_dis_guid(int *tw_id, int *screen_id,
	tw_general_guid *dis_guid)
{
	tw_dis_guid_request req;
	tw_dis_guid_response rsp;

	req.tw_id = *tw_id;
	req.screen_id = *screen_id;

	if (nmp_tw_info_handle(TW_INFO_GET_DIS_GUID, &req, &rsp) < 0)
	{
		error_msg("get dis_guid failed\n");
		return -1;
	}
	dis_guid->guid[TW_ID_LEN - 1] = '\0';
	strncpy(dis_guid->guid, rsp.dis_guid.guid, TW_ID_LEN - 1);
	dis_guid->domain_id[TW_ID_LEN - 1] = '\0';
	strncpy(dis_guid->domain_id, rsp.dis_guid.domain_id, TW_ID_LEN - 1);

	return 0;
}


static int nmp_tw_get_ec_url(char dis_guid[], char dis_domain_id[],
	char ec_guid[], char ec_domain_id[], char ec_url[], char ec_dec_plug[])
{
	tw_ec_url_request req;
	tw_ec_url_response rsp;
	memset(&req, 0, sizeof(tw_ec_url_request));
	memset(&rsp, 0, sizeof(tw_ec_url_response));

	strncpy(req.dis_guid, dis_guid, TW_ID_LEN - 1);
	strncpy(req.dis_domain_id, dis_domain_id, TW_ID_LEN - 1);
	strncpy(req.ec_guid, ec_guid, TW_ID_LEN - 1);
	strncpy(req.ec_domain_id, ec_domain_id, TW_ID_LEN - 1);

	if (nmp_tw_info_handle(TW_INFO_GET_EC_URL, &req, &rsp) < 0)
	{
		error_msg("get ec_url failed\n");
		return -1;
	}
	ec_url[TW_MAX_URL_LEN - 1] = '\0';
	strncpy(ec_url, rsp.ec_url, TW_MAX_URL_LEN - 1);
	ec_dec_plug[TW_ID_LEN - 1] = '\0';
	strncpy(ec_dec_plug, rsp.ec_dec_plug, TW_ID_LEN - 1);

	return 0;
}


int nmp_tw_get_decoder(tw_run_step_request *msg, tw_general_guid *dis_guid,
	int *tw_id, int *screen_id, int *div_num, int *div_id)
{
	*tw_id = msg->tw_id;
	*screen_id = msg->screen_id;

	if (nmp_tw_get_dis_guid(tw_id, screen_id, dis_guid) < 0)
	{
		return -1;
	}

	*div_num = msg->division_num;
	*div_id = msg->division_id;

	return 0;
}


int nmp_tw_get_tour_decoder(tw_run_tour_request *msg, tw_general_guid *dis_guid,
	int *tw_id, int *screen_id, int *div_num, int *div_id)
{
	*tw_id = msg->tw_id;
	*screen_id = msg->screen_id;

	if (nmp_tw_get_dis_guid(tw_id, screen_id, dis_guid) < 0)
	{
		return -1;
	}

	*div_num = msg->division_num;
	*div_id = msg->division_id;

	return 0;
}


int nmp_tw_get_encoder(encoder_t *p_en, tw_general_guid *dis_guid,
	tw_run_step_request *msg)
{
	assert(msg && p_en);

	memset(p_en, 0, sizeof(encoder_t));

	strncpy(p_en->ec_domain_id, msg->ec_domain_id, TW_ID_LEN - 1);
	strncpy(p_en->ec_guid, msg->ec_guid, TW_ID_LEN - 1);
	strncpy(p_en->ec_name, msg->ec_name, TW_MAX_VALUE_LEN - 1);

	if (nmp_tw_get_ec_url(dis_guid->guid, dis_guid->domain_id, p_en->ec_guid,
		p_en->ec_domain_id, p_en->ec_url, p_en->ec_dec_plug) < 0)
	{
		return -1;
	}

	return 0;
}


int nmp_tw_get_tour_encoder(encoder_t *p_en, tw_general_guid *dis_guid,
	tw_tour_step_response*msg)
{
	assert(msg && p_en);

	memset(p_en, 0, sizeof(encoder_t));

	strncpy(p_en->ec_domain_id, msg->ec_domain_id, TW_ID_LEN - 1);
	strncpy(p_en->ec_guid, msg->ec_guid, TW_ID_LEN - 1);
	p_en->ec_guid[LEVEL_POS] = (char)(msg->level % 10 + '0');	//设置码流
	strncpy(p_en->ec_name, msg->ec_name, TW_MAX_VALUE_LEN - 1);

	if (nmp_tw_get_ec_url(dis_guid->guid, dis_guid->domain_id, p_en->ec_guid,
		p_en->ec_domain_id, p_en->ec_url, p_en->ec_dec_plug) < 0)
	{
		return -1;
	}

	return 0;
}


int nmp_tw_get_group_step_encoder(encoder_t *p_en,
	tw_group_screen_response *screen, tw_group_division_response *msg)
{
	assert(msg && p_en);

	memset(p_en, 0, sizeof(encoder_t));

	strncpy(p_en->ec_domain_id, msg->ec_domain_id, TW_ID_LEN - 1);
	strncpy(p_en->ec_guid, msg->ec_guid, TW_ID_LEN - 1);
	p_en->ec_guid[LEVEL_POS] = (char)(msg->level % 10 + '0');	//设置码流
	strncpy(p_en->ec_name, msg->ec_name, TW_MAX_VALUE_LEN - 1);

	if (nmp_tw_get_ec_url(screen->dis_guid, screen->dis_domain_id,
		p_en->ec_guid, p_en->ec_domain_id, p_en->ec_url, p_en->ec_dec_plug) < 0)
	{
		return -1;
	}

	return 0;
}


void nmp_tw_get_div_pos(division_pos_t *div_pos, tw_division_position *msg)
{
	assert(msg);

	div_pos->tw_id = msg->tw_id;
	div_pos->screen_id = msg->screen_id;
	div_pos->division_num = msg->division_num;
	div_pos->division_id = -1;
}

void nmp_tw_get_div_pos_2(division_pos_t *div_pos, tw_operate *msg)
{
	assert(msg);

	div_pos->tw_id = msg->tw_id;
	div_pos->screen_id = msg->screen_id;
	div_pos->division_num = msg->division_num;
	div_pos->division_id = msg->division_id;
}

int  nmp_tw_check_tour_step_num(tw_tour_msg_response *tour_msg_rsp)
{
	int count = tour_msg_rsp->step_count;
	int i;

	for (i = 0; i < count; i++)
	{
		if (tour_msg_rsp->steps[i].step_num != i + 1)
			return -1;
	}
	return 0;
}

static void nmp_tw_uninit_tour_msg_rsp(tw_tour_msg_response *rsp)
{
	assert(rsp);

	if (rsp->steps)
		st_free(rsp->steps);
	memset(rsp, 0, sizeof(tw_tour_msg_response));
}

int nmp_tw_get_tour_info(group_t *p_gp, int tour_id, int div_num,
	tw_general_guid *dis_guid)
{
	assert(p_gp);
	func_begin("\n");

	tw_tour_msg_response tour_msg_rsp;
	encoder_t encoder;
	int step_count = 0, step_i = 0, step_num, intr;
	int err = -1;
	memset(&tour_msg_rsp, 0, sizeof(tw_tour_msg_response));
	memset(&encoder, 0, sizeof(encoder_t));

	if (nmp_tw_get_tour_msg(tour_id, &tour_msg_rsp) < 0)
	{
		error_msg("nmp_tw_get_tour_msg failed\n");
		goto get_tour_end;
	}
	if (nmp_tw_check_tour_step_num(&tour_msg_rsp) != 0)
	{
		error_msg("nmp_tw_check_tour_step_num error\n");
		goto get_tour_end;
	}

	nmp_tw_set_group_num(p_gp, tour_msg_rsp.tour_num);
	nmp_tw_set_auto_jump(p_gp, tour_msg_rsp.auto_jump);
	nmp_tw_set_group_name(p_gp, tour_msg_rsp.tour_name);

	dec_screen_t *p_screen = nmp_tw_get_tour_dis_screen(p_gp);
	if (!p_screen)
	{
		error_msg("nmp_tw_get_tour_dis_screen error\n");
		goto get_tour_end;
	}

	step_count = tour_msg_rsp.step_count;

	for (step_i = 0; step_i < step_count; ++step_i)
	{
		tw_tour_step_response *step_rsp = &tour_msg_rsp.steps[step_i];
		step_num = step_rsp->step_num;	//zyt填充时注意
		intr = step_rsp->step_ttl;

		step_t *p_step = nmp_tw_set_screen_step(p_screen,
			step_num, intr);
		if (!p_step)
		{
			error_msg("set screen step failed\n");
			goto get_tour_end;
		}

		if (nmp_tw_get_tour_encoder(&encoder, dis_guid, step_rsp) < 0)		//获取url失败不退出
		{
			error_msg("get tour encoder failed\n");
		}
		if (nmp_tw_set_step_encoder(p_step, div_num, &encoder) < 0)
		{
			error_msg("set step encoder failed\n");
			goto get_tour_end;
		}
	}

	err = 0;

get_tour_end:
	nmp_tw_uninit_tour_msg_rsp(&tour_msg_rsp);
	return err;
}


static dec_screen_t *get_empty_screen(group_t *p_gp)
{
	int screen_i;
	for (screen_i = 0; screen_i < p_gp->gp_screens; screen_i++)
	{
		if (p_gp->dis_screen_init[screen_i].screen_id == EMPTY_SCREEN_ID)
			return &p_gp->dis_screen_init[screen_i];
	}
	return NULL;
}

int nmp_tw_get_group_info(group_t *p_gp, int group_id)
{
	assert(p_gp);

	int step_count, step_i, intr;
	int get_step_err = 0, err = -1;

	tw_group_msg_response group_msg_rsp;
	memset(&group_msg_rsp, 0, sizeof(tw_group_msg_response));

	if (nmp_tw_get_group_msg(group_id, &group_msg_rsp) < 0)
	{
		error_msg("nmp_tw_get_group_msg failed\n");
		goto get_group_end;
	}

	nmp_tw_set_group_tw_id(p_gp, group_msg_rsp.tw_id);
	nmp_tw_set_group_num(p_gp, group_msg_rsp.group_num);
	nmp_tw_set_group_name(p_gp, group_msg_rsp.group_name);

	step_count = group_msg_rsp.step_count;
	for (step_i = 0; step_i < step_count; ++step_i)
	{
		tw_group_step_n_response group_step_rsp;
		memset(&group_step_rsp, 0, sizeof(tw_group_step_n_response));
		if (nmp_tw_get_group_step_n(group_id, group_msg_rsp.step_num[step_i], &group_step_rsp) < 0)
		{
			error_msg("nmp_tw_get_group_step_n failed\n");
			goto get_group_end;
		}
		intr = group_step_rsp.step_ttl;

		//如果屏数为0，则增加一个空屏作为标识，在调用nmp_tw_verify_dis_screen会视为非正常屏
		if (group_step_rsp.screen_sum == 0)
		{
			dec_screen_t *empty_screen = get_empty_screen(p_gp);
			if (!empty_screen)
			{
				empty_screen = nmp_tw_add_dis_screen(p_gp, "", "", EMPTY_SCREEN_ID);
				if (empty_screen == NULL)
				{
					error_msg("nmp_tw_add_dis_screen failed\n");
					get_step_err = 1;
					goto get_group_step_end;
				}
			}
			step_t *empty_step = nmp_tw_set_screen_step(empty_screen,
				step_i + 1, intr);
			if (empty_step == NULL)
			{
				error_msg("nmp_tw_set_screen_step failed\n");
				get_step_err = 1;
				goto get_group_step_end;
			}
		}

		int screen_i = 0;
		for (; screen_i < group_step_rsp.screen_sum; ++screen_i)
		{
			tw_group_screen_response *screen_rsp =
				&group_step_rsp.screens[screen_i];
			dec_screen_t *p_screen = NULL;
			int index = 0;

			/* 查找对应的屏是否存在 */
			for (; index < p_gp->gp_screens; ++index)
			{
				if (!strcmp(p_gp->gp_dis_screens[index].dis_guid,
					screen_rsp->dis_guid))
				{
					p_screen = &p_gp->gp_dis_screens[index];
					break;
				}
			}

			if (p_screen == NULL)
			{
				p_screen = nmp_tw_add_dis_screen(p_gp, screen_rsp->dis_guid,
					screen_rsp->dis_domain_id, screen_rsp->screen_id);
				if (p_screen == NULL)
				{
					error_msg("nmp_tw_add_dis_screen failed\n");
					get_step_err = 1;
					goto get_group_step_end;
				}
			}

			step_t *p_step = nmp_tw_set_screen_step(p_screen,
				step_i + 1, intr);
			if (p_step == NULL)
			{
				error_msg("nmp_tw_set_screen_step error\n");
				get_step_err = 1;
				goto get_group_step_end;
			}
			nmp_tw_set_step_div_id(p_step, screen_rsp->division_id);

			/* 获取分割信息 */
			int div_i = 0, div_sum = screen_rsp->div_sum;
			if (div_sum > TW_MAX_DIVISIONS)
			{
				error_msg("div_sum:%d > TW_MAX_DIVISIONS\n", div_sum);
				goto get_group_step_end;
			}
			for (; div_i < div_sum; ++div_i)
			{
				encoder_t encoder;
				memset(&encoder, 0, sizeof(encoder_t));
				tw_group_division_response *div_rsp =
					&screen_rsp->divisions[div_i];

				if (nmp_tw_get_group_step_encoder(&encoder, screen_rsp, div_rsp) < 0)
				{
					error_msg("nmp_tw_get_group_step_encoder failed\n");	//获取url失败不退出
				}
				if (nmp_tw_set_step_encoder(p_step, div_rsp->division_num,
					&encoder) < 0)
				{
					nmp_tw_log_msg("group, set step encoder:%s error\n",
						encoder.ec_guid);
					get_step_err = 1;
					goto get_group_step_end;
				}
			}

		}
get_group_step_end:
		if (group_step_rsp.screens)
			st_free(group_step_rsp.screens);
		if (get_step_err)
			break;

	}

	if (!get_step_err)
		err = 0;

get_group_end:

	return err;
}


int nmp_tw_run_step(tw_run_step_request_with_seq *msg_with_seq)
{
	assert(msg_with_seq);
	func_begin("\n");

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	tw_run_step_request *msg = &msg_with_seq->req;
	char session_id[TW_SESSION_ID_LEN] = {0};
	unsigned int cu_seq;
	encoder_t encoder;
	tw_general_guid dis_guid;
	int tw_id;
	int screen_id;
	int division_num = 0;
	int division_id;
	int error = TW_RES_EINVAL;
	memset(&encoder, 0, sizeof(encoder_t));
	memset(&dis_guid, 0, sizeof(tw_general_guid));

	if (nmp_tw_get_decoder(msg, &dis_guid, &tw_id, &screen_id, &division_num,
		&division_id) < 0)
	{
		error_msg("nmp_tw_get_decoder error\n");
		goto run_step_end;
	}

	if (nmp_tw_get_encoder(&encoder, &dis_guid, msg) < 0)
	{
		error_msg("nmp_tw_get_encoder error\n");
		goto run_step_end;
	}

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	cu_seq = msg_with_seq->seq;

	if(nmp_tw_do_step(p_tw, &dis_guid, tw_id, screen_id, division_num,
		division_id, &encoder, session_id, cu_seq) < 0)
		goto run_step_end;
	error = TW_RES_OK;

run_step_end:
	//nmp_tw_send_cmd_res(session_id, TW_RUN_STEP, -1, error);
	return error;
}


int nmp_tw_run_tour(tw_run_tour_request *msg)
{
	assert(msg);
	func_begin("\n");

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	tw_general_guid dis_guid;
	int tw_id;
	int screen_id;
	int division_num = 0;
	int division_id;
	int tour_id;
	int error = TW_RES_EINVAL;
	memset(&dis_guid, 0, sizeof(tw_general_guid));

	if (nmp_tw_get_tour_decoder(msg, &dis_guid, &tw_id, &screen_id, &division_num,
		&division_id) < 0)
	{
		error_msg("nmp_tw_get_tour_decoder error\n");
		goto run_tour_end;
	}

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	tour_id = msg->tour_id;

	group_t *p_gp = nmp_tw_new_group(tour_id, GPT_TOUR, session_id, 0);
	if (!p_gp)
		goto run_tour_end;

	nmp_tw_set_group_tw_id(p_gp, tw_id);

	dec_screen_t *p_screen = nmp_tw_add_dis_screen(p_gp, dis_guid.guid,
		dis_guid.domain_id, screen_id);
	if (!p_screen)
	{
		nmp_tw_log_msg("run tour %d nmp_tw_add_dis_screen failed\n", tour_id);
		nmp_tw_delete_group(p_gp);
		goto run_tour_end;
	}

	if (nmp_tw_set_focus_screen_div(p_screen, division_id, division_num) < 0)
	{
		error_msg("run tour %d, nmp_tw_add_dis_screen failed\n", tour_id);
		nmp_tw_delete_group(p_gp);
		goto run_tour_end;
	}

	if (nmp_tw_get_tour_info(p_gp, tour_id, division_num, &dis_guid) < 0)
	{
		nmp_tw_log_msg("tour %d get info error\n", p_gp->gp_id);
		nmp_tw_delete_group(p_gp);
		goto run_tour_end;
	}

	if (nmp_tw_add_group(&p_tw->tw_group_vec_ok, p_gp) != 0)
	{
		goto run_tour_end;
	}

	nmp_tw_log_msg("run tour %d OK\n", tour_id);
	error = TW_RES_OK;

run_tour_end:
	//nmp_tw_send_cmd_res(session_id, TW_RUN_TOUR, tour_id, error);
	return error;
}


int nmp_tw_run_group(tw_run_group_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	int group_id;
	int error = TW_RES_EINVAL;

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	group_id = msg->group_id;

	group_t *p_gp = nmp_tw_new_group(group_id, GPT_GROUP, session_id, 0);
	if (p_gp == NULL)
	{
		goto run_group_end;
	}

	if (nmp_tw_get_group_info(p_gp, group_id) < 0)
	{
		nmp_tw_log_msg("group %d get info error\n", p_gp->gp_id);
		nmp_tw_delete_group(p_gp);
		goto run_group_end;
	}

	if (nmp_tw_add_group(&p_tw->tw_group_vec_ok, p_gp) != 0)
	{
		goto run_group_end;
	}

	nmp_tw_log_msg("run group %d OK\n", group_id);
	error = TW_RES_OK;

run_group_end:
	//nmp_tw_send_cmd_res(session_id, TW_RUN_GROUP, group_id, error);
	return error;
}


static void
nmp_tw_set_action_sign_no_lock(group_vec_t *p_gv, char *dis_guid,
	int enable_action, int division_id, int division_num, int *keep_other)
{
	int group_i = 0, screen_i = 0;
	dec_screen_t *p_screen = NULL;
	action_step_t *action_step = NULL;
	int is_keep_other = 1;

	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (!p_gp)
			continue;

		for (screen_i = 0; screen_i < p_gp->gp_screens; ++screen_i)
		{
			p_screen = &p_gp->gp_dis_screens[screen_i];

			if (strcmp(p_screen->dis_guid, dis_guid))
				continue;

			if (p_screen->dis_flags & DIS_SCREEN_STOP)
				continue;

			action_step = &p_screen->action;
			if(!enable_action)
				action_step->is_action = 0;
			else
			{
				action_step->is_action = 1;
				action_step->action_time = time(NULL);
				action_step->division_id = division_id;
				action_step->division_num = division_num;
				if (p_screen->dis_division_mode.division_id != division_id)
					is_keep_other = 0;
			}
			break;
		}
	}
	if (keep_other)
		*keep_other = is_keep_other;
}


static void
nmp_tw_set_action_sign(group_vec_t *p_gv, char *dis_guid,
	int enable_action, int division_id, int division_num, int *keep_other)
{
	pthread_mutex_lock(&p_gv->gv_mutex);
	nmp_tw_set_action_sign_no_lock(p_gv, dis_guid, enable_action,
		division_id, division_num, keep_other);
	pthread_mutex_unlock(&p_gv->gv_mutex);
}

static int
nmp_tw_get_action_screen_to_dec(tw_screen_to_decoder *to_dec,
	tw_run_step_request *msg)
{
	tw_general_guid dis_guid;
	encoder_t encoder;
	int tw_id;
	int screen_id;
	int division_num = 0;
	int division_id;
	tw_division_to_decoder *division;
	memset(&dis_guid, 0, sizeof(tw_general_guid));
	memset(&encoder, 0, sizeof(encoder_t));
	memset(to_dec, 0, sizeof(tw_screen_to_decoder));

	if (nmp_tw_get_decoder(msg, &dis_guid, &tw_id, &screen_id, &division_num,
		&division_id) < 0)
	{
		error_msg("nmp_tw_get_decoder error\n");
		return -1;
	}

	if (nmp_tw_get_encoder(&encoder, &dis_guid, msg) < 0)
	{
		error_msg("nmp_tw_get_encoder error\n");
		return -1;
	}

	to_dec->gp_type = GPT_ACTION_STEP;
	to_dec->step_num = 1;
	strncpy(to_dec->dis_guid, dis_guid.guid, TW_ID_LEN - 1);
	to_dec->division_id = division_id;
	to_dec->keep_other = 0;	//需再次查找确定此值
	to_dec->div_sum = 1;

	division = &to_dec->divisions[0];
	division->division_num = division_num;
	strncpy(division->ec_name, encoder.ec_name, TW_MAX_VALUE_LEN - 1);
	strncpy(division->ec_dec_plug, encoder.ec_dec_plug, TW_ID_LEN - 1);
	division->ec_url = str_dup(encoder.ec_url);

	return 0;
}


int
nmp_tw_run_action_step(tw_run_step_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	tw_screen_to_decoder_with_seq *to_dec_with_seq = NULL;
	tw_screen_to_decoder *to_dec;
	unsigned int seq;
	int keep_other;
	int ret;

	to_dec_with_seq = nmp_tw_new_screen_to_dec();
	if (!to_dec_with_seq)
	{
		error_msg("nmp_tw_new_screen_to_dec failed, no memory!\n");
		return -1;
	}

	to_dec = &to_dec_with_seq->screen_to_dec;
	ret = nmp_tw_get_action_screen_to_dec(to_dec, msg);
	if (ret)
	{
		error_msg("nmp_tw_get_action_screen_to_dec failed\n");
		goto err;
	}

	nmp_tw_set_action_sign(&p_tw->tw_group_vec_ok, to_dec->dis_guid, 1,
		to_dec->division_id, to_dec->divisions[0].division_num, &keep_other);

	to_dec->keep_other = keep_other;

	seq = get_tw_dec_seq();
	to_dec_with_seq->seq = seq;

	if (nmp_tw_info_handle(TW_INFO_SEND_SCREEN_TO_DEC, to_dec_with_seq, NULL) != 0)
	{
		error_msg("TW_INFO_SEND_SCREEN_TO_DEC failed\n");
		goto err;
	}

	nmp_tw_action_pool_add_info(seq, msg->tw_id, msg->screen_id,
		keep_other);

	return 0;
err:
	if (to_dec_with_seq)
	{
		nmp_tw_destroy_screen_to_dec(to_dec_with_seq,
			sizeof(tw_screen_to_decoder_with_seq));
	}
	return -1;
}


int nmp_tw_stop_tour(tw_stop_tour_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	int tour_id;
	int error = TW_RES_EINVAL;

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	tour_id = msg->tour_id;

	error = nmp_tw_stop_run(&p_tw->tw_group_vec_ok, GPT_TOUR, tour_id);

//stop_tour_end:
	//nmp_tw_send_cmd_res(session_id, TW_STOP_TOUR, tour_id, error);
	return error;
}

int nmp_tw_stop_group(tw_stop_group_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	int group_id;
	int error = TW_RES_EINVAL;

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	group_id = msg->group_id;

	error = nmp_tw_stop_run(&p_tw->tw_group_vec_ok, GPT_GROUP, group_id);

//stop_group_end:
	//nmp_tw_send_cmd_res(session_id, TW_STOP_GROUP, group_id, error);
	return error;
}

int nmp_tw_if_run_tour(tw_if_run_tour_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	int tour_id;
	int error = TW_RES_EINVAL;

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	tour_id = msg->tour_id;

	//此tour存在则返回0
	error = nmp_tw_query_state(&p_tw->tw_group_vec_ok, GPT_TOUR, tour_id);

//query_tour_end:
	//nmp_tw_send_cmd_res(session_id, TW_IF_RUN_TOUR, tour_id, error);
	return error;
}

int nmp_tw_if_run_group(tw_if_run_group_request *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	int group_id;
	int error = TW_RES_EINVAL;

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	group_id = msg->group_id;

	error = nmp_tw_query_state(&p_tw->tw_group_vec_ok, GPT_GROUP, group_id);

//query_group_end:
	//nmp_tw_send_cmd_res(session_id, TW_IF_RUN_GROUP, group_id, error);
	return error;
}

int nmp_tw_stop_gpt_by_division(tw_division_position *msg)
{
	assert(msg);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	char session_id[TW_SESSION_ID_LEN] = {0};
	division_pos_t div_pos;
	int found = 0;
	int error = TW_RES_EINVAL;

	if (msg->division_num < 0 || msg->division_num >= TW_MAX_DIVISIONS)
	{
		error_msg("division_num(%d) error!\n", msg->division_num);
		goto end;
	}

	strncpy(session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	nmp_tw_get_div_pos(&div_pos, msg);

	found = nmp_tw_stop_run_by_div(&p_tw->tw_group_vec_ok, &div_pos,
		TW_STOP_TYPE_GPT);
	error = (found ? TW_RES_OK : TW_RES_ERROR);

end:
	//nmp_tw_send_cmd_res(session_id, TW_STOP_GPT_BY_DIVISION, -1, error);
	return error;
}


int nmp_tw_screen_operate(tw_operate_with_seq *msg_with_seq, TW_INFO_TYPE type)
{
	assert(msg_with_seq);
	log_msg("1223336********** operate_type:%s\n", get_operate_type(type));

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	tw_operate_to_decoder_with_seq to_dec_with_seq;
	memset(&to_dec_with_seq, 0, sizeof(tw_operate_to_decoder_with_seq));
	tw_operate *msg = &msg_with_seq->operate;
	tw_operate_to_decoder *to_dec = &to_dec_with_seq.operate_to_dec;
	int error = TW_RES_EINVAL;

	if (msg->division_num < -1 || msg->division_num >= TW_MAX_DIVISIONS)
	{
		error_msg("division_num(%d) error!\n", msg->division_num);
		goto end;
	}

	if (get_operate_info_to_dec(to_dec, msg) < 0)
	{
		error_msg("get_operate_info_to_dec failed\n");
		goto end;
	}

	switch (type)
	{
	case TW_CLEAR_TO_DEC:
	case TW_CHANGE_DIVISION_MODE_TO_DEC:
		{
			division_pos_t div_pos;
			nmp_tw_get_div_pos_2(&div_pos, msg);

			if (type == TW_CLEAR_TO_DEC)
				nmp_tw_stop_run_by_div(&p_tw->tw_group_vec_ok, &div_pos,
					TW_STOP_TYPE_DIVISION);		//能保证停止即可，无需判断是否通过此操作停止
			else if (type == TW_CHANGE_DIVISION_MODE_TO_DEC)
				nmp_tw_stop_run_by_div(&p_tw->tw_group_vec_ok, &div_pos,
					TW_STOP_TYPE_SCREEN_BY_DIVISION_ID);
		}
	case TW_FULL_SCREEN_TO_DEC:
	case TW_EXIT_FULL_SCREEN_TO_DEC:
		{
			if (type == TW_CLEAR_TO_DEC && msg->operate_mode == 1)		//停止画面
			{
				nmp_tw_deal_stop_res(msg_with_seq);
				break;
			}

			to_dec_with_seq.seq = get_tw_dec_operate_seq();
			nmp_tw_save_dec_operate_info(msg_with_seq, to_dec_with_seq.seq);

			if (nmp_tw_info_handle(type, &to_dec_with_seq, NULL) < 0)
			{
				error_msg("send nmp_tw_screen_operate info to decoder failed\n");
				goto end;
			}
			break;
		}
	default:
		{
			error_msg("type error\n");
			goto end;
		}
	}

	error = TW_RES_OK;

end:
	return error;
}


int nmp_tw_get_action_tw_info(tw_screen_to_cu *to_cu, unsigned int seq)
{
	int tw_id;
	int screen_id;
	int keep_other;
	int ret;

	ret = nmp_tw_action_pool_get_info(seq, &tw_id, &screen_id, &keep_other);
	if (ret)
	{
		return -1;
	}

	to_cu->gp_type = GPT_ACTION_STEP;
	to_cu->tw_id = tw_id;
	to_cu->screen_id = screen_id;
	to_cu->keep_other = keep_other;

	return 0;
}


int nmp_tw_deal_decoder_res(tw_decoder_rsp_with_seq *dec_rsp_with_seq)
{
	assert(dec_rsp_with_seq);

	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	tw_decoder_rsp *dec_rsp = &dec_rsp_with_seq->dec_rsp;
	unsigned int seq = dec_rsp_with_seq->seq;
	tw_screen_to_cu to_cu;
	unsigned int cu_seq = 0;
	int tour_auto_jump = 0;
	int div_i = 0;
	int error = TW_RES_OK;
	memset(&to_cu, 0, sizeof(tw_screen_to_cu));

	if (nmp_tw_get_tw_info(&to_cu, &cu_seq, &tour_auto_jump,
		&p_tw->tw_group_vec_ok, seq) != 0)
	{
		if (nmp_tw_get_action_tw_info(&to_cu, seq) != 0)
		{
			log_msg("nmp_tw_get_action_tw_info failed\n");
			error = TW_RES_EINVAL;
			goto deal_res_end;
		}
		//log_msg("nmp_tw_get_action_tw_info success\n");
	}

	to_cu.division_id = dec_rsp->division_id;
	to_cu.div_sum = dec_rsp->div_sum;
	for (div_i = 0; div_i < dec_rsp->div_sum; ++div_i)
		to_cu.divisions[div_i] = dec_rsp->divisions[div_i];

	if (to_cu.gp_type == GPT_TOUR && to_cu.divisions[0].result != 0 &&
		tour_auto_jump)		//巡回运行失败时自动跳转处理
	{
		if (nmp_tw_deal_tour_auto_jump(&p_tw->tw_group_vec_ok, seq) < 0)
		{
			error_msg("nmp_tw_deal_tour_auto_jump failed\n");
			error = TW_RES_EINVAL;
		}
	}

	if (to_cu.gp_type == GPT_STEP)
	{
		if (nmp_tw_send_cmd_res(to_cu.session_id, cu_seq, TW_RUN_STEP,
			to_cu.divisions[0].result) < 0)
		{
			error_msg("send cmd_res to cu failed\n");
			error = TW_RES_EINVAL;
		}
		if (to_cu.divisions[0].result != 0)		//单步运行失败，将不发送通告
			goto deal_res_end;
	}

	if (nmp_tw_info_handle(TW_INFO_SEND_SCREEN_TO_CU, &to_cu, NULL) < 0)
	{
		error_msg("send pu_res to cu failed\n");
		error = TW_RES_EINVAL;
	}

deal_res_end:

	return error;
}


static int nmp_tw_deal_stop_res(tw_operate_with_seq *msg_with_seq)
{
	tw_operate_result_to_cu_with_seq to_cu_with_seq;
	memset(&to_cu_with_seq, 0, sizeof(tw_operate_result_to_cu_with_seq));

	to_cu_with_seq.seq = msg_with_seq->seq;
	tw_operate *msg = &msg_with_seq->operate;
	tw_operate_result_to_cu *to_cu = &to_cu_with_seq.to_cu;

	strncpy(to_cu->session_id, msg->session_id, TW_SESSION_ID_LEN - 1);
	to_cu->operate_type = TW_OPERATE_CLEAR;
	to_cu->tw_id = msg->tw_id;
	to_cu->screen_id = msg->screen_id;
	to_cu->division_id = msg->division_id;
	to_cu->division_num = msg->division_num;
	to_cu->operate_mode = 1;
	to_cu->result = 0;

	if (nmp_tw_info_handle(TW_CLEAR_RESULT_TO_CU, &to_cu_with_seq, NULL) < 0)
	{
		error_msg("send TW_CLEAR_RESULT_TO_CU(stop) failed\n");
	}

	return 0;
}


int nmp_tw_deal_decoder_operate_res(tw_operate_decoder_rsp_with_seq *dec_rsp_with_seq,
	TW_INFO_TYPE type)
{
	assert(dec_rsp_with_seq);

	tw_operate_result_to_cu_with_seq to_cu_with_seq;
	memset(&to_cu_with_seq, 0, sizeof(tw_operate_result_to_cu_with_seq));
	int error = TW_RES_EINVAL;

	switch (type)
	{
	case TW_CLEAR_TO_DEC:
	case TW_CHANGE_DIVISION_MODE_TO_DEC:
	case TW_FULL_SCREEN_TO_DEC:
	case TW_EXIT_FULL_SCREEN_TO_DEC:
		{
			if (nmp_tw_get_operate_info(&to_cu_with_seq, dec_rsp_with_seq->seq) < 0)
			{
				error_msg("nmp_tw_get_operate_info failed\n");
				goto end;
			}
			to_cu_with_seq.to_cu.result = dec_rsp_with_seq->operate_dec_rsp.result;
			break;
		}
	default:
		{
			error_msg("type error\n");
			goto end;
		}
	}

	switch (type)
	{
	case TW_CLEAR_TO_DEC:
		{
			to_cu_with_seq.to_cu.operate_type = TW_OPERATE_CLEAR;
			if (nmp_tw_info_handle(TW_CLEAR_RESULT_TO_CU,
				&to_cu_with_seq, NULL) < 0)
			{
				error_msg("send TW_CLEAR_RESULT_TO_CU(clear) failed\n");
				goto end;
			}
			break;
		}
	case TW_CHANGE_DIVISION_MODE_TO_DEC:
		{
			to_cu_with_seq.to_cu.operate_type = TW_OPERATE_CHANGE_DIVISION_MODE;
			if (nmp_tw_info_handle(TW_CHANGE_DIVISION_MODE_RESULT_TO_CU,
				&to_cu_with_seq, NULL) < 0)
			{
				error_msg("send TW_CHANGE_DIVISION_MODE_RESULT_TO_CU failed\n");
				goto end;
			}
			break;
		}
	case TW_FULL_SCREEN_TO_DEC:
		{
			to_cu_with_seq.to_cu.operate_type = TW_OPERATE_FULL_SCREEN;
			if (nmp_tw_info_handle(TW_FULL_SCREEN_RESULT_TO_CU,
				&to_cu_with_seq, NULL) < 0)
			{
				error_msg("send TW_FULL_SCREEN_RESULT_TO_CU failed\n");
				goto end;
			}
			break;
		}
	case TW_EXIT_FULL_SCREEN_TO_DEC:
		{
			to_cu_with_seq.to_cu.operate_type = TW_OPERATE_EXIT_FULL_SCREEN;
			if (nmp_tw_info_handle(TW_EXIT_FULL_SCREEN_RESULT_TO_CU,
				&to_cu_with_seq, NULL) < 0)
			{
				error_msg("send TW_CLEAR_RESULT_TO_CU failed\n");
				goto end;
			}
			break;
		}
	default:
		goto end;
	}

	error = TW_RES_OK;

end:
	return error;
}


int nmp_tw_update_url(tw_update_url *update_url)
{
	int group_i = 0, screen_i;
	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	tw_encoder_position *pos = &update_url->ec_position;
	log_msg("nmp_tw_update_url update\n");

	if (pos->division_num < 0 ||
		pos->division_num >= TW_MAX_DIVISIONS)
	{
		error_msg("division_num(%d) error!\n", pos->division_num);
		return -1;
	}
	group_vec_t *p_gv = &p_tw->tw_group_vec_ok;

	pthread_mutex_lock(&p_gv->gv_mutex);
	for (; group_i < p_gv->gv_capacity; ++group_i)
	{
		group_t *p_gp = p_gv->gv_parr[group_i];
		if (p_gp->gp_id != pos->gp_id)
			continue;

		for (screen_i = 0; screen_i < p_gp->gp_screens; ++screen_i)
		{
			dec_screen_t *p_dis_screen = &p_gp->gp_dis_screens[screen_i];
			if (p_dis_screen->screen_id != pos->screen_id)
				continue;

			assert(pos->step_num > 0 && pos->step_num <= p_gp->gp_max_steps);
			if (pos->step_num <= 0 || pos->step_num > p_gp->gp_max_steps)
			{
				error_msg("pos->step_num = %d\n", pos->step_num);
				pthread_mutex_unlock(&p_gv->gv_mutex);
				return -2;
			}

			division_mode_t *div_mode =
				&p_dis_screen->dis_pstep[pos->step_num - 1].division_mode;

			encoder_t *p_ec = div_mode->divisions[pos->division_num];

			if (p_ec && !strcmp(update_url->dis_guid, p_dis_screen->dis_guid) &&
				!strcmp(update_url->dis_domain_id, p_dis_screen->dis_domain_id) &&
				!strcmp(update_url->ec_guid, p_ec->ec_guid))
			{
				p_ec->ec_url[TW_MAX_URL_LEN - 1] = '\0';
				strncpy(p_ec->ec_url, update_url->ec_url, TW_MAX_URL_LEN - 1);
				log_msg("nmp_tw_update_url success\n");
				pthread_mutex_unlock(&p_gv->gv_mutex);
				return 0;
			}
		}
	}
	pthread_mutex_unlock(&p_gv->gv_mutex);

	return -1;
}


int nmp_tw_tvwall_work()
{
	//func_begin("**********************************************************\n");
	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	nmp_tw_groups_work(&p_tw->tw_group_vec_ok);
	return 0;
}

int nmp_tw_tvwall_clear()
{
	func_begin("********** just test **********\n");
	tv_wall_t *p_tw = nmp_tw_get_tv_wall_p();
	nmp_tw_groups_clear(&p_tw->tw_group_vec_ok);
	return 0;
}

