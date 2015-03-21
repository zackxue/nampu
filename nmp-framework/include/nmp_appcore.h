#ifndef __NMP_APP_CORE_H__
#define __NMP_APP_CORE_H__

#include "nmp_appobj.h"
#include "nmp_msgbus.h"
#include "nmp_appmod.h"

G_BEGIN_DECLS

#define NMP_TYPE_APPCORE	(nmp_app_core_get_type())
#define NMP_IS_APPCORE(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_APPCORE))
#define NMP_IS_APPCORE_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_APPCORE))
#define NMP_APPCORE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_APPCORE, JpfAppCore))
#define NMP_APPCORE_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_APPCORE, JpfAppCoreClass))
#define NMP_APPCORE_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_APPCORE, JpfAppCoreClass))

typedef void *JpfAfxInst;

typedef struct _JpfAppCore JpfAppCore;
typedef struct _JpfAppCoreClass JpfAppCoreClass;

struct _JpfAppCore
{
	NmpAppObj parent_object;

	JpfMsgBus *p_bus;
};


struct _JpfAppCoreClass
{
	NmpAppObjClass parent_class;
};


GType nmp_app_core_get_type( void );

void jpf_afx_core_init( void );
void jpf_afx_mod_insert(JpfBusSlotPos slot, NmpAppMod *mod);
void jpf_afx_bus_bypass( void );

JpfAfxInst jpf_afx_inst_init( void );
gint jpf_afx_inst_insert_mod(JpfAfxInst inst, JpfBusSlotPos slot, NmpAppMod *mod);
void jpf_afx_inst_bus_bypass(JpfAfxInst inst);

G_BEGIN_DECLS

#endif	//__NMP_APP_CORE_H__
