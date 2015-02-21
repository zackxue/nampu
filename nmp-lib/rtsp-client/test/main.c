#include <stdio.h>
#include <glib.h>
#include "rtsp_client.h"
#include "standard_frame.h"

#define MAX_S		128

static void exception_callback(void *u, gint why)
{
	printf("exception_callback()!!!!!!!!!!\n");
}

struct session_block
{
	gint i;
	FILE *fp;
	RTSP_client_session *s;
};


struct session_block blocks[MAX_S];


static void data_callback(void *u, RTP_Frame *fr, RTP_Cnf *rc, RTP_Ext *re)
{
	struct session_block *sb = (struct session_block *)u;

	if (rc)
	{
		gchar *file_name = g_strdup_printf("/home/fangy/hik/test-%04d.hik", sb - &blocks[0]);
		sb->fp = fopen(file_name, "w");
		if (!sb->fp)
		{
			printf("open file failed!\n");
			exit(1);
		}
		g_free(file_name);
	}

	if (fr)
	{
		fwrite(fr->data + sizeof(standard_frame_t), fr->len - sizeof(standard_frame_t), 1, sb->fp);
	}
}


int main(int argc, char *argv[])
{
	RTSP_client_session *s;
	gint ret;

	g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);

	g_type_init();
	g_thread_init(NULL);

	rtsp_client_init();

	gint i = 0;

	while (i < MAX_S)
	{
		s = rtsp_client_create_session();
		if (!s)
		{
			printf("create session failed!!\n");
			return -1;
		}

		rtsp_client_set_callback(s, exception_callback, data_callback, &blocks[i]);

		ret = rtsp_client_open_session(s, "rtsp://192.168.1.9:554/dev=@HIK-DVR-88888888/media=0/channel=15&level=0", L4_TCP);
		if (ret)
		{
			printf("open session failed, err:%d\n", ret);
			rtsp_client_close_session(s);
			return -1;
		}
	
		ret = rtsp_client_play_session(s, 0, 0);
		if (ret)
		{
			printf("play session failed, err:%d\n", ret);
			rtsp_client_close_session(s);
			return -1;		
		}

		blocks[i].s = s;
		blocks[i].i = i;
	
		++i;
		usleep(100*1000);
	}

	while (1)
	{
		sleep(1);
	}

	return 0;
}

//:~ End
