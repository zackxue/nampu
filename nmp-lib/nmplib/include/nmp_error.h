#ifndef __NMP_ERROR_H__
#define __NMP_ERROR_H__

#include <errno.h>

typedef struct _nmp_error nmp_error_t;
struct _nmp_error
{
	int	err_code;
	const char* err_msg;
};

#endif	/* __NMP_ERROR_H__ */
