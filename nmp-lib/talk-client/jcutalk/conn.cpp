
#include <winsock2.h>
#include "conn.h"

static int socket_set_noblock(int fd)
{
#ifdef WIN32
	int bNoBlock = 1;

	if (ioctlsocket(fd, FIONBIO, (unsigned long *)&bNoBlock) < 0 )
		return -1;
#else
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) 
		return -1;
#endif
	return 0;
}

conn_t *conn_new(void *ctx)
{
	int snd_buf_size = MAX_SNDBUF;
	conn_t *conn = (conn_t*)calloc(1, sizeof(conn_t));
	conn->timeout = DEFAULT_TIMEOUT;
	conn->ctx = ctx;
	conn->fd = socket(AF_INET, SOCK_STREAM, 0);
	if(conn->fd < 0)
	{
		goto _error;
	}
	if(socket_set_noblock(conn->fd) < 0)
	{
		goto _error;
	}


	setsockopt(conn->fd, SOL_SOCKET, SO_SNDBUF
		, (char *)&snd_buf_size, sizeof(snd_buf_size));

	//setsockopt(conn->fd, SOL_SOCKET, SO_RCVBUF
	//	, (char *)&snd_buf_size, sizeof(snd_buf_size));

	return conn;

_error:

	if(conn->fd > 0) 
	{
#ifdef WIN32
		closesocket(conn->fd);	
#else
		close(conn->fd);
#endif
		conn->fd = -1;
	}
	if(conn)  free(conn);
	return NULL;
}

int conn_del(conn_t *conn)
{
	if(conn)
	{
		if(conn->fd > 0) 
		{
#ifdef WIN32
			closesocket(conn->fd);	
#else
			close(conn->fd);
#endif
			conn->fd = -1;
		}
		free(conn);
		conn = NULL;
	}
	return 0;
}

