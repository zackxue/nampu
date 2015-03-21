/*
 * jpf-search-device:
 * Copyright (C) 2013 by hegui <heguijiss@163.com>
 */

#ifndef __SEARCH_API_H__
#define __SEARCH_API_H__

#include <stdio.h>


#define NMP_SEARCH_ID_LEN               32
#define NMP_SEARCH_IP_LEN               16
#define NMP_SEARCH_MNFCT_LEN            12

#define NMP_SEARCH_USERNAME_LEN         32
#define NMP_SEARCH_PASSWORD_LEN         32

typedef enum JPE_SEARCH_ERROR
{
    JPE_SEARCH_SUCCESS = 0,
    JPE_SEARCH_TIMEOUT = 1,
    JPE_SEARCH_FAILED  = 2,
}JPE_SEARCH_ERROR_E;

typedef struct device_info
{
    int pu_type;                        //设备类型
    int av_mun;                         //音视频点个数
    char mnfct[NMP_SEARCH_MNFCT_LEN];   //设备厂商
    char pu_id[NMP_SEARCH_ID_LEN];      //设备标识
    char dev_ip[NMP_SEARCH_IP_LEN];     //设备ip
    char res[32];
}device_info_t;

typedef struct nmp_platform
{
    char cms_ip[NMP_SEARCH_IP_LEN];     //平台cms ip
    char mds_ip[NMP_SEARCH_IP_LEN];     //平台mds ip
    int cms_port;                       //平台cms端口号
    int mds_port;                       //平台mds端口号
    int conn_cms;                       //连接平台开关，1-连接  0-不连接
    char res[32];
}nmp_platform_t;

typedef struct nmp_search
{
    device_info_t  dev_info;
    nmp_platform_t nmp_plt;
}nmp_search_t;

typedef struct search_result
{
    char dst_id[NMP_SEARCH_ID_LEN];
    nmp_search_t nmp_srch;
}search_result_t;

typedef struct search_array
{
    int count;                          //条目
    int capacity;                       //容量
    search_result_t result[0];
}search_array_t;



typedef struct user_info
{
    char usr[NMP_SEARCH_USERNAME_LEN];
    char pwd[NMP_SEARCH_PASSWORD_LEN];
}user_info_t;

typedef struct redirect
{
    char pu_id[NMP_SEARCH_ID_LEN];      //设备标识
    char cms_ip[NMP_SEARCH_IP_LEN];     //平台cms ip
    int cms_port;                       //平台cms端口号
    int conn_cms;                       //连接平台开关，1-连接  0-不连接
}redirect_t;

typedef struct nmp_redirect
{
    user_info_t user_info;
    redirect_t redirect;
}nmp_redirect_t;


#ifdef __cplusplus
extern "C" {
#endif

    int search_device_init();
    int search_device_uninit();

    search_array_t *search_device();
    void destory_search_result(search_array_t *array);

    int set_platform_info(char *dst_id, nmp_redirect_t *nmp_red);



#ifdef __cplusplus
}
#endif

#endif //__SEARCH_API_H__

