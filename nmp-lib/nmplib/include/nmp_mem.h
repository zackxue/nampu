#ifndef __NMP_MEM_H__
#define __NMP_MEM_H__

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

void *nmp_alloc(size_t size);
void *nmp_alloc0(size_t size);
void _nmp_dealloc(void *ptr, size_t size);

#define nmp_dealloc(ptr, size) \
	_nmp_dealloc(ptr, size)

#define nmp_new(struct_type, count) \
	nmp_alloc(sizeof(struct_type)*(count))

#define nmp_new0(struct_type, count) \
	nmp_alloc0(sizeof(struct_type)*(count))

#define nmp_del(ptr, struct_type, count) \
	_nmp_dealloc((ptr), sizeof(struct_type)*(count))

#ifdef __cplusplus
}
#endif

#endif	//__NMP_MEM_H__
