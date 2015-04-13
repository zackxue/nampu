/*
 * nmp_netbuf.h
 *
 * This file describes net ring buffer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
*/

#ifndef __NMP_NETBUF_H__
#define __NMP_NETBUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE				4096
#define MAX_IO_BUFFER_SIZE		(16*PAGE_SIZE)    //64K

typedef struct _nmp_net_buf nmp_net_buf_t;

typedef int (*nmp_net_buf_flush_func)(
	char *buf, size_t count, void *user_data);

nmp_net_buf_t *nmp_net_buf_alloc(int n_blocks, nmp_net_buf_flush_func flush);
void nmp_net_buf_free(nmp_net_buf_t *buff);

int nmp_net_buf_write(nmp_net_buf_t *buff, char *buf, size_t count,
	void *user_data, int *pending);

int nmp_net_buf_flush(nmp_net_buf_t *buff, void *user_data);

#ifdef __cplusplus
}
#endif

#endif  //__NMP_NETBUF_H__
