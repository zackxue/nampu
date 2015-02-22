//#include <arpa/inet.h>
#include "nmp_proto.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_proc_xml.h"

static gint
jpf_proto_private_check(gchar *start, gchar *end)
{
	gint packet_size;
	JpfProtoHead *head;
	G_ASSERT(start != NULL && end != NULL);

	if (G_UNLIKELY(end - start <= sizeof(JpfProtoHead)))
		return 0;

	head = (JpfProtoHead*)start;

	if (!VALID_PROTO_HEAD(head))
		return -E_PACKET;

	packet_size = GET_PROTO_HEAD_L(head) + sizeof(JpfProtoHead);

	if (end - start >= packet_size)
		return packet_size;

	return 0;
}


static gint
jpf_proto_private_unpack(gchar *start, gchar *end, JpfNetPackInfo *info)
{
	JpfProtoHead *head;
	G_ASSERT(start != NULL && end != NULL && info != NULL);

	head = (JpfProtoHead*)start;
	info->total_packets = GET_PROTO_PACKETS(head);
	info->packet_no = GET_PROTO_PACKNO(head);
	info->start = start + sizeof(JpfProtoHead);
	info->size = GET_PROTO_HEAD_L(head);
	info->private_from_low_layer = (void*)GET_PROTO_HEAD_S(head);
printf("head->magic=%0x,head->seq=%d,head->payload_len=%d\n",head->magic,GET_PROTO_HEAD_S(head) ,GET_PROTO_HEAD_L(head));
	return 0;
}


static gint
jpf_proto_private_pack(gpointer pack_data, gsize payload_len,
    gchar buff[], gsize buff_size)
{
	JpfSysMsg *sysmsg;
	JpfProtoHead *head;
	guint seq;
	G_ASSERT(pack_data != NULL && buff != NULL);

	if (G_UNLIKELY(buff_size < sizeof(JpfProtoHead)))
		return -E_BUFFSIZE;

	head = (JpfProtoHead*)buff;
	sysmsg = JPF_SYSMSG(pack_data);
	seq = MSG_SEQ(sysmsg);

	SET_PROTO_HEAD_M(head);
	SET_PROTO_HEAD_S(head, seq);
    SET_PROTO_HEAD_L(head, payload_len);
    SET_PROTO_HEAD_P(head, 1);
    SET_PROTO_HEAD_N(head, 1);

	return sizeof(JpfProtoHead);
}


static gpointer
jpf_proto_xml_get_payload(gchar *start, gsize size, gpointer from_lower)
{
	JpfSysMsg *packet;

	packet = jpf_get_sysmsg_from_xml(start, size, (guint)from_lower);
	if (G_UNLIKELY(!packet))
		jpf_warning("<un_payload> parse xml failed!");

	return packet;
}


static gint
jpf_proto_xml_put_payload(gpointer pack_data,  gchar buf[], gsize size)
{
	gint buff_size = (gint)size;
	G_ASSERT(pack_data != NULL && buf != NULL);

	gint ret = jpf_proto_create_xml_str(buf, &buff_size, (JpfSysMsg*)pack_data);
	printf("==============create xml len=%d,size=%d\n",ret,size);
	return ret;
}


static void
jpf_proto_xml_destroy_msg(gpointer msg, gint err)
{
	jpf_sysmsg_destroy((JpfSysMsg*)msg);
}



JpfPacketProto jxj_packet_proto =
{
	.check		= jpf_proto_private_check,
	.unpack		= jpf_proto_private_unpack,
	.pack		= jpf_proto_private_pack
};


JpfPayloadProto jxj_xml_proto =
{
	.get_payload	= jpf_proto_xml_get_payload,
	.put_payload	= jpf_proto_xml_put_payload,
	.destroy_pointer= jpf_proto_xml_destroy_msg
};


//:~ End
