#ifndef __NMP_SYSCTL_H__
#define __NMP_SYSCTL_H__

#include <glib.h>

typedef enum
{
	SC_BASE_PORT,
	SC_MAX_LOG_SIZE,
	SC_LOG_PATH,
	SC_MDS_ID,
	SC_CMS_HOST,
	SC_TIMEZONE
}NmpSCID;

void nmp_sysctl_init( void );
void *nmp_get_sysctl_value(NmpSCID id);

#endif	//__NMP_SYSCTL_H__
