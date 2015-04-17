#ifndef __IJDCUCOMMAND_H__
#define __IJDCUCOMMAND_H__

#include "IJDCUParseXml.h"

class IJDCUCommand;
//命令回调
typedef BOOL (WINAPI *FUNC_COMMAND_CALLBACK)(IJDCUCommand* pJDCUCommand, DWORD dwMsgID, IJXmlDocument* pXmlDoc, void* pParam);
//心跳回调
typedef BOOL (WINAPI *FUNC_HEART_CALLBACK)(IJDCUCommand* pJDCUCommand, void* pParam);
//离线通知
typedef BOOL (WINAPI *FUNC_OFFLINE_CALLBACK)(IJDCUCommand* pJDCUCommand, void* pParam);
//重连通知回调
typedef BOOL (WINAPI *FUNC_RECONNECT_CALLBACK)(IJDCUCommand* pJDCUCommand, void* pParam);

class IJDCUCommand
{
public:
	virtual void JDelete() = 0;

	virtual long JInit(const char* szServer, long lPort, long lTimeout = 10*1000) = 0;
	virtual void JUninit() = 0;

	virtual long RegCmdCallback(const char* szCmd, FUNC_COMMAND_CALLBACK func, void* pUserParam) = 0;
	virtual void UnRegCmd(long lRegId) = 0;

	virtual long RegHeartCallback(FUNC_HEART_CALLBACK func, void* pUserParam) = 0;
	virtual long RegReconnCallback(FUNC_RECONNECT_CALLBACK func, void* pUserParam) = 0;
	virtual long RegOfflineCallback(FUNC_OFFLINE_CALLBACK func, void* pUserParam) = 0;

	virtual long SendCmdMsg(DWORD dwMsgID, IJXmlDocument* pXmlDoc) = 0;
	virtual long SendCmdMsg(DWORD dwMsgID, IJXmlDocument* pSendXmlDoc, IJXmlDocument*& pRecXmlDoc, long lTimeout = 10*1000) = 0;

	virtual long ForceReconn() = 0;

	virtual long GetServerIP(char* szServer) = 0;
	virtual void SetHeartTime(DWORD dwTime) = 0;//(单位:秒)
};

IJDCUCommand* WINAPI JDCUCreateCommand();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//数据流回调
class IJNetStream;
typedef BOOL (WINAPI *FUNC_STREAM_DATA)(IJNetStream* pNetStream, void* pData, long lLen, void* pCustomParam);

class IJNetStream : public IJXJUnknown
{
public:
	virtual long JInit(const char* szIP, long lPort, long lTimeOut = 10*1000) = 0;
	virtual void JUinit() = 0;

	virtual void SetRecStreamCB(FUNC_STREAM_DATA func, void* pCustomParam) = 0;
	virtual long SendStream(const void* pData, long lLen) = 0; // 非阻塞
};

IJNetStream* WINAPI JDCUCreateStream();

//组播数据流回调
class IJMulticastStream;
typedef BOOL (WINAPI *FUNC_MULTICAST_STREAM)(IJMulticastStream* pMulticastStream, void* pData, long lLen, void* pCustomParam);

class IJMulticastStream : public IJXJUnknown
{
public:
	virtual long JInit(long lLocalPort, const char* szMulticastIP, long lPort) = 0;
	virtual void JUinit() = 0;

	virtual void SetRecStreamCB(FUNC_MULTICAST_STREAM func, void* pCustomParam) = 0;
	virtual long SendStream(const void* pData, long lLen) = 0;
};

IJMulticastStream* WINAPI JDCUCreateMulticastStream();

#endif