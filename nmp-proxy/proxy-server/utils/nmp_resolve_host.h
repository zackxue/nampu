/*
 *          file: nmp_resolve_host.h
 *          description:负责将域名解析为IP 地址
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __RESOLVE_HOST_H__
#define __RESOLVE_HOST_H__

#include <arpa/inet.h>


#define MAX_IP_COUNT         8


typedef struct ifa ifa_t;

struct ifa
{
    int match;
    int addr;
};

typedef struct local_ifs local_ifs_t;

struct local_ifs
{
    int count;
    ifa_t ifa[MAX_IP_COUNT];
};


#ifdef __cplusplus
extern "C" {
#endif

int proxy_resolve_host_init();
void proxy_resolve_host_cleanup();

void resolve_host_proxy(const void *host);

char *proxy_resolve_host_immediate(const char *host, char *addr, size_t size);
char *proxy_resolve_host(const char *host, char *addr, size_t size); /* Non-blocking */

void proxy_get_local_ip(local_ifs_t *ifs);




#ifdef __cplusplus
    }
#endif

#endif  /* __RESOLVE_HOST_H__ */

