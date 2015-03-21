#ifndef __NMP_PORT_H__
#define __NMP_PORT_H__

#include "nmp_sysctl.h"

/* CMS Macros Internal */
#define __BASE_PORT				(nmp_get_sys_parm_int(SYS_PARM_BASEPORT))
#define __CU_TO_CMS_OFFSET		0
#define __PU_TO_CMS_OFFSET		1
#define __MDU_TO_CMS_OFFSET		2
#define __MSU_TO_CMS_OFFSET		3
#define __BSS_TO_CMS_OFFSET		4
#define __CMS_TO_CMS_OFFSET		5		/* CMS CASCADE */
#define __TW_TO_CMS_OFFSET		6
#define __AMS_TO_CMS_OFFSET		7
#define __IVS_TO_CMS_OFFSET		8

/* CMS Listening Ports */
#define JPFCMS_CU_PORT		(__BASE_PORT + __CU_TO_CMS_OFFSET)
#define JPFCMS_PU_PORT		(__BASE_PORT + __PU_TO_CMS_OFFSET)
#define JPFCMS_MDU_PORT		(__BASE_PORT + __MDU_TO_CMS_OFFSET)
#define JPFCMS_MSS_PORT		(__BASE_PORT + __MSU_TO_CMS_OFFSET)
#define JPFCMS_BSS_PORT		(__BASE_PORT + __BSS_TO_CMS_OFFSET)
#define JPFCMS_CMS_PORT		(__BASE_PORT + __CMS_TO_CMS_OFFSET)
#define JPFCMS_TW_PORT			(__BASE_PORT + __TW_TO_CMS_OFFSET)
#define JPFCMS_AMS_PORT		(__BASE_PORT + __AMS_TO_CMS_OFFSET)
#define JPFCMS_IVS_PORT		(__BASE_PORT + __IVS_TO_CMS_OFFSET)

#endif	//__NMP_PORT_H__
