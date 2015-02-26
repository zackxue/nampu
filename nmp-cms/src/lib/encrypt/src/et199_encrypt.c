#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "encrypt.h"
#include "ET199_32.h"
//#include "nmp_debug.h"
#include "nmp_wdd.h"

//#define DEBUG


#define FILE_COUNT		10
#define ERROR_LEN			128
#define BUF_LEN		256
#define COUNT			255
#define PCONTEXT_COUNT	1
#define EXCUTE_FILE_NAME		"1111"
//#define ET199_HARDWARE_ID_LEN	8

char *file_name[FILE_COUNT] = {"1111", "1001", "1113", "1114", "1115", "1116", "1117", "1118", "1119", "1120"};

static int et199_encrypt_init(void *encrypt);
static int et199_encrypt_creat_flie(void *encrypt, char *file_name, int size, char *passwd, int file_type);
static int et199_encrypt_write_data(void *encrypt, char *src_data, int length, FILE_TYPE type);
static int et199_encrypt_read_data(void *encrypt, char *out_data, FILE_TYPE read_type, int read_len);
static int et199_encrypt_control(void *encrypt, int cmd, void *para);
static int et199_encrypt_close_dev(void *encrypt);
static int et199_encrypt_excute_flie(void *encrypt, char *file_name, char *inbuf, char *outbuf, char *passwd);
static int et199_encrypt_get_serial_num(void *encrypt, unsigned char *serial_num);


static ENCRYPT_OPS et199_ops =
{
	.init = et199_encrypt_init,
	.write_flie = et199_encrypt_write_data,
	.read_flie = et199_encrypt_read_data,
	.creat_file = et199_encrypt_creat_flie,
	.close_dev = et199_encrypt_close_dev,
	.excute_file = et199_encrypt_excute_flie,
	.get_hardware_id = et199_encrypt_get_serial_num,
	.control = et199_encrypt_control
};


#define print printf

void PostError(char * Perfix, unsigned long dwErr)
{
	#ifdef DEBUG
	char sErr[ERROR_LEN];
	memset(sErr, 0, sizeof(sErr));
	ETFormatErrorMessage(dwErr, sErr, sizeof(sErr));
	print("%s Error:%s\r\n",Perfix, sErr);
	#endif
}


static ENCRYPT_OPS *et199_get_ops()
{
	return (&et199_ops);
}


void *et199_encrypt(void *priv_data)
{
	ENCRYPT_PASSWD *passwd = (ENCRYPT_PASSWD *)priv_data;
	ENCRYPT *encrypt_ins = NULL;

	encrypt_ins = (ENCRYPT *)malloc(sizeof(ENCRYPT));
	if(encrypt_ins == NULL)
	{
		return NULL;
	}

	memset(encrypt_ins, 0, sizeof(ENCRYPT));

	struct ET_CONTEXT *pContext =
		(struct ET_CONTEXT *)malloc(sizeof(struct ET_CONTEXT) * PCONTEXT_COUNT);
	if(pContext == NULL)
	{
		return NULL;
	}

	memset(pContext, 0, sizeof(struct ET_CONTEXT) * PCONTEXT_COUNT);

	encrypt_ins->private_data = (void*)pContext;

	strncpy(encrypt_ins->dev_passwd, passwd->dev_passwd, WDD_PASSWD_LEN - 1);
	strncpy(encrypt_ins->user_passwd, passwd->user_passwd, WDD_PASSWD_LEN - 1);
	encrypt_ins->init_flag = 0;
	encrypt_ins->ops = et199_get_ops();

	return (void *)encrypt_ins;
}


static int et199_encrypt_get_serial_num(void *encrypt, unsigned char *serial_num)
{
	unsigned long count = 0;
	int dwRet = 0;
	ENCRYPT *encrypt_ins = encrypt;
	struct ET_CONTEXT *pContext = (struct ET_CONTEXT *)encrypt_ins->private_data;
	dwRet = ETControl(&pContext[0], ET_GET_SERIAL_NUMBER, NULL, 0, (unsigned char *)serial_num, WDD_HARDWARE_ID_LEN, &count);
	if(dwRet)
	{
		PostError("et199_encrypt_get_serial_num", dwRet);
		printf("dwtret = %x\n", dwRet);
		return GET_SERIAL_ERR;
	}
	return SUCCESS;
}

static int et199_encrypt_verfiy_serial_num(void *encrypt)
{
	int dwRet = 0;
	wdd header;
	int header_len = WDD_HEAD_LEN;
	unsigned char sn[WDD_HARDWARE_ID_LEN] = {0};
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	memset(&header, 0, sizeof(wdd));
	dwRet = et199_encrypt_get_serial_num(encrypt_ins, sn);
	if(dwRet != 0)
	{
		return dwRet;
	}

	dwRet = et199_encrypt_read_data((void *)encrypt_ins, (char *)&header, 1, header_len);
	if(dwRet != 0)
	{
		return dwRet;
	}

	if(memcmp(sn, (const char *)header.serial_code.hardware_id, WDD_HARDWARE_ID_LEN) != 0)
	{
		return SERIAL_INVALID;
	}
	return SUCCESS;
}

static int et199_encrypt_init(void *encrypt)
{
	int dwCount = 0;
	int dwRet = 0;
	ENCRYPT *encrypt_ins = encrypt;
	struct ET_CONTEXT *pContext = (struct ET_CONTEXT *)encrypt_ins->private_data;

	dwRet = ETEnum(0, &dwCount);
	if(dwRet != ET_E_INSUFFICIENT_BUFFER && dwRet)
	{
		PostError("ETEnum 1", dwRet);
		return PROBE_ERR;
	}

	if(dwCount > PCONTEXT_COUNT)
	{
		return MORE_DEV;
	}

	memset(pContext, 0, sizeof(struct ET_CONTEXT)*dwCount);

	dwRet = ETEnum(pContext, &dwCount);
	if(dwRet)
	{
		PostError("ETEnum 2", dwRet);
		return PROBE_ERR;
	}

	//Open ET199
	dwRet = ETOpen(&pContext[0]);
	if(dwRet)
	{
		PostError("ETOpen", dwRet);
		return OPENDEV_ERR;
	}

	dwRet = et199_encrypt_verfiy_serial_num(encrypt_ins);
	if(dwRet != 0)
	{
		return dwRet;
	}

	return SUCCESS;
}


static int et199_encrypt_creat_flie(void *encrypt, char *file_name, int size, char *passwd, int file_type)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	struct ET_CONTEXT *pContext = (struct ET_CONTEXT *)encrypt_ins->private_data;
	int dwRet = 0;

	dwRet = ETVerifyPin(&pContext[0], (unsigned char *)passwd, strlen(passwd), ET_DEV_PIN);
	if(dwRet)
	{
		PostError("ETVerifyPin",dwRet);
		return VERIFYPASSWD_ERR;
	}
	dwRet = ETCreateFile(&pContext[0], file_name, size, file_type);
	if(dwRet)
	{
		PostError("ETCreateFile", dwRet);
		return CREATEFILE_ERR;
	}
	return SUCCESS;
}

static int et199_encrypt_write_data(void *encrypt, char *src_data, int length, FILE_TYPE type)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	struct ET_CONTEXT*pContext = (struct ET_CONTEXT*)encrypt_ins->private_data;
	int dwRet = 0;
	if(length > DATA_FILE_LENGTH)
	{
		return WRITE_LEN_LONGER;
	}
	dwRet = ETVerifyPin(&pContext[0], (unsigned char *)encrypt_ins->dev_passwd,
		strlen(encrypt_ins->dev_passwd), ET_DEV_PIN);
	if(dwRet)
	{
		if(dwRet == ET_E_KEY_REMOVED || dwRet == ET_E_NO_LIST)
		{
			return DEV_REMOVED;
		}
		PostError("ETVerifyPin", dwRet);
		return VERIFYPASSWD_ERR;
	}

	dwRet = ETWriteFile(&pContext[0], file_name[type], 0, (unsigned char *)src_data, length);
	if(dwRet)
	{
		if(dwRet == ET_E_KEY_REMOVED || dwRet == ET_E_NO_LIST)
		{
			return DEV_REMOVED;
		}
		PostError("ETWriteFile", dwRet);
		return WRITEFILE_ERR;
	}

	return 0;
}

static int et199_encrypt_read_data(void *encrypt, char *out_data, FILE_TYPE read_type, int read_len)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	char outbuf[BUF_LEN] = {0};
	char *p = out_data;
	char inbuf[BUF_LEN] = {0};
	int r_len = 0;
	int total_len = 0;
	int count = read_len/COUNT;
	short left = read_len%COUNT;
	short i = 0;
	short length = COUNT;

	if(p == NULL)
	{
		return INPARA_ERR;
	}
	for(i = 0; i < count; i++)
	{
		memset(outbuf, 0, BUF_LEN);
		sprintf(inbuf, "%s<%d><%d>", file_name[read_type], length, i);
		r_len = et199_encrypt_excute_flie(encrypt_ins, EXCUTE_FILE_NAME, inbuf, outbuf, encrypt_ins->user_passwd);
		if(r_len < 0)
		{
			return r_len;
		}

		memcpy(p, outbuf, r_len);
		p += r_len;
		total_len += r_len;
		if(r_len < length)
		{
			goto END;
		}
	}
	if(left > 0)
	{
		memset(outbuf, 0, BUF_LEN);
		sprintf(inbuf, "%s<%d><%d>", file_name[read_type], left, i);
		r_len = et199_encrypt_excute_flie(encrypt_ins, EXCUTE_FILE_NAME, inbuf, outbuf, encrypt_ins->user_passwd);
		if(r_len < 0)
		{
			print("encrypt_excute_file error ret = %d\n", r_len);
			return r_len;
		}
		memcpy(p, outbuf, r_len);
		total_len += r_len;
		if(r_len < left)
		{
			goto END;
		}
	}
END:
	return 0;
}

static int et199_encrypt_close_dev(void *encrypt)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	struct ET_CONTEXT*pContext = (struct ET_CONTEXT*)encrypt_ins->private_data;

	ETClose(&pContext[0]);
	return SUCCESS;
}


static int et199_encrypt_excute_flie(void *encrypt, char *file_name, char *inbuf, char *outbuf, char *passwd)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	struct ET_CONTEXT*pContext = (struct ET_CONTEXT*)encrypt_ins->private_data;
	int dwRet = 0;
	long unsigned int len = 0;

	dwRet = ETVerifyPin(&pContext[0], (unsigned char *)passwd, strlen(passwd), ET_USER_PIN);
	if(dwRet)
	{
		if(dwRet == ET_E_KEY_REMOVED || dwRet == ET_E_NO_LIST)
		{
			return DEV_REMOVED;
		}
		PostError("et199_encrypt_excute_flie ETVerifyPin", dwRet);
		return VERIFYPASSWD_ERR;
	}

	dwRet = ETExecute(&pContext[0], file_name, (unsigned char *)inbuf, strlen(inbuf), (unsigned char *)outbuf, BUF_LEN, &len);
	if(dwRet)
	{
		if(dwRet == ET_E_KEY_REMOVED || dwRet == ET_E_NO_LIST)
		{
			return DEV_REMOVED;
		}
		PostError("et199_encrypt_excute_flie ETExecute",dwRet);
		return EXCUTEFILE_ERR;
	}

	return len;
}

static int et199_encrypt_control(void *encrypt, int cmd, void *para)
{
	ENCRYPT *encrypt_ins = (ENCRYPT *)encrypt;
	struct ET_CONTEXT*pContext = (struct ET_CONTEXT*)encrypt_ins->private_data;
	int dwRet = 0;
	dwRet = ETControl(&pContext[0], cmd, (unsigned char *)para, sizeof(para), NULL, 0, NULL);
	if(dwRet)
	{
		PostError("ETControl ",dwRet);
		return CONTROL_ERR;
	}
	return SUCCESS;
}
