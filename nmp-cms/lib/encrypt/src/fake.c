#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "encrypt.h"
#include "fake.h"

static int fake_encrypt_init(void *encrypt);
static int fake_encrypt_write_data(void *encrypt, char *src_data, int length, FILE_TYPE type);
static int fake_encrypt_read_data(void *encrypt, char *out_data, FILE_TYPE read_type, int read_len);
static int fake_encrypt_uninit_data(void *encrypt);

static ENCRYPT_OPS *fake_set_ops()
{
	ENCRYPT_OPS *ops = (ENCRYPT_OPS *)malloc(sizeof(ENCRYPT_OPS));
	memset(ops, 0, sizeof(ENCRYPT_OPS));
	ops->init = fake_encrypt_init;
	ops->write_flie = fake_encrypt_write_data;
	ops->read_flie = fake_encrypt_read_data;
	ops->close_dev = fake_encrypt_uninit_data;
	ops->creat_file = NULL;
	ops->excute_file = NULL;
	ops->control = NULL;
	return ops;
}


void *fake_encrypt(void *priv_data)
{
	ENCRYPT *encrypt_ins = NULL;
	//int dwRet = 0;
	encrypt_ins = (ENCRYPT *)malloc(sizeof(ENCRYPT));
	if(encrypt_ins == NULL)
	{
		return NULL;
	}
	encrypt_ins->private_data = NULL;
	/*
	encrypt_ins->ops.init = fake_encrypt_init;
	encrypt_ins->ops.write_flie = fake_encrypt_write_data;
	encrypt_ins->ops.read_flie = fake_encrypt_read_data;
	encrypt_ins->ops.close_dev = fake_encrypt_uninit_data;
	*/
	encrypt_ins->ops = fake_set_ops();
	return (void *)encrypt_ins;
}

static int fake_encrypt_init(void *encrypt)
{
	ENCRYPT *encrypt_ins = encrypt;
	FAKE_DATA_ENCRYPT *fake_data = (FAKE_DATA_ENCRYPT *)malloc(sizeof(FAKE_DATA_ENCRYPT));
	encrypt_ins->private_data = (void*)fake_data;
	memset(encrypt_ins->private_data, 0, sizeof(FAKE_DATA_ENCRYPT));

	return SUCCESS;
}

static int fake_encrypt_write_data(void *encrypt, char *src_data, int length, FILE_TYPE type)
{
	ENCRYPT *encrypt_ins = encrypt;
	FAKE_DATA_ENCRYPT *virtual_data = (FAKE_DATA_ENCRYPT *)encrypt_ins->private_data;
	if(virtual_data == NULL)
	{
		return NOEXIST;
	}
	memcpy(virtual_data->buf, src_data, length);

	return SUCCESS;
}

static int fake_encrypt_read_data(void *encrypt, char *out_data, FILE_TYPE read_type, int read_len)
{
	int len;
	ENCRYPT *encrypt_ins = encrypt;
	FAKE_DATA_ENCRYPT *virtual_data = (FAKE_DATA_ENCRYPT *)encrypt_ins->private_data;
	if(virtual_data == NULL)
	{
		return NOEXIST;
	}
	len = (read_len <= strlen(virtual_data->buf)) ? read_len : strlen(virtual_data->buf);
	memcpy(out_data, virtual_data->buf, len);

	return SUCCESS;
}

static int fake_encrypt_uninit_data(void *encrypt)
{
	ENCRYPT *encrypt_ins = encrypt;
	free(encrypt_ins->ops);
	free(encrypt_ins->private_data);
	free(encrypt_ins);

	return SUCCESS;
}