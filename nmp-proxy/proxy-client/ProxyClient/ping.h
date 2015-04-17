// ping.h

#include <iostream>
#include <iomanip>
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;


//IP数据报头
typedef struct
{
	unsigned char hdr_len :4;		// length of the header
	unsigned char version :4;		// version of IP
	unsigned char tos;				// type of service
	unsigned short total_len;		// total length of the packet
	unsigned short identifier;		// unique identifier
	unsigned short frag_and_flags;	// flags
	unsigned char ttl;				// time to live
	unsigned char protocol;			// protocol(TCP, UDP etc)
	unsigned short checksum;		// IPchecksum

	unsigned long sourceIP;			// source IP address
	unsigned long destIP;			//destination IP address

} IP_HEADER;


// ICMP 数据报头
typedef struct
{
	BYTE type;			// 8 位类型
	BYTE code;			// 8 位代码
	USHORT cksum;		// 16 位校验和
	USHORT id;			// 16 位标识符
	USHORT seq;			// 16 位序列号

} ICMP_HEADER, *PICMP_HEADER;


// 解码结果
typedef struct
{
	USHORT usSeqNO;				// 包序列号
	DWORD dwRoundTripTime;		// 往返时间
	in_addr dwIPaddr;			// 对端 IP 地址

} DECODE_RESULT;


#define ICMP_ECHO_REQUEST		8		// 请求回显
#define ICMP_ECHO_REPLY			0		// 回显应答
#define ICMP_TIMEOUT			11		// 传输超时

#define IP_HEADER_SIZE			sizeof(IP_HEADER)
#define DEF_ICMP_DATA_SIZE		32		// 默认 ICMP 数据部分长度
#define MAX_ICMP_PACKET_SIZE	1024	// 最大 ICMP 数据报的大小
#define ICMP_HEADER_SIZE		sizeof(ICMP_HEADER)
#define ICMP_PACK_SIZE			(ICMP_HEADER_SIZE + DEF_ICMP_DATA_SIZE)

#define SEND_PACKAGE_TIMES		4		// 发送包的次数


BOOL Ping(const char *pIP);
