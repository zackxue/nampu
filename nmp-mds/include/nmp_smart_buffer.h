/* *
 * This file is part of Media Dispath Server(MDS).
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __NMP_SMART_BUFFER_H__
#define __NMP_SMART_BUFFER_H__

#include <glib.h>

#define SMART_BUFFER_SIZE		(16*1024)

typedef struct _JpfSmartBuffer JpfSmartBuffer;
struct _JpfSmartBuffer
{
	gchar	buffer[SMART_BUFFER_SIZE];	/* buffer for recving/sending packet */
	gint	offset;		/* last recv/send mark */
	gint	bytes;		/* bytes in buffer */		
};

#endif	//__NMP_SMART_BUFFER_H__
