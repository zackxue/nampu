#ifndef __USR_CALLBACK_H__
#define __USR_CALLBACK_H__

#include "rtsp_client.h"

typedef struct _Usr_Callback Usr_Callback;
struct _Usr_Callback
{
	gint			mode;
	void			*u;
	Exp_Callback	exp_cb;
	Data_Callback	dat_cb;
};

#endif 	/* __USR_CALLBACK_H__ */
