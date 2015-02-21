
#ifndef __TALK_CLIENT_H__
#define __TALK_CLIENT_H__

#include "macros.h"
#include "media_struct.h"
#include "loop.h"
#include "fd_set.h "
#include "hi_talk_cli.h"

enum
{
	JXJ = 0x01,
	PROXY,
	PF
};

#define TYPE_JXJ "JXJ"
#define TYPE_HIK "HIK"
#define TYPE_DAH "DAH"

typedef struct _talk_client
{
	fd_node_t base;          
	media_info_t mi;      
	
	talk_parm_t parm;
	/* private */
	int dev_type;
	int ref_count;  
	int fd;
	void *u;              /* user data */
	POINTER_BACK_TO(loop_t)
}talk_client_t;

talk_client_t * client_new(const char *ip, int port, media_info_t *mi, void *ctx, talk_parm_t *ops, loop_t *loop);
int client_del(talk_client_t *cli);

talk_client_t *client_ref(talk_client_t *cli);
void client_unref(talk_client_t *cli);

int set_device_type(talk_client_t *cli, char *pu_id);
void set_media_info(talk_client_t *cli, media_info_t *info);

#endif
