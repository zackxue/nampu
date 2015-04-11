
#ifndef __NMP_BASE_64_H__
#define __NMP_BASE_64_H__

#include <stdio.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

int base64_encode(unsigned char *data, unsigned int data_len, 
		unsigned char **enc_data, unsigned int *enc_data_len);
int base64_decode(unsigned char *data, unsigned int data_len, 
		unsigned char **dec_data, unsigned int *dec_data_len);

void base64_free(void *data, size_t size);

#ifdef __cplusplus
}
#endif


#endif //__NMP_BASE_64_H__

