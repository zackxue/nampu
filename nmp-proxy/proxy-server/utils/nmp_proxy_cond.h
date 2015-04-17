/*
 *          file: nmp_proxy_cond.h
 *          description:封装条件变量接口，简化使用
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __PROXY_COND_H__
#define __PROXY_COND_H__

#define MAX_INIT_WAITE_TIMES            5


typedef struct proxy_cond proxy_cond_t;


#ifdef __cplusplus
extern "C" {
#endif

int proxy_cond_new(proxy_cond_t **cond);
int proxy_cond_wait(proxy_cond_t *cond);
int proxy_cond_timed_wait(proxy_cond_t *cond, int sec);
int proxy_cond_signal(proxy_cond_t *cond);
int proxy_cond_free(proxy_cond_t *cond);


#ifdef __cplusplus
    }
#endif

#endif  //__PROXY_COND_H__

