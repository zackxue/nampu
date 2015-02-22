#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "encrypt.h"
//#include "nmp_wdd.h"
#include "nmp_wdd_interface.h"

#define WDD_HARDWARE_ID_LEN_ 8
static int last_read_len = 0;
#define MAX_READ_BUF 10*1024
#define SLEEP_USEC		(500*1000)
#define RE_TRY_GET_HARDWARE_ID_COUNT	6
probe encrypt_probe_init = NULL;


static void encrypt_backup_data(ENCRYPT *inst, char *src_data, int len)
{
	if(src_data != NULL && len > 0)
	{
		memset(inst->global_wdd_data, 0, GLOBAL_WDD_DATA_LEN);
		memcpy(inst->global_wdd_data, src_data, len);

	}
}

static int encrypt_restore_data(ENCRYPT *inst, char *dest_data, int len)
{
	if(inst == NULL && dest_data == NULL)
		return RESTORE_WDD_ERR;

	memcpy(dest_data, inst->global_wdd_data, len);

	return 0;
}

static void encrypt_set_flag(ENCRYPT *inst, int state)
{
	inst->init_flag = state;
}

static int encrypt_get_flag(ENCRYPT *inst)
{
	return (inst->init_flag);
}


static void encrypt_release(encrypt_handle encrypt)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	if(inst == NULL)
	{
		return;
	}
	if(inst->private_data)
		free(inst->private_data);

	free(inst);
	return;
}

int encrypt_register_probe(probe p)
{
	if (p == NULL)
	{
		return NOEXIST;
	}

	if (encrypt_probe_init == NULL)
	{
		encrypt_probe_init = p;
		return SUCCESS;
	}

	return EXIST;
}

static encrypt_handle encrypt_probe(void *priv_data)
{
	encrypt_handle inst;
	if(encrypt_probe_init != NULL)
	{
		inst = (encrypt_probe_init)(priv_data);
		return inst;
	}
	return NULL;
}

encrypt_handle encrypt_init(void *priv_data, int *err)
{
	ENCRYPT *inst = NULL;
	int ret;

	inst = (ENCRYPT *)encrypt_probe(priv_data);
	if(inst == NULL)
	{
		*err = FUCTION_NULL;
		return NULL;
	}
	if(inst->ops && inst->ops->init)
	{
		ret = (*inst->ops->init)(inst);
		if(ret < 0)
		{
			encrypt_release((void *)inst);
			*err = ret;
			return NULL;
		}
	}
	if(encrypt_get_flag(inst) == 0)
	{
		encrypt_set_flag(inst, 1);
		last_read_len = 0;
	}
	inst->state = WORKING;
	*err = 0;
	return inst;
}

int encrypt_write_data(encrypt_handle encrypt, char *src_data, int length, FILE_TYPE type)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	int ret = 0;

	if(!inst || !inst->ops)
	{
		return NOEXIST;
	}
	if(src_data == NULL)
	{
		return INPARA_ERR;
	}

	if (inst->ops->write_flie)
	{
		ret = inst->ops->write_flie(inst, src_data, length, type);
		if(ret != 0)
		{
			return ret;
		}
		encrypt_backup_data(inst, src_data, length);
	}
	else
		return FUCTION_NULL;

	return ret;
}

static int encrypt_cmp_hardware_id(ENCRYPT *inst)
{
	int ret;
	unsigned char hardware_id[WDD_HARDWARE_ID_LEN_] = {0};
	ret = inst->ops->get_hardware_id((void *)inst, hardware_id);
	if(ret != 0)
	{
		return ret;
	}
	if(memcmp(inst->hardware_id, hardware_id, WDD_HARDWARE_ID_LEN_) != 0)
	{
		return SERIAL_INVALID;
	}
	return 0;
}


int encrypt_read_data(encrypt_handle encrypt, char *out_data, FILE_TYPE read_type, int read_len)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;

	char buf[MAX_READ_BUF] = {0};
	int ret = 0;

	int hardware_count = 0;
	if(!inst || !inst->ops)
	{
		return NOEXIST;
	}
	if(encrypt_get_flag(inst) == 1)
	{
		if (inst->ops->read_flie)
		{
			memset(buf, 0, MAX_READ_BUF);
			ret = inst->ops->read_flie(inst, buf, read_type, read_len);
			if(ret != 0)
			{
				return ret;
			}
			if(last_read_len == 0)
			{
				last_read_len = read_len;
			}

		START1:
			ret = inst->ops->get_hardware_id((void *)inst, inst->hardware_id);
			if(ret != 0)
			{
				if(hardware_count++ < RE_TRY_GET_HARDWARE_ID_COUNT)
				{
					usleep(SLEEP_USEC);
					goto START1;
				}
				return ret;
			}
			encrypt_backup_data(inst, buf, read_len);
			encrypt_set_flag(inst, 0);

			encrypt_restore_data(inst, out_data, read_len);
			return ret;
		}
		else
			return FUCTION_NULL;
	}
	else if(encrypt_get_flag(inst) == 0)
	{
	START2:
		ret = encrypt_cmp_hardware_id(inst);
		if(ret != 0)
		{
			if(hardware_count++ < RE_TRY_GET_HARDWARE_ID_COUNT)
			{
				usleep(SLEEP_USEC);
				goto START2;
			}
			return ret;
		}
		if(read_len > last_read_len)
		{
			memset(buf, 0, MAX_READ_BUF);
			ret = inst->ops->read_flie(inst, buf, read_type, read_len);
			if(ret != 0)
			{
				return ret;
			}
			last_read_len = read_len;
			encrypt_backup_data(inst, buf, read_len);
		}
		encrypt_restore_data(inst, out_data, read_len);
	}

	return ret;
}

int encrypt_creat_file(encrypt_handle encrypt, char *filename, int size, char *passwd, int file_type)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	int ret = 0;

	if(!inst || !inst->ops)
	{
		return NOEXIST;
	}
	if (inst->ops->creat_file)
		ret = inst->ops->creat_file(inst, filename, size, passwd, file_type);
	else
		return FUCTION_NULL;

	return ret;
}

int encrypt_uninit(encrypt_handle encrypt)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	if(!inst)
	{
		return NOEXIST;
	}
	if (inst->ops->close_dev)
	{
		inst->ops->close_dev(inst);
		encrypt_release((encrypt_handle)inst);
	}
	else
	{
		encrypt_release((encrypt_handle)inst);
		return FUCTION_NULL;
	}
	return SUCCESS;
}

int encrypt_excute_file(encrypt_handle encrypt, char *file_name, char *inbuf, char *outbuf, char *passwd)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	int ret = 0;

	if(!inst || !inst->ops)
	{
		return NOEXIST;
	}
	if (inst->ops->excute_file)
		ret = inst->ops->excute_file(inst, file_name, inbuf, outbuf, passwd);
	else
		return FUCTION_NULL;

	return ret;
}


int encrypt_control(encrypt_handle encrypt, int cmd, void *para)
{
	ENCRYPT *inst = (ENCRYPT *)encrypt;
	int ret = 0;

	if(!inst || !inst->ops)
	{
		return NOEXIST;
	}
	if (inst->ops->control)
	{
		ret = inst->ops->control(inst, cmd, para);
	}
	else
		return FUCTION_NULL;

	return ret;
}
