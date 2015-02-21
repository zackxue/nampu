
#ifndef __J_CU_TALK_H__
#define __J_CU_TALK_H__

#include "media_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILD_STATIC_LIB
	#define JCUTALK_API extern
#else
	
	#ifdef JCUTALK_API_EXPORT
	#define JCUTALK_API __declspec(dllexport)
	#else
	#define JCUTALK_API __declspec(dllimport)
	#endif

#endif

typedef void talk_hdl;

typedef struct _jcu_talk_parm 
{
	int (*recv)(talk_hdl *hdl, talk_frame_hdr_t *frm, void *ctx);
	int (*start)(talk_hdl *hdl, void *ctx);
	int (*stop)(talk_hdl *hdl, void *ctx);
}jcu_talk_parm_t; 

JCUTALK_API int jcu_talk_cli_init(jcu_talk_parm_t *parm);
JCUTALK_API int jcu_talk_cli_uninit();

JCUTALK_API talk_hdl*jcu_talk_cli_open(char *ip, unsigned short port, void *ctx, media_info_t *mi);                            
JCUTALK_API int jcu_talk_cli_close(talk_hdl *hdl);
JCUTALK_API int jcu_talk_cli_send(talk_hdl *hdl, talk_frame_hdr_t *frm);

JCUTALK_API void set_user_data(talk_hdl *hdl, void *u);
JCUTALK_API void* get_user_data(talk_hdl *hdl);

JCUTALK_API void talk_cli_ref(talk_hdl *hdl);
JCUTALK_API void talk_cli_unref(talk_hdl *hdl);

#ifdef __cplusplus
}
#endif

#endif