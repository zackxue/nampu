#ifndef _IOERROR_H_
#define _IOERROR_H_
#include "GeneralError.h"

// -101~ -200
#define SN_ERROR_DISKIO_BEGIN				(-100)

//文件不存在
#define SN_ERROR_FILE_NOT_FOUND             (SN_ERROR_DISKIO_BEGIN - 1)

//文件路径不存在
#define SN_ERROR_PATH_NOT_FOUND             (SN_ERROR_DISKIO_BEGIN - 2)

//打开磁盘时发生错误
#define SN_ERROR_DISKIO_OPEN				(SN_ERROR_DISKIO_BEGIN - 3)

//读磁盘时发生错误
#define SN_ERROR_DISKIO_READ				(SN_ERROR_DISKIO_BEGIN - 4)

//写磁盘时发生错误
#define SN_ERROR_DISKIO_WRITE				(SN_ERROR_DISKIO_BEGIN - 5)

//定位(seek)文件位置时错误
#define SN_ERROR_DISKIO_SEEK				(SN_ERROR_DISKIO_BEGIN - 6)

//读写磁盘到了末尾
#define SN_ERROR_DISKIO_END					(SN_ERROR_DISKIO_BEGIN - 7)

//磁盘空间不足，或磁盘空间已满
#define SN_ERROR_DISK_SPACE_FULL			(SN_ERROR_DISKIO_BEGIN - 8)

//磁盘不存在
#define SN_ERROR_DISK_NOT_EXISTENT			(SN_ERROR_DISKIO_BEGIN - 9)

//磁盘写保护
#define SN_ERROR_DISK_WRITE_PROTECT			(SN_ERROR_DISKIO_BEGIN - 10)

//磁盘未格式化
#define SN_ERROR_DISK_NOT_FORMAT			(SN_ERROR_DISKIO_BEGIN - 12)

//磁盘错误，需要断电重启
#define SN_ERROR_DISK_ERROR					(SN_ERROR_DISKIO_BEGIN - 13)

//磁盘正在格式化
#define SN_ERROR_DISK_FORMATTING			(SN_ERROR_DISKIO_BEGIN - 14)

//--------------------串口------------------------//
//打开串口时发生错误。
#define SN_ERROR_COM_OPEN					(SN_ERROR_DISKIO_BEGIN - 50)

//读取串口(com)数据时发生错误
#define SN_ERROR_COM_READ					(SN_ERROR_DISKIO_BEGIN - 51)

//向串口写数据时发生错误。
#define SN_ERROR_COM_WRITE					(SN_ERROR_DISKIO_BEGIN - 52)

#endif // 
