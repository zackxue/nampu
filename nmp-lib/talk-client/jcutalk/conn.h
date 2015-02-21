
#ifndef __CONN_H__
#define __CONN_H__

#define DEFAULT_TIMEOUT 0//(5*1000)
#define MAX_SNDBUF  (100*1024)

typedef enum _CONN_STAT {
	CONN_CONNECTING   = 0x01
	, CONN_WAIT
	, CONN_RW
	, CONN_FIN
}CONN_STAT_E;

typedef struct conn_s {
	int  fd;
	int  timeout;
	int  stat;
	int  recv_stat;
	void *ctx;
}conn_t;

conn_t *conn_new(void *ctx);
int conn_del(conn_t *conn);

#endif