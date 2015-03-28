#ifndef __NMP_SYSCTL_H__
#define __NMP_SYSCTL_H__

#include <glib.h>


#define MAX_FILE_PATH			4096		/* sc_ */

#define SC_MAX_LOG_PATH			2048
#define SC_MAX_ID_LEN			64
#define SC_MAX_HOST_LEN			64
#define SC_MAX_STORAGE_TYPE		64
#define SC_TIMEZONE_LEN			16

#define SC_CONFIG_ENV_NAME		"MSS_CONFIG_PATH"
#define SC_DEFAULT_CONFIG_PATH	"/etc/"
#define SC_CONFIG_FILE_NAME		"nmp_server.conf"


typedef enum
{
	SC_BASE_PORT,
	SC_MAX_LOG_SIZE,
	SC_LOG_PATH,
	SC_MSS_ID,
	SC_CMS_HOST,
	SC_STORAGE_TYPE,
	SC_TIMEZONE
}NmpSCID;


void nmp_sysctl_init( void );
void *nmp_get_sysctl_value(NmpSCID id);

#endif	//__NMP_SYSCTL_H__
