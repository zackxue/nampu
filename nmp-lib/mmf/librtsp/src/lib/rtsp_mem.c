
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rtsp_mem.h"
#include "rtsp_log.h"



void *my_try_alloc(size_t size)
{
	void *ptr = malloc(size);
	
	if (ptr == NULL)
	{
		mem_error("malloc mem(size = %d) failed\n", size);
	}
	
	return ptr;
}


void *my_alloc(size_t size)
{
	void *ptr = malloc(size);
	
	if (ptr == NULL)
	{
		mem_error("malloc mem(size = %d) failed\n", size);
		exit(1);
	}
	
	return ptr;
}


void my_free_not_reset(void *ptr, size_t size)
{
	free(ptr);
}


char *my_strdup(const char *str)
{
	char *new_str;
	size_t length;

	if (str)
	{
		length = STRLEN1(str);
		new_str = (char *)my_alloc(length);
		memcpy(new_str, str, length);
	}
	else
		new_str = NULL;

	return new_str;
}


char *my_strndup(const char *str, size_t n)
{
	char *new_str;

	if (str)
	{
		new_str = (char *)my_alloc(n + 1);
		strncpy(new_str, str, n);
		new_str[n] = '\0';
	}
	else
		new_str = NULL;

	return new_str;
}


void *my_memdup(const void *mem, size_t bype_size)
{
	void *new_mem;
	if (mem)
	{
		new_mem = my_alloc(bype_size);
		memcpy(new_mem, mem, bype_size);
	}
	else
		new_mem = NULL;

	return new_mem;
}


/*
 *	把字符串最后的所有空格字符去掉
 *	空格字符包括 空格('')、定位字符('\t')、CR('\r')、换行('\n')、
 *	垂直定位字符('\v')或翻页('\f')的情况
 */
char *my_strchomp(char *str)
{
	size_t len;

	return_val_if_fail(str != NULL, NULL);

	len = strlen(str);
	while (len--)
	{
		if (isspace(str[len]))
			str[len] = '\0';
		else
			break;
	}

	return str;
}

char **my_strsplit(const char *string, const char *delimiter, int max_tokens)
{
	char **str_array, *s;
	uint32 n = 0;
	const char *remainder;
	int tokens = 0;
	
	return_val_if_fail(string != NULL, NULL);
	return_val_if_fail(delimiter != NULL, NULL);
	return_val_if_fail(delimiter[0] != '\0', NULL);

	if (max_tokens < 1)
		max_tokens = MAX_INT32;
	tokens = max_tokens;
	
	/* first time to get the length */
	remainder = string;
	s = strstr(remainder, delimiter);
	if (s)
	{
		size_t delimiter_len = strlen(delimiter);

		while (--tokens && s)
		{
			n++;
			remainder = s + delimiter_len;
			s = strstr(remainder, delimiter);
		}
	}
	if (*string)
		n++;

	str_array = (char **)my_alloc(sizeof(char *) * (n + 1));

	n = 0;
	tokens = max_tokens;
	remainder = string;
	s = strstr(remainder, delimiter);
	if (s)
	{
		size_t delimiter_len = strlen(delimiter);

		while (--tokens && s)
		{
			size_t len = s - remainder;
			str_array[n] = (char *)my_strndup(remainder, len);
			n++;
			remainder = s + delimiter_len;
			s = strstr(remainder, delimiter);
		}
	}
	if (*string)
	{
		str_array[n] = (char *)my_strdup(remainder);
		n++;
	}

	str_array[n] = NULL;

	return str_array;
}

char *my_strdown(const char *str, int len)
{
	char *result, *s;

	return_val_if_fail(str != NULL, NULL);

	if (len < 0)
		len = strlen(str);

	result = my_strndup(str, len);
	for (s = result; *s; s++)
		*s = tolower(*s);

	return result;
}

void my_strfreev(char **str_array)
{
	if (str_array)
	{
		int i;

		for (i = 0; str_array[i] != NULL; i++)
			my_free(str_array[i], STRLEN1(str_array[i]));

		my_free(str_array, sizeof(char *) * (i + 1));
	}
}

mbool my_str_has_prefix(const char *str, const char *prefix)
{
	int str_len;
	int prefix_len;

	return_val_if_fail(str != NULL, 0);
	return_val_if_fail(prefix != NULL, 0);

	str_len = strlen(str);
	prefix_len = strlen(prefix);

	if (str_len < prefix_len)
		return 0;

	return strncmp(str, prefix, prefix_len) == 0;
}

mb *mb_new(int length)
{
	mb *mb;
	int size = length + sizeof(mb);

	mb = my_alloc(size);
	mb->size = size;
	return mb;
}


char *mb_content(mb *mb)
{
	return mb->content;
}


mb *ptr_to_mb(char *str)
{
	return (mb*)(str - ((int)(((mb*)0)->content)));
}


void mb_del(mb *mb)
{
	my_free(mb, mb->size);
}

