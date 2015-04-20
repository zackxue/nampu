
#include "nmp_errno.h"

#include "nmp_packet.h"
#include "nmp_xmlmsg.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_info.h"
#include "nmp_proto_impl.h"


#define MAX_TMEP_BUFFER_SIZE        (4096*8)

static __inline__ JTime *
get_local_time(JTime *ts)
{
    time_t t;
    struct tm *local;

    t = time(NULL);
    local = localtime(&t);

    ts->year   = local->tm_year;
    ts->month  = local->tm_mon + 1;
    ts->date   = local->tm_mday;
    ts->hour   = local->tm_hour;
    ts->minute = local->tm_min;
    ts->second = local->tm_sec;

    return ts;
}

static int nmp_proto_private_check(char *start, char *end)
{
    int packet_size;
    nmp_proto_head_t *head;

    NMP_ASSERT(start && end);

    if (NMP_UNLIKELY(end - start <= sizeof(nmp_proto_head_t)))
        return 0;

    head = (nmp_proto_head_t*)start;
    if (!VALID_PROTO_HEAD(head))
        return -E_PACKET;

    packet_size = GET_PROTO_HEAD_L(head) + sizeof(nmp_proto_head_t);
    if (end - start >= packet_size)
        return packet_size;

    return 0;
}

static int 
nmp_proto_private_unpack(char *start, char *end, nmp_net_packinfo_t *info)
{
    nmp_proto_head_t *head;

    NMP_ASSERT(start && end && info);

    head = (nmp_proto_head_t*)start;

    info->total_packets = GET_PROTO_PACKETS(head);
    info->packet_no = GET_PROTO_PACKNO(head);
    info->start = start + sizeof(nmp_proto_head_t);
    info->size = GET_PROTO_HEAD_L(head);
    info->private_from_low_layer = (void*)GET_PROTO_HEAD_S(head);

    return 0;
}

static int
nmp_proto_private_pack(void *pack_data, size_t payload_size, char buff[], size_t buff_size)
{
    int seq;
    msg_t *msg;
    nmp_proto_head_t *head;
    NMP_ASSERT(pack_data != NULL && buff != NULL);

    if (NMP_UNLIKELY(buff_size < sizeof(nmp_proto_head_t)))
        return -E_BUFFSIZE;

    head = (nmp_proto_head_t*)buff;
    msg = (msg_t*)pack_data;
    seq = MSG_SEQ(msg);

    SET_PROTO_HEAD_M(head);
    SET_PROTO_HEAD_S(head, seq);
    SET_PROTO_HEAD_L(head, payload_size);
    SET_PROTO_HEAD_P(head, 1);
    SET_PROTO_HEAD_N(head, 1);

    return sizeof(nmp_proto_head_t);
}


static void *
nmp_proto_xml_get_payload(char *start,    size_t size, void *from_lower)
{
    int err = -1;
    msg_t *msg = NULL;
    NmpXmlMsg *xml_msg = NULL;
    
char tmp[MAX_TMEP_BUFFER_SIZE];
if (MAX_TMEP_BUFFER_SIZE > size)
{
    strncpy(tmp, start, size);
    tmp[size] = '\0';
}
else
{
    strncpy(tmp, start, MAX_TMEP_BUFFER_SIZE-1);
    tmp[MAX_TMEP_BUFFER_SIZE-1] = '\0';
}
printf("recv: %s\n", tmp);

    xml_msg = parse_xml(start, size, &err, 0);
    if (xml_msg)
    {
        msg = alloc_new_msg_2(xml_msg->id, xml_msg->priv_obj, 
        xml_msg->priv_size, (int)from_lower, xml_msg->destroy);

        nmp_xml_msg_destroy_2(xml_msg);
    }

    return msg;
}

static int
nmp_proto_xml_put_payload(void *pack_data,  char buf[], size_t size)
{
    int ret = -1;
    NmpXmlMsg *xml_msg = NULL;
    msg_t *msg = (msg_t*)pack_data;

    xml_msg = nmp_xml_msg_new(MSG_ID(msg), MSG_DATA(msg), MSG_DATA_SIZE(msg));
    if (xml_msg)
    {
        ret = create_xml(xml_msg, buf, size, 0);
        nmp_xml_msg_destroy(xml_msg);
    }

char tmp[MAX_TMEP_BUFFER_SIZE];
if (MAX_TMEP_BUFFER_SIZE > ret)
{
    strncpy(tmp, buf, ret);
    tmp[ret] = '\0';
}
else
{
    strncpy(tmp, buf, MAX_TMEP_BUFFER_SIZE-1);
    tmp[MAX_TMEP_BUFFER_SIZE-1] = '\0';
}
printf("send: %s\n", tmp);

    return ret;
}

static void
nmp_proto_xml_destroy_msg(void *msg, int err)
{
    NMP_ASSERT(msg);
    free_msg((msg_t*)msg);
}

static nmp_packet_proto_t nmp_packet_proto =
{
    .check  = nmp_proto_private_check,
    .unpack = nmp_proto_private_unpack,
    .pack   = nmp_proto_private_pack
};

static nmp_payload_proto_t nmp_payload_proto =
{
    .get_payload     = nmp_proto_xml_get_payload,
    .put_payload     = nmp_proto_xml_put_payload,
    .destroy_pointer = nmp_proto_xml_destroy_msg
};


static msg_t *
create_special_packet(void *init_data, PACKID_E id, int seq)
{
    msg_t *msg = NULL;
    proxy_plt_t *plt_info;
    proxy_reg_t *reg_info; 

    NMP_ASSERT(init_data);

    JUserHeart heart;
    MdsInfoPacket mds_info_packet;
    RegisterRequestPacket register_packet;
    HeartBeatRequestPacket heart_beat_packet;

    switch (id)
    {
        case PACK_PTL_REGISTER:
            reg_info = (proxy_reg_t*)init_data;
            memset(&register_packet, 0, sizeof(RegisterRequestPacket));

            strncpy(register_packet.pu_id, reg_info->pu_id, 
                sizeof(register_packet.pu_id)-1);
            strncpy(register_packet.cms_ip, reg_info->cms_host, 
                sizeof(register_packet.cms_ip)-1);
            strncpy(register_packet.dev_ip, reg_info->dev_host, 
                sizeof(register_packet.dev_ip)-1);
            register_packet.pu_type = reg_info->pu_type;

            msg = alloc_new_msg(REGISTER_REQUEST_ID, 
                    &register_packet, sizeof(RegisterRequestPacket), seq);
            break;

        case PACK_PTL_HEARTBEAT:
            reg_info = (proxy_reg_t*)init_data;
            memset(&heart_beat_packet, 0, sizeof(HeartBeatRequestPacket));

            strncpy(heart_beat_packet.pu_id, reg_info->pu_id, 
                sizeof(heart_beat_packet.pu_id)-1);
            strncpy(heart_beat_packet.dev_ip, reg_info->dev_host, 
                sizeof(heart_beat_packet.dev_ip)-1);

            msg = alloc_new_msg(HEART_BEAT_REQUEST_ID, 
                    &heart_beat_packet, sizeof(HeartBeatRequestPacket), seq);
            break;

        case PACK_CFG_REGISTER:
            msg = alloc_new_msg(USER_LONGI_RESULT_ID, init_data, sizeof(int), seq);
            break;

        case PACK_CFG_HEARTBEAT:
            memset(&heart, 0, sizeof(JUserHeart));
            get_local_time(&heart.server_time);

            msg = alloc_new_msg(USER_HEART_RESPONSE_ID, 
                    &heart, sizeof(JUserHeart), seq);
            break;

        case PACK_PLT_MDSCHANGED:
            break;

        case PACK_PLT_GETMDSINFO:
            plt_info = (proxy_plt_t*)init_data;
            memset(&mds_info_packet, 0, sizeof(MdsInfoPacket));

            strncpy(mds_info_packet.pu_id, plt_info->pu_id, 
                sizeof(mds_info_packet.pu_id)-1);
            strncpy(mds_info_packet.cms_ip, plt_info->cms_host, 
                sizeof(mds_info_packet.cms_ip)-1);

            msg = alloc_new_msg(GET_MDS_INFO_REQUEST_ID, 
                    &mds_info_packet, sizeof(MdsInfoPacket), seq);
            break;

        default:
            break;
    }

    return msg;
}
static int check_special_packet(msg_t *msg, PACKID_E id)
{
    int result = -1;
    NMP_ASSERT(msg);

    switch (id)
    {
        case PACK_PTL_REGISTER:
            if (REGISTER_RESPONSE_ID == MSG_ID(msg))
                result = 0;
            break;

        case PACK_PTL_HEARTBEAT:
            if (HEART_BEAT_RESPONSE_ID == MSG_ID(msg))
                result = 0;
            break;

        case PACK_CFG_REGISTER:
            if (USER_LONGI_REQUEST_ID == MSG_ID(msg))
                result = 0;
            break;

        case PACK_CFG_HEARTBEAT:
            if (USER_HEART_REQUEST_ID == MSG_ID(msg))
                result = 0;
            break;

        case PACK_PLT_MDSCHANGED:
            if (CHANGE_DISPATCH_REQUEST_ID == MSG_ID(msg))
                result = 0;
            break;

        case PACK_PLT_GETMDSINFO:
            if (GET_MDS_INFO_RESPONSE_ID == MSG_ID(msg))
                result = 0;
            break;

        default:
            break;
    }

    return result;
}
static int 
process_special_packet(msg_t *msg, void *init_data, PACKID_E id)
{
    int ret;
    proxy_plt_t *plt_info;

    NMP_ASSERT(msg && init_data);

    MdsInfoPacket *mds_info_packet;
    RegisterResponsePacket *register_packet;
    HeartBeatResponsePacket *heart_beat_packet;

    switch (id)
    {
        case PACK_PTL_REGISTER:
            BUG_ON(REGISTER_RESPONSE_ID != MSG_ID(msg));

            plt_info = (proxy_plt_t*)init_data;
            register_packet = (RegisterResponsePacket*)MSG_DATA(msg);

            ret = -register_packet->result.code;
            if (!ret)
            {
                strncpy(plt_info->mds_host, register_packet->mds_ip, 
                    sizeof(plt_info->mds_host)-1);
                plt_info->mds_port = register_packet->mds_port;
                plt_info->keep_alive_freq = register_packet->keep_alive;
            }
            break;

        case PACK_PTL_HEARTBEAT:
            BUG_ON(HEART_BEAT_RESPONSE_ID != MSG_ID(msg));

            plt_info = (proxy_plt_t*)init_data;
            heart_beat_packet = (HeartBeatResponsePacket*)MSG_DATA(msg);            

            ret = 0;
            break;

        case PACK_CFG_REGISTER:
            BUG_ON(USER_LONGI_REQUEST_ID != MSG_ID(msg));

            ret = 0;
            break;

        case PACK_CFG_HEARTBEAT:
            BUG_ON(USER_HEART_REQUEST_ID != MSG_ID(msg));

            ret = 0;
            break;

        case PACK_PLT_GETMDSINFO:
            BUG_ON(GET_MDS_INFO_RESPONSE_ID != MSG_ID(msg));

            plt_info = (proxy_plt_t*)init_data;
            mds_info_packet = (MdsInfoPacket*)MSG_DATA(msg);

            strncpy(plt_info->mds_host, mds_info_packet->mds_ip, 
                sizeof(plt_info->mds_host)-1);
            plt_info->mds_port = mds_info_packet->mds_port;

            ret = 0;
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}

packet_opt_t pkt_opt = 
{
    {
        &nmp_packet_proto,
        &nmp_payload_proto,
        NULL,
    },
    create_special_packet,
    check_special_packet,
    process_special_packet,
};


//~End
