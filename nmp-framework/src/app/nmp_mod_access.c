/*
 * nmp_mod_access.c
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_share_errno.h"
#include "nmp_share_debug.h"
#include "nmp_mod_access.h"


G_DEFINE_TYPE(NmpModAccess, nmp_mod_acc, NMP_TYPE_APPMOD);


static void 
nmp_mod_acc_io_fin(gpointer orig_mod, NmpNetIO *io, gint err)
{
    NmpModAccess *mod = (NmpModAccess*)orig_mod;
    BUG_ON(!mod);
    NMP_MODACCESS_GET_CLASS(mod)->io_close(mod, io, err);
}


static gint
nmp_mod_acc_io_init(gpointer orig_mod, NmpNetIO *io)
{
    NmpModAccess *mod = (NmpModAccess*)orig_mod;
    BUG_ON(!mod);
    return NMP_MODACCESS_GET_CLASS(mod)->io_init(mod, io);  
}


static void
nmp_mod_acc_on_io_close(NmpModAccess *self, NmpNetIO *io, gint err)
{
    nmp_warning("<NmpModAccess> io_close() called!");
}


static gint
nmp_mod_acc_on_io_init(NmpModAccess *self, NmpNetIO *io)
{
    nmp_warning("<NmpModAccess> io_init() called!");
    return -E_NOTSUPPORT;
}


NmpNetIO *
nmp_mod_acc_create_listen_io(NmpModAccess *self, struct sockaddr *sa, 
	gint *err)
{
    G_ASSERT(self != NULL);

    return nmp_net_create_listen_io_2(self->net, sa, err);
}


static gint
nmp_mod_acc_recv_sysmsg(gpointer priv_data, NmpNetIO *net_io, 
	gpointer msg)
{
	NmpModAccess *self;
	NmpSysMsg *sys_msg;
	G_ASSERT(priv_data != NULL);

	self = (NmpModAccess*)priv_data;
	sys_msg = NMP_SYSMSG(msg);
	nmp_sysmsg_attach_io(sys_msg, net_io);

	return nmp_app_mod_rcv_f(NMP_APPMOD(self), sys_msg);
}


static void
nmp_mod_acc_init(NmpModAccess *self)
{
	self->net = NULL;
}


gint
nmp_mod_acc_init_net(NmpModAccess *self, NmpPacketProto *pl, NmpPayloadProto *ph)
{
    self->net = nmp_net_new(pl, ph, self);
    if (G_UNLIKELY(!self->net))
    {
        nmp_error("<NmpModAccess> alloc NmpNet object failed!");
        FATAL_ERROR_EXIT;

		return -1;
    }

	nmp_net_set_reader(
		 self->net,
		 nmp_mod_acc_recv_sysmsg
	);

    nmp_net_set_funcs(
        self->net,
        nmp_mod_acc_io_init,
        nmp_mod_acc_io_fin
    );

	return 0;	
}


gint
nmp_mod_acc_init_net_full(NmpModAccess *self, guint nloop, gboolean gather,
	NmpPacketProto *pl, NmpPayloadProto *ph)
{
    self->net = nmp_net_new_full(nloop, gather, pl, ph, self);
    if (G_UNLIKELY(!self->net))
    {
        nmp_error("<NmpModAccess> alloc NmpNet object failed!");
        FATAL_ERROR_EXIT;

		return -1;
    }

	nmp_net_set_reader(
		 self->net,
		 nmp_mod_acc_recv_sysmsg
	);

    nmp_net_set_funcs(
        self->net,
        nmp_mod_acc_io_init,
        nmp_mod_acc_io_fin
    );

	return 0;	
}


static void
nmp_mod_acc_consume_msg(NmpAppMod *s_self, NmpSysMsg *msg)
{
    NmpNetIO *io;
    gint ret;
    G_ASSERT(NMP_IS_APPMOD(s_self) && NMP_IS_SYSMSG(msg));

	io = MSG_IO(msg);

	if (G_LIKELY(io))
	{
		ret = nmp_net_write_io(io, msg);	/* trigger io_close() */
		if (G_UNLIKELY(ret < 0))
		{
			nmp_print(
				"<NmpModAccess> consume(), send packet failed, err:%d", -ret
			);
		}
	}
	else
		nmp_warning("<NmpModAccess> consume(), no io specified!");

    nmp_sysmsg_destroy(msg);
}


static void
nmp_mod_acc_class_init(NmpModAccessClass *c_self)
{
    NmpAppModClass *super_c;
    
    super_c = (NmpAppModClass*)c_self;
    c_self->io_init      = nmp_mod_acc_on_io_init;
    c_self->io_close     = nmp_mod_acc_on_io_close;
    super_c->consume_msg = nmp_mod_acc_consume_msg;
}


void
nmp_mod_acc_release_io(NmpModAccess *self, NmpNetIO *io)
{
    G_ASSERT(self != NULL && io != NULL);

    BUG_ON(!self->net);
    nmp_net_kill_io(self->net, io);   
}


//:~ End
