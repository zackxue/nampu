
#include "config.h"
#include "nmp_xmlmem.h"


static __inline__ void *default_alloc(size_t size)
{
	J_ASSERT(size);
	
	return calloc(1, size);//*/return malloc(size);
}

static __inline__ void default_dealloc(void *ptr, size_t size)
{
	J_ASSERT(ptr && size);
	
	free(ptr);
	ptr = NULL;
}


g_alloc_t   g_alloc   = default_alloc;
g_dealloc_t g_dealloc = default_dealloc;

void *j_xml_alloc(size_t size)
{
	J_ASSERT(size);
	
	return (*g_alloc)(size);
}

void j_xml_dealloc(void *ptr, size_t size)
{
	J_ASSERT(ptr && size);
	
	return (*g_dealloc)(ptr, size);
}

