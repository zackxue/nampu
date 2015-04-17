#ifndef __GB2312_TO_UTF8_H__
#define __GB2312_TO_UTF8_H__

#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>


/***********************************************************************
* input : gb2312 encoding 
* output : utf8 encoding
************************************************************************/
int gb2312_to_utf8(const uint8_t *input, uint32_t input_len, uint8_t *output);

#endif
