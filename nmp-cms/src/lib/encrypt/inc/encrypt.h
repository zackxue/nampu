#ifndef __ET199_ENCRYPT__
#define __ET199_ENCRYPT__

//#include "ET199_32.h"
#include "nmp_wdd_interface.h"
#define GLOBAL_WDD_DATA_LEN	2048
#define WDD_HARDWARE_NUM_LEN 8
typedef struct encrypt_ops
{
	int (*init)(void *encrypt);
	int (*write_flie)(void *encrypt, char *src_data, int length, FILE_TYPE type);
	int (*read_flie)(void *encrypt, char *out_data, FILE_TYPE read_type, int read_len);
	int (*creat_file)(void *encrypt, char *file_name, int size, char *passwd, int file_type);
	int (*excute_file)(void *encrypt, char *file_name, char *inbuf, char *outbuf, char *passwd);
	int (*close_dev)(void *encrypt);
	int (*get_hardware_id)(void *encrypt, unsigned char *serial_num);
	int (*control)(void *encrypt, int cmd, void *para);
}ENCRYPT_OPS;

typedef struct encrypt
{
	ENCRYPT_STATE state;
	void *private_data;
	char dev_passwd[WDD_PASSWD_LEN];
	char user_passwd[WDD_PASSWD_LEN];
	unsigned char global_wdd_data[GLOBAL_WDD_DATA_LEN];
	int init_flag;
	unsigned char hardware_id[WDD_HARDWARE_NUM_LEN];
	ENCRYPT_OPS *ops;

}ENCRYPT;

#endif