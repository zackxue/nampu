#ifndef __J_ERROR_H__
#define __J_ERROR_H__

#include <errno.h>

typedef struct _JError
{
	int	err_code;
	const char* err_msg;
}JError;

#endif	/* __J_ERROR_H__ */
