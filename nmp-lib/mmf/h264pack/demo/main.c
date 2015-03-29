#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "rtp.h"
#include "file.h"

#include "h264_filter.h"
#include "h264_framer.h"
#include "h264pack.h"

//#define UDP_SEND
#define USE_UNPACK_FRM

typedef struct rtp_ch_s {
    uint16_t pt;
    uint16_t size;
}rtp_ch_t;

#if 1

extern int blk_mem_size(struct ctx_parm_s *parm
        , void *_uargs
        , int *data_s
        , int *hdr_s);
extern mem_t
    blk_mem_alloc(struct ctx_parm_s *parm
            , int data_s 
            , int hdr_s);
extern void  
    blk_mem_free(mem_t *mem);


int main_framer(void)
{
    int ret;
    int size;
    rtp_ch_t ch;
    char tmp_buf[16*1024];
    int in = open("./test.rtp", O_RDONLY);
    int out= open("./test.nal", O_CREAT|O_RDWR|O_TRUNC);
    
    assert((in > 0) && (out > 0));
    
    h264_frm_t *framer = h264_frm_init(512*1024);
    while(1)
    {
        ret = read(in, &ch, sizeof(rtp_ch_t));
        if(ret != sizeof(rtp_ch_t))
        {
            printf("err & eof:(%d) read in:%d, rtp_ch_t\n", errno, in);
            break;
        }
        if(ch.pt == 96 && ch.size <= sizeof(tmp_buf))
        {
            ret = read(in, tmp_buf, ch.size);
            if(ret != ch.size)
            {
                printf("err & eof: read in:%d, rtp_pkt_t\n", in);
                break;
            }
            frm_mem_t *o = NULL;

            if(h264_frm_fill(framer, (rtp_pkt_t*)tmp_buf, ch.size, &o) > 0)
            {
                
                frame_t *frame = (frame_t*)o->pdata(o, &size);

                printf(">>>>>> video frm_size:%d, frm_type:%d, nal_num:%d\n"
                        , frame->hdr.size
                        , frame->hdr.type
                        , frame->nal_desc->nal_num);

                ret = write(out, frame->data, frame->hdr.size);
                if(ret != frame->hdr.size)
                {
                    printf("err: write out:%d\n", out);
                }
                else
                {
                    ;
                }
                o->unref(o);
            }
        }
        else if(ch.size <= sizeof(tmp_buf))
        {
            ret = read(in, &tmp_buf, ch.size);
            continue;
        }
        else
        {
            printf("err & eof: ch.size:%d, too max.\n", ch.size);
            break;
        }
    }
}


/* unpack */
int main_unpack(void)
{
    int ret = 0, c = 0;
    rtp_ch_t ch;
    int rtp_ext_len;
    node_t nal;
    node_t pkt;
    rtp_pkt_t hdr;
    char tmp_buf[16*1024];

    int in = open("./test.rtp", O_RDONLY);
    int out= open("./test.nal", O_CREAT|O_RDWR|O_TRUNC);
    
    assert((in > 0) && (out > 0));

    ctx_parm_t parm;
    parm.udata0= buf_pool_new(4, 512*1024, 0/*BUF_FLAG_GROWTH */);
    parm.udata1= 512*1024;
    parm._size = blk_mem_size;
    parm._alloc= blk_mem_alloc;
    parm._free = blk_mem_free;

    h264_unpack_ctx_t* unpack = h264_unpack_new(&parm); 


    while(1)
    {
        ret = read(in, &ch, sizeof(rtp_ch_t));
        if(ret != sizeof(rtp_ch_t))
        {
            printf("err & eof:(%d) read in:%d, rtp_ch_t\n", errno, in);
            break;
        }
#if 0
        printf("ch.pt  :%d\n", ch.pt);
        printf("ch.size:%d\n", ch.size);
#endif
        if(ch.pt == 96 && ch.size <= sizeof(tmp_buf))
        {
            //video;
            rtp_ext_len = 0;
            
            ret = read(in, &hdr, sizeof(rtp_pkt_t));
            if(ret != sizeof(rtp_pkt_t))
            {
                printf("err & eof: read in:%d, rtp_pkt_t\n", in);
                break;
            }
#if 0
            printf("hdr.version     :%d\n", hdr.version);
            printf("hdr.extension   :%d\n", hdr.extension);
            printf("hdr.padding     :%d\n", hdr.padding);
            printf("hdr.seq         :%d\n", ntohs(hdr.seq));
            printf("hdr.csrc_count  :%d\n", hdr.csrc_count);
#endif      
            if(hdr.extension)
            {
                int ext;
                int ext_profile, ext_length;
                ret = read(in, &ext, sizeof(int));
                if(ret != sizeof(int))
                {
                    printf("err & eof: read in:%d, ext\n", in);
                    break;
                }
#if 0
                printf("ext:0x%08x, profile:%d, length:%d "
                        , ext
                        , ext_profile = ntohs(ext&0xffff)
                        , ext_length  = ntohs((ext>>16)&0xffff));
#else
                ext_profile = ntohs(ext&0xffff);
                ext_length  = ntohs((ext>>16)&0xffff);
#endif      
                //rtp_ext_len = (ext_profile/8 * ext_length);
                rtp_ext_len = (4 * ext_length);

                j_rtp_av_ext_t *j_ext = (j_rtp_av_ext_t*)tmp_buf;

                ret = read(in, tmp_buf, rtp_ext_len);
                if(ret != rtp_ext_len)
                {
                    printf("err & eof: read in:%d, rtp extension\n", in);
                    break;
                }
                rtp_ext_len += sizeof(int);
            
                printf("frm_size:%d\n", ntohl(j_ext->v.frame_length));
            }

            //printf("ch.size:%d, rtp_ext_len:%d\n", ch.size, rtp_ext_len);
            pkt.times = ntohl(hdr.timestamp);
            pkt.mem.hdr_s = 0;
            pkt.mem.data  = tmp_buf+pkt.mem.hdr_s;
            pkt.mem.data_s= read(in, pkt.mem.data, (ch.size-sizeof(rtp_pkt_t)-rtp_ext_len));
            if(pkt.mem.data_s != (ch.size-sizeof(rtp_pkt_t)-rtp_ext_len))
            {
                printf("err & eof: read in:%d, pt\n", in);
                break;
            }
        }
        else if(ch.size <= sizeof(tmp_buf))
        {
            ret = read(in, &tmp_buf, ch.size);
            continue;
        }
        else
        {
            printf("err & eof: ch.size:%d, too max.\n", ch.size);
            break;
        }


        if(hdr.payload_type == 96)
        {
#ifdef USE_UNPACK_FRM
            //printf("rtp: read in:%d, rtp_pt size:%d\n", in, pkt.mem.data_s);

            if(h264_unpack_frm(unpack, &pkt, &nal, FRM_T) > 0)
            {
#if 0
                printf("nal.mem.hdr_s :%d\n", nal.mem.hdr_s);
                printf("nal.mem.data_s:%d\n", nal.mem.data_s);
                printf("nal.mem.data  :0x%x\n", nal.mem.data);
#endif       
                c += (mem_data_s(&nal.mem) - NAL_INFO_SIZE);
                printf(">>>>>> video sum frm_size:%d, frm_type:%d, nal_num:%d\n"
                        , c
                        , nal.type
                        , *((int*)(mem_data(&nal.mem)+NAL_OFF_NUM)));
                c = 0;

                ret = write(out, mem_data(&nal.mem)+NAL_INFO_SIZE, mem_data_s(&nal.mem) - NAL_INFO_SIZE);
                if(ret != (mem_data_s(&nal.mem) - NAL_INFO_SIZE))
                {
                    printf("err: write out:%d\n", out);
                }
                else
                {
                    //printf("nal: write out:%d, nal_size:%d\n", out, mem_size(&nal.mem));
                }
                blk_mem_free(&nal.mem);
            }
#else
            //printf("rtp: read in:%d, rtp_pt size:%d\n", in, pkt.mem.data_s);
            if(h264_unpack_nal(unpack, &pkt, &nal, NAL_T) > 0)
            {
#if 0
                printf("nal.mem.hdr_s :%d\n", nal.mem.hdr_s);
                printf("nal.mem.data_s:%d\n", nal.mem.data_s);
                printf("nal.mem.data  :0x%x\n", nal.mem.data);
#endif       
                c += mem_size(&nal.mem);
                if(hdr.marker)
                {
                    printf(">>>>>> video sum frm_size:%d\n", c);
                    c = 0;
                }
               

                *((int*)mem_hdr(&nal.mem)) = 0x01000000;
                
                
                ret = write(out, mem_hdr(&nal.mem), mem_size(&nal.mem));
                if(ret != mem_size(&nal.mem))
                {
                    printf("err: write out:%d\n", out);
                }
                else
                {
                    //printf("nal: write out:%d, nal_size:%d\n", out, mem_size(&nal.mem));
                }
                blk_mem_free(&nal.mem);
            }
#endif
        }
        else
        {
                    printf(">>>>>> other sum frm_size:%d\n", pkt.mem.data_s);
        }
    }
   
    h264_unpack_del(unpack);

    if(parm.udata0)buf_pool_unref(parm.udata0);
    close(in);
    close(out);

    return 0;
}
#endif

/* pack */
#if 1
int socket_create(void)
{
#ifdef UDP_SEND
    int sock_fd = -1;
    int sock_resuse = 1; 

    if ((sock_fd= socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("error: sock_fd socket create, (%m)\n");
        goto __error;
    }

    int ret, snd_buf_size = 200*1024;
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (char *)&snd_buf_size, sizeof(snd_buf_size));

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sock_resuse, sizeof(int)) < 0)
    {
        printf("error: fd setsockopt SO_REUSEADDR, (%m)\n");
        goto __error;
    }
    return sock_fd;
#else
    int out = open("./test.rtp", O_RDWR|O_CREAT|O_TRUNC);
    printf("out:%d\n", out);
    return out;
#endif
__error:
    return -1;
}

int socket_write(int fd, char *buf, int size, unsigned short av_type)
{
#ifdef UDP_SEND
    int ret;
    struct sockaddr_in dst_addr;
    socklen_t addrlen = sizeof(dst_addr);
    
    
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(av_type);
    inet_aton("192.168.1.151", &dst_addr.sin_addr);

    if((ret=sendto(fd, buf, size, 0, (struct sockaddr *)&dst_addr, addrlen)) != size)
    {
        printf("error: sendto ret:%d, size:%d\n", ret, size);
    }
    return ret; 
#else
    int ret;
    rtp_ch_t *ch = (rtp_ch_t*)(buf-4);
    ch->pt   = (av_type == 2222)?96:8;
    ch->size = size;
    //printf("write file size:%d\n", size);
    if((ret=write(fd, ch, size+sizeof(rtp_ch_t))) != (size+sizeof(rtp_ch_t)))
    {
        printf("error: write rtp, ret:%d, size:%d\n", ret, size+sizeof(rtp_ch_t));
    }
    fsync(fd);
#endif
}

#include <time.h>





//#define USE_ADD_TIME    1


#include "list.h"

typedef struct frm_list_s {
    unsigned int count;
    unsigned int num;
    struct list_head head;
    pthread_mutex_t  mutex;
}frm_list_t;

typedef struct frm_node_s {
   struct list_head head;
   void *data;
}frm_node_t;


frm_list_t*
    frm_list_new(int num)
{
    frm_list_t *l = (frm_list_t*)calloc(1, sizeof(frm_list_t));

    l->count = 0;
    l->num = num;
    pthread_mutex_init(&l->mutex, NULL);
    return l;
}

int frm_list_full(frm_list_t *l)
{
    return (l->count == l->num);
}
int frm_list_empty(frm_list_t *l)
{
    return (l->count == 0);
}
int frm_list_add_tail(frm_list_t *l, void *data)
{
    
}
void *frm_list_del_head(frm_list_t *l)
{

}


int main_readfile(void *parm)
{

}

int main_pack(void *parm)
{
    int i, j, k, ch;
      
     int fr_type = 0;
    char frame[512*1024];
    int  frame_size = 0;
    frame_t frm;
    nal_desc_t  nal_desc;

    int fd = file_open("VideoEnc02.h264");
    assert(fd > 0);

    /*int vfd = file_open("dvr.h264");
    int afd = file_open("dvr.h264");
    f3520_hdr_skip(vfd);
    f3520_hdr_skip(afd);*/

    int vbasetimes = 123456; 
    int abasetimes = 654321;
    int vfrmno = 0;
    int afrmno = 0;
    struct timespec ts = {0, 0};
    unsigned int vptimestamp = 0;
    unsigned int aptimestamp = 0;

    unsigned int clock_time1 = 0;
    unsigned int clock_time2 = 0;
    int delay_time = 0;

    int udp_fd = socket_create();
    
    assert(/*(vfd > 0) && (afd > 0) && */(udp_fd > 0));

    h264_fl_op_t *f = h264_fl_op_init(1600);

    for(i = 0; i < 1/*00*10000*/; i++)
    {
		clock_gettime(CLOCK_MONOTONIC, &ts);
        clock_time1 = ts.tv_sec*1000 + ts.tv_nsec/(1000*1000);
    
        for(ch = 0; ch < 2; ch++)
        {
       
        frame_size = file_read(fd, frame, sizeof(frame), &fr_type);
        /*if(ch == 0)
            frame_size = f3520_read_a(afd, frame, sizeof(frame), &fr_type);
        else
            frame_size = f3520_read_v(vfd, frame, sizeof(frame), &fr_type);*/
        
        if(fr_type != FRAME_A)
        {    
            frm.hdr.no        = vfrmno++;
            frm.hdr.enc_type  = ENC_H264;
            #ifdef USE_ADD_TIME
            frm.hdr.timestamp = (vbasetimes += 40); //ms
            #else
            frm.hdr.timestamp = clock_time1;
            #endif
            //printf(">>>>>>>>>>>> video d:%u\n", frm.hdr.timestamp - vptimestamp);
            vptimestamp = frm.hdr.timestamp;
            frm.hdr.type      = fr_type;
            frm.hdr.v.width   = 704;
            frm.hdr.v.height  = 576;
            frm.hdr.size      = frame_size;
            frm.data          = frame;
            frm.nal_desc      = &nal_desc;
            nal_desc.nal_num  = 0;
        }
        else /* FRAME_A */
        {
            frm.hdr.no        = afrmno++;
            frm.hdr.enc_type  = ENC_G711A;
            #ifdef USE_ADD_TIME
            frm.hdr.timestamp = (abasetimes += 40); //ms
            #else
            frm.hdr.timestamp = clock_time1;
            #endif
            //printf(">>>>>>>>>>>> audio d:%u\n", frm.hdr.timestamp - aptimestamp);
            aptimestamp = frm.hdr.timestamp;
            frm.hdr.type      = fr_type;
            frm.hdr.v.width   = 0;
            frm.hdr.v.height  = 0;
            frm.hdr.size      = frame_size;
            frm.data          = frame;
            frm.nal_desc      = &nal_desc;
            nal_desc.nal_num  = 0;            
        }

        h264_fl_op_fill(f, &frm, 0);

        int c = 0;
        int s = 0;
        while(1)
        {
            int err = 0, size = 0;
            rtp_mem_t *rtp;
            uint8_t *buf;
            
            if((err = h264_fl_op_pull(f, &rtp, &size)) == 0)
            {
                rtp_pkt_t *rtp_hdr = (rtp_pkt_t*)(rtp->pdata(rtp, &size));
#if 1
                printf("pull rtp => seq:%d, time:%d, marker:%d, pt:%d, ext:%d, size:%04d "
                        , ntohs(rtp_hdr->seq)
                        , ntohl(rtp_hdr->timestamp)
                        , rtp_hdr->marker
                        , rtp_hdr->payload_type
                        , rtp_hdr->extension
                        , size);

                if(rtp_hdr->extension)
                {
                    buf = (uint8_t*)rtp_hdr;
                    buf += 12+4;
                    j_rtp_av_ext_t *av_ext = (j_rtp_av_ext_t*)buf;
                    printf("frm_no:%d ", ntohl(av_ext->v.frame_num));
                }
                if(rtp_hdr->payload_type == 96)
                {
                    buf = (uint8_t*)rtp_hdr;
                    buf += ((rtp_hdr->extension)?(12+4+12):12);
                    uint8_t nal_type = (buf[0] & 0x1F);
    
                    printf("NALU[F:%d,NRI:%d,TYPE:%d] "
                            , (buf[0]>>7)
                            , (buf[0]>>5)&0x3
                            , (buf[0]&0x1F));
    
                    switch(nal_type)
                    {
                        case 28: case 29:          /* FU-A or FU-B */
                            printf("FU-A[S:%d,E:%d,R:%d,TYPE:%d] "
                                    , (buf[1]>>7)
                                    , (buf[1]>>6)&0x01
                                    , (buf[1]>>5)&0x01
                                    , (buf[1]&0x1F));     
                            break;
                        default:
                            printf("SIGNAL NAL ");
                           break;  
                    }
                }
                printf("\n");
#endif
                socket_write(udp_fd, rtp_hdr, size, (rtp_hdr->payload_type==96)?2222:2224);
#ifdef UDP_SEND
#if 0
                if(c==0)
                {
                printf("pull rtp => seq:%d, time:%d, marker:%d, ext:%d, size:%04d "
                        , ntohs(rtp_hdr->seq)
                        , ntohl(rtp_hdr->timestamp)
                        , rtp_hdr->marker
                        , rtp_hdr->extension
                        , size);

                printf("\n");
                }
#endif
               //if(c%6==0)usleep(1);
#endif
                c++;
                s+=size;
                rtp->unref(rtp);
            }
            else
            {
                if(err == -EAGAIN)
                {
                    //printf("pull rtp, empty.\n");
                }
                else
                {
                    printf("pull rtp, err(%d).\n", err);
                }
#if 0
                printf("pull => frame_no:%d, frame_type:%X, rtp_num:%d, frame_size:%d, rtp_sum_size:%d, count_size:%d\n"
                        , i, fr_type, c, frame_size, s, h264_fl_op_count(f, frame_size, fr_type));
#endif
                break;
            }
         }
        }

		clock_gettime(CLOCK_MONOTONIC, &ts);		// 需要链接时钟库: -lrt
        clock_time2 = ts.tv_sec*1000 + ts.tv_nsec/(1000*1000);
        
        if((delay_time = (40 - (clock_time2 - clock_time1))) > 0)
        {
            //printf(">>>>>>>>>>>>>>>>>> delay_time:%d\n", delay_time);
            usleep((delay_time*0.8)*1000);
        }
    }
   
    h264_fl_op_fin(f);

    printf("exit.\n");
    return 0;
}
#endif

int main(int argc, char *argv[])
{
    if(!strcmp(argv[1], "pack"))
    {
        /*pthread_t t0;
        pthread_t t1;
        pthread_create(&t0, NULL, main_pack, (void*)96);
        pthread_create(&t1, NULL, main_readfile, (void*)8);
        while(1) sleep(10);*/
        main_pack((void*)96);
    }
    else if (!strcmp(argv[1], "unpack"))
    {
        main_unpack();
    }
    else if (!strcmp(argv[1], "framer"))
    {
        main_framer();
    }

    return 0;
}



