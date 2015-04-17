
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <poll.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>

#include "nmp_proxy_log.h"
#include "nmp_system_ctrl.h"

#include "nmp_config_info.h"
#include "nmp_config_backup.h"

#define SOCKET_ERROR                -1
#define INVALID_SOCKET              -1

#define DEF_LISTENER_KEEP_LIVE      60		//s
#define DEF_BACKUP_KEEP_LIVE        30
#define DEF_BACKUP_PORT             8552
#define DEF_BACKUP_KEY              0x4241434b
#define DEF_BACKUP_FRAME_SIZE       1024

#define DEF_START_BACKUP            system("mkdir ./ProxyBackupTemp");
#define DEF_END_BACKUP              system("rm -rf ./ProxyBackupTemp");

#define DEF_BACKUP_FILE             "/tmp/backup.tar.bz2"
#define DEF_REVERT_FILE             "/tmp/revert.tar.bz2"

#define MIN(a, b)                   ((a)<(b))?(a):(b)

typedef struct backup_frame backup_frame_t;

struct backup_frame
{
    uint32_t magic;
    uint32_t seq;
    uint32_t payload_len;
    uint32_t secret_key;
};


config_backup_t g_cfg_bkp;


static __inline__ int my_recv(int fd, void *buf, size_t size, int flag)
{
    int ret, err;

    while (1)
    {
        ret = recv(fd, buf, size, flag);
        if (0 > ret)
        {
            err = errno;
            if (EINTR == err)
            {
                err = 0;
                continue;
            }
            else if (EAGAIN == err)
            {//阻塞型
                show_info("recv timeout!!!!!!!\n");
                break;
            }
            else
            {
                show_info("recv error.[err: %d]\n", err);
                break;
            }
        }
        else if (0 == ret)	//对端的socket已正常关闭.
        {
            err = ECONNRESET;
            break;
        }
        else
            break;
    }

    return ret;
}

int init_config_backup()
{
    g_cfg_bkp.lock = nmp_mutex_new();
    g_cfg_bkp.state = NORMAL;
    g_cfg_bkp.left = 0;

    return 0;
}

config_backup_t *get_config_backup()
{
	return &g_cfg_bkp;
}

int get_backup_state(config_backup_t *cfg_bkp)
{
    int state;

    NMP_ASSERT(cfg_bkp);

    nmp_mutex_lock(cfg_bkp->lock);
    state = cfg_bkp->state;
    nmp_mutex_unlock(cfg_bkp->lock);

    return state;
}

int set_backup_state(config_backup_t *cfg_bkp, int state)
{
    NMP_ASSERT(cfg_bkp);

    nmp_mutex_lock(cfg_bkp->lock);
    cfg_bkp->state = state;
    nmp_mutex_unlock(cfg_bkp->lock);

    return state;
}

int try_change_backup_state(config_backup_t *cfg_bkp, int state)
{
    NMP_ASSERT(cfg_bkp);

    nmp_mutex_lock(cfg_bkp->lock);
    if (NORMAL == cfg_bkp->state)
        cfg_bkp->state = state;
    else
        state = NORMAL;
    nmp_mutex_unlock(cfg_bkp->lock);

    return state;
}

int get_backup_left_size(config_backup_t *cfg_bkp)
{
    int size;

    NMP_ASSERT(cfg_bkp);

    nmp_mutex_lock(cfg_bkp->lock);
    size = cfg_bkp->left;
    nmp_mutex_unlock(cfg_bkp->lock);

    return size;
}

int set_backup_left_size(config_backup_t *cfg_bkp, int size)
{
    NMP_ASSERT(cfg_bkp);

    if (0 > size)
        return -1;

    nmp_mutex_lock(cfg_bkp->lock);
    cfg_bkp->left = size;
    nmp_mutex_unlock(cfg_bkp->lock);

    return size;
}

int config_figure_up_file_len(config_backup_t *cfg_bkp)
{
    int len = 0;
    char sys_cmd[DEF_SYS_COMMAND_SIZE];

    sprintf(sys_cmd, "cd %s;tar -jcvf %s %s %s %s;cd -", 
        (char*)proxy_get_data_file_path(), DEF_BACKUP_FILE, 
        DEF_USER_INFO_FILE, DEF_DEVICE_INFO_FILE, DEF_SERVER_CONFIG_FILE);
    if (!system(sys_cmd))
    {
        FILE *fpr;
        if ((fpr = fopen(DEF_BACKUP_FILE, "r")))
        {
            fseek(fpr, 0, SEEK_END);
            len = ftell(fpr);
            fseek(fpr, 0, SEEK_SET);
            fclose(fpr);

            if (len%DEF_BACKUP_FRAME_SIZE)
                len += (len/DEF_BACKUP_FRAME_SIZE+1)*sizeof(backup_frame_t);
            else
                len += (len/DEF_BACKUP_FRAME_SIZE)*sizeof(backup_frame_t);
        }
    }

    set_backup_left_size(cfg_bkp, len);
    return len;
}

static __inline__ char *
config_simple_encrypt(backup_frame_t *frame, char *buf, size_t size)
{
    int i, j, l;
    char *key, tmp[DEF_BACKUP_FRAME_SIZE];

    memcpy(tmp, buf, size);
    l = sizeof(frame->secret_key);
    key = (char*)&frame->secret_key;

    for (i=l; i<size; i+=l)
    {
        for (j=0; j<l; j++)
        {
            buf[i-l+j] = ~(tmp[i-j-1]^key[j]);
        }
    }

    //frame->secret_key = *((int*)&tmp[i-l]);

    return buf;
}

static __inline__ char *
config_simple_decrypt(backup_frame_t *frame, char *buf, size_t size)
{
    int i, j, l;
    char *key, tmp[DEF_BACKUP_FRAME_SIZE];

    memcpy(tmp, buf, size);
    l = sizeof(frame->secret_key);
    key = (char*)&frame->secret_key;

    for (i=l; i<size; i+=l)
    {
        for (j=0; j<l; j++)
        {
            buf[i-j-1] = ~tmp[i-l+j]^key[j];
        }
    }

    return buf;
}

static void config_download_data(int fd, config_backup_t *cfg_bkp)
{
    FILE *fpr;
    backup_frame_t frame;

    int n_read, n_left, n_write;
    char buf[DEF_BACKUP_FRAME_SIZE];

    fpr = fopen(DEF_BACKUP_FILE, "r");
    if (!fpr)
        return ;

    frame.magic = DEF_REVERT_MAGIC;
    frame.seq = 0;
    frame.payload_len = DEF_BACKUP_FRAME_SIZE;
    frame.secret_key = DEF_BACKUP_KEY;

    n_left = get_backup_left_size(cfg_bkp);
    show_debug("left: %d<-----------------------\n", n_left);
    while (n_left > 0)
    {
        n_left -= sizeof(backup_frame_t);
        n_read = MIN(n_left, DEF_BACKUP_FRAME_SIZE);

        frame.payload_len = fread(buf, 1, n_read, fpr);
        config_simple_encrypt(&frame, buf, frame.payload_len);

        if (0 > send(fd, &frame, sizeof(backup_frame_t), 0))
        {
            show_info("recv error.[err: %d]\n", errno);
            break;
        }

        n_write = send(fd, buf, frame.payload_len, 0);
        if (0 > n_write)
        {
            show_info("recv error.[err: %d]\n", errno);
            break;
        }

        n_left -= n_write;
        frame.seq++;
    }

    show_debug("left: %d<-----------------------\n", n_left);
    fclose(fpr);
}
static void config_upload_data(int fd, config_backup_t *cfg_bkp)
{
    FILE *fpw;
    backup_frame_t frame;

    int n_read, n_left, seq = 0;
    char buf[DEF_BACKUP_FRAME_SIZE];

    fpw = fopen(DEF_REVERT_FILE, "w");
    if (!fpw)
        return ;

    n_left = get_backup_left_size(cfg_bkp);
    show_debug("left: %d<-----------------------\n", n_left);
    while (n_left > 0)
    {
        n_read = my_recv(fd, &frame, sizeof(backup_frame_t), 0);
        if (n_read != sizeof(backup_frame_t))
            break;

        n_left -= n_read;

        if (DEF_REVERT_MAGIC != frame.magic)
            break;
        if (seq++ != frame.seq)
            break;
        if (0 >= frame.payload_len)
            break;
        if (DEF_BACKUP_KEY != frame.secret_key)
            break;

        n_read = my_recv(fd, buf, frame.payload_len, 0);
        if (n_read != frame.payload_len)
            break;

        config_simple_decrypt(&frame, buf, n_read);
        fwrite(buf, 1, n_read, fpw);

        n_left -= n_read;
        set_backup_left_size(cfg_bkp, n_left);
    }

    show_debug("left: %d<-----------------------\n", n_left);
    fclose(fpw);

    if (!n_left)
    {
        char sys_cmd[DEF_SYS_COMMAND_SIZE];
        sprintf(sys_cmd, "tar -jxvf %s -C %s", DEF_REVERT_FILE, 
            (char*)proxy_get_data_file_path());
        system(sys_cmd);

        proxy_task_t *task;
        task = proxy_new_task(REBOOT_SERVER, NULL, 0, NULL, NULL);
        if (task)
            proxy_thread_pool_push(task);
    }
}


static void *config_backup_thread_proxy(void *data)
{
    fd_set fd_st;
    config_backup_t *cfg_bkp;
    int err, magic;
    int listener, client_fd = 0;
    struct timeval timeout = {DEF_BACKUP_KEEP_LIVE, 0};

    pthread_detach(pthread_self());

    listener = (int)data;
    cfg_bkp = get_config_backup();

    // initialize file descriptor set
    FD_ZERO(&fd_st);
    FD_SET(listener, &fd_st);

    show_info("=========== Backup Selecting...[listener: %d] =========== \n", listener);
    err = select(listener+1, &fd_st, NULL, NULL, &timeout);
    if (-1 == err)
    {
        show_info("Select error.\n");
    }
    else if (0 == err)
    {
        show_info("Select timeout.\n\n");
    }
    else
    {
        client_fd = accept(listener, NULL, NULL);
        if (INVALID_SOCKET != client_fd)
        {show_info("===========>> accept: %d \n", client_fd);
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, 
                &timeout, sizeof(timeout));
            switch (get_backup_state(cfg_bkp))
            {
                case BACKUP:
                    recv(client_fd, &magic, sizeof(magic), 0);
                    printf("MAGIC: 0x%x, 0x%x, 0x%x\n", magic, DEF_BACKUP_MAGIC, ntohl(magic));
                    if (DEF_BACKUP_MAGIC != ntohl(magic))
                    {
                        show_warn("magic incorrect!!!!!!!!!!\n");
                        break;
                    }

                    config_download_data(client_fd, cfg_bkp);
                    break;
                case REVERT:
                    recv(client_fd, &magic, sizeof(magic), 0);
                    printf("MAGIC: 0x%x, 0x%x\n", magic, DEF_REVERT_MAGIC);
                    if (DEF_REVERT_MAGIC != ntohl(magic))
                    {
                        show_warn("magic incorrect!!!!!!!!!!\n");
                        break;
                    }

                    config_upload_data(client_fd, cfg_bkp);
                    break;
            }
            show_info("===========>> close: %d \n", client_fd);
            close(client_fd);
        }
    }

    set_backup_left_size(cfg_bkp, 0);
    set_backup_state(cfg_bkp, NORMAL);
    close(listener);
    return NULL;
}

int config_create_backup_listener(int *listener)
{
    int retval, port = DEF_BACKUP_PORT;
    struct sockaddr_in sockaddr;

    if (SOCKET_ERROR == (*listener = socket(AF_INET, SOCK_STREAM, 0)))
    {
        show_warn("socket error.\n");
        return SOCKET_ERROR;
    }
    else
        show_info("Create socket[%d] succ ...\n", *listener);

    do
    {
        /* set sockaddr_in */
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port   = htons(port); //将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        bzero(&(sockaddr.sin_zero), sizeof(sockaddr.sin_zero));

        retval = bind(*listener, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr));
        if (SOCKET_ERROR == retval)
        {
            show_warn("bind error.\n");
            if (65535 <= ++port)
            {
                close(*listener);
                return SOCKET_ERROR;
            }
        }
        else
        {
            show_info("Bind sockfd succ ...\n");
            break;
        }
    }while (1);

    if (SOCKET_ERROR == listen(*listener, 1))
    {
        show_warn("listen error.\n");
        close(*listener);
        return SOCKET_ERROR;
    }
    else
    {
        show_info("[backup]Server listening [listener: %d]...\n", *listener);

        retval = fcntl(*listener, F_GETFL);
        if ((0 > retval) || 
            (0 > fcntl(*listener, F_SETFL, retval | O_NONBLOCK)))   //非阻塞
        {
            show_warn("fcntl");
            close(*listener);
            return SOCKET_ERROR;
        }
    }

    return port;
}

int config_start_backup_listen_thread(int listener)
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, (void*)config_backup_thread_proxy, 
            (void*)listener))
    {
        show_warn("Create send pthread error!\n");
        close(listener);
        set_backup_left_size(&g_cfg_bkp, 0);
        set_backup_state(&g_cfg_bkp, NORMAL);
        return -1;
    }

    return 0;
}

