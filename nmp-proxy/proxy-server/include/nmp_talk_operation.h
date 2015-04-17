
#ifndef __TALK_OPERATION_H__
#define __TALK_OPERATION_H__

#include "talk_api.h"
#include "nmp_proxy_info.h"

#define MAX_ENCODE_BUFFER_LEN       (3 * 1024)

typedef struct talk_info talk_info_t;

struct talk_info
{
    int handle;
    int channel;
    long talk_handle;
	void	*init_g722_encoder;
	void 	*init_g722_decoder;

    frame_t *frm;
    talk_handle_t *hdl;

    char media_info[DEF_MAX_MEDIA_SIZE];
    size_t media_size;

    void *pri_data;
    double length;

    unsigned long ts;           //Ê±¼ä´Á
    stream_state_t state;

    struct service *sdk_srv;        /* pointer back to service */
};

typedef struct talk_list talk_list_t;

struct talk_list
{
    nmp_list_t *list;
};

typedef struct talk_opt talk_opt_t;

struct talk_opt
{
    int (*talk_open)(struct service *srv, struct talk_info *talk, talk_handle_t *hdl, media_info_t *info);
    int (*talk_close)(struct service *srv, struct talk_info *talk);
    int (*talk_recv)(struct service *srv, struct talk_info *talk, char *data, int len, media_info_t *info);
};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
    }
#endif 

#endif //__TALK_OPERATION_H__
