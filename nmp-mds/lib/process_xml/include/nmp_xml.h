 
#ifndef __NMP_XML__
#define __NMP_XML__

#define MAX_CMD_ID_LEN                  64

typedef void (*NmpMsgInfoFin)(void *data, int size);
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


/* Message Structs */

#define MDS_ID_LEN			32
#define DOMAIN_ID_LEN		32
#define TIME_INFO_LEN		32
#define MAX_IP_LEN          128

typedef struct _NmpMsgErrCode NmpMsgErrCode;
struct _NmpMsgErrCode
{
	int			err_no;
};

#define RES_CODE(res) (((NmpMsgErrCode*)(res))->err_no)
#define SET_CODE(res, code) (RES_CODE(res) = (code))

typedef struct _NmpMdsRegister NmpMdsRegister;
struct _NmpMdsRegister
{
    char mds_id[MDS_ID_LEN];
    char cms_ip[MAX_IP_LEN];
};


typedef struct _NmpmdsRegisterRes NmpmdsRegisterRes;
struct _NmpmdsRegisterRes
{
    NmpMsgErrCode	code;
    char           	domain_id[DOMAIN_ID_LEN];
    char           	mds_id[MDS_ID_LEN];	
    int         	keep_alive_time;
    int				pu_port;
    int				rtsp_port;

};

typedef struct _NmpMdsHeart NmpMdsHeart;
struct _NmpMdsHeart
{
    char		mds_id[MDS_ID_LEN];
    char		cms_ip[MAX_IP_LEN];
};

typedef struct _NmpMdsHeartRes NmpMdsHeartRes;
struct _NmpMdsHeartRes
{
    NmpMsgErrCode	code;
    char			server_time[TIME_INFO_LEN];
};

#endif
