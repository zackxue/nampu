//#include <arpa/inet.h>
#include "nmp_proto.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_proc_xml.h"

static gint
nmp_proto_private_check(gchar *start, gchar *end)
{
	gint packet_size;
	NmpProtoHead *head;
	G_ASSERT(start != NULL && end != NULL);

	if (G_UNLIKELY(end - start <= sizeof(NmpProtoHead)))
		return 0;

	head = (NmpProtoHead*)start;

	if (!VALID_PROTO_HEAD(head))
		return -E_PACKET;

	packet_size = GET_PROTO_HEAD_L(head) + sizeof(NmpProtoHead);

	if (end - start >= packet_size)
		return packet_size;

	return 0;
}


static gint
nmp_proto_private_unpack(gchar *start, gchar *end, NmpNetPackInfo *info)
{
	NmpProtoHead *head;
	G_ASSERT(start != NULL && end != NULL && info != NULL);

	head = (NmpProtoHead*)start;
	info->total_packets = GET_PROTO_PACKETS(head);
	info->packet_no = GET_PROTO_PACKNO(head);
	info->start = start + sizeof(NmpProtoHead);
	info->size = GET_PROTO_HEAD_L(head);
	info->private_from_low_layer = (void*)GET_PROTO_HEAD_S(head);
printf("head->magic=%0x,head->seq=%d,head->payload_len=%d\n",head->magic,GET_PROTO_HEAD_S(head) ,GET_PROTO_HEAD_L(head));
	return 0;
}


static gint
nmp_proto_private_pack(gpointer pack_data, gsize payload_len,
    gchar buff[], gsize buff_size)
{
	NmpSysMsg *sysmsg;
	NmpProtoHead *head;
	guint seq;
	G_ASSERT(pack_data != NULL && buff != NULL);

	if (G_UNLIKELY(buff_size < sizeof(NmpProtoHead)))
		return -E_BUFFSIZE;

	head = (NmpProtoHead*)buff;
	sysmsg = NMP_SYSMSG(pack_data);
	seq = MSG_SEQ(sysmsg);

	SET_PROTO_HEAD_M(head);
	SET_PROTO_HEAD_S(head, seq);
    SET_PROTO_HEAD_L(head, payload_len);
    SET_PROTO_HEAD_P(head, 1);
    SET_PROTO_HEAD_N(head, 1);

	return sizeof(NmpProtoHead);
}


static gpointer
nmp_proto_xml_get_payload(gchar *start, gsize size, gpointer from_lower)
{
	NmpSysMsg *packet;

	packet = nmp_get_sysmsg_from_xml(start, size, (guint)from_lower);
	if (G_UNLIKELY(!packet))
		nmp_warning("<un_payload> parse xml failed!");

	return packet;
}


static gint
nmp_proto_xml_put_payload(gpointer pack_data,  gchar buf[], gsize size)
{
	gint buff_size = (gint)size;
	G_ASSERT(pack_data != NULL && buf != NULL);

	gint ret = nmp_proto_create_xml_str(buf, &buff_size, (NmpSysMsg*)pack_data);
	printf("==============create xml len=%d,size=%d\n",ret,size);
	return ret;
}


static void
nmp_proto_xml_destroy_msg(gpointer msg, gint err)
{
	nmp_sysmsg_destroy((NmpSysMsg*)msg);
}



NmpPacketProto nmp_packet_proto =
{
	.check		= nmp_proto_private_check,
	.unpack		= nmp_proto_private_unpack,
	.pack		= nmp_proto_private_pack
};


NmpPayloadProto nmp_xml_proto =
{
	.get_payload	= nmp_proto_xml_get_payload,
	.put_payload	= nmp_proto_xml_put_payload,
	.destroy_pointer= nmp_proto_xml_destroy_msg
};


//:~ End
