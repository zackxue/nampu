#ifndef __NMP_WDD_INTERFACE_H__
#define __NMP_WDD_INTERFACE_H__


#define WDD_PASSWD_LEN		128
#define DATA_FILE_LENGTH	1024

#define ET_LED_UP		0x00000001	// LED灯亮
#define ET_LED_DOWN	0x00000002	// LED灯灭
#define ET_LED_WINK 	0x00000003 	// LED闪

#define SUCCESS								0
#define NOEXIST 							-1

#define CREATEFILE_ERR				-2
#define PROBE_ERR						-3
#define OPENDEV_ERR						-4
#define READFILE_ERR					-5
#define WRITEFILE_ERR					-6
#define VERIFYPASSWD_ERR			-7
#define EXCUTEFILE_ERR				-8
#define CHANGEDIR_ERR					-9
#define CONTROL_ERR						-10
#define INPARA_ERR						-11
#define DEV_REMOVED						-12
#define DEV_INIT_ERR					-13
#define EXIST							-14
#define FUCTION_NULL					-15
#define MORE_DEV							-16
#define WRITE_LEN_LONGER			-17
#define SERIAL_INVALID				-18			//硬件序列号不一致
#define GET_SERIAL_ERR				-19			//获取硬件序列号失败
#define RESTORE_WDD_ERR				-20			//获取全局数据失败


typedef  void * encrypt_handle;

typedef enum
{
	EXCUTE_DATA,				//可执行数据类型
	DATA_TYPE_1,				//普通数据类型
	DATA_TYPE_2,
	DATA_TYPE_3
}FILE_TYPE;

typedef enum
{
	RE_INIT,
	WORKING,
	REMOVED
}ENCRYPT_STATE;

typedef struct encrypt_passwd
{
	char dev_passwd[WDD_PASSWD_LEN];
	char user_passwd[WDD_PASSWD_LEN];
}ENCRYPT_PASSWD;

typedef void *(*probe)(void *priv_data);

void encrypt_set_inst_state(encrypt_handle encrypt, ENCRYPT_STATE state);
ENCRYPT_STATE encrypt_get_inst_state(encrypt_handle encrypt);

int encrypt_register_probe(probe p);
encrypt_handle encrypt_init(void *priv_data, int *err);

int encrypt_write_data(encrypt_handle encrypt, char *src_data, int length, FILE_TYPE type);
int encrypt_read_data(encrypt_handle encrypt, char *out_data, FILE_TYPE read_type, int read_len);
int encrypt_uninit(encrypt_handle encrypt);

int encrypt_control(encrypt_handle encrypt, int cmd, void *para);

void *fake_encrypt(void *priv_data);
void *et199_encrypt(void *priv_data);

#endif