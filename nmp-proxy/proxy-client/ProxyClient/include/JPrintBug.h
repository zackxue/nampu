#ifndef __JPRINTBUG_H__
#define __JPRINTBUG_H__

#pragma warning(disable : 4996)

inline void OutputDebug(const char* szOut, ...);

inline void OutputDebug(const char* szOut, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap , szOut);
	int len = vsnprintf(buf,sizeof(buf) - 1 ,szOut,ap);
	va_end(ap);
	if(len < 0)
	{
		len = 0;
	}
	if(len > sizeof(buf) - 1)
	{
		len = sizeof(buf) - 1;
	}
	buf[len] = 0;

	OutputDebugStringA(buf);
}

#endif