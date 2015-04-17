// ping.cpp

#include "stdafx.h"
#include "ping.h"

// 产生校验和
static USHORT GenerateChecksum(USHORT *pBuff, int nSize)
{
	unsigned long cksum = 0;
	while(nSize > 1)
	{
		cksum += *pBuff++;
		nSize -= sizeof(USHORT);
	}
	if(nSize)
	{
		cksum += *(UCHAR*)pBuff;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (USHORT)(~cksum);
}



// 解码得到的数据报
static
BOOL DecodeIcmpResponse(char *pBuff, int nPacketSize, DECODE_RESULT& stDecodeResult)
{
	// 检查数据的合法性
	IP_HEADER *pIpHdr = (IP_HEADER*)pBuff;
	int nIpHdrLen = pIpHdr->hdr_len * 4;		// 以 4 字节为单位(具体见 IP 数据包的格式)
	if(nPacketSize < (int)(nIpHdrLen + sizeof(ICMP_HEADER)))
	{
		return FALSE;
	}
	
	// 按照 ICMP 包类型检查 id 字段和序列号以确定是否是程序应接收的 ICMP 包
	ICMP_HEADER *pIcmpHdr = (ICMP_HEADER*)(pBuff + nIpHdrLen);	// 剥去 IP 的头部
	USHORT usID, usSquNO;
	
	if(pIcmpHdr->type == ICMP_ECHO_REPLY)
	{
		usID = pIcmpHdr->id;
		usSquNO = pIcmpHdr->seq;
	}
	else if(pIcmpHdr->type == ICMP_TIMEOUT)
	{
		char *pInnerIpHdr = pBuff + nIpHdrLen + sizeof(ICMP_HEADER);	// 载荷中的 IP 头
		int nInnerIpHdrLen = ((IP_HEADER*)pInnerIpHdr)->hdr_len * 4;	// 载荷中的 IP 头长
		ICMP_HEADER *pInnerIcmpHdr = (ICMP_HEADER*)(pInnerIpHdr + nInnerIpHdrLen);	//载荷中的ICMP头
		usID = pInnerIcmpHdr->id;
		usSquNO = pInnerIcmpHdr->seq;
	}
	else
	{
		return FALSE;
	}

	if(usID != (USHORT)GetCurrentProcessId() || usSquNO != stDecodeResult.usSeqNO)
	{
		return FALSE;
	}

	// 处理正确收到的 ICMP 数据报
	if(pIcmpHdr->type == ICMP_ECHO_REPLY || pIcmpHdr->type == ICMP_TIMEOUT)
	{
		// 返回解码结果
		stDecodeResult.dwIPaddr.s_addr = pIpHdr->sourceIP;
		stDecodeResult.dwRoundTripTime = GetTickCount() - stDecodeResult.dwRoundTripTime;

		// 打印屏幕信息
		if(stDecodeResult.dwRoundTripTime)
		{
			// cout << setw(6) << stDecodeResult.dwRoundTripTime << "ms" << flush;
		}
		else
		{
			// cout << setw(6) << "< 1 " << "ms" << flush;
		}

		return TRUE;
	}

	return FALSE;
}



// 
BOOL Ping(const char *pIP)
{
	if(pIP == NULL)
	{
		return FALSE;
	}

	// 初始化 winsock2 环境
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{

		return FALSE;
	}

	if(getprotobyname("icmp") == NULL)
	{
		return FALSE;
	}
	
	// 将 IP 字符串转换为 IP 地址
	u_long ulDstIp = inet_addr(pIP);

	SOCKADDR_IN dstSockAddr;
	ZeroMemory(&dstSockAddr, sizeof(SOCKADDR_IN));
	dstSockAddr.sin_family = AF_INET;
	dstSockAddr.sin_addr.s_addr = ulDstIp;
	dstSockAddr.sin_port = htons(0);

	SOCKET RawSock = WSASocket(AF_INET, SOCK_RAW, 
		IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(RawSock == INVALID_SOCKET)
	{

		return FALSE;
	}

	// 设置接收环境变量, 超时时间为 1 秒
	int nTimeOut = 1000;
	if(setsockopt(RawSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR)
	{

		return FALSE;
	}

	// 设置发送环境变量, 超时时间为 1 秒
	if(setsockopt(RawSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR)
	{

		return FALSE;
	}

	// 创建 ICMP 包发送缓冲区和接收缓冲区
	char szIcmpSendBuff[ICMP_PACK_SIZE];
	char szIcmpRecvBuff[MAX_ICMP_PACKET_SIZE];

	DECODE_RESULT stDecodeResult;
	BOOL bPingSuccess = FALSE;		// 主要用于控制打印信息, 没有其他用途
	int nReceivedCount = 0;


	memset(szIcmpSendBuff, 0, sizeof(szIcmpSendBuff));
	memset(szIcmpRecvBuff, 0, sizeof(szIcmpSendBuff));

	// 填充待发送的 ICMP 包
	PICMP_HEADER pIcmpHdr = (PICMP_HEADER)szIcmpSendBuff;
	pIcmpHdr->type = ICMP_ECHO_REQUEST;
	pIcmpHdr->code = 0;
	pIcmpHdr->id = (USHORT)::GetCurrentProcessId();
	memset(szIcmpSendBuff + sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

	pIcmpHdr->seq = 0;
	pIcmpHdr->cksum = GenerateChecksum((USHORT*)szIcmpSendBuff, ICMP_PACK_SIZE);

	// 记录序列号和当前时间
	stDecodeResult.usSeqNO = ((ICMP_HEADER*)szIcmpSendBuff)->seq;
	stDecodeResult.dwRoundTripTime = GetTickCount();

	int nRet = ::sendto(RawSock, szIcmpSendBuff, ICMP_PACK_SIZE, 
		0, (SOCKADDR*)&dstSockAddr, sizeof(dstSockAddr));
	if(nRet == SOCKET_ERROR)
	{

		return FALSE;
	}
	sockaddr_in from;
	int nFromLen = sizeof(from);
	int nReadDataLen;


	nReadDataLen = recvfrom(RawSock, szIcmpRecvBuff, 
		MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &nFromLen);

	if(nReadDataLen != SOCKET_ERROR)
	{
		if(DecodeIcmpResponse(szIcmpRecvBuff, nReadDataLen, stDecodeResult))
		{
			bPingSuccess = TRUE;
			nReceivedCount++;
			
		}
	}
	else if(WSAGetLastError() == WSAETIMEDOUT)
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}

	if(bPingSuccess == TRUE)
	{
		return TRUE;
	}


	return 0;
}

