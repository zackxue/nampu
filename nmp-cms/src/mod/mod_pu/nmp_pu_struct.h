#ifndef __NMP_PU_H__
#define __NMP_PU_H__

#include "nmp_mods.h"
#include "nmp_res_ctl.h"
#include "nmp_shared.h"

#define TIMEOUT_N_PERIODS		3
#define DEFAULT_KEEPALIVE_SEC	3

G_BEGIN_DECLS

typedef enum
{
	STAT_PU_REGISTERING,	/* Register packet was sent, but no response yet */
	STAT_PU_ONLINE,			/* Register successfully */
	STAT_PU_OFFLINE			/* Keep alive timout */
}JpfPuState;


typedef struct _JpfPu JpfPu;
struct _JpfPu
{
	JpfGuestBase	guest_base;		/* Including PU ID */

	JpfPuType		type;			/* Type: DVR, IPC ... */
	JpfPuState		state;			/* PU State */
	guint			age;	/* heart beating counter  */

	gint				recheck;		/* to recheck if in database */

	JpfResourcesCtl	res_ctl;
};


G_END_DECLS

#endif	//__NMP_PU_H__
