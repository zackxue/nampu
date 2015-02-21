#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tr_avs.h"
#include "file.h"
#include "h264_filter.h"
#include "nalu.h"


#define PORT		6001

#define TEST_FILE_PATH "VideoEnc02.h264"

//历史流接口测试

int32_t hs_test_probe(int32_t channel, int32_t level, int32_t type,       
    	uint8_t *start_time, uint8_t *end_time, uint8_t *property,          /* 时间格式:YYYYMMDDHHMMSS */
    	media_info_t *mi)
{
	char frame[512*1024];
	int frame_type, frame_size = 0;

	int fd = file_open(TEST_FILE_PATH);
	if (fd < 0)
	{
		printf("################file_open failed!\n");
		return -1;
	}

    mi->begin_time = 0;
    mi->end_time   = 100000;
	frame_size = file_read(fd, frame, sizeof(frame), &frame_type);
	mi->mask = MEDIA_VIDEO;
	mi->video.enc_type = ENC_H264;
	mi->video.bitrate = 8000;
	mi->video.samples_per_sec = SAMPLE_90K;
	mi->video.frame_rate = 25;
	nalu_get_sps_pps(frame, frame_size, &mi->video.pic_parm);
	file_close(fd);

	return 0;
}


int32_t hs_test_open(avs_media *m, int32_t channel, int32_t level,
        int32_t type, uint8_t *start_time, uint8_t *end_time, uint8_t *property)
{
	int *fd = malloc(3*sizeof(int));

	*fd = file_open(TEST_FILE_PATH);
	if (*fd < 0)
	{
		printf("################file_open failed!\n");
		return -1;		
	}

	*(fd + 1) = 0;
	*(fd + 2) = 0;
	avs_media_set_u(m, fd);

	return 0;
}


int32_t hs_test_play(avs_media *m)
{
	return 0;
}


void hs_test_close(avs_media *m)
{
	int *fd = avs_media_get_u(m);
	file_close(*fd);
	free(fd);
	avs_media_set_u(m, NULL);
}

static uint32_t utc_time_count = 0;

int32_t hs_test_pause(avs_media *m)
{
    printf("hs_test_pause.\n");
    return 0;
}

int32_t hs_test_lseek(avs_media *m, uint32_t ts)
{
    printf("hs_test_lseek. ts:%d\n", ts);

    utc_time_count = ts;
    return 0;
}


int32_t hs_test_pull(avs_media *m, frame_t **frm)
{
#define FRAME_SIZE 512*1024
	int *fd = avs_media_get_u(m);
    int frame_size = 0;
    int ft;
    frame_t *fr;
    nal_desc_t nd;
    nd.nal_num = 0;

	fr = avs_alloc_frame(FRAME_SIZE, 0);
	if (!fr)
		return -1;

	for (;;)
	{
	    frame_size = file_read(*fd, (char*)fr->data, FRAME_SIZE, &ft);
	    if(frame_size < 0)
	    {
	    	printf("###############file_read() failed!\n");
	        continue;
	    }

	    break;
	}

    fr->hdr.no = (*(fd + 1))++;
    fr->hdr.enc_type = ENC_H264;
    fr->hdr.timestamp = (*(fd+2) += 40); //ms
    fr->hdr.type = ft;
    fr->hdr.v.width = 0;
    fr->hdr.v.height = 0;
    fr->hdr.v.utc_time = utc_time_count++;//time(NULL);
    fr->hdr.size = frame_size;
	*frm = fr;
	return 0;	
}


static hs_avs_ops hs_test_ops = 
{
	.probe	= hs_test_probe,
	.open	= hs_test_open,
	.play	= hs_test_play,
	.pause  = hs_test_pause,
    .lseek  = hs_test_lseek,
    .close  = hs_test_close,
	.pull   = hs_test_pull
};


static void jpf_exp(void *u, int32_t err)
{
	printf("jpf_exp():err '%d'\n", err);
}


int main(int argc, char *argv[])
{
	int err;

    avs_log_set_verbose(10);
    avs_init();
	avs_register_ops(NULL, &hs_test_ops);
	if (avs_start(PORT))
	{
		printf("avs_start() failed!!\n");
		sleep(1);
		exit(1);
	}
#if 0
	err = avs_start_pf_service((uint8_t*)"192.168.1.163", 10000, 1,
    	(uint8_t*)"JXJ-DVR-99999999", 0, 10, jpf_exp, NULL);
	if (err)
	{
		printf("avs_start_pf_service() failed!!\n");
		exit(1);
	}
#endif
	while (1)
	{
		sleep(1);
	}

	return 0;
}


//:~ End
