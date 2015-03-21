/*
 * nmp_mod_access.h
 *
 * A mod who has the ability to access network.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_MOD_ACCESS_H__
#define __NMP_MOD_ACCESS_H__

#include "nmp_appmod.h"
#include "nmp_net.h"

G_BEGIN_DECLS

#define NMP_TYPE_MODACCESS	(nmp_mod_acc_get_type())
#define NMP_IS_MODACCESS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODACCESS))
#define NMP_IS_MODACCESS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODACCESS))
#define NMP_MODACCESS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODACCESS, NmpModAccess))
#define NMP_MODACCESS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODACCESS, NmpModAccessClass))
#define NMP_MODACCESS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODACCESS, NmpModAccessClass))

typedef struct _NmpModAccess NmpModAccess;
typedef struct _NmpModAccessClass NmpModAccessClass;

struct _NmpModAccess
{
	NmpAppMod parent_object;

	NmpNet *net;
};


struct _NmpModAccessClass
{
	NmpAppModClass parent_class;

	gint (*io_init)(NmpModAccess *self, NmpNetIO *io);
	void (*io_close)(NmpModAccess *self, NmpNetIO *io, gint err);
};


GType nmp_mod_acc_get_type( void );

gint nmp_mod_acc_init_net(NmpModAccess *self, NmpPacketProto *pl, NmpPayloadProto *ph);

gint nmp_mod_acc_init_net_full(NmpModAccess *self, guint nloop, gboolean gather,
	NmpPacketProto *pl, NmpPayloadProto *ph);

NmpNetIO *nmp_mod_acc_create_listen_io(NmpModAccess *self, struct sockaddr *sa, 
	gint *err);

void nmp_mod_acc_release_io(NmpModAccess *self, NmpNetIO *io);


G_END_DECLS

#endif	//__NMP_MOD_ACCESS_H__
