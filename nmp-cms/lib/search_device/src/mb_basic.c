#include "mb_basic.h"





list_t*
    list_new(void)
{
    list_t *ls = (list_t *)malloc(sizeof(list_t));
    INIT_LIST_HEAD(&ls->head);
    pthread_mutex_init(&ls->mutex, NULL);
    atomic_set(&ls->ref_count, 1);

    return ls;
}
list_t *list_ref(list_t *ls)
{
    assert(ls != NULL &&
        atomic_get(&ls->ref_count) > 0);

    atomic_inc(&ls->ref_count);

    return ls;
}
void list_unref(list_t *ls)
{
    assert( ls != NULL &&
        atomic_get(&ls->ref_count) > 0);

    if (atomic_dec_and_test_zero(&ls->ref_count))
    {
        pthread_mutex_destroy(&ls->mutex);
        if(ls)free(ls);
    }
}

seq_t*
    seq_new(void)
{
    seq_t *sg = (seq_t *)malloc(sizeof(seq_t));
    atomic_set(&sg->ref_count, 1);
    atomic_set(&sg->seq_no, 1);
    return sg;
}
seq_t*
    seq_ref(seq_t *sg)
{
    assert(sg != NULL &&
        atomic_get(&sg->ref_count) > 0);

    atomic_inc(&sg->ref_count);

    return sg;
}
unsigned int
    seq_generate(seq_t *sg)
{
    assert(sg != NULL &&
        atomic_get(&sg->ref_count) > 0);

    return atomic_inc(&sg->seq_no);
}

void
    seq_unref(seq_t *sg)
{
    assert( sg != NULL &&
        atomic_get(&sg->ref_count) > 0);

    if (atomic_dec_and_test_zero(&sg->ref_count))
    {
        if(sg)free(sg);
    }

    return;
}

unsigned int abs_time_gen(unsigned int sec)
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + sec;
}

int curr_time_greater(unsigned int tm)
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec > tm;
}



int mb_get_mac_addr(const char *if_name, char *macaddr, size_t len)
{
    int fd, i, find_mac;
    find_mac = 0;
    char buffer[COMM_ADDRSIZE];

    struct ifreq buff[10];
    memset(buff, 0, sizeof(buff));
    struct ifconf ifc;

    int intrface;
    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (caddr_t) buff;
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0))< 0)
    {
        printf("socket dgram eror!\n");
        return -1;
    }
    if (ioctl (fd, SIOCGIFCONF, (char *) &ifc))
    {
        close(fd);
        printf("SIOCGIFCONF failed\n");
        return -1;
    }

     intrface = ifc.ifc_len / sizeof (struct ifreq);
     printf("interface num is intrface=%d\n",intrface);
     for(i=0; i<intrface; i++)
     {
        printf ("net device %s\n", buff[i].ifr_name);
        if(strstr(buff[i].ifr_name, "lo") != NULL || strstr(buff[i].ifr_name, ":") != NULL)
        {
            printf("skip this interface! name:%s\n", buff[i].ifr_name);
            continue;
        }
        else
        {
            if (ioctl(fd, SIOCGIFHWADDR, &buff[i]) == 0)
            {
                snprintf(buffer, COMM_ADDRSIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[0],
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[1],
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[2],
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[3],
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[4],
                    (unsigned char)buff[i].ifr_hwaddr.sa_data[5]);
                find_mac = 1;
                break;
            }
        }
    }


    if (strlen(buffer) > len-1)
    {
        close(fd);
        return(-1);
    }
    strncpy(macaddr, buffer, len);

    close(fd);
    return(0);
}
int mb_get_ip_addr(const char *if_name, char *ipaddr, size_t len)
{
    int fd;
    char buffer[COMM_ADDRSIZE];
    struct ifreq ifr;
    struct sockaddr_in *addr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
        {
            addr = (struct sockaddr_in *)&(ifr.ifr_addr);
            inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
        }
        else
        {
            close(fd);
            return(-1);
        }
    }
    else
    {
        perror("os_getIpAddr error :");
        return(-1);
    }

    if (strlen(buffer) > len-1)
    {
        return(-1);
    }
    strncpy(ipaddr, buffer, len);
    close(fd);
    return(0);
}


int mb_listen(char *mb_addr, unsigned short port, int is_sock_reuse)
{
    system("route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0 &");
    printf("add route !\n");
    char eth0_ip_addr[16] = {0};
    //struct ip_mreqn mreq;
    struct in_addr if_req;
    struct sockaddr_in servaddr_m;
    int listen_sock = -1;
    int sock_resuse = 1;
    int ttl = 10;
    int nSize = 2*1024 * 1024;
    socklen_t mylen = sizeof(nSize);

    if((listen_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("sock error!:%s\n", strerror(errno));
        goto __error;
    }
    /* reuse */
    if(is_sock_reuse)
    {
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &sock_resuse, sizeof(int)) < 0)
        {
            printf("error: listen_sock setsockopt SO_REUSEADDR, (%m)\n");
            goto __error;
        }
    }

    if (setsockopt(listen_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,sizeof(int)) < 0)
    {
        printf("error: listen_sock setsockopt ttl, (%m)\n");
        goto __error;
    }

    if(setsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, sizeof(nSize)) < 0)
    {
        printf("error: listen_sock setsockopt rcvbuf, (%m)\n");
        goto __error;
    }

    if(getsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, &mylen) < 0)
    {
        printf("error: listen_sock getsockopt rcvbuf, (%m)\n");
        goto __error;
    }
    printf("recv buf len %d\n", nSize);

    /* bind */
    memset(&servaddr_m, 0, sizeof(servaddr_m));
    servaddr_m.sin_family = AF_INET;
    inet_aton(mb_addr, &servaddr_m.sin_addr);
    servaddr_m.sin_port = htons((uint16_t)port);
    if (bind(listen_sock, (struct sockaddr *)&servaddr_m, sizeof(servaddr_m)) < 0)
    {
        printf("error: listen_sock bind, (%m)\n");
        goto __error;
    }
    /* add member */



    struct   ip_mreqn   mreq;
//mreq.imr_multiaddr.s_addr   =   inet_addr( "224.1.1.1 ");
mreq.imr_address.s_addr   =   htonl(INADDR_ANY);
mreq.imr_ifindex   =   if_nametoindex("eth0");
    printf(">>>>>  if_nametoindex: %d\n", if_nametoindex("eth0"));


    inet_aton(mb_addr, &mreq.imr_multiaddr);
    //inet_aton(eth0_ip_addr, &mreq.imr_interface);
    if(setsockopt(listen_sock
                    , IPPROTO_IP
                    , IP_ADD_MEMBERSHIP
                    , &mreq
                    , sizeof(mreq)) < 0)
    {
        printf("error: listen_sock add mb group, (%m)\n");
        goto __error;
    }
    inet_aton(eth0_ip_addr, &if_req);
    /* set out interface*/
    #if 0
    if(setsockopt(listen_sock
                    , IPPROTO_IP
                    , IP_MULTICAST_IF
                    , &if_req
                    , sizeof(struct in_addr)) < 0 )
    {
        printf("error: listen_sock set mb send eth0, (%m)\n");
        goto __error;
    }
    #endif
    printf("listen_sock:%d\n", listen_sock);
    return listen_sock;
__error:
    if(listen_sock)close(listen_sock);
    printf("error -1\n");
    return -1;
}

#if 1
int mb_listen_2(char *mb_addr, unsigned short port, int is_sock_reuse)
{
    register int fd, intrface, listen_sock, i= 0;
    fd = -1;
    listen_sock = -1;
    int sock_resuse = 1;
    int enable_loop = 0;

    int ttl = 10;

    struct sockaddr_in servaddr_m;
    int nSize = 2*1024 * 1024;
    socklen_t mylen = sizeof(nSize);
    struct ifreq buf[100];
    memset(buf, 0, sizeof(buf));
    struct ifconf ifc;
    memset(&ifc, 0 , sizeof(struct ifconf));
    char name_buf[32] = {0};


    if ((fd = socket (AF_INET, SOCK_DGRAM, 0))< 0)
    {
        printf("socket dgram error!\n");
        goto __error;
    }
    else
    {
        ifc.ifc_len = 100 * sizeof(struct ifreq);
        ifc.ifc_buf = (caddr_t) buf;
        if (ioctl (fd, SIOCGIFCONF, (char *) &ifc))
        {
            printf("SIOCGIFCONF failed\n");
            goto __error;
        }
         intrface = ifc.ifc_len / sizeof (struct ifreq);
         printf("interface num is intrface=%d\n",intrface);
         for(i=0; i<intrface; i++)
         {
            printf ("net device >>>>>%s\n", buf[i].ifr_name);

            //mac
            if (ioctl(fd, SIOCGIFHWADDR, (char *)&buf[i]) == 0)
            {
                snprintf(name_buf, COMM_ADDRSIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[0],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[1],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[2],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[3],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[4],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[5]);
                printf("mac:%s\n", name_buf);
            }
            else
            {
                char str[256];
                sprintf (str, "cpm: ioctl get mac device %s\n", buf[intrface].ifr_name);
                perror (str);
            }
         }
    }
    if((listen_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("sock error!:%s\n", strerror(errno));
        goto __error;
    }
    /* reuse */
    if(is_sock_reuse)
    {
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &sock_resuse, sizeof(int)) < 0)
        {
            printf("error: listen_sock setsockopt SO_REUSEADDR, (%m)\n");
            goto __error;
        }
    }

    if (setsockopt(listen_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,sizeof(int)) < 0)
    {
        printf("error: listen_sock setsockopt ttl, (%m)\n");
        goto __error;
    }

    if(setsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, sizeof(nSize)) < 0)
    {
        printf("error: listen_sock setsockopt rcvbuf, (%m)\n");
        goto __error;
    }

    if(getsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, &mylen) < 0)
    {
        printf("error: listen_sock getsockopt rcvbuf, (%m)\n");
        goto __error;
    }
    printf("recv buf len %d\n", nSize);

    /* bind */
    memset(&servaddr_m, 0, sizeof(servaddr_m));
    servaddr_m.sin_family = AF_INET;
    inet_aton(mb_addr, &servaddr_m.sin_addr);
    servaddr_m.sin_port = htons((uint16_t)port);
    if (bind(listen_sock, (struct sockaddr *)&servaddr_m, sizeof(servaddr_m)) < 0)
    {
        printf("error: listen_sock bind, (%m)\n");
        goto __error;
    }
    /* add member */



    struct   ip_mreqn   mreq;
    for(i=0; i<intrface; i++)
    {
        if(strstr(buf[i].ifr_name, "lo") != NULL || strstr(buf[i].ifr_name, ":") != NULL)
        {
            printf("skip this interface! name:%s\n", buf[i].ifr_name);
            continue;
        }
        printf("interface_name:>>>>>>>>>>>>>>%s\n", buf[i].ifr_name);
        mreq.imr_address.s_addr   =   htonl(INADDR_ANY);
        mreq.imr_ifindex   =   if_nametoindex(buf[i].ifr_name);
        printf(">>>>>  if_nametoindex:%d \n", mreq.imr_ifindex);


        inet_aton(mb_addr, &mreq.imr_multiaddr);
    //inet_aton(eth0_ip_addr, &mreq.imr_interface);
        if(setsockopt(listen_sock
                    , IPPROTO_IP
                    , IP_ADD_MEMBERSHIP
                    , &mreq
                    , sizeof(mreq)) < 0)
        {
            printf("error: listen_sock add mb group, (%m)\n");
        }
    }
    #if 1
    if (setsockopt(listen_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &enable_loop, sizeof(enable_loop)) < 0)
    {
        printf("error: listen_sock disable_loop (%m)\n");
    }
    #endif
    printf("listen_sock:%d\n", listen_sock);
    if(fd >= 0)close(fd);
    return listen_sock;
__error:
    if(fd >= 0)close(fd);
    if(listen_sock >= 0)close(listen_sock);
    return -1;
}
#endif


