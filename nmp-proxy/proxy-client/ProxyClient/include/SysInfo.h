#pragma once

//获取CPU序列号
const char* GetCPUID();

//获取磁盘序列号
const char* GetDiskID();

//获取网卡MAC,IP
struct stIPAddrString
{
	char szIP[16];
	char szMask[16];
};

struct stAdaptersInfo
{
	char szMac[32];
	BOOL bEnableDHCP;
	int  nIPCount;
	stIPAddrString IPAddr[16];

	int nGatewayCount;
	stIPAddrString GatewayAddr[4];
};

BOOL GetNetAdaptersInfo(stAdaptersInfo* pInfo,int* nCount);


//获取内存使用情况（单位M）
void GetMemoryStatus(DWORD& dwTotal , DWORD& dwUsed);

//获取进程内存使用情况（单位M）
BOOL GetProcMemoryUsed(DWORD dwPrcID,long& lUsed);

//获取CPU使用情况
BOOL GetCPUUsed(long& lUsed);

//获取进程CPU使用情况
BOOL GetProcessCPUUsed(DWORD dwPrcID,long& lUsed);

//获取CPU数量
DWORD GetdwNumberOfProcessors();

//互斥操作函数
LONG InterlockedAddLong(LONG volatile *lpAddend,LONG lAddValue);
LONG InterlockedExchangeLong(LONG volatile *lpValue,LONG lNewValue);
LONG InterlockedCompareExchangeLong(LONG volatile *lpValue,LONG lNewValue,LONG lCmpValue);
