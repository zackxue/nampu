#include <stdio.h>
#include "rtsp-server.h"
#include "standard_frame.h"
#include "stream_api.h"


gint s_init(gchar *device, gint type, gint channel, gint level, gchar *pri, gchar **media_info, gsize *gsize)
{
	printf("===============> s_init()\n");
	*media_info = g_malloc(40);
	*gsize = 40;
	return 0;
}


gpointer stream_opened = 0;

gint s_open(gpointer stream, gchar *device, gint type, gint channel, gint level, gchar *pri)
{
	printf("=====> s_open(), %d, %d\n", channel, level);

	if (!stream_opened)
		stream_opened = stream;
	return 0;
}

void s_close(gpointer stream)
{
	printf("=====> s_close()\n");
	stream_opened = 0;
}


Stream_operation so =
{
	.init = s_init,
	.open = s_open,
	.close = s_close
};



Reverse_connector *rctor;

void conn_cb(Reverse_connector *cntr, gint err, gpointer user_data)
{
	if (err != E_CONN_OK)
	{
		rtsp_server_release_reverse_connector((RTSP_server*)user_data, cntr);
		rctor = 0;
		printf("########################################################conn_cb, err:%d\n", err);
	}
	else
	{
		printf("########################################################Connect MDS OK!!!\n");
	}
}


int main(int argc, char *argv[])
{
	g_type_init();
	g_thread_init(NULL);

	RTSP_server *server = rtsp_server_new();

	rtsp_server_set_port(server, 6553);
	rtsp_server_bind_port(server);


	while (1)
	{

		if (!rctor)
		{
			rctor = rtsp_server_reverse_connect(server, "192.168.1.163", 10000, "JXJ-IPC-87654321", 5, 1, conn_cb, server);		
		}

		sleep (3);
	}

	return 0;
}
