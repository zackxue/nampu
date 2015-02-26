#ifndef _ZTE_GB2312_TO_UTF8_H
#define	_ZTE_GB2312_TO_UTF8_H

#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>

//typedef unsigned char uint8_t;
//typedef unsigned int  uint8_t;

struct __utf8_t_
{
	uint8_t u[4];
	int count;
}__attribute__((packed));

typedef struct __utf8_t_ utf8_t;
extern int gb2312_to_utf8(const uint8_t *input,uint32_t input_len,uint8_t *output);

#endif
