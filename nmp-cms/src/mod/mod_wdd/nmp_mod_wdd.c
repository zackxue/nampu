#include <pthread.h>
#include <time.h>
#include "nmp_wdd_interface.h"
#include "nmp_mod_wdd.h"
#include "nmp_msg_cu.h"
#include "nmp_internal_msg.h"
#include "nmp_sysctl.h"


#define CMS_BIRTHDAY					"2013-4-23"

#define WDD_MAX_BUF_LEN				(1024)
#define WDD_ONE_MINUTE_SECONDS		(60)
#define WDD_ONE_DAY_SECONDS			(3600 * 24)

#define WDD_MAX_EOPEN_ERROR_LEN		(WDD_ONE_DAY_SECONDS * 5)	//5 days

#define WDD_WORK_TIME_LEN				(10)		//10 s
#define WDD_WORK_NUMS_ONE_MINUTE	(WDD_ONE_MINUTE_SECONDS / WDD_WORK_TIME_LEN)
#define WDD_TIME_WARNING_DAYS		(10)		//10 days
#define WDD_TIME_WARNING_LEN			(WDD_ONE_DAY_SECONDS * WDD_TIME_WARNING_DAYS)
#define WDD_TIME_OUT_WARNING_MINS	(0)		//0 mins
#define WDD_TIME_OUT_WARNING_LEN		(WDD_ONE_MINUTE_SECONDS * WDD_TIME_OUT_WARNING_MINS)

#define WDD_TIME_DECREASE_MINS		(10)		//累积10mins写一次dog
#define WDD_TIME_DECREASE_LEN			(WDD_ONE_MINUTE_SECONDS * WDD_TIME_DECREASE_MINS)

#define WDD_INEXISTENT_MAX_MINS		(3)
#define WDD_MAX_UINT					(0xffffffff)
#define CUR_TIME	(time(NULL))

//#define WDD_TEST

typedef enum
{
	WDD_TYPE_0 = 0,
	WDD_TYPE_1,
	WDD_TYPE_2
} WDD_AUTHORIZATION_EXPIRED_TYPE;

typedef enum
{
	WDD_SYSTEM_TIME_ERROR = -2,
	WDD_TIME_ERROR = -1,
	WDD_TIME_OK = 0,
	WDD_TIME_WARNING,
	WDD_TIME_OUT_WARNING,
	WDD_TIME_OUT,
} WDD_TIME_DEAL_RESULT;

pthread_t g_wdd_tid;
static encrypt_handle g_handler;
static ENCRYPT_PASSWD g_wdd_passwd;
static time_t g_last_write_time = 0;

typedef struct
{
	gint		type;	//WDD_TIME_TYPE
	gchar	info[GENERAL_MSG_PARM_LEN];
	pthread_mutex_t mutex;
} wdd_time_info;

static wdd_time_info g_wdd_time_info =
{
	0, "", PTHREAD_MUTEX_INITIALIZER
};

static gint g_wdd_version = VER_TEST;

static JpfModWdd *g_jpf_mod_wdd;

G_DEFINE_TYPE(JpfModWdd, jpf_mod_wdd, JPF_TYPE_APPMOD);

void jpf_mod_wdd_register_msg_handler(JpfModWdd *self);

static JpfMsgWddDevCapInfo g_default_wdd_dev_cap_info =
{
	.version = 1,
	.module_bits = WDD_MAX_UINT,
	.modules_data = {WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT,
	WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT},
	.expired_time = {WDD_FOREVER_TIME, ""},
	.max_dev = 2048,
	.max_av = 2048,
	.max_ds = 2048,
	.max_ai = 2048,
	.max_ao = 2048
};

static JpfMsgWddDevCapInfo g_test_wdd_dev_cap_info =
{
	.version = VER_TEST,
	.module_bits = WDD_MAX_UINT,
	.modules_data = {WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT,
	WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT, WDD_MAX_UINT},
	.expired_time = {WDD_FOREVER_TIME, ""},
	.max_dev = 8,
	.max_av = 8,
	.max_ds = 0,
	.max_ai = 8,
	.max_ao = 8
};

static JpfMsgWddDevCapInfo g_tmp_wdd_dev_cap_info;

#define JPF_WDD_FORBIDDEN_NUM		(1)
static gint g_wdd_maker_forbidden[JPF_WDD_FORBIDDEN_NUM] = {17};


wdd g_wdd_test;

void jpf_wdd_init_data_test()
{
	int time_style = 1;

	g_wdd_test.version.ver_num = htonl(1);
	wdd_time *w_time = &g_wdd_test.data.authorize;

	g_wdd_test.data.max_dev = htonl(99999);
	g_wdd_test.data.max_av = htonl(99999);
	g_wdd_test.data.max_ds = htonl(99999);
	g_wdd_test.data.max_ai = htonl(99999);
	g_wdd_test.data.max_ao = htonl(99999);

	if (time_style == WDD_STYLE_DATE)
	{
		w_time->style = htonl(WDD_STYLE_DATE);
		w_time->expire_year = htons(2013);
		w_time->expire_mon = 1;
		w_time->expire_date = 25;
	}
	else if (time_style == WDD_STYLE_DAY)
	{
		w_time->style = htonl(WDD_STYLE_DAY);
		w_time->expire_ttd = htonl(100);
	}
	else
	{
		w_time->style = htonl(WDD_STYLE_MINUTE);
		w_time->expire_ttd = htonl(1);
	}
}

gint jpf_wdd_read_data_test(char *wdd_info)
{
	jpf_print("-------------------------- read data test ----------------------");
	g_assert(sizeof(wdd) < WDD_MAX_BUF_LEN);
	memset(wdd_info, 0, WDD_MAX_BUF_LEN);
	memcpy(wdd_info, &g_wdd_test, sizeof(wdd));
	return 0;
}

gint jpf_wdd_write_data_test(char *wdd_info)
{
	jpf_print("-------------------------- write data test ----------------------");
	memcpy(&g_wdd_test, wdd_info, sizeof(wdd));
	return 0;
}


JpfModWdd *jpf_get_mod_wdd()
{
	return g_jpf_mod_wdd;
}


static void
jpf_set_wdd_version(gint ver)
{
	g_wdd_version = ver;
}

static gint
jpf_get_wdd_version()
{
	return g_wdd_version;
}


gint jpf_if_system_time_error()
{
	struct tm t_tm;
	gint year, mon, day;
	time_t birth_time = 0;

	sscanf(CMS_BIRTHDAY, "%d-%d-%d", &year, &mon, &day);

	t_tm.tm_year = year - 1900;
	t_tm.tm_mon = mon - 1;
	t_tm.tm_mday = day;
	t_tm.tm_hour = 0;
	t_tm.tm_min = 0;
	t_tm.tm_sec = 0;

	birth_time = mktime(&t_tm);
	//jpf_warning("haaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, birth_time:%u\n", birth_time);
	//jpf_warning("haaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, cur_time:%u\n", CUR_TIME);
	if (CUR_TIME < birth_time)
		return 1;

	return 0;
}

gchar *jpf_get_cur_time_str()
{
	#define __WDD_TIME_LEN (32)
	static gchar cur_time_str[__WDD_TIME_LEN];
	time_t cur_time;
	struct tm t_tm;

	cur_time = CUR_TIME;
	localtime_r(&cur_time, &t_tm);
	snprintf(cur_time_str, __WDD_TIME_LEN, "%d-%d-%d %d:%d:%d",
		t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday,
		t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);

	return cur_time_str;
}

void jpf_wdd_flash()
{
#ifdef WDD_TEST
	return ;
#endif
	g_assert(g_handler);
	unsigned long num = 5;

	if (encrypt_control(g_handler, ET_LED_WINK, &num) != 0)
	{
		jpf_warning("<JpfModWdd>wdd encrypt_control error, maybe watchdog has " \
			"been removed.");
	}
}


void jpf_wdd_work_init()
{
	g_handler = NULL;

	encrypt_register_probe(et199_encrypt);

	memset(&g_wdd_passwd, 0, sizeof(ENCRYPT_PASSWD));
	strncpy(g_wdd_passwd.dev_passwd, DEV_PASSWD, WDD_PASSWD_LEN - 1);
	strncpy(g_wdd_passwd.user_passwd, USER_PASSWD, WDD_PASSWD_LEN - 1);
}


gint jpf_wdd_read_data(char *info)
{
	g_assert(info);
	wdd *wdd_info;
	gint head_size, size;
	guint ver_num;
	gint ret, err;

	if (!g_handler)
	{
		g_handler = encrypt_init(&g_wdd_passwd, &err);
		if (!g_handler)
		{
			jpf_warning("<JpfModWdd>wdd init failed, err = %d", err);
			return err;
		}
	}

	head_size = WDD_HEAD_LEN;
	ret = encrypt_read_data(g_handler, info, DATA_TYPE_1, head_size);
	if (ret != 0)
	{
		jpf_warning("<JpfModWdd>wdd read data error, maybe watchdog has " \
			"been removed.");
		return ret;
	}

	wdd_info = (wdd *)info;
	if (strcmp((char *)(wdd_info->label.label), WDD_LABEL))
	{
		jpf_warning("<JpfModWdd> label error, label:%s", wdd_info->label.label);
		return -2;
	}

	ver_num = ntohl(wdd_info->version.ver_num);
	if (ver_num == WDD_VERSION_1)
	{
		size = WDD_HEAD_LEN + sizeof(wdd_version1);
	}
	else
	{
		jpf_warning("<JpfModWdd> version error, version=%d", ver_num);
		return -1;
	}

	g_assert(size <= WDD_MAX_BUF_LEN);
	ret = encrypt_read_data(g_handler, info, DATA_TYPE_1, size);
	if (ret != 0)
	{
		jpf_warning("<JpfModWdd>wdd read data error, maybe watchdog has " \
			"been removed.");
		return ret;
	}

	return 0;
}


static __inline__ void
__jpf_wdd_backup_time_info_date(wdd_time_info *time_info, struct tm *out_tm)
{
	time_info->type = WDD_EXPIRED_DATE;
	snprintf(time_info->info, GENERAL_MSG_PARM_LEN, "%d-%d-%d 23:59:59",
		out_tm->tm_year + 1900, out_tm->tm_mon + 1, out_tm->tm_mday);
}

static void jpf_wdd_backup_time_info_date(struct tm *out_tm)
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_backup_time_info_date(&g_wdd_time_info, out_tm);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}


static __inline__ void
__jpf_wdd_backup_time_info_min(wdd_time_info *time_info, gint mins)
{
	if (mins < 0)
		mins = 0;
	time_info->type = WDD_REMAINED_TIME;
	snprintf(time_info->info, GENERAL_MSG_PARM_LEN, "%d", mins);
}

static void jpf_wdd_backup_time_info_min(gint mins)
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_backup_time_info_min(&g_wdd_time_info, mins);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}


static __inline__ void
__jpf_wdd_backup_time_info_forever(wdd_time_info *time_info)
{
	time_info->type = WDD_FOREVER_TIME;
}

static void jpf_wdd_backup_time_info_forever()
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_backup_time_info_forever(&g_wdd_time_info);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}


static __inline__ void
__jpf_wdd_backup_time_info_forbidden(wdd_time_info *time_info)
{
	time_info->type = WDD_DOG_FORBIDDEN;
}

static void jpf_wdd_backup_time_info_forbidden()
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_backup_time_info_forbidden(&g_wdd_time_info);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}

#if 0
static __inline__ void
__jpf_wdd_backup_time_info_inexistent(wdd_time_info *time_info)
{
	time_info->type = WDD_DOG_INEXISTENT;
}

static void jpf_wdd_backup_time_info_inexistent()
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_backup_time_info_inexistent(&g_wdd_time_info);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}
#endif

static __inline__ void
__jpf_wdd_get_wdd_time_info(wdd_time_info *time_info, wdd_time_info *get_info)
{
	get_info->type = time_info->type;
	get_info->info[GENERAL_MSG_PARM_LEN - 1] = '\0';
	strncpy(get_info->info, time_info->info, GENERAL_MSG_PARM_LEN - 1);
}

static void jpf_wdd_get_wdd_time_info(wdd_time_info *get_info)
{
	pthread_mutex_lock(&g_wdd_time_info.mutex);
	__jpf_wdd_get_wdd_time_info(&g_wdd_time_info, get_info);
	pthread_mutex_unlock(&g_wdd_time_info.mutex);
}


WDD_TIME_DEAL_RESULT jpf_wdd_deal_version1_date(wdd_time *w_time)
{
	struct tm out_tm;
	time_t cur_time, out_time;

	out_tm.tm_year = (gint)(ntohs(w_time->expire_year)) - 1900;
	out_tm.tm_mon = w_time->expire_mon - 1;
	out_tm.tm_mday = w_time->expire_date;
	out_tm.tm_hour = 23;		//23:59:59
	out_tm.tm_min = 59;
	out_tm.tm_sec = 59;

	jpf_wdd_backup_time_info_date(&out_tm);

	out_time = mktime(&out_tm);
	cur_time = CUR_TIME;
	//jpf_print("<JpfModWdd>_____ date style _, out_time-> %d-%d-%d.",
	//	(gint)(ntohs(w_time->expire_year)), (gint)(w_time->expire_mon),
	//	(gint)(w_time->expire_date));

	if (out_time > cur_time)
	{
		if (out_time - cur_time < WDD_TIME_WARNING_LEN)
			return WDD_TIME_WARNING;
		return WDD_TIME_OK;
	}
	else
	{
		if (cur_time - out_time < WDD_TIME_OUT_WARNING_LEN)
			return WDD_TIME_OUT_WARNING;
		return WDD_TIME_OUT;
	}
}


WDD_TIME_DEAL_RESULT jpf_wdd_deal_version1_day(char *wdd_info)
{
	struct tm t_tm, start_tm;
	time_t cur_time, out_time;
	wdd_version1 *data = (wdd_version1 *)(&wdd_info[WDD_HEAD_LEN]);
	wdd_time *w_time = &data->authorize;
	guint ttd_day = ntohl(w_time->expire_ttd);
	gint ret;

	if (w_time->start_year == 0)	//未激活
	{
		cur_time = CUR_TIME;
		localtime_r(&cur_time, &t_tm);
		w_time->start_year = htons(t_tm.tm_year + 1900);
		w_time->start_mon = t_tm.tm_mon + 1;
		w_time->start_date = t_tm.tm_mday;
		jpf_print("<JpfModWdd>_____ day style _, w_time->start_year:%d, " \
			"ttd_day = %d", ntohs(w_time->start_year), ttd_day);

		guint size = WDD_HEAD_LEN + sizeof(wdd_version1);
#ifdef WDD_TEST
		ret = jpf_wdd_write_data_test(wdd_info);
#else
		ret = encrypt_write_data(g_handler, wdd_info, size, DATA_TYPE_1);
#endif
		if (ret != 0)
		{
			jpf_warning("<JpfModWdd>wdd write data error, maybe watchdog has " \
				"been removed.");
			return WDD_TIME_ERROR;
		}

		out_time = cur_time + WDD_ONE_DAY_SECONDS * ttd_day;
		localtime_r(&out_time, &t_tm);
		jpf_wdd_backup_time_info_date(&t_tm);

		if (ttd_day < WDD_TIME_WARNING_DAYS)
			return WDD_TIME_WARNING;
		return WDD_TIME_OK;
	}

	start_tm.tm_year = (gint)(ntohs(w_time->start_year)) - 1900;
	start_tm.tm_mon = w_time->start_mon - 1;
	start_tm.tm_mday = w_time->start_date;
	start_tm.tm_hour = 23;
	start_tm.tm_min = 59;
	start_tm.tm_sec = 59;

	out_time = mktime(&start_tm) + WDD_ONE_DAY_SECONDS * ttd_day;
	localtime_r(&out_time, &t_tm);
	jpf_wdd_backup_time_info_date(&t_tm);
	cur_time = CUR_TIME;
	//jpf_print("<JpfModWdd>day style, cur_time=%ld, out_time=%ld.", cur_time, out_time);

	if (out_time > cur_time)
	{
		if (out_time - cur_time < WDD_TIME_WARNING_LEN)
			return WDD_TIME_WARNING;
		return WDD_TIME_OK;
	}
	else
	{
		if (cur_time - out_time < WDD_TIME_OUT_WARNING_LEN)
			return WDD_TIME_OUT_WARNING;
		return WDD_TIME_OUT;
	}
}


gint jpf_if_write_dog()
{
	time_t cur_time = CUR_TIME;

	if (cur_time < g_last_write_time ||
		cur_time - g_last_write_time > WDD_TIME_DECREASE_LEN * 2)		//系统更改时间
	{
		g_last_write_time = cur_time;
		return 1;
	}

	if (cur_time - g_last_write_time > WDD_TIME_DECREASE_LEN)
	{
		g_last_write_time += WDD_TIME_DECREASE_LEN;
		return 1;
	}

	return 0;
}


WDD_TIME_DEAL_RESULT jpf_wdd_deal_version1_minute(char *wdd_info)
{
	static gint g_out_times = 0, g_accumulate_times = 0;
	wdd_version1 *data = (wdd_version1 *)(&wdd_info[WDD_HEAD_LEN]);
	wdd_time *w_time = &data->authorize;
	guint ttd_minute = ntohl(w_time->expire_ttd);
	gint ret;

	if (ttd_minute == 0)	//overtime
	{
		if (++g_out_times < WDD_TIME_OUT_WARNING_LEN / WDD_WORK_TIME_LEN)
		{
			jpf_wdd_backup_time_info_min(0);
			return WDD_TIME_OUT_WARNING;
		}
		return WDD_TIME_OUT;
	}
	else
		g_out_times = 0;
	//jpf_print("<JpfModWdd>_____ minute style _, ttd_minute:%d", ttd_minute);

	if (jpf_if_write_dog())		//write dog
	{
		g_accumulate_times = 0;
		if (ttd_minute > WDD_TIME_DECREASE_MINS)
		{
			guint dec = ttd_minute % WDD_TIME_DECREASE_MINS;
			if (dec == 0)
				dec = WDD_TIME_DECREASE_MINS;
			ttd_minute -= dec;
		}
		else
		{
			ttd_minute = 0;
		}
		jpf_wdd_backup_time_info_min(ttd_minute);
		w_time->expire_ttd = htonl(ttd_minute);

		guint size = WDD_HEAD_LEN + sizeof(wdd_version1);
#ifdef WDD_TEST
		ret = jpf_wdd_write_data_test(wdd_info);
#else
		ret = encrypt_write_data(g_handler, wdd_info, size, DATA_TYPE_1);
#endif
		if (ret != 0)
		{
			jpf_warning("<JpfModWdd>wdd write data error, maybe watchdog has " \
				"been removed.");
			return WDD_TIME_ERROR;
		}
	}
	else
	{
		g_accumulate_times++;
		jpf_wdd_backup_time_info_min(ttd_minute -
			g_accumulate_times * WDD_WORK_TIME_LEN / WDD_ONE_MINUTE_SECONDS);
	}

	if (ttd_minute * WDD_ONE_MINUTE_SECONDS < WDD_TIME_WARNING_LEN)
	{
		if (ttd_minute == 0)
			return WDD_TIME_OUT_WARNING;
		return WDD_TIME_WARNING;
	}

	return WDD_TIME_OK;
}


static void jpf_wdd_deal_time_result(WDD_TIME_DEAL_RESULT ret)
{
	JpfNotifyMessage notify_info;
	JpfMsgWddAuthErrorInfo auth_info;
	static gint g_sys_time_err_times = 0;
	static gint g_timeout_times = 0;
	static gint g_inexistent_times = 0;
	static gint g_time_warning_times = 0;

	memset(&notify_info, 0, sizeof(JpfNotifyMessage));
	if (ret != WDD_SYSTEM_TIME_ERROR)
		g_sys_time_err_times = 0;
	if (ret != WDD_TIME_OUT)
		g_timeout_times = 0;
	if (ret != WDD_TIME_ERROR)
		g_inexistent_times = 0;
	if (ret != WDD_TIME_WARNING)
		g_time_warning_times = 0;

	switch (ret)
	{
	case WDD_TIME_OK:
	{
		break;
	}
	case WDD_TIME_WARNING:			// to cu
	{
		g_time_warning_times++;
		if (g_time_warning_times % WDD_WORK_NUMS_ONE_MINUTE != 1)
			break;
		jpf_warning("<JpfModWdd> to cu: wdd time warning");

		wdd_time_info tmp_info;
		memset(&tmp_info, 0, sizeof(wdd_time_info));
		jpf_wdd_get_wdd_time_info(&tmp_info);
		if (tmp_info.type != WDD_EXPIRED_DATE &&
			tmp_info.type != WDD_REMAINED_TIME)
		{
			jpf_warning("<JpfModWdd> error: deal result WDD_TIME_WARNING, " \
			"time_info type is %d", tmp_info.type);
			break;
		}

		notify_info.msg_id = MSG_AUTHORIZATION_EXPIRED;
		sprintf(notify_info.param1, "%d", WDD_TYPE_0);
		sprintf(notify_info.param2, "%d", tmp_info.type);
		strncpy(notify_info.param3, tmp_info.info, GENERAL_MSG_PARM_LEN - 1);
		jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
			sizeof(JpfNotifyMessage));
		break;
	}
	case WDD_TIME_OUT_WARNING:		// to cu, no use
	{
		break;
		jpf_warning("<JpfModWdd> to cu: wdd time out warning");
		notify_info.msg_id = MSG_AUTHORIZATION_EXPIRED;
		sprintf(notify_info.param1, "%d", WDD_TYPE_1);
		jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
			sizeof(JpfNotifyMessage));
		break;
	}
	case WDD_TIME_OUT:
	{
		g_timeout_times++;

		if (g_timeout_times == 1)		// 1 time later, overtime, to cu
		{
			jpf_warning("<JpfModWdd> to cu: wdd time out");
			notify_info.msg_id = MSG_AUTHORIZATION_EXPIRED;
			sprintf(notify_info.param1, "%d", WDD_TYPE_2);
			jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
			sizeof(JpfNotifyMessage));
		}

		if (g_timeout_times >= 1)		// 1 time later, to dbs
		{
			jpf_warning("<JpfModWdd> to dbs: wdd time out");
			auth_info.type = WDD_AUTH_EXPIRED;
			jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
				BUSSLOT_POS_DBS, MSG_WDD_AUTH_ERROR, &auth_info,
				sizeof(JpfMsgWddAuthErrorInfo));
		}
		break;
	}
	case WDD_TIME_ERROR:
	{
		g_inexistent_times++;

		jpf_warning("<JpfModWdd> to cu: wdd inexistent");
		notify_info.msg_id = MSG_WDD_INEXISTENT;
		if (g_inexistent_times >= WDD_INEXISTENT_MAX_MINS *
			WDD_WORK_NUMS_ONE_MINUTE)		// 3 times later, to cu
			sprintf(notify_info.param1, "%d", 1);
		jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
			sizeof(JpfNotifyMessage));

		if (g_inexistent_times >= WDD_INEXISTENT_MAX_MINS *
			WDD_WORK_NUMS_ONE_MINUTE)		// 3 times later, to dbs
		{
			jpf_warning("<JpfModWdd> to dbs: wdd inexistent");
			auth_info.type = WDD_AUTH_INEXISTENT;
			jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
				BUSSLOT_POS_DBS, MSG_WDD_AUTH_ERROR, &auth_info,
				sizeof(JpfMsgWddAuthErrorInfo));
		}
		break;
	}
	case WDD_SYSTEM_TIME_ERROR:
	{
		g_sys_time_err_times++;
		if (g_sys_time_err_times % WDD_WORK_NUMS_ONE_MINUTE != 1)
			break;

		jpf_warning("<JpfModWdd> to cu: system time error");
		notify_info.msg_id = MSG_SYSTEM_TIME_ERROR;
		snprintf(notify_info.param1, GENERAL_MSG_PARM_LEN, "%s",
			jpf_get_cur_time_str());
		jpf_warning("<JpfModWdd> cms system time error, %s", jpf_get_cur_time_str());
		jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
			sizeof(JpfNotifyMessage));

		if (g_sys_time_err_times >= 2800 * WDD_WORK_NUMS_ONE_MINUTE + 1)		// 2 days later, to dbs
		{
			jpf_warning("<JpfModWdd> to dbs: system time error");
			auth_info.type = WDD_SYS_TIME_ERROR;
			jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
				BUSSLOT_POS_DBS, MSG_WDD_AUTH_ERROR, &auth_info,
				sizeof(JpfMsgWddAuthErrorInfo));
		}
		break;
	}
	default:
	{
		jpf_warning("<JpfModWdd> error:time deal result is:%d", ret);
	}
    }
}


void jpf_wdd_deal_version1(char *wdd_info)
{
	guint time_style;
	WDD_TIME_DEAL_RESULT ret;
	JpfMsgWddDevCapInfo info;
	wdd_version1 *data = (wdd_version1 *)(&wdd_info[WDD_HEAD_LEN]);
	gint i;

	if (jpf_if_system_time_error())
	{
		jpf_wdd_deal_time_result(WDD_SYSTEM_TIME_ERROR);
	}

	time_style = ntohl(data->authorize.style);

	if (time_style == WDD_STYLE_DATE)
	{
		ret = jpf_wdd_deal_version1_date(&data->authorize);
	}
	else if (time_style == WDD_STYLE_DAY)
	{
		ret = jpf_wdd_deal_version1_day(wdd_info);
	}
	else if (time_style == WDD_STYLE_MINUTE)
	{
		ret = jpf_wdd_deal_version1_minute(wdd_info);
	}
	else if (time_style == WDD_STYLE_FOREVER)
	{
		ret = WDD_TIME_OK;
		jpf_wdd_backup_time_info_forever();
	}
	else
	{
		jpf_warning("<JpfModWdd>error, time_style=%d.", time_style);
	}

	jpf_wdd_deal_time_result(ret);

	if (ret == WDD_TIME_OK || ret == WDD_TIME_WARNING ||
		ret == WDD_TIME_OUT_WARNING)
	{
		jpf_wdd_flash();

		memset(&info, 0, sizeof(JpfMsgWddDevCapInfo));

		info.version = ntohl(data->sys_ver);
		info.module_bits = ntohl(data->sys_modules);
		jpf_set_wdd_version(info.version);
		if (info.version == VER_NDMS &&
			!strstr(jpf_get_sys_parm_str(SYS_PARM_SRETYPE), "special"))
		{
			info.module_bits = 0;
			jpf_warning("<JpfModWdd> wdd version is special, but cms sre is %s",
				jpf_get_sys_parm_str(SYS_PARM_SRETYPE));
		}
		for (i = 0; i < MODULES_MAX; i++)
			info.modules_data[i] = ntohl(data->modules_data[i]);

		wdd_time_info tmp_info;
		memset(&tmp_info, 0, sizeof(wdd_time_info));
		jpf_wdd_get_wdd_time_info(&tmp_info);

		info.expired_time.type = tmp_info.type;
		strncpy(info.expired_time.expired_time, tmp_info.info, TIME_LEN - 1);
		info.max_dev = ntohl(data->max_dev);
		info.max_av = ntohl(data->max_av);
		info.max_ds = ntohl(data->max_ds);
		info.max_ai = ntohl(data->max_ai);
		info.max_ao = ntohl(data->max_ao);

		jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
			BUSSLOT_POS_DBS, MSG_WDD_DEV_CAP_INFO, &info,
			sizeof(JpfMsgWddDevCapInfo));
	}
}


static void jpf_wdd_deal_eopen_err()
{
	JpfNotifyMessage notify_info;
	memset(&notify_info, 0, sizeof(JpfNotifyMessage));
	jpf_warning("<JpfModWdd> to cu: wdd eopen error");

	jpf_wdd_backup_time_info_forever();

	notify_info.msg_id = MSG_WDD_EOPEN_ERROER;
	jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
		BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
		sizeof(JpfNotifyMessage));

	jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
		BUSSLOT_POS_DBS, MSG_WDD_DEV_CAP_INFO, &g_default_wdd_dev_cap_info,
		sizeof(JpfMsgWddDevCapInfo));
}


static void jpf_wdd_deal_version_test()
{
	JpfNotifyMessage notify_info;
	memset(&notify_info, 0, sizeof(JpfNotifyMessage));
	jpf_warning("<JpfModWdd> deal version test");

	jpf_wdd_backup_time_info_forever();

	jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
		BUSSLOT_POS_DBS, MSG_WDD_DEV_CAP_INFO, &g_test_wdd_dev_cap_info,
		sizeof(JpfMsgWddDevCapInfo));
}



static void jpf_wdd_deal_forbidden()
{
	JpfNotifyMessage notify_info;
	memset(&notify_info, 0, sizeof(JpfNotifyMessage));
	jpf_warning("<JpfModWdd> to cu: wdd forbidden");

	jpf_wdd_backup_time_info_forbidden();

	notify_info.msg_id = MSG_WDD_FORBIDDEN;
	jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
		BUSSLOT_POS_CU, MESSAGE_BROADCAST_GENERAL_MSG, &notify_info,
		sizeof(JpfNotifyMessage));

	memset(&g_tmp_wdd_dev_cap_info, 0, sizeof(g_tmp_wdd_dev_cap_info));
	g_tmp_wdd_dev_cap_info.version = 1;
	g_tmp_wdd_dev_cap_info.expired_time.type = WDD_DOG_FORBIDDEN;

	jpf_cms_mod_deliver_msg_2((JpfAppObj *)jpf_get_mod_wdd(),
		BUSSLOT_POS_DBS, MSG_WDD_DEV_CAP_INFO, &g_tmp_wdd_dev_cap_info,
		sizeof(JpfMsgWddDevCapInfo));
}


static gint
jpf_wdd_check_maker_code(gint maker_code)
{
	gint i;

	for (i = 0; i < JPF_WDD_FORBIDDEN_NUM; i++)
	{
		if (maker_code == g_wdd_maker_forbidden[i])
			return -1;
	}
	return 0;
}


void jpf_wdd_work()
{
	char wdd_info[WDD_MAX_BUF_LEN] = {0};
	static gint g_eopen_err_times = 0;
	guint maker_code;
	guint ver_num;
	gint ret;

#ifdef WDD_TEST
	if (jpf_wdd_read_data_test(wdd_info) != 0)
#else
	if ((ret = jpf_wdd_read_data(wdd_info)) != 0)
#endif
	{
		if (ret == OPENDEV_ERR)
		{
			jpf_wdd_deal_eopen_err();
			g_eopen_err_times++;
			if (g_eopen_err_times >= WDD_MAX_EOPEN_ERROR_LEN / WDD_WORK_TIME_LEN)
			{
				jpf_warning("<JpfModWdd> wdd eopen error times enough, system reboot!!!");
				system("reboot &");
			}
		}
		else
		{
			jpf_warning("<JpfModWdd> jpf_wdd_read_data error, ret = %d", ret);
			jpf_set_wdd_version(VER_TEST);
			jpf_wdd_deal_version_test();
			//jpf_wdd_deal_time_result(WDD_TIME_ERROR);
			//jpf_wdd_backup_time_info_inexistent();
		}
		goto error;
	}
	g_eopen_err_times = 0;

	maker_code = ntohl(((wdd *)wdd_info)->serial_code.maker_code);
	if (jpf_wdd_check_maker_code(maker_code) != 0)
	{
		jpf_warning("<JpfModWdd> maker_code(%d) forbidden!!!", maker_code);
		jpf_wdd_deal_forbidden();
		goto error;
	}

	ver_num = ntohl(((wdd *)wdd_info)->version.ver_num);
	if (ver_num == WDD_VERSION_1)
	{
		jpf_wdd_deal_version1(wdd_info);
	}
	return ;

error:
	{
#ifndef WDD_TEST
	if (g_handler)
	{
		encrypt_uninit(g_handler);
		g_handler = NULL;
		g_last_write_time = 0;
	}
#endif
	}
}


void *jpf_mod_wdd_work(void *arg)
{
	jpf_wdd_work_init();
#ifdef WDD_TEST
	jpf_wdd_init_data_test();
#endif
	while (1)
	{
		jpf_wdd_work();
		sleep(WDD_WORK_TIME_LEN);
	}
	return ((void *)1);
}


gint
jpf_wdd_get_expired_time(JpfExpiredTime *expired_time)
{
	wdd_time_info tmp_info;
	memset(&tmp_info, 0, sizeof(wdd_time_info));
	jpf_wdd_get_wdd_time_info(&tmp_info);

	expired_time->type = tmp_info.type;
	expired_time->expired_time[TIME_LEN - 1] = '\0';
	strncpy(expired_time->expired_time, tmp_info.info, TIME_LEN - 1);

	return 0;
}


gint
jpf_wdd_get_version()
{
	return jpf_get_wdd_version();
}


gint
jpf_mod_wdd_setup(JpfAppMod *am_self)
{
	JpfModWdd *self;
	G_ASSERT(am_self != NULL);

	self = (JpfModWdd*)am_self;

	jpf_app_mod_set_name(am_self, "MOD-WDD");
	jpf_mod_wdd_register_msg_handler(self);
	return 0;
}


static void
jpf_mod_wdd_init(JpfModWdd *self)
{
	gint ret;

	g_jpf_mod_wdd = self;

	ret = pthread_create(&g_wdd_tid, NULL, jpf_mod_wdd_work, NULL);
	if (ret != 0)
		jpf_warning("<JpfModWdd>pthread_create failed\n");
}


static void
jpf_mod_wdd_class_init(JpfModWddClass *k_class)
{
	JpfAppModClass *am_class = (JpfAppModClass*)k_class;

	am_class->setup_mod	= jpf_mod_wdd_setup;
}

