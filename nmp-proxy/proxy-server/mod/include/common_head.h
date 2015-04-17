#ifndef __COMMON_HEAD_H

#define __COMMON_HEAD_H

#define UINT         unsigned int
#define LONG         long
#define ULONG        unsigned long
#define DWORD        unsigned long
#define WORD         unsigned short
#define FOURCC       unsigned long
#define DOUBLE       double
#define CALLBACK     
#define HWND         int
#define BOOL         int
#define INT          int
#define BYTE         char
#define PBYTE        char *

#define LPVOID       void *
#define LPDWORD      unsigned long*

#define SOCKET       int 
#define HANDLE       int 
#define FILETIME     unsigned int
#define _bstr_t      char *   //string

#define CHAR         char
#define TCHAR        char
#define UCHAR        char

#define WPARAM       void *
#define LPARAM       void *

//#define string       char *
//#define NULL         0
#define TRUE         1
#define FALSE        0

#define USHORT       unsigned short
#define Int8         int
#define Int16        int
#define UInt16       unsigned int
#define CRITICAL_SECTION     pthread_mutex_t
#define NETSDKDLL_API
#define __stdcall
#define WINAPI    
#define DLLPLAYER_API

#define Sleep(time)        usleep(1000*(time))
//typedef _T(const char* str)    str ;

#define		AV_FALG		"TopseeH264"	//avhd

#define		ALLOW_LOGIN_MORETHAN_ONE			1

//2G
#define		AVI_RECORD_RESERVED_FREE_SPACE	2048


#endif






