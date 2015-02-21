#ifndef __J_MEM_H__
#define __J_MEM_H__

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

void *j_alloc(size_t size);
void *j_alloc0(size_t size);
void _j_dealloc(void *ptr, size_t size);

#define j_dealloc(ptr, size) \
	_j_dealloc(ptr, size)

#define j_new(struct_type, count) \
	j_alloc(sizeof(struct_type)*(count))

#define j_new0(struct_type, count) \
	j_alloc0(sizeof(struct_type)*(count))

#define j_del(ptr, struct_type, count) \
	_j_dealloc((ptr), sizeof(struct_type)*(count))

#ifdef __cplusplus
}
#endif

#endif	//__J_MEM_H__
