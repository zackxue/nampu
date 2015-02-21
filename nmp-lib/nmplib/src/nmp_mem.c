#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "nmp_mem.h"
#include "nmp_macros.h"

#define JLIB_MEM_DEBUG  1
#define MAGIC		0xf1f2f3f4

struct __alloc_cookie
{
	int32_t magic;
	int32_t length;
};

struct __tail_cookie
{
	int32_t magic;
};


static __inline__ void *
__malloc(size_t size)
{
#ifndef JLIB_MEM_DEBUG
	return malloc(size);
#else
	struct __alloc_cookie *head;
	struct __tail_cookie *tail;
	size_t real_size = ALIGN(size, sizeof(void*));

	real_size += sizeof(struct __alloc_cookie);
	real_size += sizeof(struct __tail_cookie);

	head = (struct __alloc_cookie*)malloc(real_size);
	if (head)
	{
		head->magic = MAGIC;
		head->length = size;

		tail = (struct __tail_cookie*)((void*)(head + 1) + ALIGN(size, sizeof(void*)));
		tail->magic = MAGIC;
		return head + 1;
	}

	return NULL;
#endif
}


static __inline__ void
__free(void *ptr, size_t size)
{
	if (J_UNLIKELY(!ptr))
		return;

#ifndef JLIB_MEM_DEBUG
	free(ptr);
#else
	struct __alloc_cookie *head;
	struct __tail_cookie *tail;

	head = (struct __alloc_cookie*)ptr;
	head -= 1;

	if (head->magic != MAGIC)
	{
		fprintf(stderr, "__free(), head->magic != MAGIC!!!\n");
		BUG();
	}

	if (head->length != size)
	{
		fprintf(stderr, "__free(), head->length(%d) != size(%d)!!!\n", head->length, size);
		BUG();
	}

	tail = (struct __tail_cookie*)(ptr + ALIGN(head->length, sizeof(void*)));
	if (tail->magic != MAGIC)
	{
		fprintf(stderr, "__free(), tail->magic != MAGIC!!!\n");
		BUG();
	}

	free(head);
#endif	
}


void *j_alloc(size_t size)
{
	void *ptr;

	ptr = __malloc(size);
	if (!ptr)
		BUG();

	return ptr;
}


void *j_alloc0(size_t size)
{
	void *ptr;

	ptr = __malloc(size);
	if (!ptr)
		BUG();

	memset(ptr, 0, size);
	return ptr;
}


void _j_dealloc(void *ptr, size_t size)
{
	__free(ptr, size);
}


//:~ End
