#include "nmp_data.h"
#include "nmp_memory.h"

G_DEFINE_TYPE(NmpData, nmp_data, G_TYPE_OBJECT);

static void
nmp_data_init(NmpData *self)
{
	self->data_addr = NULL;
	self->data_size = 0;
	self->space_capacity = 0;
}


static void
nmp_data_dispose(GObject *object)
{
	NmpData *nmp_data = NMP_DATA(object);

	if (G_UNLIKELY(!nmp_data))
		return;

	if (nmp_data->data_addr)
	{
		nmp_mem_kfree(nmp_data->data_addr, nmp_data->space_capacity);
		nmp_data_init(nmp_data);
	}

	G_OBJECT_CLASS(nmp_data_parent_class)->dispose(object);
}


static void
nmp_data_class_init(NmpDataClass *c_self)
{
	GObjectClass *gobject_class = (GObjectClass*)c_self;

	gobject_class->dispose = nmp_data_dispose;
}


//:~ End
