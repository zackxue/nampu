// NvdcDll.h : NvdcDll DLL 的主头文件
//

#ifndef _NVDCDLL_H_
#define _NVDCDLL_H_

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "SNPlatOS.h"
#include "SN_Struct.h"
#include "SNError.h"
#if (defined(WIN32) || defined(_WIN32_WCE))
#include "EventConst.h"
#endif

#ifndef byte
	typedef unsigned char byte;
#endif
using namespace NVDC_STRUCT;
//namespace NVDC_FUC
//{
//获取SDK版本号
void		 SN_C_API NvdSdk_GetVersion(long* p_nVersion);
bool		 SN_C_API NvdSdk_Is_Handle_Valid(const long p_hHandle );
long		 SN_C_API NvdSdk_SetCharSet(const int p_nCharSet);//0:GB2312; 1:UTF8

//初始化接口，在使用Remote C接口之前必须调用Remote_Nvd_Init初始化函数，使用完后应当调用Remote_Nvd_UnInit释放环境
long		 SN_C_API Remote_Nvd_Init(long* p_handle, const ST_DeviceInfo* p_stDeviceInfo, const long p_nTransferProtocol);
long		 SN_C_API Remote_Nvd_InitEx(long* p_handle, const ST_DeviceInfoEx* p_stDeviceInfoEx, const long p_nTransferProtocol);
long		 SN_C_API Remote_Nvd_UnInit(long p_hHandle);
void		 SN_C_API Remote_Nvd_formatMessage(const int p_nErrorCode, char* p_pszErrorMessage, const long p_nMessageBufLen);

//RemoteCamera C Interface
long SN_C_API Remote_Camera_SetDefaultImageFormatId(long p_hHandle, const unsigned long p_nImageFormatId);
long SN_C_API Remote_Camera_SetDefaultFrameRate(long p_hHandle, const unsigned long p_nFrameRate);
long SN_C_API Remote_Camera_SetDefaultBitRate(long p_hHandle, const unsigned long p_nBitRateType, const unsigned long p_nBitRate);
long SN_C_API Remote_Camera_SetDefaultQuality(long p_hHandle, const unsigned long p_nQuality);
long SN_C_API Remote_Camera_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_Camera_Close(long p_hHandle);
long SN_C_API Remote_Camera_Read(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Camera_ReadTS(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Camera_StartAudio(long p_hHandle);
long SN_C_API Remote_Camera_StopAudio(long p_hHandle);
long SN_C_API Remote_Camera_GetVideoInfo(long p_hHandle, ST_VideoInfo* p_stVideoInfo);
long SN_C_API Remote_Camera_GetAudioInfo(long p_hHandle, ST_AudioInfo* p_stAudioInfo);
long SN_C_API Remote_Camera_GetVideoSystem(long p_hHandle, long* p_nVideoSystem);
long SN_C_API Remote_Camera_MakeKeyFrame(long p_hHandle);
long SN_C_API Remote_Camera_SetCurrentImageFormatId(long p_hHandle, const unsigned long p_nImageFormatId);
long SN_C_API Remote_Camera_GetCurrentImageFormatId(long p_hHandle, unsigned long* p_nImageFormatId);
long SN_C_API Remote_Camera_SetCurrentFrameRate(long p_hHandle, const unsigned long p_nFrameRate);
long SN_C_API Remote_Camera_GetCurrentFrameRate(long p_hHandle, unsigned long* p_nFrameRate);
long SN_C_API Remote_Camera_SetCurrentConfirmBitRate(long p_hHandle, const unsigned long p_nBitRateType, const unsigned long p_nConfirmBitRate);
long SN_C_API Remote_Camera_GetCurrentConfirmBitRate(long p_hHandle, unsigned long* p_nBitRateType, unsigned long* p_nConfirmBitRate);
long SN_C_API Remote_Camera_SetCurrentQuant(long p_hHandle, const unsigned long p_nQuant);
long SN_C_API Remote_Camera_GetCurrentQuant(long p_hHandle, unsigned long* p_nQuant);
long SN_C_API Remote_Camera_SetCurrentQuality(long p_hHandle, const unsigned long p_nQuality);
long SN_C_API Remote_Camera_GetCurrentQuality(long p_hHandle, unsigned long* p_nQuality);

//RemoteCamera2 C Interface
long SN_C_API Remote_Camera2_SetDefaultStreamId(long p_hHandle, const unsigned long p_nStreamId);
long SN_C_API Remote_Camera2_SetMulticastFlag(long p_hHandle, const bool p_bFlag);
bool SN_C_API Remote_Camera2_GetMulticastFlag(long p_hHandle);
long SN_C_API Remote_Camera2_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_Camera2_Close(long p_hHandle);
long SN_C_API Remote_Camera2_Read(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Camera2_ReadTS(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Camera2_StartAudio(long p_hHandle);
long SN_C_API Remote_Camera2_StopAudio(long p_hHandle);
long SN_C_API Remote_Camera2_SetCurrentStreamId(long p_hHandle, const unsigned long p_nStreamId);
long SN_C_API Remote_Camera2_GetCurrentStreamId(long p_hHandle, unsigned long* p_nStreamId);
long SN_C_API Remote_Camera2_MakeKeyFrame(long p_hHandle);
long SN_C_API Remote_Camera2_Pause(long p_hHandle);
long SN_C_API Remote_Camera2_Resume(long p_hHandle);
long SN_C_API Remote_Camera2_KeepAlive(long p_hHandle);

//RemoteCamera3 C Interface
long SN_C_API Remote_Camera3_SetDefaultStreamId(long p_hHandle, const unsigned long p_nStreamId);
long SN_C_API Remote_Camera3_SetStreamFormat(long p_hHandle, const int p_nStreamFormat);
long SN_C_API Remote_Camera3_SetMulticastFlag(long p_hHandle, const bool p_bFlag);
bool SN_C_API Remote_Camera3_GetMulticastFlag(long p_hHandle);
long SN_C_API Remote_Camera3_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_Camera3_Close(long p_hHandle);


long SN_C_API Remote_Camera3_SetMessageCallback(long p_hHandle, 
												 long (CALLBACK* fMessageCallback)(long p_hHandle, const int p_nMessageID, void* pUserData), 
												 void* pUserData);
long SN_C_API Remote_Camera3_SetAVDateCallback(long p_hHandle, 
												 long (CALLBACK* fAVDateCallback)(long p_hHandle, ST_AVFrameData* p_stAVFrameData, void* pUserData), 
												 void* pUserData);

long SN_C_API Remote_Camera3_StartAudio(long p_hHandle);
long SN_C_API Remote_Camera3_StopAudio(long p_hHandle);
long SN_C_API Remote_Camera3_SetCurrentStreamId(long p_hHandle, const unsigned long p_nStreamId);
long SN_C_API Remote_Camera3_GetCurrentStreamId(long p_hHandle, unsigned long* p_nStreamId);
long SN_C_API Remote_Camera3_MakeKeyFrame(long p_hHandle);
long SN_C_API Remote_Camera3_Pause(long p_hHandle);
long SN_C_API Remote_Camera3_Resume(long p_hHandle);
long SN_C_API Remote_Camera3_KeepAlive(long p_hHandle);


//RemoteMicrophone C Interface
long SN_C_API Remote_Microphone_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_Microphone_Close(long p_hHandle);
long SN_C_API Remote_Microphone_Read(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Microphone_ReadTS(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Microphone_GetAudioInfo(long p_hHandle, ST_AudioInfo* p_stAudioInfo);

//RemoteMicrophone2 C Interface
long SN_C_API Remote_Microphone2_SetEncodeType(long p_hHandle, const int p_nEncodeType);
long SN_C_API Remote_Microphone2_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_Microphone2_Close(long p_hHandle);
long SN_C_API Remote_Microphone2_Read(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Microphone2_ReadTS(long p_hHandle, ST_AVFrameData* p_stAVFrameData);

//RemoteAudioPlayer C Interface(在设备端播放音频）
long SN_C_API Remote_AudioPlayer_Open(long p_hHandle);
long SN_C_API Remote_AudioPlayer_Close(long p_hHandle);
long SN_C_API Remote_AudioPlayer_Write(long p_hHandle, const ST_AVFrameData* p_pstAVFrameData);
long SN_C_API Remote_AudioPlayer_WriteTS(long p_hHandle, const ST_AVFrameData* p_pstAVFrameData);

//RemoteAudioPlayer C Interface(在设备端播放音频）
long SN_C_API Remote_AudioPlayer2_SetEncodeType(long p_hHandle, const int p_nEncodeType);
long SN_C_API Remote_AudioPlayer2_Open(long p_hHandle);
long SN_C_API Remote_AudioPlayer2_Close(long p_hHandle);
long SN_C_API Remote_AudioPlayer2_Write(long p_hHandle, const ST_AVFrameData* p_pstAVFrameData);


//RemoteRS485 C Interface
long SN_C_API Remote_RS485_SetTimeout(long p_hHandle, const int p_nTimeout);
long SN_C_API Remote_RS485_Open(long p_hHandle);
long SN_C_API Remote_RS485_Close(long p_hHandle);
long SN_C_API Remote_RS485_SetComId(long p_hHandle, const int p_nComId);
long SN_C_API Remote_RS485_SetOpenMode(long p_hHandle, const char p_btComOpenMode);
long SN_C_API Remote_RS485_Write(long p_hHandle, const char* p_pszWriteDate, const int p_nDateLen);

long SN_C_API Remote_RS485Ex_Open(long p_hHandle);
long SN_C_API Remote_RS485Ex_Close(long p_hHandle);
long SN_C_API Remote_RS485Ex_SetRS485Device(long p_hHandle, const ST_RS485Param* p_pstRS485Device);
long SN_C_API Remote_RS485Ex_Read(long p_hHandle, char* p_pszReadBuf, const int p_nBufLen);
long SN_C_API Remote_RS485Ex_Write(long p_hHandle, const char* p_pszWriteDate, const int p_nDateLen);

//RemoteSensor C Interface
long SN_C_API Remote_Sensor_SetTimeout(long p_hHandle, const int p_nTimeout);
long SN_C_API Remote_Sensor_Open(long p_hHandle);
long SN_C_API Remote_Sensor_Close(long p_hHandle);
long SN_C_API Remote_Sensor_GetSensorType(long p_hHandle, int &p_nSensorType);
long SN_C_API Remote_Sensor_SetParameters(long p_hHandle, const char* p_pszParameters, const int p_nLength);
long SN_C_API Remote_Sensor_GetAllParameters(long p_hHandle, char *p_pszParameterBuffer,int p_nParameterBufferLen, int* p_nOutDataLen);
long SN_C_API Remote_Sensor_ResetParameters(long p_hHandle);
long SN_C_API Remote_Sensor_Save(long p_hHandle);
long SN_C_API Remote_Sensor_SetSecretArea(long p_hHandle, const char* p_pszSecretArea, const int p_nLength);

long SN_C_API Remote_Sensor2_SetTimeout(long p_hHandle, const int p_nTimeout);
long SN_C_API Remote_Sensor2_Open(long p_hHandle);
long SN_C_API Remote_Sensor2_Close(long p_hHandle);
long SN_C_API Remote_Sensor2_SetBrightness(long p_hHandle, const int p_nValue);
long SN_C_API Remote_Sensor2_GetBrightness(long p_hHandle, int* p_pnValue);
long SN_C_API Remote_Sensor2_SetSharpness(long p_hHandle, const int p_nValue);
long SN_C_API Remote_Sensor2_GetSharpness(long p_hHandle, int* p_pnValue);
long SN_C_API Remote_Sensor2_SetHue(long p_hHandle, const int p_nValue);
long SN_C_API Remote_Sensor2_GetHue(long p_hHandle, int* p_pnValue);
long SN_C_API Remote_Sensor2_SetContrast(long p_hHandle, const int p_nValue);
long SN_C_API Remote_Sensor2_GetContrast(long p_hHandle, int* p_pnValue);
long SN_C_API Remote_Sensor2_SetSaturation(long p_hHandle, const int p_nValue);
long SN_C_API Remote_Sensor2_GetSaturation(long p_hHandle, int* p_pnValue);
long SN_C_API Remote_Sensor2_ResetParameters(long p_hHandle);
long SN_C_API Remote_Sensor2_Save(long p_hHandle);


//RemoteSensorUI C Interface

#if (defined(WIN32) || defined(_WIN32_WCE))
long SN_C_API Remote_SensorUI_show(long p_hHandle,int p_nLanguage);
#endif

//RemotePTZ C Interface
long SN_C_API Remote_PTZ_SetDeviceId(long p_hHandle, const long p_nPTZId);
long SN_C_API Remote_PTZ_SetProtocol(long p_hHandle, const long p_nPTZProtocol);
long SN_C_API Remote_PTZ_SetCOMId(long p_hHandle, const long p_nCOMId);
long SN_C_API Remote_PTZ_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_PTZ_Close(long p_hHandle);
long SN_C_API Remote_PTZ_Stop(long p_hHandle);
long SN_C_API Remote_PTZ_RotateUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateLeft(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateRight(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateLeftUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateLeftUpEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZ_RotateLeftDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateLeftDownEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZ_RotateRightUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateRightUpEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZ_RotateRightDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZ_RotateRightDownEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZ_ZoomIn(long p_hHandle);
long SN_C_API Remote_PTZ_ZoomOut(long p_hHandle);
long SN_C_API Remote_PTZ_FocusFar(long p_hHandle);
long SN_C_API Remote_PTZ_FocusNear(long p_hHandle);
long SN_C_API Remote_PTZ_IrisIncrease(long p_hHandle);
long SN_C_API Remote_PTZ_IrisDecrease(long p_hHandle);
long SN_C_API Remote_PTZ_PresetSet(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_PresetInvoke(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_PresetRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_Scan(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_ScanRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_ScanSetStartPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_ScanSetStopPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_RunAutoFocus(long p_hHandle);
long SN_C_API Remote_PTZ_RunAutoIris(long p_hHandle);
long SN_C_API Remote_PTZ_SetAutoStudyStartPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_SetAutoStudyEndPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_RunAutoStudy(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_AutoStudyRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZ_Reset(long p_hHandle);
long SN_C_API Remote_PTZ_SendOperation(long p_hHandle, const byte* p_bytePTZCommand, const long p_nCommandLen);

//RemotePTZEx C Interface
long SN_C_API Remote_PTZEx_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_PTZEx_Close(long p_hHandle);
long SN_C_API Remote_PTZEx_Stop(long p_hHandle);
long SN_C_API Remote_PTZEx_RotateUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateLeft(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateRight(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateLeftUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateLeftUpEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZEx_RotateLeftDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateLeftDownEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZEx_RotateRightUp(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateRightUpEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZEx_RotateRightDown(long p_hHandle, const long p_nSpeedValue);
long SN_C_API Remote_PTZEx_RotateRightDownEx(long p_hHandle, const long p_nPanSpeedValue, const long p_nTiltSpeedValue);
long SN_C_API Remote_PTZEx_ZoomIn(long p_hHandle);
long SN_C_API Remote_PTZEx_ZoomOut(long p_hHandle);
long SN_C_API Remote_PTZEx_FocusFar(long p_hHandle);
long SN_C_API Remote_PTZEx_FocusNear(long p_hHandle);
long SN_C_API Remote_PTZEx_IrisIncrease(long p_hHandle);
long SN_C_API Remote_PTZEx_IrisDecrease(long p_hHandle);
long SN_C_API Remote_PTZEx_PresetSet(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_PresetInvoke(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_PresetRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_Scan(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_ScanRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_ScanSetStartPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_ScanSetStopPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_RunAutoFocus(long p_hHandle);
long SN_C_API Remote_PTZEx_RunAutoIris(long p_hHandle);
long SN_C_API Remote_PTZEx_setAutoStudyStartPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_setAutoStudyEndPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_RunAutoStudy(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_AutoStudyRemove(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_threeDimensionalPositioning(long p_hHandle, const long p_nX, const long p_nY, const long p_nZoomRate);
long SN_C_API Remote_PTZEx_SetTourStartPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_AddTourPoint(long p_hHandle, const long p_nPresetValue, const long p_nSpeedValue, const long p_nTimeValue);
long SN_C_API Remote_PTZEx_SetTourEndPoint(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_RunTour(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_StopTour(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_DeleteTour(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_SetKeeper(long p_hHandle, const long p_nTypeValue, const long p_nIdValue, const long p_nTimeValue);
long SN_C_API Remote_PTZEx_RunKeeper(long p_hHandle, const long p_nValue);
long SN_C_API Remote_PTZEx_Reset(long p_hHandle);
long SN_C_API Remote_PTZEx_RunBrush(long p_hHandle);
long SN_C_API Remote_PTZEx_OpenLight(long p_hHandle);
long SN_C_API Remote_PTZEx_CloseLight(long p_hHandle);

long SN_C_API Remote_PTZEx_SetInfrared(long p_hHandle, const int p_nMode, const int p_nNear, const int p_nMiddle, const int p_nFar);
long SN_C_API Remote_PTZEx_SetInfraredV2(long p_hHandle, const int p_nOpenMode, const int p_nBrightnessMode, const int p_nNear, const int p_nMiddle, const int p_nFar);
long SN_C_API Remote_PTZEx_SetNorthPostion(long p_hHandle);
long SN_C_API Remote_PTZEx_SetPostion(long p_hHandle, const int p_nType, const float p_nPan, const float p_nTilt, const float p_nZoom);
long SN_C_API Remote_PTZEx_GetPostion(long p_hHandle, float* p_nPan, float* p_nTilt, float* p_nZoom, int* p_nDirection);

long SN_C_API Remote_PTZEx_SendOperation(long p_hHandle, const char* p_bytePTZCommand, const long p_nCommandLen);

//RemoteVideoFile C Interface (文件下载）
long SN_C_API Remote_VideoFile_Open(long p_hHandle, const char* p_pszFileName);	//此接口废弃
long SN_C_API Remote_VideoFile_OpenEx(long p_hHandle, const ST_RecordInfo* p_stRecordInfo);
long SN_C_API Remote_VideoFile_Close(long p_hHandle);
long SN_C_API Remote_VideoFile_Read(long p_hHandle, char* p_pszBuf, const unsigned int p_nBufSize);
long SN_C_API Remote_VideoFile_Seek(long p_hHandle, const INT64  p_nOffset, const unsigned char p_nSeekMode);
long SN_C_API Remote_VideoFile_GetFileLength(long p_hHandle, INT64 * p_nLength); 

#if (defined(WIN32) || defined(_WIN32_WCE))

//RemoteLivePlayer C Interface (实时预览）
long SN_C_API Remote_LivePlayer_SetUseTimeStamp(long p_hHandle, const bool bUseFlag);
long SN_C_API Remote_LivePlayer_SetPlayBufferSize(long p_hHandle, const unsigned long p_nSecSize);//p_nSecSize, 以毫秒(s)为单位 必须在0秒-最大5000毫秒缓冲。注：播放缓冲越大，播放越平滑，但延时将增大，当为0时，无论是否用时间戳，视频将立即被播放
long SN_C_API Remote_LivePlayer_SetStretchMode(long p_hHandle, const bool p_bStretchMode);
long SN_C_API Remote_LivePlayer_SetNotifyWindow(long p_hHandle, const unsigned long p_nNotifyWnd, const unsigned long p_nMessage, const void * p_pParam);
long SN_C_API Remote_LivePlayer_SetTryConnectCount(long p_hHandle, const unsigned long p_nTryCount );
long SN_C_API Remote_LivePlayer_SetAutoConnectFlag(long p_hHandle, const bool p_bAutoFlag );
long SN_C_API Remote_LivePlayer_SetDefaultImageFormatId(long p_hHandle, const unsigned long p_nImageFormatId);
long SN_C_API Remote_LivePlayer_SetDefaultFrameRate(long p_hHandle, const unsigned long p_nFrameRate);
long SN_C_API Remote_LivePlayer_SetDefaultBitRate(long p_hHandle, const unsigned long p_nBitRateType, const unsigned long p_nBitRate);
long SN_C_API Remote_LivePlayer_SetDefaultQuality(long p_hHandle, const unsigned long p_nQuality);
long SN_C_API Remote_LivePlayer_SetAutoResizeFlag(long p_hHandle, const bool p_bAutoResizeFlag = true);
long SN_C_API Remote_LivePlayer_SetVideoWindow(long p_hHandle, const unsigned long p_hDisplayWnd, const long x, const long y, const long width, const long height);
long SN_C_API Remote_LivePlayer_SetVideoWindowEx(long p_hHandle, const unsigned long p_hDisplayWnd);
long SN_C_API Remote_LivePlayer_SetDrawCallBack(long p_hHandle, long hHandle,long (CALLBACK *DrawCallBack)(long hHandle, HDC hDc, void *pUserData), void* pUserData);
long SN_C_API Remote_LivePlayer_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_LivePlayer_Close(long p_hHandle);
long SN_C_API Remote_LivePlayer_Play(long p_hHandle);
long SN_C_API Remote_LivePlayer_Pause(long p_hHandle);
long SN_C_API Remote_LivePlayer_GetPlayStatus(long p_hHandle, long* p_nPlayStatus);
long SN_C_API Remote_LivePlayer_PlaySound(long p_hHandle);
long SN_C_API Remote_LivePlayer_StopSound(long p_hHandle);
long SN_C_API Remote_LivePlayer_IsOnSound(long p_hHandle, bool* p_bOnSound);
long SN_C_API Remote_LivePlayer_ResizeWindow(long p_hHandle, const long x, const long y, const long width, const long height);
long SN_C_API Remote_LivePlayer_Refresh(long p_hHandle);
long SN_C_API Remote_LivePlayer_GetDeviceInfo(long p_hHandle, ST_DeviceInfo* p_stDeviceInfo);
long SN_C_API Remote_LivePlayer_GetDeviceInfoEx(long p_hHandle, ST_DeviceInfoEx* p_stDeviceInfoEx);
long SN_C_API Remote_LivePlayer_SetRecorderFile(long p_hHandle, const char* p_pszFileName);
long SN_C_API Remote_LivePlayer_StartRecord(long p_hHandle);
long SN_C_API Remote_LivePlayer_StopRecord(long p_hHandle);
long SN_C_API Remote_LivePlayer_GetRecorderStatus(long p_hHandle, long* p_nStatus);
long SN_C_API Remote_LivePlayer_SnapShot(long p_hHandle, const char* p_pszFileName);
long SN_C_API Remote_LivePlayer_GetVideoSystem(long p_hHandle, long* p_nVideoSystem);
long SN_C_API Remote_LivePlayer_SetCurrentFrameRate(long p_hHandle, const unsigned long p_nFrameRate);
long SN_C_API Remote_LivePlayer_GetCurrentFrameRate(long p_hHandle, unsigned long* p_nFrameRate);
long SN_C_API Remote_LivePlayer_SetCurrentImageFormatId(long p_hHandle, const unsigned long p_nCurrentImageFormatId);
long SN_C_API Remote_LivePlayer_GetCurrentImageFormatId(long p_hHandle, unsigned long* p_nCurrentImageFormatId);
long SN_C_API Remote_LivePlayer_SetCurrentConfirmBitRate(long p_hHandle, const unsigned long p_nBitRateType, const unsigned long p_nConfirmBitRate);
long SN_C_API Remote_LivePlayer_GetCurrentConfirmBitRate(long p_hHandle, unsigned long* p_nBitRateType, unsigned long* p_nConfirmBitRate);
long SN_C_API Remote_LivePlayer_SetCurrentQuant(long p_hHandle, const unsigned long p_nQuant);
long SN_C_API Remote_LivePlayer_GetCurrentQuant(long p_hHandle, unsigned long* p_nQuant);
long SN_C_API Remote_LivePlayer_SetCurrentQuality(long p_hHandle, const unsigned long p_nQuality);
long SN_C_API Remote_LivePlayer_GetCurrentQuality(long p_hHandle, unsigned long* p_nQuality);
long SN_C_API Remote_LivePlayer_SetCurrentBrightness(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer_GetCurrentBrightness(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer_SetCurrentContrast(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer_GetCurrentContrast(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer_SetCurrentHue(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer_GetCurrentHue(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer_SetCurrentSaturation(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer_GetCurrentSaturation(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer_ResetPictureAdjustFilter(long p_hHandle);
long SN_C_API Remote_LivePlayer_GetCurrentBitRate(long p_hHandle, unsigned long* p_nBitRate);
long SN_C_API Remote_LivePlayer_GetPictureSize(long p_hHandle, long* p_nWidth, long* p_nHeight);
long SN_C_API Remote_LivePlayer_SetVolume(long p_hHandle, const long p_nVolume);
long SN_C_API Remote_LivePlayer_GetVolume(long p_hHandle, long* p_nVolume);
long SN_C_API Remote_LivePlayer_ZoomInVideoEx(long p_hHandle, const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height);
long SN_C_API Remote_LivePlayer_ZoomInVideo(long p_hHandle);
long SN_C_API Remote_LivePlayer_ZoomOutVideo(long p_hHandle);
long SN_C_API Remote_LivePlayer_RestoreVideo(long p_hHandle);

//RemoteLivePlayer2 C Interface (实时预览）
long SN_C_API Remote_LivePlayer2_SetUseTimeStamp(long p_hHandle, const bool p_bUseFlag);
long SN_C_API Remote_LivePlayer2_SetPlayBufferSize(long p_hHandle, const unsigned long p_nSecSize);//p_nSecSize, 以毫秒(s)为单位 必须在0秒-最大5000毫秒缓冲。注：播放缓冲越大，播放越平滑，但延时将增大，当为0时，无论是否用时间戳，视频将立即被播放
long SN_C_API Remote_LivePlayer2_SetStretchMode(long p_hHandle, const bool p_bStretchMode);
long SN_C_API Remote_LivePlayer2_SetNotifyWindow(long p_hHandle, const unsigned long p_nNotifyWnd, const unsigned long p_nMessage, const void * p_pParam);
long SN_C_API Remote_LivePlayer2_SetTryConnectCount(long p_hHandle, const unsigned long p_nTryCount );
long SN_C_API Remote_LivePlayer2_SetAutoConnectFlag(long p_hHandle, const bool p_bAutoFlag );
long SN_C_API Remote_LivePlayer2_SetDefaultStreamId(long p_hHandle, const unsigned long p_nStreamId);
long SN_C_API Remote_LivePlayer2_SetAutoResizeFlag(long p_hHandle, const bool p_bAutoResizeFlag = true);
long SN_C_API Remote_LivePlayer2_SetVideoWindow(long p_hHandle, const unsigned long p_hDisplayWnd, const long x, const long y, const long width, const long height);
long SN_C_API Remote_LivePlayer2_SetVideoWindowEx(long p_hHandle, const unsigned long p_hDisplayWnd);
long SN_C_API Remote_LivePlayer2_SetDrawCallBack(long p_hHandle, long hHandle,long (CALLBACK *DrawCallBack)(long hHandle, HDC hDc, void *pUserData), void* pUserData);
long SN_C_API Remote_LivePlayer2_Open(long p_hHandle, const long p_nCameraID);
long SN_C_API Remote_LivePlayer2_Close(long p_hHandle);
long SN_C_API Remote_LivePlayer2_Play(long p_hHandle);
long SN_C_API Remote_LivePlayer2_Pause(long p_hHandle);
long SN_C_API Remote_LivePlayer2_GetPlayStatus(long p_hHandle, long* p_nPlayStatus);
long SN_C_API Remote_LivePlayer2_PlaySound(long p_hHandle);
long SN_C_API Remote_LivePlayer2_StopSound(long p_hHandle);
long SN_C_API Remote_LivePlayer2_IsOnSound(long p_hHandle, bool* p_bOnSound);
long SN_C_API Remote_LivePlayer2_ResizeWindow(long p_hHandle, const long x, const long y, const long width, const long height);
long SN_C_API Remote_LivePlayer2_Refresh(long p_hHandle);
long SN_C_API Remote_LivePlayer2_GetDeviceInfoEx(long p_hHandle, ST_DeviceInfoEx* p_stDeviceInfoEx);
long SN_C_API Remote_LivePlayer2_SnapShot(long p_hHandle, const char* p_pszFileName);
long SN_C_API Remote_LivePlayer2_SetCurrentStreamId(long p_hHandle, const unsigned long p_nCurrentStreamId);
long SN_C_API Remote_LivePlayer2_GetCurrentStreamId(long p_hHandle, unsigned long* p_nCurrentStreamId);
long SN_C_API Remote_LivePlayer2_SetCurrentBrightness(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer2_GetCurrentBrightness(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer2_SetCurrentContrast(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer2_GetCurrentContrast(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer2_SetCurrentHue(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer2_GetCurrentHue(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer2_SetCurrentSaturation(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_LivePlayer2_GetCurrentSaturation(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_LivePlayer2_ResetPictureAdjustFilter(long p_hHandle);
long SN_C_API Remote_LivePlayer2_GetCurrentFrameRate(long p_hHandle, unsigned long* p_nFrameRate);
long SN_C_API Remote_LivePlayer2_GetCurrentBitRate(long p_hHandle, unsigned long* p_nBitRate);
long SN_C_API Remote_LivePlayer2_GetPictureSize(long p_hHandle, long* p_nWidth, long* p_nHeight);
long SN_C_API Remote_LivePlayer2_SetVolume(long p_hHandle, const long p_nVolume);
long SN_C_API Remote_LivePlayer2_GetVolume(long p_hHandle, long* p_nVolume);
long SN_C_API Remote_LivePlayer2_ZoomInVideoEx(long p_hHandle, const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height);
long SN_C_API Remote_LivePlayer2_ZoomInVideo(long p_hHandle);
long SN_C_API Remote_LivePlayer2_ZoomOutVideo(long p_hHandle);
long SN_C_API Remote_LivePlayer2_RestoreVideo(long p_hHandle);

//RemoteInterphone C Interface (语音对讲）
long SN_C_API Remote_Interphone_Start(long p_hHandle);
long SN_C_API Remote_Interphone_Stop(long p_hHandle);

//RemoteInterphone2 C Interface (语音对讲）
long SN_C_API Remote_Interphone2_SetEncodeType(long p_hHandle, const int p_nEncodeType);
long SN_C_API Remote_Interphone2_Start(long p_hHandle);
long SN_C_API Remote_Interphone2_Stop(long p_hHandle);

////RemotePlayBack C Interface (远端文件回放）
long SN_C_API Remote_PlayBack_SetStretchMode(long p_hHandle, const bool p_bStretchMode);
long SN_C_API Remote_PlayBack_SetNotifyWindow(long p_hHandle, const unsigned long p_nNotifyWnd, const unsigned long p_nMessage, const void * p_pParam);
long SN_C_API Remote_PlayBack_SetTryConnectCount(long p_hHandle, const unsigned long p_nTryCount );
long SN_C_API Remote_PlayBack_SetAutoConnectFlag(long p_hHandle, const bool p_bAutoFlag );
long SN_C_API Remote_PlayBack_SetVideoWindow(long p_hHandle, const unsigned long p_hDisplayWnd, const long x, const long y, const long width, const long height);
long SN_C_API Remote_PlayBack_SetPlayFileName(long p_hHandle, const char* p_pszPlayFileName);  //此接口废弃
long SN_C_API Remote_PlayBack_SetRecordInfo(long p_hHandle, const ST_RecordInfo* p_stRecordInfo);
long SN_C_API Remote_PlayBack_Open(long p_hHandle);
long SN_C_API Remote_PlayBack_Close(long p_hHandle);
long SN_C_API Remote_PlayBack_Play(long p_hHandle);
long SN_C_API Remote_PlayBack_Pause(long p_hHandle);
long SN_C_API Remote_PlayBack_Stop(long p_hHandle);
long SN_C_API Remote_PlayBack_ResizeWindow(long p_hHandle, const long x, const long y, const long width, const long height);
long SN_C_API Remote_PlayBack_Refresh(long p_hHandle);
long SN_C_API Remote_PlayBack_GetPlayStatus(long p_hHandle, long* p_nPlayStatus);
long SN_C_API Remote_PlayBack_PlayNextFrame(long p_hHandle);
long SN_C_API Remote_PlayBack_PlayPreviousFrame(long p_hHandle);
long SN_C_API Remote_PlayBack_PlaySound(long p_hHandle);
long SN_C_API Remote_PlayBack_StopSound(long p_hHandle);
long SN_C_API Remote_PlayBack_IsOnSound(long p_hHandle, bool* p_bOnSound);
long SN_C_API Remote_PlayBack_SetVolume(long p_hHandle, const long p_nVolume);
long SN_C_API Remote_PlayBack_GetVolume(long p_hHandle, long* p_nVolume);
long SN_C_API Remote_PlayBack_GetDuration(long p_hHandle, long* p_nDuration);
long SN_C_API Remote_PlayBack_GetPlayPosByTime(long p_hHandle, long* p_nDuration);
long SN_C_API Remote_PlayBack_SetPlayPosByTime(long p_hHandle, const long p_nDuration);
long SN_C_API Remote_PlayBack_GetPlayPosByPercent(long p_hHandle, long* p_nPercent);
long SN_C_API Remote_PlayBack_SetPlayPosByPercent(long p_hHandle, const long p_nPercent);
long SN_C_API Remote_PlayBack_SetSpeed( long p_hHandle, float p_nSpeed );
long SN_C_API Remote_PlayBack_GetSpeed( long p_hHandle,float* p_nSpeed );
long SN_C_API Remote_PlayBack_GetFileTotalFrames(long p_hHandle, long* p_nTotalFrames);
long SN_C_API Remote_PlayBack_GetPlayedFrames(long p_hHandle, long* p_nPlayedFrames);
long SN_C_API Remote_PlayBack_SetCurrentFrameNum(long p_hHandle, const long p_nCurrentFrameNum);
long SN_C_API Remote_PlayBack_SetBrightness(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_PlayBack_GetBrightness(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_PlayBack_SetContrast(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_PlayBack_GetContrast(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_PlayBack_SetHue(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_PlayBack_GetHue(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_PlayBack_SetSaturation(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Remote_PlayBack_GetSaturation(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Remote_PlayBack_ResetPictureAdjustFilter(long p_hHandle);
long SN_C_API Remote_PlayBack_SnapShot(long p_hHandle, const char* p_pszFileName);
long SN_C_API Remote_PlayBack_ZoomInVideoEx(long p_hHandle, const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height);
long SN_C_API Remote_PlayBack_ZoomInVideo(long p_hHandle);
long SN_C_API Remote_PlayBack_ZoomOutVideo(long p_hHandle);
long SN_C_API Remote_PlayBack_RestoreVideo(long p_hHandle);

#endif //end win32

//RemoteSystem C Interface(参数设置接口）
long SN_C_API Remote_System_SetTimeout(long p_hHandle, const int p_nTimeout);
long SN_C_API Remote_System_Open(long p_hHandle);
long SN_C_API Remote_System_Close(long p_hHandle);

//重启
long SN_C_API Remote_System_Restart(long p_hHandle);

//复位
long SN_C_API Remote_System_Reset(long p_hHandle);

//关机
long SN_C_API Remote_System_ShutDown(long p_hHandle);

//格式化磁盘
long SN_C_API Remote_System_FormatDisk(long p_hHandle);
long SN_C_API Remote_System_FormatDiskEx(long p_hHandle, const char* p_szDiskName, long p_nFileSystemType);

//设置设备ID
long SN_C_API Remote_System_SetDeviceId(long p_hHandle, const char* p_pszDeviceId);
//设置设备名
long SN_C_API Remote_System_SetDeviceName(long p_hHandle, const char* p_pszDeviceName);
//获取概要信息
long SN_C_API Remote_System_GetSystemInfo(long p_hHandle, const ST_InetAddr p_stAddr, ST_DeviceSummaryParam* p_pstDeviceSummaryInfo, long p_nTransferProtocol);

//获取设备能力
long SN_C_API Remote_System_GetDeviceAbility(long p_hHandle, ST_DeviceAbility* p_pstDeviceAbility);
long SN_C_API Remote_System_GetDeviceExAbility(long p_hHandle, ST_DeviceExAbility* p_pstDeviceExAbility);

//获取工作状态
long SN_C_API Remote_System_GetDeviceWorkState(long p_hHandle, ST_DeviceWorkState* p_pstDeviceWorkState);
//获取磁盘信息
long SN_C_API Remote_System_GetDeviceDiskInfo(long p_hHandle, ST_AllDiskStatistic* p_pstDiskStatisticList);

//设置获取通道信息
long SN_C_API Remote_System_SetAllCameraDevice(long p_hHandle, const ST_AllCameraInfoParam* p_pstAllCameraInfoParam);
long SN_C_API Remote_System_GetAllCameraDevice(long p_hHandle, ST_AllCameraInfoParam* p_pstAllCameraInfoParam);
long SN_C_API Remote_System_SetCameraDevice(long p_hHandle, const long p_nCameraId, const ST_CameraInfoParam* p_pstCameraDevice);
long SN_C_API Remote_System_GetCameraDevice(long p_hHandle, const long p_nCameraId, ST_CameraInfoParam* p_pstCameraDevice);

//设置拾音器参数
long SN_C_API Remote_System_SetAllToneArmParam(long p_hHandle, const ST_AllToneArmParam* p_pstAllToneArmParam);
long SN_C_API Remote_System_GetAllToneArmParam(long p_hHandle, ST_AllToneArmParam* p_pstAllToneArmParam);
long SN_C_API Remote_System_SetToneArmParam(long p_hHandle, const long p_nCameraId, const ST_ToneArmParam* p_pstToneArmParam);
long SN_C_API Remote_System_GetToneArmParam(long p_hHandle, const long p_nCameraId, ST_ToneArmParam* p_pstToneArmParam);

//设置获取扬声器参数
long SN_C_API Remote_System_SetAllLoudhailerParam(long p_hHandle, const ST_AllLoudhailerParam* p_pstAllLoudhailerParam);
long SN_C_API Remote_System_GetAllLoudhailerParam(long p_hHandle, ST_AllLoudhailerParam* p_pstAllLoudhailerParam);
long SN_C_API Remote_System_SetLoudhailerParam(long p_hHandle, const long p_nCameraId, const ST_LoudhailerParam* p_stLoudhailerParam);
long SN_C_API Remote_System_GetLoudhailerParam(long p_hHandle, const long p_nCameraId, ST_LoudhailerParam* p_pstLoudhailerParam);

//获取指定通道制式
long SN_C_API Remote_System_GetCameraVideoSystem(long p_hHandle, const long p_nCameraId, long* p_nVideoSystem);

//设置获取RS485信息
long SN_C_API Remote_System_SetAllRS485Device(long p_hHandle, const ST_AllRS485Param* p_pstAllRS485DeviceParam);
long SN_C_API Remote_System_GetAllRS485Device(long p_hHandle, ST_AllRS485Param* p_pstAllRS485DeviceParam);
long SN_C_API Remote_System_SetRS485Device(long p_hHandle, const long p_nRS485ComId, const ST_RS485Param* p_pstRS485Device);
long SN_C_API Remote_System_GetRS485Device(long p_hHandle, const long p_nRS485ComId, ST_RS485Param* p_pstRS485Device);

//设置获取报警输入信息
long SN_C_API Remote_System_SetAllAlarmInDevice(long p_hHandle, const ST_AllAlarmInParam* p_pstAllAlarmInParam);
long SN_C_API Remote_System_GetAllAlarmInDevice(long p_hHandle, ST_AllAlarmInParam* p_pstAllAlarmInParam);
long SN_C_API Remote_System_SetAlarmInDevice(long p_hHandle, const long p_nAlarmInId, const ST_AlarmInParam* p_pstAlarmInParam);
long SN_C_API Remote_System_GetAlarmInDevice(long p_hHandle, const long p_nAlarmInId, ST_AlarmInParam* p_pstAlarmInParam);

//设备获取报警输出信息
long SN_C_API Remote_System_SetAllAlarmOutDevice(long p_hHandle, const ST_AllAlarmOutParam* p_pstAllAlarmOutParam);
long SN_C_API Remote_System_GetAllAlarmOutDevice(long p_hHandle, ST_AllAlarmOutParam* p_pstAllAlarmOutParam);
long SN_C_API Remote_System_SetAlarmOutDevice(long p_hHandle, const long p_nAlarmOutId, const ST_AlarmOutParam* p_pstAlarmOutParam);
long SN_C_API Remote_System_GetAlarmOutDevice(long p_hHandle, const long p_nAlarmOutId, ST_AlarmOutParam* p_pstAlarmOutParam);

//设置获取设备时间
long SN_C_API Remote_System_SetDeviceLongTime(long p_hHandle, const unsigned long p_nDeviceTime);
long SN_C_API Remote_System_SetDeviceTime(long p_hHandle, const unsigned short p_nYear, const unsigned short p_nMonth, 
											const unsigned short p_nDay, const unsigned short p_nHour, 
											const unsigned short p_nMinute, const unsigned short p_nSecond);
long SN_C_API Remote_System_GetDeviceLongTime(long p_hHandle, unsigned long* p_nDeviceTime);
long SN_C_API Remote_System_GetDeviceTime(long p_hHandle, unsigned short * p_nYear, unsigned short * p_nMonth, 
											unsigned short * p_nDay, unsigned short * p_nHour, 
											unsigned short * p_nMinute, unsigned short * p_nSecond);

//设置获取网络信息
long SN_C_API Remote_System_SetHostNetwork(long p_hHandle, const long p_nIPProtoVer, const ST_HostNetworkParam* p_pstHostNetworkParam);
long SN_C_API Remote_System_GetHostNetwork(long p_hHandle, const long p_nIPProtoVer, ST_HostNetworkParam* p_pstHostNetworkParam);

long SN_C_API Remote_System_GetAdslHostNetwork(long p_hHandle, const long p_nIPProtoVer, ST_HostNetworkParam* p_pstHostNetworkParam);

//设置获取端口参数
long SN_C_API Remote_System_SetDevicePort(long p_hHandle, const ST_DevicePortParam* p_pstDevicePortParam);
long SN_C_API Remote_System_GetDevicePort(long p_hHandle, ST_DevicePortParam* p_pstDeviceParam);

//设置获取NTP参数
long SN_C_API Remote_System_SetNTPParam(long p_hHandle, const long p_nIPProtoVer, const ST_NTPParam* p_pstNTPParam);
long SN_C_API Remote_System_GetNTPParam(long p_hHandle, const long p_nIPProtoVer, ST_NTPParam* p_pstNTPParam);

long SN_C_API Remote_System_SetPPPoEParam(long p_hHandle, const ST_PPPoEParam* p_pstPPPoEParam);
long SN_C_API Remote_System_GetPPPoEParam(long p_hHandle, ST_PPPoEParam* p_pstPPPoEParam);

//设置获取DDNS参数
long SN_C_API Remote_System_SetDDNSParam(long p_hHandle, const ST_DDNSParam* p_pstDDNSParam);
long SN_C_API Remote_System_GetDDNSParam(long p_hHandle, ST_DDNSParam* p_pstDDNSParam);

//设置获取FTP参数
long SN_C_API Remote_System_SetFTPParam(long p_hHandle, const long p_nIPProtoVer, const ST_FTPParam* p_pstFTPParam);
long SN_C_API Remote_System_GetFTPParam(long p_hHandle, const long p_nIPProtoVer, ST_FTPParam* p_pstFTPParam);

//设置获取FTP参数
long SN_C_API Remote_System_SetSMTPParam(long p_hHandle, const ST_SMTPParam* p_pstSMTPParam);
long SN_C_API Remote_System_GetSMTPParam(long p_hHandle, ST_SMTPParam* p_pstSMTPParam);

//设置获取OSD参数
long SN_C_API Remote_System_SetAllVideoOSD(long p_hHandle, const ST_AllVideoOSDInfoParam * p_pstAllVideoOSDInfoParam); //此接口废弃
long SN_C_API Remote_System_GetAllVideoOSD(long p_hHandle, ST_AllVideoOSDInfoParam * p_pstAllVideoOSDInfoParam);		//此接口废弃
long SN_C_API Remote_System_SetVideoOSDByImageFormat(long p_hHandle, const long p_nCameraId, const long p_nImagetFormatId, const ST_VideoOSDInfoParam* p_pstVideoOSDInfoParam); //此接口废弃
long SN_C_API Remote_System_SetVideoOSD(long p_hHandle, const long p_nCameraId, const ST_AllVideoOSDInfoParam* p_pstOSDInfoParamList);  //此接口废弃
long SN_C_API Remote_System_GetVideoOSDByImageFormat(long p_hHandle, const long p_nCameraId, const long p_nImagetFormatId, ST_VideoOSDInfoParam* p_pstVideoOSDInfoParam);  //此接口废弃
long SN_C_API Remote_System_GetVideoOSD(long p_hHandle, const long p_nCameraId, ST_AllVideoOSDInfoParam* p_pstOSDInfoParamList);  //此接口废弃

long SN_C_API Remote_System_SetAllVideoOSD_V2(long p_hHandle, const ST_AllVideoOSDInfoParam * p_pstAllVideoOSDInfoParam);
long SN_C_API Remote_System_GetAllVideoOSD_V2(long p_hHandle, ST_AllVideoOSDInfoParam * p_pstAllVideoOSDInfoParam);
long SN_C_API Remote_System_SetVideoOSD_V2(long p_hHandle, const long p_nCameraId, const ST_VideoOSDInfoParam* p_pstVideoOSDInfoParam);
long SN_C_API Remote_System_GetVideoOSD_V2(long p_hHandle, const long p_nCameraId, ST_VideoOSDInfoParam* p_pstVideoOSDInfoParam);



//设置获取编码格式
long SN_C_API Remote_System_SetVideoEncodeFormat(long p_hHandle, const long p_nVideoEncodeFormat);	//此接口无效
long SN_C_API Remote_System_GetVideoEncodeFormat(long p_hHandle, long* p_nVideoEncodeFormat);		//此接口无效

//设置获取升级参数
long SN_C_API Remote_System_SetUpdateParam(long p_hHandle, const long p_nIPProtoVer, const ST_UpdateParam* p_pstUpdateParam);	//此接口无效
long SN_C_API Remote_System_GetUpdateParam(long p_hHandle, const long p_nIPProtoVer, ST_UpdateParam* p_pstUpdateParam);			//此接口无效

//设置获取心跳参数
long SN_C_API Remote_System_SetHeartbeatParam(long p_hHandle, const long p_nIPProtoVer, const ST_HeartbeatParam* p_pstHeartbeatParam);	//此接口无效
long SN_C_API Remote_System_GetHeartbeatParam(long p_hHandle, const long p_nIPProtoVer, ST_HeartbeatParam* p_pstHeartbeatParam);		//此接口无效

//设置获取注册参数
long SN_C_API Remote_System_SetRegisterParam(long p_hHandle, const long p_nIPProtoVer, const ST_RegisterParam* p_pstRegisterParam);		//此接口无效
long SN_C_API Remote_System_GetRegisterParam(long p_hHandle, const long p_nIPProtoVer, ST_RegisterParam* p_pstRegisterParam);			//此接口无效

//设置通信参数
long SN_C_API Remote_System_SetCommunicationParam(long p_hHandle, const ST_CommunicationParam* p_pstCommunicationParam);
long SN_C_API Remote_System_GetCommunicationParam(long p_hHandle, ST_CommunicationParam* p_pstCommunicationParam);

//设置获取广播参数
long SN_C_API Remote_System_SetBroadcastParam(long p_hHandle, const ST_BroadcastParam* p_pstBroadcastParam);
long SN_C_API Remote_System_GetBroadcastParam(long p_hHandle, ST_BroadcastParam* p_pstBroadcastParam);

//设置获取PTZ参数
long SN_C_API Remote_System_SetAllPTZParam(long p_hHandle, const ST_AllPTZParam* p_pstAllPTZParam);
long SN_C_API Remote_System_GetAllPTZParam(long p_hHandle, ST_AllPTZParam* p_pstPTZParam);

long SN_C_API Remote_System_SetPTZParam(long p_hHandle, const long p_nCameraId, const ST_PTZParam* p_pstPTZParam);
long SN_C_API Remote_System_GetPTZParam(long p_hHandle, const long p_nCameraId, ST_PTZParam* p_pstPTZParam);

long SN_C_API Remote_System_SetIPDomePTZId(long p_hHandle, const int p_nPTZId);
long SN_C_API Remote_System_GetIPDomePTZId(long p_hHandle, int* p_nPTZId);

long SN_C_API Remote_System_SetPTZKeyboardParam(long p_hHandle, const ST_PTZKeyboardParam* p_pobjPTZKeyboardParam);
long SN_C_API Remote_System_GetPTZKeyboardParam(long p_hHandle, ST_PTZKeyboardParam* p_pobjPTZKeyboardParam);

//设置获取点钞机参数
long SN_C_API Remote_System_SetCashRegistersParam(long p_hHandle, const ST_CashRegistersParam* p_pobjCashRegistersParam);
long SN_C_API Remote_System_GetCashRegistersParam(long p_hHandle, ST_CashRegistersParam* p_pobjCashRegistersParam);

//本地录像接口
long SN_C_API Remote_System_SetRecordStorageParam(long p_hHandle, const ST_RecordStorageParam* p_pstRecordStorageParam);	//此接口废弃
long SN_C_API Remote_System_GetRecordStorageParam(long p_hHandle, ST_RecordStorageParam* p_pstRecordStorageParam);			//此接口废弃

long SN_C_API Remote_System_SetRecordQualityParam(long p_hHandle, const long p_nCameraId, const ST_RecordQualityParam* p_pstRecordQualityParam);	//此接口废弃
long SN_C_API Remote_System_GetRecordQualityParam(long p_hHandle, const long p_nCameraId, ST_RecordQualityParam* p_pstRecordQualityParam);			//此接口废弃
long SN_C_API Remote_System_SetAllRecordQualityParam(long p_hHandle, const ST_AllRecordQualityParam* p_pstAllRecordQualityParam);					//此接口废弃
long SN_C_API Remote_System_GetAllRecordQualityParam(long p_hHandle, ST_AllRecordQualityParam* p_pstAllRecordQualityParam);							//此接口废弃

long SN_C_API Remote_System_SetRecordFileHead(long p_hHandle, const char * p_pszHeadDate);
long SN_C_API Remote_System_GetRecordFileHead(long p_hHandle, char * p_pszHeadDate);
long SN_C_API Remote_System_SetAllScheduleRecordParam(long p_hHandle, const ST_AllScheduleRecordParam* p_pstAllScheduleRecordParam);				//此接口废弃
long SN_C_API Remote_System_GetAllScheduleRecordParam(long p_hHandle, ST_AllScheduleRecordParam* p_pstAllScheduleRecordParam);						//此接口废弃
long SN_C_API Remote_System_SetScheduleRecordParam(long p_hHandle, const long p_nCameraId, const ST_ScheduleRecordParam* p_pstScheduleRecordParam);	//此接口废弃
long SN_C_API Remote_System_GetScheduleRecordParam(long p_hHandle, const long p_nCameraId, ST_ScheduleRecordParam* p_pstScheduleRecordParam);		//此接口废弃

long SN_C_API Remote_System_SetRecordPolicy(long p_hHandle, const int p_nCameraId, const ST_RecordPolicyParam* p_pstRecordPolicyParam);
long SN_C_API Remote_System_GetRecordPolicy(long p_hHandle, const int p_nCameraId, ST_RecordPolicyParam* p_pstRecordPolicyParam);
long SN_C_API Remote_System_SetAllRecordPolicy(long p_hHandle, const ST_AllRecordPolicyParam* p_pstAllRecordPolicyParam);
long SN_C_API Remote_System_GetAllRecordPolicy(long p_hHandle, ST_AllRecordPolicyParam* p_pstAllRecordPolicyParam);
long SN_C_API Remote_System_SetRecordDirInfo(long p_hHandle, const ST_RecordDirInfoList* p_pstRecordDirInfoList);
long SN_C_API Remote_System_GetRecordDirInfo(long p_hHandle, ST_RecordDirInfoList* p_pstRecordDirInfoList);


//报警接口
//设置报警服务器参数
long SN_C_API Remote_System_SetAlarmServiceParam(long p_hHandle, const long p_nIPProtoVer, const ST_AlarmServiceParam* p_pstAlarmServiceParam);
long SN_C_API Remote_System_GetAlarmServiceParam(long p_hHandle, const long p_nIPProtoVer, ST_AlarmServiceParam* p_pstAlarmServiceParam);

//IO报警
long SN_C_API Remote_System_SetAllAlarmIOEvent(long p_hHandle, const ST_AllAlarmIOEventParam* p_pstAllAlarmIOEventParam);
long SN_C_API Remote_System_GetAllAlarmIOEvent(long p_hHandle, ST_AllAlarmIOEventParam* p_pstAllAlarmIOEventParam);
long SN_C_API Remote_System_SetAlarmIOEvent(long p_hHandle, const long p_nAlarmInId, const ST_AlarmIOEventParam* p_pstAlarmIOEventParam);
long SN_C_API Remote_System_GetAlarmIOEvent(long p_hHandle, const long p_nAlarmInId, ST_AlarmIOEventParam* p_pstAlarmIOEventParam);

//移动侦测报警
long SN_C_API Remote_System_SetAllMotionDetectionEvent(long p_hHandle, const ST_AllMotionDetectionEventParam* p_pstAllMotionDetectionEventParam);
long SN_C_API Remote_System_GetAllMotionDetectionEvent(long p_hHandle, ST_AllMotionDetectionEventParam* p_pstAllMotionDetectionEventParam);
long SN_C_API Remote_System_SetMotionDetectionEvent(long p_hHandle, const long p_nCameraId, const ST_MotionDetectionEventParam* p_pstMotionDetectionEventParam);
long SN_C_API Remote_System_GetMotionDetectionEvent(long p_hHandle, const long p_nCameraId, ST_MotionDetectionEventParam* p_pstMotionDetectionEventParam);

//遮挡报警
long SN_C_API Remote_System_SetAllOcclusionDetectionEvent(long p_hHandle, const ST_AllOcclusionDetectionEventParam* p_pstAllOcclusionDetectionEventParam);
long SN_C_API Remote_System_GetAllOcclusionDetectionEvent(long p_hHandle, ST_AllOcclusionDetectionEventParam* p_pstAllOcclusionDetectionEventParam);
long SN_C_API Remote_System_SetOcclusionDetectionEvent(long p_hHandle, const long p_nCameraId, const ST_OcclusionDetectionEventParam* p_pstOcclusionDetectionEventParam);
long SN_C_API Remote_System_GetOcclusionDetectionEvent(long p_hHandle, const long p_nCameraId, ST_OcclusionDetectionEventParam* p_pstOcclusionDetectionEventParam);

//视频丢失报警
long SN_C_API Remote_System_SetAllVideoLoseDetectionEvent(long p_hHandle, const ST_AllVideoLoseDetectionEventParam* p_pstAllVideoLoseDetectionEventParam);
long SN_C_API Remote_System_GetAllVideoLoseDetectionEvent(long p_hHandle, ST_AllVideoLoseDetectionEventParam* p_pstAllVideoLoseDetectionEventParam);
long SN_C_API Remote_System_SetVideoLoseDetectionEvent(long p_hHandle, const long p_nCameraId, const ST_VideoLoseDetectionEventParam* p_pstVideoLoseDetectionEventParam);
long SN_C_API Remote_System_GetVideoLoseDetectionEvent(long p_hHandle, const long p_nCameraId, ST_VideoLoseDetectionEventParam* p_pstVideoLoseDetectionEventParam);

//磁盘报警
long SN_C_API Remote_System_SetDiskAlarmParam(long p_hHandle, const ST_DiskAlarmParam* p_pstDiskAlarmParam);
long SN_C_API Remote_System_GetDiskAlarmParam(long p_hHandle, ST_DiskAlarmParam* p_pstDiskAlarmParam);

//手动报警
long SN_C_API Remote_System_ManualAlarm(long p_hHandle, const long p_nAlarmType, const long p_nAlarmSourceId, const ST_AlarmActionStrategy* p_pstAlarmActionStrategy);
long SN_C_API Remote_System_SetAlarmOut(long p_hHandle, const long p_nAlarmDeviceType, const long p_nAlarmOutId, const long p_nAlarmOutFlag);

//报警参数
long SN_C_API Remote_System_SetAlarmParam(long p_hHandle, const ST_AlarmParam* p_pstAlarmParam);
long SN_C_API Remote_System_GetAlarmParam(long p_hHandle, ST_AlarmParam* p_pstAlarmParam);

long SN_C_API Remote_System_SetSnapshotParam(long p_hHandle, const long p_nCameraId, const ST_SnapshotParam* p_pstSnapshotParam);
long SN_C_API Remote_System_GetSnapshotParam(long p_hHandle, const long p_nCameraId, ST_SnapshotParam* p_pstSnapshotParam);
long SN_C_API Remote_System_SetAllSnapshotParam(long p_hHandle, const ST_AllSnapshotParam* p_pstAllSnapshotParam);
long SN_C_API Remote_System_GetAllSnapshotParam(long p_hHandle, ST_AllSnapshotParam* p_pstAllSnapshotParam);

long SN_C_API Remote_System_SetAllAVStreamParam(long p_hHandle, const ST_AllAVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_GetAllAVStreamParam(long p_hHandle, ST_AllAVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_SetAllCameraAVStreamParam(long p_hHandle, const long p_nCameraId, const ST_AllAVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_GetAllCameraAVStreamParam(long p_hHandle, const long p_nCameraId, ST_AllAVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_SetAVStreamParam(long p_hHandle, const long p_nCameraId, const long p_nStreamId, const ST_AVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_GetAVStreamParam(long p_hHandle, const long p_nCameraId, const long p_nStreamId, ST_AVStreamParam* p_pstAllStreamParam);
long SN_C_API Remote_System_GetStreamURI(long p_hHandle, const long p_nCameraId, const long p_nStreamId, char* p_pszStreamURI, const long p_nInputLen, long* p_nOutputLen);
long SN_C_API Remote_System_GetStreamURIEx(long p_hHandle, const long p_nCameraId, const long p_nStreamId, const long p_nRtspTransferProtocol, char* p_pszStreamURI, const long p_nInputLen, long* p_nOutputLen);

//查询录像文件接口
long SN_C_API Remote_System_QueryRecordFile(long p_hHandle, const ST_RecordFileSearchParam* p_pstRecordFileSearchParam);	//此接口废弃
long SN_C_API Remote_System_GetRecordFileCount(long p_hHandle, long * p_nFileCount);										//此接口废弃
long SN_C_API Remote_System_GetRecordFile(long p_hHandle, const long p_nIndex,ST_RecordFile * p_pstRecordFile );			//此接口废弃
long SN_C_API Remote_System_ClearRecordFileQueryResult(long p_hHandle);														//此接口废弃			
long SN_C_API Remote_System_DeleteRecordFile(long p_hHandle, const char* p_pszFileName);									//此接口废弃

long SN_C_API Remote_System_QueryRecord(long p_hHandle, const ST_RecordQueryCondition* p_pstRecordQueryCondition);
long SN_C_API Remote_System_GetRecordQueryResultCount(long p_hHandle, long* p_nRecordQueryResultCount);
long SN_C_API Remote_System_GetRecordQueryResult(long p_hHandle, const long p_nIndex,ST_RecordQueryResult* p_pstRecordQueryResult);
long SN_C_API Remote_System_ClearRecordQueryResult(long p_hHandle);

//查询系统日志接口
long SN_C_API Remote_System_QuerySystemLog(long p_hHandle, const ST_LogRequestParam* p_pstLogRequestParam);
long SN_C_API Remote_System_GetSystemLogCount(long p_hHandle, long * p_nLogInfoCount);
long SN_C_API Remote_System_GetSystemLog(long p_hHandle, const long p_nIndex,ST_LogInfo* p_pstLogInfo);
long SN_C_API Remote_System_ClearSystemLogQueryResult(long p_hHandle);

//隐私遮蔽接口
long SN_C_API Remote_System_SetBlindAreaParam(long p_hHandle, const long p_nCameraId, const ST_BlindAreaParam* p_pstBlindAreaParam);
long SN_C_API Remote_System_GetBlindAreaParam(long p_hHandle, const long p_nCameraId, ST_BlindAreaParam* p_pstBlindAreaParam);
long SN_C_API Remote_System_SetAllBlindAreaParam(long p_hHandle, const ST_AllBlindAreaParam* p_pstAllBlindAreaParam);
long SN_C_API Remote_System_GetAllBlindAreaParam(long p_hHandle, ST_AllBlindAreaParam* p_pstAllBlindAreaParam);

//Decoder
long SN_C_API Remote_System_GetWindowMode(long p_hHandle, int* p_pnWindowMode);
long SN_C_API Remote_System_SetWindowMode(long p_hHandle, const int p_nWindowMode);


long SN_C_API Remote_Snapshot_SetTimeout(long p_hHandle, const int p_nTimeout);
long SN_C_API Remote_Snapshot_Open(long p_hHandle);
long SN_C_API Remote_Snapshot_Close(long p_hHandle);
long SN_C_API Remote_Snapshot_GetSnapshotPicture(long p_hHandle, ST_RemoteSnapshotParam* p_pstRemoteSnapshotParam, ST_AVFrameData* p_pstAVFrameData);


//CMS C Interface(CMS服务初始化和反初始化，调用CMS_*函数时需调用以下两个函数初始化和反初始化）
long SN_C_API CMS_Init(long* p_hHandle);
long SN_C_API CMS_UnInit(long p_hHandle);

//CMSAlarmCenter C Interface(报警接收）
long SN_C_API CMS_AlarmCenter_setListenAddr(long p_hHandle, const ST_InetAddr* p_stInetAddr);
long SN_C_API CMS_AlarmCenter_setAlarmCallback(long p_hHandle, long (CALLBACK* fAlarmCallback)(long p_hHandle, ST_AlarmInfo* p_stAlarmInfo, void* pUserData), void* pUserData);
long SN_C_API CMS_AlarmCenter_setAlarmCallbackEx(long p_hHandle, long (CALLBACK* fAlarmCallback)(long p_hHandle, ST_AlarmInfoEx* p_stAlarmInfo, void* pUserData), void* pUserData);
long SN_C_API CMS_AlarmCenter_open(long p_hHandle);
long SN_C_API CMS_AlarmCenter_close(long p_hHandle);

long SN_C_API CMS_AlarmCenter2_setAlarmCallbackEx(long p_hHandle, long (CALLBACK* fAlarmCallback)(long p_hHandle, ST_AlarmInfoEx* p_stAlarmInfo, void* pUserData), void* pUserData);
long SN_C_API CMS_AlarmCenter2_AddDeviceInfo(long p_hHandle, const ST_DeviceInfoEx * p_stDeviceInfo);
long SN_C_API CMS_AlarmCenter2_RemoveDeviceInfo(long p_hHandle, const char * p_szDeviceId);
long SN_C_API CMS_AlarmCenter2_RemoveAll(long p_hHandle);
long SN_C_API CMS_AlarmCenter2_open(long p_hHandle);
long SN_C_API CMS_AlarmCenter2_close(long p_hHandle);

//CMSDeviceSearcher C Interface(设备广播搜索）
long SN_C_API CMS_DeviceSearcher_SetListenAddr(long p_hHandle, const ST_InetAddr* p_stInetAddr);
long SN_C_API CMS_DeviceSearcher_SetDeviceFilterFlag(long p_hHandle,  const unsigned char bFlag);
long SN_C_API CMS_DeviceSearcher_SetDeviceInfoCallback(long p_hHandle, 
											  long (CALLBACK* fSearchCallback)(long p_hHandle, ST_NetVideoDeviceInfo* p_pstNetVideoDeviceInfo, void* pUserData), 
											  void* pUserData);
long SN_C_API CMS_DeviceSearcher_Start(long p_hHandle);
long SN_C_API CMS_DeviceSearcher_Stop(long p_hHandle);

//CMSDeviceSearcherEx C Interface(设备主动搜索）
long SN_C_API CMS_DeviceSearcherEx_SetDevicePort(long p_hHandle, const unsigned short p_nPort = 30001);
long SN_C_API CMS_DeviceSearcherEx_SetDeviceFilterFlag(long p_hHandle, const bool p_bFlag);
long SN_C_API CMS_DeviceSearcherEx_SetSearchCount(long p_hHandle, const int p_nSearchCount);
long SN_C_API CMS_DeviceSearcherEx_SetSearchInterval(long p_hHandle, const int p_nSearchInterval);
long SN_C_API CMS_DeviceSearcherEx_AddSearchDeviceIPRange(long p_hHandle, const char* p_szDeviceIPBegin, const char* p_szDeviceIPEnd);
long SN_C_API CMS_DeviceSearcherEx_ClearSearchDeviceIP(long p_hHandle);
long SN_C_API CMS_DeviceSearcherEx_SetDeviceInfoCallback(long p_hHandle, 
													   long (CALLBACK* fSearchCallback)(long p_hHandle, ST_NetVideoDeviceInfo* p_pstNetVideoDeviceInfo, void* pUserData), 
													   void* pUserData);
long SN_C_API CMS_DeviceSearcherEx_Start(long p_hHandle);
long SN_C_API CMS_DeviceSearcherEx_Stop(long p_hHandle);

//CMSDeviceOnlineService C Interface(设备在线服务）
//设置监听者
long SN_C_API CMS_DeviceOnlineService_SetDeviceInfoCallback(long p_hHandle, 
															long (CALLBACK* fDeviceInfoCallBack)(long p_hHandle, ST_DeviceBaseInfo* p_pstDeviceBaseInfo, void* pUserData), 
															void* pUserData);
long SN_C_API CMS_DeviceOnlineService_SetDeviceStateCallback(long p_hHandle, 
															 long (CALLBACK* fDeviceInfoCallBack)(long p_hHandle, ST_DeviceStateInfo* p_pstDeviceStateInfo, void* pUserData), 
															 void* pUserData);
void SN_C_API CMS_DeviceOnlineService_SetIPProtoVer(long p_hHandle, const int p_nIPProtoVer);

//广播需调用接口
//设置是否使用广播
void SN_C_API CMS_DeviceOnlineService_SetUseBroadcastFlag(long p_hHandle, const unsigned char p_bFlag);
void SN_C_API CMS_DeviceOnlineService_SetBroadcastListenAddr(long p_hHandle, const ST_InetAddr* p_pstListenAddr);

//主动搜索需调用接口
long SN_C_API CMS_DeviceOnlineService_AddSearchDeviceAddr1(long p_hHandle, const char* p_szDeviceIPBegin, const char* p_szDeviceIPEnd, const int p_nSearchPort);
long SN_C_API CMS_DeviceOnlineService_AddSearchDeviceAddr2(long p_hHandle, const char* p_szDeviceIP, const int p_nSearchPort);
long SN_C_API CMS_DeviceOnlineService_AddSearchDeviceAddr3(long p_hHandle, const ST_InetAddr* p_pstInetAddr);
long SN_C_API CMS_DeviceOnlineService_Start(long p_hHandle);
void SN_C_API CMS_DeviceOnlineService_Stop(long p_hHandle);
long SN_C_API CMS_DeviceOnlineService_FindDeviceStateInfo(long p_hHandle, const char* p_pszDeviceId, ST_DeviceStateInfo* p_pstDeviceStateInfo);



#if (defined(WIN32) || defined(_WIN32_WCE))

//RemoteAudioBroadcast C Interface(远程语音广播服务初始化和反初始化，调用Remote_AudioBroadcast_*函数时需调用以下两个函数初始化和反初始化）
long SN_C_API Remote_AudioBroadcast_Init(long* p_hHandle);
long SN_C_API Remote_AudioBroadcast_UnInit(long p_hHandle);

long SN_C_API Remote_AudioBroadcast_AddBroadcastDevice(long p_hHandle, const ST_DeviceInfoEx* p_stDeviceInfo, const int p_nAudioChannel = 1);
long SN_C_API Remote_AudioBroadcast_Start(long p_hHandle);
void SN_C_API Remote_AudioBroadcast_Stop(long p_hHandle);
void SN_C_API Remote_AudioBroadcast_RemoveBroadcastDevice(long p_hHandle, const char* p_pszDeviceId, const int p_nAudioChannel = 1);

#endif //end WIN32

#ifdef RECORD_INTERFACE
//CMS录像接口
long SN_C_API CMS_RecordCenter_Init(long* p_hHandle);
long SN_C_API CMS_RecordCenter_UnInit(long p_hHandle);
long SN_C_API CMS_RecordCenter_SetRecordDirInfoList(long p_hHandle, const ST_RecordDirInfoList* p_stRecordDirInfoList);
long SN_C_API CMS_RecordCenter_OpenRecord(long p_hHandle, const char* p_pszDeviceId, const char* p_pszDeviceIp, int p_nCameraId, int p_nDiskGroupId, int p_nSaveDays, long* p_hRecorderHandle);
long SN_C_API CMS_RecordCenter_CloseRecord(long p_hHandle, const char* p_pszDeviceId, const char* p_pszDeviceIp, int p_nCameraId);
long SN_C_API CMS_RecordCenter_CloseRecordEx(long p_hHandle, long p_hRecorderHandle);
long SN_C_API CMS_RecordCenter_CloseAllRecord(long p_hHandle);
bool SN_C_API CMS_RecordCenter_IsRecording(long p_hHandle, const char* p_pszDeviceId, const char* p_pszDeviceIp, int p_nCameraId);
//录像写接口，此次使用的p_hRecordHandle是调用CMS_RecordCenter_OpenRecord返回的p_hRecordHandle
long SN_C_API CMS_Recorder_Write(long p_hRecorderHandle, ST_AVFrameData* p_stAVFrameData);

//录像查询接口
long SN_C_API CMS_RecordSearcher_Init(long* p_hHandle);
long SN_C_API CMS_RecordSearcher_UnInit(long p_hHandle);
long SN_C_API CMS_RecordSearcher_SetRecordDirInfoList(long p_hHandle, const ST_RecordDirInfoList* p_stRecordDirInfoList);
long SN_C_API CMS_RecordSearcher_QueryRecord(long p_hHandle, const ST_RecordQueryCondition* p_pstRecordQueryCondition);
long SN_C_API CMS_RecordSearcher_GetRecordQueryResultCount(long p_hHandle, long* p_nRecordQueryResultCount);
long SN_C_API CMS_RecordSearcher_GetRecordQueryResult(long p_hHandle, const long p_nIndex,ST_RecordQueryResult* p_pstRecordQueryResult);
long SN_C_API CMS_RecordSearcher_ClearRecordQueryResult(long p_hHandle);

//录像读取接口
long SN_C_API CMS_RecordRetriever_Init(long* p_hHandle);
long SN_C_API CMS_RecordRetriever_UnInit(long p_hHandle);
long SN_C_API CMS_RecordRetriever_SetRecordDirInfoList(long p_hHandle, const ST_RecordDirInfoList* p_stRecordDirInfoList);
long SN_C_API CMS_RecordRetriever_Open(long p_hHandle, const ST_RecordInfo* p_stRecordInfo);
long SN_C_API CMS_RecordRetriever_Close(long p_hHandle);
long SN_C_API CMS_RecordRetriever_Read(long p_hHandle, char *p_szAVData,int p_nDataLength);
long SN_C_API CMS_RecordRetriever_LocateTime(long p_hHandle, unsigned long p_nTime);
long SN_C_API CMS_RecordRetriever_LocateNextIFrame(long p_hHandle);
long SN_C_API CMS_RecordRetriever_LocatePreIFrame(long p_hHandle);
#endif

#if (defined(WIN32) || defined(_WIN32_WCE))
//本地录像回放接口
long SN_C_API Local_PlayBack_Init(long* p_hHandle);
long SN_C_API Local_PlayBack_UnInit(long p_hHandle);
long SN_C_API Local_PlayBack_SetRecordDirInfoList(long p_hHandle, const ST_RecordDirInfoList* p_stRecordDirInfoList);
long SN_C_API Local_PlayBack_SetStretchMode(long p_hHandle, const bool p_bStretchMode);
long SN_C_API Local_PlayBack_SetNotifyWindow(long p_hHandle, const unsigned long p_nNotifyWnd, const unsigned long p_nMessage, const void* p_pParam);
long SN_C_API Local_PlayBack_SetVideoWindow(long p_hHandle, const unsigned long p_hDisplayWnd, const long x, const long y, const long width, const long height);
long SN_C_API Local_PlayBack_Open(long p_hHandle, ST_RecordInfo* p_stRecordInfo);
long SN_C_API Local_PlayBack_Close(long p_hHandle);
long SN_C_API Local_PlayBack_Play(long p_hHandle);
long SN_C_API Local_PlayBack_Pause(long p_hHandle);
long SN_C_API Local_PlayBack_Stop(long p_hHandle);
long SN_C_API Local_PlayBack_ResizeWindow(long p_hHandle, const long x, const long y, const long width, const long height);
long SN_C_API Local_PlayBack_Refresh(long p_hHandle);
long SN_C_API Local_PlayBack_GetPlayStatus(long p_hHandle, long* p_nPlayStatus);
long SN_C_API Local_PlayBack_PlayNextFrame(long p_hHandle);
long SN_C_API Local_PlayBack_PlayPreviousFrame(long p_hHandle);
long SN_C_API Local_PlayBack_PlaySound(long p_hHandle);
long SN_C_API Local_PlayBack_StopSound(long p_hHandle);
long SN_C_API Local_PlayBack_IsOnSound(long p_hHandle, bool* p_bOnSound);
long SN_C_API Local_PlayBack_SetVolume(long p_hHandle, const long p_nVolume);
long SN_C_API Local_PlayBack_GetVolume(long p_hHandle, long* p_nVolume);
long SN_C_API Local_PlayBack_GetPlayPosByTime(long p_hHandle, unsigned long* p_nDuration);
long SN_C_API Local_PlayBack_SetPlayPosByTime(long p_hHandle, const unsigned long p_nDuration);
long SN_C_API Local_PlayBack_SetRate (long p_hHandle, const float p_nRate);
long SN_C_API Local_PlayBack_GetRate(long p_hHandle, float& p_nRate );
long SN_C_API Local_PlayBack_SetBrightness(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Local_PlayBack_GetBrightness(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Local_PlayBack_SetContrast(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Local_PlayBack_GetContrast(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Local_PlayBack_SetHue(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Local_PlayBack_GetHue(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Local_PlayBack_SetSaturation(long p_hHandle, const unsigned long p_nValue);
long SN_C_API Local_PlayBack_GetSaturation(long p_hHandle, unsigned long* p_nValue);
long SN_C_API Local_PlayBack_ResetPictureAdjustFilter(long p_hHandle);
long SN_C_API Local_PlayBack_GetPictureSize(long p_hHandle, long* p_nWidth, long* p_nHeight);
long SN_C_API Local_PlayBack_SnapShot(long p_hHandle, const char* p_pszFileName);
long SN_C_API Local_PlayBack_ZoomInVideoEx(long p_hHandle, const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height);
long SN_C_API Local_PlayBack_ZoomInVideo(long p_hHandle);
long SN_C_API Local_PlayBack_ZoomOutVideo(long p_hHandle);
long SN_C_API Local_PlayBack_RestoreVideo(long p_hHandle);
#endif //end WIN32

//手动录像接口，在使用手动录像C接口之前必须调用Manual_Recorder_Init初始化函数，使用完后应当调用Manual_Recorder_UnInit释放环境
long SN_C_API Manual_Recorder_Init(long* p_handle, const ST_DeviceInfoEx* p_stDeviceInfo, const int p_nCameraId, const int p_nStreamId, const int p_nTransferProtocol);
long SN_C_API Manual_Recorder_SetRecordAudioFlag(long p_hHandle, const bool p_bFlag);
long SN_C_API Manual_Recorder_SetRecordFileName(long p_hHandle, const char* p_pszRecordFileName);
long SN_C_API Manual_Recorder_SetMessageCallback(long p_hHandle, 
														 long (CALLBACK* fRecordMessageCallback)(long p_hHandle, const int p_nMessageID, void* pUserData), 
														 void* pUserData);
long SN_C_API Manual_Recorder_StartRecord(long p_hHandle);
long SN_C_API Manual_Recorder_StopRecord(long p_hHandle);
long SN_C_API Manual_Recorder_UnInit(long p_hHandle);

//解码器接口
long SN_C_API Remote_Decoder_Init(long* p_handle, const ST_DeviceInfoEx* p_stDeviceInfo, const int p_nWindowID);
long SN_C_API Remote_Decoder_UnInit(long p_hHandle);
long SN_C_API Remote_Decoder_Open(long p_hHandle);
long SN_C_API Remote_Decoder_Decode(long p_hHandle, ST_AVFrameData* p_stAVFrameData);
long SN_C_API Remote_Decoder_Close(long p_hHandle);


//} //namespace
#ifdef __cplusplus 
}
#endif //__cplusplus 
#endif
