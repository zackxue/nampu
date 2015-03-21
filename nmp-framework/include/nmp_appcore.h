#ifndef __NMP_APP_CORE_H__
#define __NMP_APP_CORE_H__

#include "nmp_appobj.h"
#include "nmp_msgbus.h"
#include "nmp_appmod.h"

G_BEGIN_DECLS

#define NMP_TYPE_APPCORE	(nmp_app_core_get_type())
#define NMP_IS_APPCORE(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_APPCORE))
#define NMP_IS_APPCORE_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_APPCORE))
#define NMP_APPCORE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_APPCORE, NmpAppCore))
#define NMP_APPCORE_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_APPCORE, NmpAppCoreClass))
#define NMP_APPCORE_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_APPCORE, NmpAppCoreClass))

typedef void *NmpAfxInst;

typedef struct _NmpAppCore NmpAppCore;
typedef struct _NmpAppCoreClass NmpAppCoreClass;

struct _NmpAppCore
{
	NmpAppObj parent_object;

	NmpMsgBus *p_bus;
};


struct _NmpAppCoreClass
{
	NmpAppObjClass parent_class;
};


GType nmp_app_core_get_type( void );

void nmp_afx_core_init( void );
void nmp_afx_mod_insert(NmpBusSlotPos slot, NmpAppMod *mod);
void nmp_afx_bus_bypass( void );

NmpAfxInst nmp_afx_inst_init( void );
gint nmp_afx_inst_insert_mod(NmpAfxInst inst, NmpBusSlotPos slot, NmpAppMod *mod);
void nmp_afx_inst_bus_bypass(NmpAfxInst inst);

G_BEGIN_DECLS

#endif	//__NMP_APP_CORE_H__
