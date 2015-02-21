#ifndef __mb_basic__
#define __mb_basic__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include <sched.h>
#include <sys/timeb.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/syscall.h>

#include "list.h"
#include "mb_api.h"


#define MB_MSG_MAX_SIZE (64 * 1024)
#define COMM_ADDRSIZE   20
#define MB_MAX_ETH_NAME_SIZE 32
#define MB_MAX_ID_SIZE       32


/*
 * msg head
 */
typedef struct mb_msg_s {
 
    char    src[MB_SERIAL_LEN];     //源ID
    char    dst[MB_SERIAL_LEN];     //目标ID
    int     msg_id;                 //命令ID
    int     req_no;                 //命令序号
    int     args;                   //是设置还是获取MB_MSG_DIR_E
    int     port;                   //通过哪个端口发出去的

    int     error;                  //错误码

    char    user[MB_USER_LEN];      //用户名cfg时用到
    char    pass[MB_PASS_LEN];      //密码      cfg时用

    int     size;
    char    data[0];                //消息体
}mb_msg_t;

#define MB_MAX_ETH_NUM   (10)

typedef struct _eth_addr_s {
    int    listen_fd;
    char   eth_name[MB_MAX_ETH_NAME_SIZE];
    char   src_id[MB_MAX_ID_SIZE];   
}eth_addr_t;


typedef int atomic_t;
#define atomic_set(p, val) ((*(p)) = (val))
#define atomic_get(p) (*(p))
#define atomic_add(p, val) ((*(p)) += (val))
#define atomic_inc(p) atomic_add(p, 1)
#define atomic_dec_and_test_zero(p) (atomic_add(p, -1), !*(p))

/* list_t */
typedef struct list_s {
    int ref_count;
    struct list_head head;
    pthread_mutex_t mutex;
}list_t;


#ifdef __cplusplus
extern "C" {
#endif

list_t*
     list_new(void);
list_t*
     list_ref(list_t *ls);
void list_unref(list_t *ls);

/* seq_t */
typedef struct seq_s {
    int ref_count;
    unsigned int seq_no;
}seq_t;

seq_t*
    seq_new(void);
seq_t*
    seq_ref(seq_t *sg);
unsigned int 
    seq_generate(seq_t *sg);
void 
    seq_unref(seq_t *sg);

unsigned int abs_time_gen(unsigned int sec);
int curr_time_greater(unsigned int tm);


/*basic mb_api*/

int mb_get_mac_addr(const char *if_name, char *macaddr, size_t len);
int mb_get_ip_addr(const char *if_name, char *ipaddr, size_t len);
int mb_listen(char *mb_addr, unsigned short port, int is_sock_reuse);
int mb_listen_2(char *mb_addr, unsigned short port, int is_sock_reuse);
int mb_listen_3(char *mb_addr, unsigned short port, int is_sock_reuse, eth_addr_t* eth_addr, int *eth_active_num);

#ifdef __cplusplus
}
#endif


#endif
