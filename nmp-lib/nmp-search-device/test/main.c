/*
 * jpf-search-device:
 * Copyright (C) 2013 by hegui <heguijiss@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "search_device.h"

user_info_t user_info = {"admin", "admin"};

static void *search_thread1(void *data)
{
    int ret;
    jpf_redirect_t jpf_red;
    redirect_t *redirect;
    
    search_array_t *array;
    search_result_t *result;

    array = search_device();
    if (array && array->count)
    {printf("thread 1, count: %d\n", array->count);
        result = &array->result[0];

        /*memcpy(&jpf_red.user_info, &user_info, sizeof(user_info_t));
        redirect = &jpf_red.redirect;
        snprintf(redirect->pu_id, NMP_SEARCH_ID_LEN, "JXJ-IPC-00000001");
        snprintf(redirect->cms_ip, NMP_SEARCH_IP_LEN, "192.168.1.12");
        redirect->cms_port = 9902;
        redirect->conn_cms = 1;

        printf("\nRedirect info:\n");
        printf("    pu_id   : %s\n", redirect->pu_id);
        printf("    cms_ip  : %s\n", redirect->cms_ip);
        printf("    cms_port: %d\n", redirect->cms_port);
        printf("    conn_cms: %d\n", redirect->conn_cms);

//        ret = set_platform_info(result->dst_id, &jpf_red);
        printf("set_platform_info %s! error: %d\n", !ret ? "Successful" : "failure", ret);*/
    }
    destory_search_result(array);
}
static void *search_thread2(void *data)
{
    int ret;
    jpf_redirect_t jpf_red;
    redirect_t *redirect;

    search_array_t *array;
    search_result_t *result;

    array = search_device();
    if (array->count)
    {printf("thread 2, count: %d\n", array->count);
        result = &array->result[0];

        memcpy(&jpf_red.user_info, &user_info, sizeof(user_info_t));
        redirect = &jpf_red.redirect;
        snprintf(redirect->pu_id, NMP_SEARCH_ID_LEN, "JXJ-IPC-00000001");
        snprintf(redirect->cms_ip, NMP_SEARCH_IP_LEN, "192.168.1.12");
        redirect->cms_port = 9902;
        redirect->conn_cms = 1;

//        ret = set_platform_info(result->dst_id, &jpf_red);
        printf("set_platform_info %s! error: %d\n", !ret ? "Successful" : "failure", ret);
    }
    destory_search_result(array);
}

int main()
{
    char *p = "何桂";
    int i = 0x8048bce;
    char tmp[32];
    sprintf(tmp, "%s", &i);
    //printf("%s, 0x%x, %s, %s\n", p, p, tmp, &i);exit(0);
    printf("jpf-search-device\n");

    int tid1, tid2;

    search_device_init();
    
    if(pthread_create(&tid1, NULL, search_thread1, NULL))
    {
        printf("create search thread 1 failed\n");
        return NULL;
    }
    
    /*if(pthread_create(&tid1, NULL, search_thread2, NULL))
    {
        printf("create search thread 2 failed\n");
        return NULL;
    }*/

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    search_device_uninit();

    return 0;
}


