#include "nmp_data.h"
#include "nmp_memory.h"

G_DEFINE_TYPE(JpfData, jpf_data, G_TYPE_OBJECT);

static void
jpf_data_init(JpfData *self)
{
	self->data_addr = NULL;
	self->data_size = 0;
	self->space_capacity = 0;
}


static void
jpf_data_dispose(GObject *object)
{
	JpfData *jpf_data = NMP_DATA(object);

	if (G_UNLIKELY(!jpf_data))
		return;

	if (jpf_data->data_addr)
	{
		jpf_mem_kfree(jpf_data->data_addr, jpf_data->space_capacity);
		jpf_data_init(jpf_data);
	}

	G_OBJECT_CLASS(jpf_data_parent_class)->dispose(object);
}


static void
jpf_data_class_init(JpfDataClass *c_self)
{
	GObjectClass *gobject_class = (GObjectClass*)c_self;

	gobject_class->dispose = jpf_data_dispose;
}


//:~ End
