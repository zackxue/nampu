/*
 *	author:	zyt
 *	time:	begin in 2012/9/17
 */
#ifndef __NMP_TW_INTERFACE_H__
#define __NMP_TW_INTERFACE_H__


#define	TW_MAX_STEPS_DEF		(32)		//可扩展
#define	TW_MAX_STEPS_TOP		(256)	//步数最高界限
#define	TW_MAX_DIVISIONS		(64)		//不可扩展

#define	TW_ID_LEN				(32)
#define	TW_SESSION_ID_LEN		(16)
#define	TW_MAX_VALUE_LEN		(64)
#define	TW_MAX_URL_LEN		(1024)

#define	TW_RES_OK						(0)
#define	TW_RES_ERROR					(-1)
#define	TW_RES_EINVAL					(-2)
#define	TW_RES_SENT_TO_DEC_FAILED	(-5)

#define	TW_ILLEGAL_SEQ_NUM			(0xffffffff)
#define	TW_ACTION_TIME_LEN			(15)


typedef enum
{
	GPT_NONE	= 0,
	GPT_STEP,
	GPT_TOUR,
	GPT_GROUP,
	GPT_ACTION_STEP
} TW_GPT_TYPE;

typedef enum
{
	TW_INFO_GET_TOUR		= 0,
	TW_INFO_GET_GROUP,
	TW_INFO_GET_GROUP_STEP_N,
	TW_INFO_GET_DIS_GUID,
	TW_INFO_GET_EC_URL,
	TW_INFO_QUERY_IF_UPDATE_URL,

	TW_INFO_SEND_SCREEN_TO_DEC,
	TW_INFO_SEND_SCREEN_TO_CU,

	TW_INFO_SEND_CMD_RES_TO_CU,

	TW_CLEAR_TO_DEC,
	TW_CLEAR_RESULT_TO_CU,

	TW_CHANGE_DIVISION_MODE_TO_DEC,
	TW_CHANGE_DIVISION_MODE_RESULT_TO_CU,

	TW_FULL_SCREEN_TO_DEC,
	TW_FULL_SCREEN_RESULT_TO_CU,

	TW_EXIT_FULL_SCREEN_TO_DEC,
	TW_EXIT_FULL_SCREEN_RESULT_TO_CU,
} TW_INFO_TYPE;

typedef enum
{
	TW_RUN_STEP	= 0,
	TW_RUN_TOUR,
	TW_RUN_GROUP,
	TW_STOP_TOUR,
	TW_STOP_GROUP,
	TW_IF_RUN_TOUR,
	TW_IF_RUN_GROUP,
	TW_STOP_GPT_BY_DIVISION
} TW_CMD_TYPE;

typedef enum
{
	TW_OPERATE_FULL_SCREEN	= 0,
	TW_OPERATE_EXIT_FULL_SCREEN,
	TW_OPERATE_CHANGE_DIVISION_MODE,
	TW_OPERATE_CLEAR
} TW_OPERATE_TYPE;


typedef struct
{
	char		guid[TW_ID_LEN];
	char		domain_id[TW_ID_LEN];
} tw_general_guid;


/*============================================================
 *				电视墙操作通信协议 对应 数据结构
 *============================================================*/

//单步执行数据结构			<jpf_tw_run_step>

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_id;					//分屏模式
	int		division_num;				//分割号
	char		ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	char		ec_domain_id[TW_ID_LEN];		//编码器所在的域id
	char		ec_guid[TW_ID_LEN];				//编码器guid
} tw_run_step_request;

typedef struct
{
	tw_run_step_request		req;
	unsigned int		seq;
} tw_run_step_request_with_seq;


//巡回执行数据结构			<jpf_tw_run_tour>

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_id;					//分屏模式
	int		division_num;				//分割号
	int		tour_id;						//巡回id
} tw_run_tour_request;


//群组执行数据结构			<jpf_tw_run_group>

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		group_id;					//群组id
} tw_run_group_request;


//巡回停止命令			<jpf_tw_stop_tour>

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		tour_id;						//巡回id
} tw_stop_tour_request;


//群组停止命令			<jpf_tw_stop_group>

typedef tw_run_group_request tw_stop_group_request;

//查询巡回是否运行			<jpf_tw_if_run_tour>

typedef tw_stop_tour_request tw_if_run_tour_request;

//查询群组是否运行			<jpf_tw_if_run_group>

typedef tw_run_group_request tw_if_run_group_request;


//停止分割运行数据结构		<jpf_tw_stop_gpt_by_division>

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_num;				//分割号
} tw_division_position;



/*============================================================
 *			电视墙获取单步巡回群组详细信息 数据结构
 *============================================================*/

//-> 向数据库获取巡回信息		->TW_INFO_GET_TOUR

typedef struct
{
	int		tour_id;						//巡回id
} tw_tour_msg_request;


//<- 数据库返回巡回信息		<-TW_INFO_GET_TOUR

typedef struct
{
	int		step_num;					//步序号
	int		step_ttl;						//步长
	char 	ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	char 	ec_domain_id[TW_ID_LEN];		//编码器所在的域id
	char 	ec_guid[TW_ID_LEN];				//编码器guid
	int		level;							//码流等级
} tw_tour_step_response;

typedef struct
{
	int		result;
	int		tour_num;					//巡回编号，用于键盘
	int		auto_jump;					//巡回无效时是否自动跳转
	char		tour_name[TW_MAX_VALUE_LEN];	//巡回名称
	int		step_count;					//巡回总步数
	tw_tour_step_response	*steps;			//每一步信息
} tw_tour_msg_response;


//-> 向数据库获取群组概要信息	->TW_INFO_GET_GROUP

typedef struct
{
	int		group_id;					//群组id
} tw_group_msg_request;


//<- 数据库返回群组概要信息		<-TW_INFO_GET_GROUP

typedef struct
{
	int		result;
	int		tw_id;						//电视墙id
	int		group_num;					//群组编号，用于键盘
	char		group_name[TW_MAX_VALUE_LEN];	//群组名称
	int		step_count;					//群组总步数
	unsigned int	step_num[TW_MAX_STEPS_TOP];	//步号
} tw_group_msg_response;


//-> 向数据库获取群组第n步信息	->TW_INFO_GET_GROUP_STEP_N

typedef struct
{
	int		group_id;					//群组id
	int		step_num;					//步序号，从1开始
} tw_group_step_n_request;


//<- 数据库返回群组第n步信息	<-TW_INFO_GET_GROUP_STEP_N

typedef struct
{
	int		division_num;				//分割号，从0开始
	char		ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	char		ec_domain_id[TW_ID_LEN];		//编码器所在的域id
	char		ec_guid[TW_ID_LEN];				//编码器guid
	int		level;
} tw_group_division_response;

typedef struct
{
	int		screen_id;					//显示屏id
	char		dis_guid[TW_ID_LEN];			//显示屏guid
	char		dis_domain_id[TW_ID_LEN];	//显示屏域id
	int		division_id;					//分屏模式
	int		div_sum;					//分割总数
	tw_group_division_response	divisions[TW_MAX_DIVISIONS];		//分割信息
} tw_group_screen_response;

typedef struct
{
	int		result;
	int		step_ttl;						//步长
	int		screen_sum;					//使用的总屏数
	tw_group_screen_response	*screens;	//每一屏信息
} tw_group_step_n_response;



//-> 根据tw_id,screen_id 获取 显示屏guid		->TW_INFO_GET_DIS_GUID

typedef struct
{
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
} tw_dis_guid_request;


//<- 根据tw_id,screen_id 返回 显示屏guid		<-TW_INFO_GET_DIS_GUID

typedef struct
{
	int		result;
	tw_general_guid	dis_guid;				//显示屏guid
} tw_dis_guid_response;


//-> 根据编码器域id,guid 获取 编码器urlf		->TW_INFO_GET_EC_URL

typedef struct
{
	char		dis_guid[TW_ID_LEN];				//解码器guid
	char		dis_domain_id[TW_ID_LEN];		//解码器所在的域id
	char		ec_guid[TW_ID_LEN];				//编码器guid
	char		ec_domain_id[TW_ID_LEN];		//编码器所在的域id
} tw_ec_url_request;


//<- 根据编码器域id,guid 返回 编码器url		<-TW_INFO_GET_EC_URL

typedef struct
{
	int		result;
	char		ec_url[TW_MAX_URL_LEN];			//编码器url
	char		ec_dec_plug[TW_ID_LEN];
} tw_ec_url_response;



/*============================================================
 *			电视墙屏幕切换通信协议 对应 数据结构
 *============================================================*/

//电视墙一屏幕切换信息				->TW_INFO_SEND_SCREEN_TO_DEC

typedef struct
{
	int		division_num;				//分割号
	char		ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	char		ec_dec_plug[TW_ID_LEN];
	char		*ec_url;						//编码器对应的转发单元地址(指针)
} tw_division_to_decoder;

typedef struct
{
	TW_GPT_TYPE	gp_type;				//group_t结构类型，1:STEP,2:TOUR,3:GROUP
	char		gp_name[TW_MAX_VALUE_LEN];	//group_t结构的名称
	int		step_num;					//步序号
	char		dis_guid[TW_ID_LEN];				//显示屏guid
	int		division_id;					//分屏模式
	int		keep_other;					//是否保持其它分割的运行状态
	int		div_sum;					//分割总数
	tw_division_to_decoder		divisions[TW_MAX_DIVISIONS];
} tw_screen_to_decoder;

typedef struct
{
	tw_screen_to_decoder	screen_to_dec;
	unsigned int	seq;						//序列号
} tw_screen_to_decoder_with_seq;



/*============================================================
 *				屏幕切换结果反馈 对应 数据结构
 *============================================================*/

//	<jpf_tw_deal_decoder_res>

typedef struct
{
	int		result;
	int		division_num;				//分割号
	char		ec_name[TW_MAX_VALUE_LEN];	//编码器名称
	int		ec_channel;
	int		level;
} tw_decoder_division_rsp;

typedef struct
{
	int		result;
	int		division_id;
	int		div_sum;
	tw_decoder_division_rsp		divisions[TW_MAX_DIVISIONS];
} tw_decoder_rsp;

typedef struct
{
	tw_decoder_rsp	dec_rsp;
	unsigned int	seq;						//序列号
} tw_decoder_rsp_with_seq;



/*============================================================
 *			运行状态反馈通信协议 对应 数据结构
 *============================================================*/

//发送一屏幕切换结果到cu			->TW_INFO_SEND_SCREEN_TO_CU

typedef tw_decoder_division_rsp tw_division_to_cu;

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	TW_GPT_TYPE	gp_type;				//group_t结构类型，1:STEP,2:TOUR,3:GROUP
	char		gp_name[TW_MAX_VALUE_LEN];	//group_t结构的名称
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_id;					//分屏模式
	int		keep_other;					//是否保持其它分割的运行状态
	int		div_sum;					//分割总数
	tw_division_to_cu		divisions[TW_MAX_DIVISIONS];
} tw_screen_to_cu;


//发送电视墙操作结果到cu		->TW_INFO_SEND_CMD_RES_TO_CU

typedef struct
{
	char				session_id[TW_SESSION_ID_LEN];			//cu会话id
	unsigned int		cu_seq;
	TW_CMD_TYPE	tw_cmd_type;				//电视墙操作类型
	int				result;						//0:正确，非0:错误
} tw_run_res;



/*============================================================
 *					屏操作 对应 数据结构
 *============================================================*/

typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_id;					//分屏模式
	int		division_num;				//分割号
	int		operate_mode;				//清除模式时，0:清除分割 1:停止画面
} tw_operate;

typedef struct
{
	tw_operate	operate;
	unsigned int	seq;
} tw_operate_with_seq;


typedef struct
{
	char		session_id[TW_SESSION_ID_LEN];			//cu会话id
	TW_OPERATE_TYPE	operate_type;				//操作类型
	int		tw_id;						//电视墙id
	int		screen_id;					//显示屏id
	int		division_id;					//分屏模式
	int		division_num;				//分割号
	int		operate_mode;				//清除模式时，0:清除分割 1:停止画面
	int		result;
} tw_operate_result_to_cu;

typedef struct
{
	tw_operate_result_to_cu	to_cu;
	unsigned int	seq;
} tw_operate_result_to_cu_with_seq;


typedef struct
{
	char		dis_guid[TW_ID_LEN];			//显示屏guid
	int		division_id;					//分屏模式
	int		division_num;				//分割号
} tw_operate_to_decoder;

typedef struct
{
	tw_operate_to_decoder		operate_to_dec;
	unsigned int	seq;						//序列号
} tw_operate_to_decoder_with_seq;


typedef struct
{
	int		result;
} tw_operate_decoder_rsp;

typedef struct
{
	tw_operate_decoder_rsp operate_dec_rsp;
	unsigned int	seq;						//序列号
} tw_operate_decoder_rsp_with_seq;



/*============================================================
 *				查看更新url 对应 数据结构
 *============================================================*/

typedef struct
{
	int		gp_id;
	int		screen_id;
	int		step_num;		//从1开始
	int		division_num;	//从0开始
} tw_encoder_position;

typedef struct
{
	tw_encoder_position	ec_position;
	char		dis_guid[TW_ID_LEN];				//解码器guid
	char		dis_domain_id[TW_ID_LEN];		//解码器所在的域id
	char		ec_guid[TW_ID_LEN];				//编码器主码流guid
	char		ec_domain_id[TW_ID_LEN];		//编码器所在的域id
	char		ec_url[TW_MAX_URL_LEN];			//编码器对应的转发单元地址
} tw_update_url;




void jpf_tw_init_log_path(const char *folder_path, const char *name);

void jpf_tw_init_tvwall_module();

int jpf_tw_tvwall_work();

int jpf_tw_tvwall_clear();		//测试时使用


int jpf_tw_run_step(tw_run_step_request_with_seq *msg_with_seq);

int jpf_tw_run_tour(tw_run_tour_request *msg);

int jpf_tw_run_group(tw_run_group_request *msg);

int jpf_tw_run_action_step(tw_run_step_request *msg);

int jpf_tw_stop_tour(tw_stop_tour_request *msg);

int jpf_tw_stop_group(tw_stop_group_request *msg);

int jpf_tw_if_run_tour(tw_if_run_tour_request *msg);

int jpf_tw_if_run_group(tw_if_run_group_request *msg);

int jpf_tw_stop_gpt_by_division(tw_division_position *msg);

int jpf_tw_screen_operate(tw_operate_with_seq *msg_with_seq, TW_INFO_TYPE type);


typedef int (*event_handler_t)(TW_INFO_TYPE cmd, void *in_parm, void *out_parm);

void jpf_tw_set_event_handler(event_handler_t hook);


int jpf_tw_deal_decoder_res(tw_decoder_rsp_with_seq *dec_rsp_with_seq);

int jpf_tw_deal_decoder_operate_res(tw_operate_decoder_rsp_with_seq *dec_rsp_with_seq,
	TW_INFO_TYPE type);


int jpf_tw_destroy_screen_to_dec(tw_screen_to_decoder_with_seq *to_dec_with_seq,
	int size);


int jpf_tw_update_url(tw_update_url *update_url);


#endif

