
#include <stdio.h>
#include <pthread.h>

#include "media_struct.h"
#include "talk_api.h"
#include "talk_server.h"

#define MAX_FRAME_LEGNTH (160)
#define FILE_PATH "lsy_andy.ulaw"

static void send_thread(void *arg)
{
    int ret = 0;
    talk_frame_hdr_t *frm = calloc(1, sizeof(talk_frame_hdr_t) + MAX_FRAME_LEGNTH); 
    int *fd = (int *)arg;
    char frame_buf[MAX_FRAME_LEGNTH] = {0};
    int frame_length = MAX_FRAME_LEGNTH;
    int file_fd = *(fd + 3);
    
    talk_handle_t *hdl = (talk_handle_t *)(*(fd + 1));
    pthread_detach(pthread_self());

    //usleep(500 * 1000);

    while (*(fd + 2) == 0x1)
    {
        ret = read_file(file_fd, frame_buf, frame_length);
        if (ret < 0)
        {
            printf("read_file err:%d\n", ret);
            //break;
            continue;
        }
        frm->magic = htonl(TALK_MAGIC);
        frm->frame_length = htonl(ret);
        frm->pts = 0;
        frm->frame_num = htonl((*(fd + 4))++);
        memcpy(frm->frame_data, frame_buf, ret);
        
        send_talk_data(hdl, frm);

        usleep(20 * 1000);
    }

    printf("send_thread quit thread id:%p\n", *(fd + 0));
    talk_handle_unref(hdl);
    close_file(file_fd);
}

int test_open(talk_handle_t *hdl, const char* device, const int channel, media_info_t *info)
{
//return -1;
    int ret;
    pthread_t thid;
    int *fd = (int *)calloc(1, sizeof(int)*8);
    printf("@@@@@ test_open hdl: %p device: %s channel: %d encode_type: %d@@@@@\n", hdl, device, channel, info->attr.encode_type);

    *(fd + 3) = open_file(FILE_PATH);
    if (*(fd + 3) < 0)
    {
        printf("fail to open file errno:%d\n", *(fd + 3));
        return -1;
    }

    *(fd + 1) = (int *)hdl;
    *(fd + 2) = (int *)0x1;   // thread runing flag
    *(fd + 4) = 0;  /* frame_no */
    set_user_data(hdl, (void *)fd);
    
    talk_handle_ref(hdl);
    ret = pthread_create(&thid, NULL, &send_thread, (void *)fd);
    if (ret)
    {
        talk_handle_unref(hdl);
        printf("fail to create pthread errno:%d\n", ret);
        return -1;
    }
    else
    {//@不能在这里给线程的参数赋值，因为线程有时直接就启动了，这时不没有来得及赋值
        *(fd + 0) = (int *)thid;        
        printf("succeed to create thread id:%p arg:%p\n", thid, fd);
    }
    
    return 0;
}

int test_close(talk_handle_t *hdl)
{
    int *fd = (int *)get_user_data(hdl);
    printf("@@@@@ test_close hdl: %p user_data: %p @@@@@\n", hdl, fd);

    *(fd + 2) = 0x0;
    return 0;
}

int test_recv(talk_handle_t *hdl, frame_t *frm)
{
    //printf("@@@@@ test_recv hdl: %p @@@@@\n", hdl);
    return 0;
}

static talk_ops_t ops = 
{
    .open  = test_open,
    .close = test_close,
    .recv  = test_recv
};

int main(int argc, char*argv[])
{
    int ret;
    
    talk_hdl *s = talk_server_new();
    register_talk_ops(&ops);
    ret = talk_server_start(s, TALK_PORT);
    if (ret < 0)
    {
        printf("fail talk_server_start err:%d\n", ret);
        talk_server_free(s);
        return -1;
    }

    for (;;)
    {
        sleep(5);
    }
    
    talk_server_free(s);
    
    return 0;
}
