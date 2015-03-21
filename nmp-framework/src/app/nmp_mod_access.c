/*
 * nmp_mod_access.c
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_mod_access.h"


G_DEFINE_TYPE(JpfModAccess, nmp_mod_acc, NMP_TYPE_APPMOD);


static void 
nmp_mod_acc_io_fin(gpointer orig_mod, JpfNetIO *io, gint err)
{
    JpfModAccess *mod = (JpfModAccess*)orig_mod;
    BUG_ON(!mod);
    NMP_MODACCESS_GET_CLASS(mod)->io_close(mod, io, err);
}


static gint
nmp_mod_acc_io_init(gpointer orig_mod, JpfNetIO *io)
{
    JpfModAccess *mod = (JpfModAccess*)orig_mod;
    BUG_ON(!mod);
    return NMP_MODACCESS_GET_CLASS(mod)->io_init(mod, io);  
}


static void
nmp_mod_acc_on_io_close(JpfModAccess *self, JpfNetIO *io, gint err)
{
    jpf_warning("<JpfModAccess> io_close() called!");
}


static gint
nmp_mod_acc_on_io_init(JpfModAccess *self, JpfNetIO *io)
{
    jpf_warning("<JpfModAccess> io_init() called!");
    return -E_NOTSUPPORT;
}


JpfNetIO *
nmp_mod_acc_create_listen_io(JpfModAccess *self, struct sockaddr *sa, 
	gint *err)
{
    G_ASSERT(self != NULL);

    return jpf_net_create_listen_io_2(self->net, sa, err);
}


static gint
nmp_mod_acc_recv_sysmsg(gpointer priv_data, JpfNetIO *net_io, 
	gpointer msg)
{
	JpfModAccess *self;
	NmpSysMsg *sys_msg;
	G_ASSERT(priv_data != NULL);

	self = (JpfModAccess*)priv_data;
	sys_msg = NMP_SYSMSG(msg);
	jpf_sysmsg_attach_io(sys_msg, net_io);

	return nmp_app_mod_rcv_f(NMP_APPMOD(self), sys_msg);
}


static void
nmp_mod_acc_init(JpfModAccess *self)
{
	self->net = NULL;
}


gint
nmp_mod_acc_init_net(JpfModAccess *self, JpfPacketProto *pl, JpfPayloadProto *ph)
{
    self->net = jpf_net_new(pl, ph, self);
    if (G_UNLIKELY(!self->net))
    {
        jpf_error("<JpfModAccess> alloc JpfNet object failed!");
        FATAL_ERROR_EXIT;

		return -1;
    }

	jpf_net_set_reader(
		 self->net,
		 nmp_mod_acc_recv_sysmsg
	);

    jpf_net_set_funcs(
        self->net,
        nmp_mod_acc_io_init,
        nmp_mod_acc_io_fin
    );

	return 0;	
}


gint
nmp_mod_acc_init_net_full(JpfModAccess *self, guint nloop, gboolean gather,
	JpfPacketProto *pl, JpfPayloadProto *ph)
{
    self->net = jpf_net_new_full(nloop, gather, pl, ph, self);
    if (G_UNLIKELY(!self->net))
    {
        jpf_error("<JpfModAccess> alloc JpfNet object failed!");
        FATAL_ERROR_EXIT;

		return -1;
    }

	jpf_net_set_reader(
		 self->net,
		 nmp_mod_acc_recv_sysmsg
	);

    jpf_net_set_funcs(
        self->net,
        nmp_mod_acc_io_init,
        nmp_mod_acc_io_fin
    );

	return 0;	
}


static void
nmp_mod_acc_consume_msg(NmpAppMod *s_self, NmpSysMsg *msg)
{
    JpfNetIO *io;
    gint ret;
    G_ASSERT(NMP_IS_APPMOD(s_self) && NMP_IS_SYSMSG(msg));

	io = MSG_IO(msg);

	if (G_LIKELY(io))
	{
		ret = jpf_net_write_io(io, msg);	/* trigger io_close() */
		if (G_UNLIKELY(ret < 0))
		{
			jpf_print(
				"<JpfModAccess> consume(), send packet failed, err:%d", -ret
			);
		}
	}
	else
		jpf_warning("<JpfModAccess> consume(), no io specified!");

    jpf_sysmsg_destroy(msg);
}


static void
nmp_mod_acc_class_init(JpfModAccessClass *c_self)
{
    NmpAppModClass *super_c;
    
    super_c = (NmpAppModClass*)c_self;
    c_self->io_init      = nmp_mod_acc_on_io_init;
    c_self->io_close     = nmp_mod_acc_on_io_close;
    super_c->consume_msg = nmp_mod_acc_consume_msg;
}


void
nmp_mod_acc_release_io(JpfModAccess *self, JpfNetIO *io)
{
    G_ASSERT(self != NULL && io != NULL);

    BUG_ON(!self->net);
    jpf_net_kill_io(self->net, io);   
}


//:~ End
