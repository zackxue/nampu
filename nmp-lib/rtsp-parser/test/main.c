#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>
   #include <sys/select.h>
#include <pthread.h>
       /* According to earlier standards */
       #include <sys/time.h>
       #include <sys/types.h>
       #include <unistd.h>
#include "rtspparse.h"



/* ·þÎñ¶Ë*/

#define SERVER_PORT		17000

void* rtsp (void *arg)
{
	GstRTSPResult res;
	GstRTSPMessage *msg;
	int fd = (int)arg;

	GstRtspParser *parser = gst_rtsp_parser_new(4096, NULL);

	while (1)
	{
		fd_set	rset;
		FD_ZERO(&rset);
		FD_SET(fd, &rset);
		if (select (fd +1, &rset, NULL, NULL, NULL) < 0)
		{
			perror("select()");
			exit(1);
		};
		
		res = gst_rtsp_parser_recv(parser, fd, &msg, NULL);
		if (res == GST_RTSP_OK)
		{
			gst_rtsp_message_dump(msg);
		}
		else if (res == GST_RTSP_EEOF)
		{
			printf("################## connection reset!!\n");
			close(fd);
			gst_rtsp_parser_free(parser);
			break;
		}
		else if (res == GST_RTSP_EINTR)
		{
			printf("================= eintr!!\n");
		}
		else
		{
			printf("$$$$$$$$$$$$$$$other error!!!\n");
		}
	}

	return NULL;
}


void start_server( void )
{
	int lis_sock, cli_sock;
	struct sockaddr_in sin;

	lis_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (lis_sock < 0)
	{
		perror("socket()");
		exit(1);
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(lis_sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		perror("bind()");
		exit(1);
	}

	listen(lis_sock, 5);

	while (1)
	{
		cli_sock = accept(lis_sock, NULL, NULL);
		if (cli_sock < 0)
		{
			perror("accept()");
			exit(1);
		}

		pthread_t id;
		pthread_create(&id, NULL, rtsp, (void*)cli_sock);
	}

	close(lis_sock);
}

int main(int argc, char *argv[])
{
	start_server();
	return 0;
}



