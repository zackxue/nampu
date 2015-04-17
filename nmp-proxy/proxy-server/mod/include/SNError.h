#ifndef _SNERROR_H_
#define _SNERROR_H_

#include "GeneralError.h"
#include "IOError.h"
#include "DbError.h"
#include "NetworkIOError.h"
#include "CommandError.h"
//-501 => -1000

#define SN_ERROR_BEGIN				(-500)

//---------管理-----------------//
//不是期望的应答
#define SN_ERROR_NOT_EXPECT_RESPONSE						(SN_ERROR_BEGIN - 1)

//远端处理失败
#define SN_ERROR_REMOTE_FAILED								(SN_ERROR_BEGIN - 2)

//设备未打开
#define SN_ERROR_DEVICE_NOT_OPEN 							(SN_ERROR_BEGIN - 3)

//设备打开失败
#define SN_ERROR_DEVICE_OPEN_FAILED 						(SN_ERROR_BEGIN - 4)

//设备被占用
#define SN_ERROR_DEVICE_USED								(SN_ERROR_BEGIN - 5)

//不支持的设备
#define SN_ERROR_DEVICE_NOT_SUPPORT							(SN_ERROR_BEGIN - 6)

//登录用户名错误
#define SN_ERROR_LOGIN_USERNAME_ERROR						(SN_ERROR_BEGIN - 7)

//登录口令错误
#define SN_ERROR_LOGIN_USERPASSWORD_ERROR					(SN_ERROR_BEGIN - 8)

//拨号失败
#define SN_ERROR_ADSL_DIAL_FAILED							(SN_ERROR_BEGIN - 9)

//串口被独占
#define SN_ERROR_COM_ISUSED									(SN_ERROR_BEGIN - 10)

//连接数已经达到最大
#define SN_ERROR_MAX_CONNECTION								(SN_ERROR_BEGIN - 11)

//权限不足
#define SN_ERROR_NO_PRIVILEGE								(SN_ERROR_BEGIN - 12)

//设备未配置
#define SN_ERROR_DEVICE_NOT_CONFIGURE						(SN_ERROR_BEGIN - 13)

//磁盘正在使用无法格式化
#define SN_ERROR_FORMAT_FAIL_BY_USING						(SN_ERROR_BEGIN - 14)

//登录用户已锁定
#define SN_ERROR_USER_LOCKED								(SN_ERROR_BEGIN - 15)

//登录用户重登录，当用户不支持多点登录时，重复登录返回该错误。
#define SN_ERROR_USER_REPEATED								(SN_ERROR_BEGIN - 16)

//用户正在登录
#define SN_ERROR_USER_LONGINING								(SN_ERROR_BEGIN - 17)

//设备能力不足
#define SN_ERROR_DEVICE_ABILITY_NOT_ENOUGH					(SN_ERROR_BEGIN - 18)

//验证码错误
#define SN_ERROR_VALIDATE_CODE								(SN_ERROR_BEGIN - 19)

//---------VIDEO---------------//
//视频会话已经关闭
#define SN_ERROR_VIDEO_SESSION_CLOSE						(SN_ERROR_BEGIN - 50)

//视频会话线程已经关闭
#define SN_ERROR_VIDEO_SESSION_THREAD_CLOSE					(SN_ERROR_BEGIN - 51)

//创建Directshow视频组件失败
#define SN_ERROR_DIRECTSHOW_VIDEO_CREATE_FAILED				(SN_ERROR_BEGIN - 52)

//操作DirectShow视频组件失败
#define SN_ERROR_DIRECTSHOW_VIDEO_OPERATION_FAILED			(SN_ERROR_BEGIN - 53)

//解码能力不足
#define SN_ERROR_DECODER_ABILITY_NOT_ENOUGH					(SN_ERROR_BEGIN - 55)

//---------AUDIO---------------//
//音频会话已经关闭
#define SN_ERROR_AUDIO_SESSION_CLOSE						(SN_ERROR_BEGIN - 101)

//音频会话线程已经关闭。
#define SN_ERROR_AUDIO_SESSION_THREAD_CLOSE					(SN_ERROR_BEGIN - 102)

//创建Directshow音频组件失败
#define SN_ERROR_DIRECTSHOW_AUDIO_CREATE_FAILED				(SN_ERROR_BEGIN - 103)

//操作Directshow音频组件失败
#define SN_ERROR_DIRECTSHOW_AUDIO_OPERATION_FAILED			(SN_ERROR_BEGIN - 104)

//初始化DirectDraw组件失败
#define SN_ERROR_DIRECTDRAW_CREATE_FAILED					(SN_ERROR_BEGIN - 105)

//初始化解码器失败
#define SN_ERROR_VIDEO_DECODER_INIT_FAILED					(SN_ERROR_BEGIN - 106)

//解码失败
#define SN_ERROR_VIDEO_DECODE_FAILED						(SN_ERROR_BEGIN - 107)


//OSD行越界
#define SN_ERROR_VIDEO_OSD_ROW_OVERLOAD						(SN_ERROR_BEGIN - 150)
//OSD字符数越界
#define SN_ERROR_VIDEO_OSD_CHARACTER_OVERLOAD				(SN_ERROR_BEGIN - 151)


//ONVIF错误
#define SN_ERROR_SOAP_FAULT											(SN_ERROR_BEGIN - 152)

#endif // _SNERROR_H_
