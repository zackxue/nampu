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
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODACCESS, JpfModAccess))
#define NMP_MODACCESS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODACCESS, JpfModAccessClass))
#define NMP_MODACCESS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODACCESS, JpfModAccessClass))

typedef struct _JpfModAccess JpfModAccess;
typedef struct _JpfModAccessClass JpfModAccessClass;

struct _JpfModAccess
{
	NmpAppMod parent_object;

	JpfNet *net;
};


struct _JpfModAccessClass
{
	NmpAppModClass parent_class;

	gint (*io_init)(JpfModAccess *self, JpfNetIO *io);
	void (*io_close)(JpfModAccess *self, JpfNetIO *io, gint err);
};


GType nmp_mod_acc_get_type( void );

gint nmp_mod_acc_init_net(JpfModAccess *self, JpfPacketProto *pl, JpfPayloadProto *ph);

gint nmp_mod_acc_init_net_full(JpfModAccess *self, guint nloop, gboolean gather,
	JpfPacketProto *pl, JpfPayloadProto *ph);

JpfNetIO *nmp_mod_acc_create_listen_io(JpfModAccess *self, struct sockaddr *sa, 
	gint *err);

void nmp_mod_acc_release_io(JpfModAccess *self, JpfNetIO *io);


G_END_DECLS

#endif	//__NMP_MOD_ACCESS_H__
