


//#include "jpf_msg_sdk.h"

//#include "jpf_client_connect.h"
//#ifndef	__SHARE__
//#define __SHARE__
//#include "nmp_msg_share.h"
//#endif



#ifndef __INTER__
#define __INTER__


#include "nmp_msg_share.h"

typedef struct _server_callback_function
{
	void *proc_register_paramer;
	int (*proc_register)(void *, void *, void *);
}server_callback_function;


/******************************************数据结构体****************************************/

//客户端数据结构体
typedef struct _jpf_QueryUser_info
{

	int client_count;	//查询客户端个数
	int start_line; 	//开始的行序号
	gchar key;			//模糊查询的内容，如果是按类型查询则是0
	gchar type;			//查询类型，0：查询所有用户，1：按组名，2：按等级

}jpf_QueryUser_info;

typedef struct _jpf_QueryUser_info_res
{
	JpfMsgErrCode		code;

	char   username[USER_NAME_LEN]; 	//账号
	char   password[USER_PASSWD_LEN];	 //密码
	int    group_id;		//所属组
	int    sex; 	//性别
	char   user_phone[PHONE_NUM_LEN];  //联系方式
	char   user_description[DESCRIPTION_INFO_LEN];	//备注信息

}jpf_QueryUser_info_res;

typedef struct _jpf_user_QueryUser_info
{
	char domain[USER_NAME_LEN];
	jpf_QueryUser_info *QueryUser_info;

}jpf_user_QueryUser_info;

typedef struct _jpf_user_QueryUser_info_res_inter
{
	int flag;
	int count;
	jpf_QueryUser_info_res *data_buf;
}jpf_user_QueryUser_info_res_inter;

//用户组信息
typedef struct _jpf_user_group_info
{
	int count;			//查询用户组个数
	int start_line; 	//开始的行序号
	int key;			//模糊查询的内容，如果是按类型查询则是0
	int type;			//查询类型，0：查询所有用户，1：按组名，2：按等级

}jpf_user_group_info;


typedef struct _jpf_user_group_info_res
{
	JpfMsgErrCode		code;


	char name[64];	//组名
	char id[50]; //组id号
	int level; //等级
	int permissions; //用户组权限

}jpf_user_group_info_res;

typedef struct _jpf_user_group_info_res_inter
{
	int flag;
	int count;
	jpf_user_group_info_res *data_buf;
}jpf_user_group_info_res_inter;

typedef struct _jpf_user_group_info_inter
{
	char domain[USER_NAME_LEN];
	jpf_user_group_info *QueryUserGroup_info;

}jpf_user_group_info_inter;

//管理员信息
typedef struct _jpf_manager_info_res
{
	JpfMsgErrCode		code;

		char name[64];
		int type;	//管理员类型，1：超级管理员，2：普通管理员
}jpf_manager_info_res;

typedef struct _jpf_manager_info_res_inter
{
	int flag;
	int count;
	jpf_manager_info_res *date_buff;
}jpf_manager_info_res_inter;

typedef struct _jpf_manager_info
{
	int count;			//查询用户组个数
	int start_line; 	//开始的行序号
	int key;			//模糊查询的内容，如果是按类型查询则是0
	int type;			//查询类型，0：查询所有用户，1：按组名，2：按等级

}jpf_manager_info;


typedef struct _jpf_manager_info_inter
{
	char domain[USER_NAME_LEN];
	jpf_manager_info *QueryManager_info;

}jpf_manager_info_inter;


//设备信息
typedef struct _jpf_device_info
{
	int count;			//查询用户组个数
	int start_line; 	//开始的行序号
	int key;			//模糊查询的内容，如果是按类型查询则是0
	int type;			//查询类型，0：查询所有用户，1：按组名，2：按等级

}jpf_device_info;


typedef struct _jpf_device_info_inter
{
	char domain[USER_NAME_LEN];
	jpf_device_info *QueryDevice_info;

}jpf_device_info_inter;


typedef struct _jpf_device_info_res
{
	JpfMsgErrCode		code;

	char puid[16];			//设备id
	char puName[64];		//设备名
	char dev_type[20];		//设备类型
	char domain_id[8];		//域id
	char area_id[8];		//区域id
	char mdu_id[50];		//转发服务器id
	int heatbeat;			//心跳
	char equipment_manufacturers[50];	//厂商名

}jpf_device_info_res;

typedef struct _jpf_device_info_res_inter
{
	int flag;
	int count;
	jpf_device_info_res *date_buff;
}jpf_device_info_res_inter;

//业务点信息
typedef struct _jpf_profession_point_info
{
	char domainid[16]; //域Id
	char guid[24]; //业务点id
	char puid[24]; //设备id
	int count;	 //查询个数
	int start_line; 	//开始的行序号
	int type;			//查询类型，0：查询所有Gu；1：根据域id和puid查询；2：根据域id和guid查询
}jpf_profession_point_info;

typedef struct _jpf_profession_point_inter
{
	char domain[USER_NAME_LEN];
	jpf_profession_point_info *QueryProfession_info;

}jpf_profession_point_inter;

typedef struct _jpf_profession_point_res
{
	JpfMsgErrCode		code;


	char guid[24];	//业务id
	char id[16]; //业务所属的域
	char puid[16]; //设备id
	char guName[64]; //业务点名
	char type[10];
	int ptz; //1表示有云台，0表示没有
	int guAlarmBypass;	//1表示有旁路报警，0表示没有

}jpf_profession_point_res;

typedef struct _jpf_profession_point_res_inter
{
	int flag;
	int count;
	jpf_profession_point_res *date_buff;
}jpf_profession_point_res_inter;

//区域信息
typedef struct _JpfQueryRegion JpfQueryRegion;
struct _JpfQueryRegion
{
    gint               req_num;
    gint               start_num;
    gint               area_id;
};



typedef struct _jpf_region_info
{

	gint				area_id;
	gchar				area_name[AREA_NAME_LEN];
	gint				area_parent;

}jpf_region_info;

typedef struct _jpf_query_region_info_inter
{
	char domain[USER_NAME_LEN];
	JpfQueryRegion *QueryRegion;

}jpf_query_region_info_inter;

typedef struct _JpfQueryRegionRes JpfQueryRegionRes;
struct _JpfQueryRegionRes
{
    JpfMsgErrCode      code;
    gint               total_num;
    gint               req_num;
    jpf_region_info    *region_info;
};



/*
typedef struct _jpf_region_info_res
{
	JpfMsgErrCode		code;
	char Name[64]; //区域名
	char parent[32];	//该级区域的一级父区域名
	char current_parent[32];		//当前该区域的父区域名
	int region_level;	//区域等级，例如一级区域，二级区域
	int sub_level_region_flag;   //-1:表示该区域下没有子级区域，1：表示有子级区域

}jpf_region_info_res;

typedef struct _jpf_region_info_res_inter
{
	int flag;
	int count;
	jpf_region_info_res *date_buff;
}jpf_region_info_res_inter;
*/


//转发服务器信息
typedef struct _jpf_relay_server_info
{

	int count;	//查询个数
	int start_line; 	//开始的行序号
	int type;			//查询类型，0：查询所有设备，1:根据区域查询设备
	int key;			//key=1时表示区域查询
}jpf_relay_server_info;


typedef struct _jpf_relay_server_inter
{
	char domain[USER_NAME_LEN];
	jpf_relay_server_info *QueryRelayServer_info;

}jpf_relay_server_inter;



typedef struct _jpf_relay_server_info_res
{
	JpfMsgErrCode		code;

	char id[10];
	char relay_server_name[32];
	char type[32];
	int heatbeat;
	int device_port;
	int media_port;
}jpf_relay_server_info_res;

typedef struct _jpf_relay_server_info_res_inter
{
	int flag;
	int count;
	jpf_relay_server_info_res *date_buff;
}jpf_relay_server_info_res_inter;


//厂商信息
typedef struct _jpf_query_factory_info
{
	int count;	//查询个数
	int start_line; 	//开始的行序号
	int type;			//查询类型，0：查询所有设备，1:根据区域查询设备
	int key;			//key=1时表示区域查询
}jpf_query_factory_info;

typedef struct _jpf_query_factory_info_inter
{
	char domain[USER_NAME_LEN];
	jpf_query_factory_info *QueryFactory_info;

}jpf_query_factory_info_inter;

typedef struct _jpf_query_factory_info_res
{
	JpfMsgErrCode		code;

	char factory_name[32];
	char flag[10];

}jpf_query_factory_info_res;

typedef struct _jpf_query_factory_info_res_inter
{
	int flag;
	int count;
	jpf_query_factory_info_res *date_buff;
}jpf_query_factory_info_res_inter;

//存储服务器信息
typedef struct _jpf_query_storage_server_info
{
	int count;			//查询个数
	int start_line; 	//开始的行序号
	int type;			//查询类型，0：查询所有设备，1:根据区域查询设备
	int key;			//key=1时表示区域查询
}jpf_query_storage_server_info;

typedef struct _jpf_query_storage_server_info_inter
{
	char domain[USER_NAME_LEN];
	jpf_query_storage_server_info *QueryStorageServer_info;

}jpf_query_storage_server_info_inter;

typedef struct _jpf_query_storage_server_info_res
{
	JpfMsgErrCode		code;

	char id[32];
	char name[32];
	int heatbeat;

}jpf_query_storage_server_info_res;

typedef struct _jpf_query_storage_server_info_res_inter
{
	int flag;
	int count;
	jpf_query_storage_server_info_res *date_buff;
}jpf_query_storage_server_info_res_inter;


/**************************************接口*****************************************/


//server端初始化
void jpf_server_mods_init( void );

//void jpf_server_lib_init( void );


//平台初始化
int sdk_init(int heartbeat, int port);


//查询区域信息
int get_region_info(jpf_query_region_info_inter *in_para, JpfQueryRegionRes *out);

//查询存储服务器信息
int get_storage_server_info(jpf_query_storage_server_info_inter *in_para, jpf_query_storage_server_info_res *out);

//查询厂商信息
int get_factory_info(jpf_query_factory_info_inter *in_para, jpf_query_factory_info_res *out);

//查询转发服务器列表信息
int get_relay_server_info(jpf_relay_server_inter *in_para, jpf_relay_server_info_res *out);

//查询业务点列表信息
int get_profession_point_info(jpf_profession_point_inter *in_para, jpf_profession_point_res *out);

//查询设备列表信息
int get_device_info(jpf_device_info_inter *in_para, jpf_device_info_res *out);

//查询管理员列表信息
int get_manager_info(jpf_manager_info_inter *in_para, jpf_user_group_info_res *out);

//查询用户组信息
int get_user_group_info(jpf_user_group_info_inter *in_para, jpf_user_group_info_res *out);

//查询客户端信息
int get_client_user_info(jpf_user_QueryUser_info *in_para, jpf_QueryUser_info_res *out);

int jpf_server_register_callback(server_callback_function *fun, void *ower);



#endif






