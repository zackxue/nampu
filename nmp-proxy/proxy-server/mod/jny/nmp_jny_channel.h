#ifndef __J_JNY_CHANNEL_H__
#define __J_JNY_CHANNEL_H__


#include "nmp_jny_srv_impl.h"

typedef struct _frame_data_info
{
	long					nStreamFormat;						//1表示原始流，2表示TS混合流
	long					nESStreamType;						//原始流类型，1表示视频，2表示音频
	long					nEncoderType;						//编码格式
	long					nCameraNo;							//摄像机号，表示数据来自第几路
	unsigned long			nSequenceId;						//数据帧序号
	long					nFrameType;							//数据帧类型,1表示I帧, 2表示P帧, 0表示未知类型
	long long				nTimeStamp;							//数据采集时间戳，单位为毫秒
	long					nFrameRate;							//帧率
	long					nBitRate;							//当前码率　
	long					nImageFormatId;						//当前格式
	long					nImageWidth;						//视频宽度
	long					nImageHeight;						//视频高度
	long					nVideoSystem;						//当前视频制式
	unsigned long			nFrameBufLen;						//当前缓冲长度
	long					nStreamId;							// 流ID
	long					nTimezone;							// 时区
	long					nDaylightSavingTime;				//夏令时
	unsigned long			nDataLength;						//数据有效长度
	char					pszData[0];							//数据
}frame_data_info;


#ifdef __cplusplus
extern "C" {
#endif

extern stream_operation_t jny_strm_opt;


#ifdef __cplusplus
    }
#endif


#endif