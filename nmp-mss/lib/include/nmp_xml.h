 
#ifndef __NMP_XML__
#define __NMP_XML__

#define MAX_CMD_ID_LEN                  64

typedef void (*NmpMsgInfoFin)(void *data, int size);
typedef void (*NmpMsgInfoPrivDes)(void* priv, int size);
typedef struct _NmpMsgInfo NmpMsgInfo;
struct _NmpMsgInfo
{
	char       		msg_id[MAX_CMD_ID_LEN];
	int			    private_size;
	void 			*private_data;
	NmpMsgInfoFin	priv_destroy;
};

NmpMsgInfo * 
nmp_parse_str_xml(const char *xml_buff, int xml_len, unsigned int seq);

int 
nmp_create_str_xml(char *xml_buff, int *buff_len, NmpMsgInfo *sys_msg);

void
nmp_init_xml_cmd();

void
nmp_free_msginfo(NmpMsgInfo *sys_msg);   

void
nmp_free_msginfo_head(NmpMsgInfo *sys_msg);


#endif
