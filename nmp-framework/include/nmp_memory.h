#ifndef __NMP_MEMORY_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* Exports
 *     Memory allocate/deallocate Interfaces.
*/
gpointer jpf_mem_kalloc(guint size);
gpointer jpf_mem_kalloc0(guint size);
void jpf_mem_kfree(gpointer p, guint size);

gpointer jpf_mem_alloc(guint size);
gpointer jpf_mem_alloc0(guint size);
void jpf_mem_free(gpointer p);

#define jpf_new(struct_type, count) \
	jpf_mem_alloc(sizeof(struct_type) * count)
#define jpf_new0(struct_type, count) \
	jpf_mem_alloc0(sizeof(struct_type) * count)
#define jpf_free(ptr) jpf_mem_free(ptr)

G_END_DECLS

#endif	//__NMP_MEMORY_H__
