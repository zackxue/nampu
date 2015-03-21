#ifndef __NMP_MEMORY_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* Exports
 *     Memory allocate/deallocate Interfaces.
*/
gpointer nmp_mem_kalloc(guint size);
gpointer nmp_mem_kalloc0(guint size);
void nmp_mem_kfree(gpointer p, guint size);

gpointer nmp_mem_alloc(guint size);
gpointer nmp_mem_alloc0(guint size);
void nmp_mem_free(gpointer p);

#define nmp_new(struct_type, count) \
	nmp_mem_alloc(sizeof(struct_type) * count)
#define nmp_new0(struct_type, count) \
	nmp_mem_alloc0(sizeof(struct_type) * count)
#define nmp_free(ptr) nmp_mem_free(ptr)

G_END_DECLS

#endif	//__NMP_MEMORY_H__
