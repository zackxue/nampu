#ifndef __TALK_SERVER_H__
#define __TALK_SERVER_H__

#include "talk_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void talk_hdl;

talk_hdl* talk_server_new(); 
int talk_server_start(talk_hdl *s, int port);
void talk_server_free(talk_hdl *s);

#ifdef __cplusplus
}
#endif

#endif
