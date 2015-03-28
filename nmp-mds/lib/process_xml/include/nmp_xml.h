 
#ifndef __JPF_XML__
#define __JPF_XML__

#define MAX_CMD_ID_LEN                  64

typedef void (*JpfMsgInfoFin)(void *data, int size);
typedef struct _JpfMsgInfo JpfMsgInfo;
struct _JpfMsgInfo
{
	char       		msg_id[MAX_CMD_ID_LEN];
	int			    private_size;
	void 			*private_data;
	JpfMsgInfoFin	priv_destroy;
};

JpfMsgInfo * 
nmp_parse_str_xml(const char *xml_buff, int xml_len, unsigned int seq);

int 
nmp_create_str_xml(char *xml_buff, int *buff_len, JpfMsgInfo *sys_msg);

void
nmp_init_xml_cmd();

void
nmp_free_msginfo(JpfMsgInfo *sys_msg);   

void
nmp_free_msginfo_head(JpfMsgInfo *sys_msg);


/* Message Structs */

#define MDS_ID_LEN			32
#define DOMAIN_ID_LEN		32
#define TIME_INFO_LEN		32
#define MAX_IP_LEN          128

typedef struct _JpfMsgErrCode JpfMsgErrCode;
struct _JpfMsgErrCode
{
	int			err_no;
};

#define RES_CODE(res) (((JpfMsgErrCode*)(res))->err_no)
#define SET_CODE(res, code) (RES_CODE(res) = (code))

typedef struct _JpfMdsRegister JpfMdsRegister;
struct _JpfMdsRegister
{
    char mds_id[MDS_ID_LEN];
    char cms_ip[MAX_IP_LEN];
};


typedef struct _JpfmdsRegisterRes JpfmdsRegisterRes;
struct _JpfmdsRegisterRes
{
    JpfMsgErrCode	code;
    char           	domain_id[DOMAIN_ID_LEN];
    char           	mds_id[MDS_ID_LEN];	
    int         	keep_alive_time;
    int				pu_port;
    int				rtsp_port;

};

typedef struct _JpfMdsHeart JpfMdsHeart;
struct _JpfMdsHeart
{
    char		mds_id[MDS_ID_LEN];
    char		cms_ip[MAX_IP_LEN];
};

typedef struct _JpfMdsHeartRes JpfMdsHeartRes;
struct _JpfMdsHeartRes
{
    JpfMsgErrCode	code;
    char			server_time[TIME_INFO_LEN];
};

#endif
