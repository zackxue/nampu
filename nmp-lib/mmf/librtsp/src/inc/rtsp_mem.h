/*
 *	author:	zyt
 *	time:	begin in 2012/7/24
 */
#ifndef __MEM_H_20120724__
#define __MEM_H_20120724__


#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint16
#define uint16 unsigned short
#endif

#ifndef uint32
#define uint32 unsigned int
#endif

#ifndef size_t
#define size_t unsigned int
#endif

#define mbool char

#define MIN_INT32	((int)0x80000000)
#define MAX_INT32	((int)0x7fffffff)

#define STRLEN1(str)	(strlen(str) + 1)	//使用时注意

#define my_free(ptr, size) {	\
	if (ptr != NULL) {	\
		my_free_not_reset(ptr, size);	\
		ptr = NULL;	\
	}	\
}

typedef struct _mb			//内部使用
{
	int size;
	char content[0];
}mb;


void *my_try_alloc(size_t size);

void *my_alloc(size_t size);

void my_free_not_reset(void *ptr, size_t size);

char *my_strdup(const char *str);

char *my_strndup(const char *str, size_t n);

void *my_memdup(const void *mem, size_t bype_size);

char *my_strchomp(char *str);

char **my_strsplit(const char *string, const char *delimiter, int max_tokens);

char *my_strdown(const char *str, int len);

void my_strfreev(char **str_array);

mbool my_str_has_prefix(const char *str, const char *prefix);

mb *mb_new(int length);

char *mb_content(mb *mb);

mb *ptr_to_mb(char *str);

void mb_del(mb *mb);

#endif

