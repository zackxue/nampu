
#include <Windows.h>
#include <io.h> 

#include <string>
using namespace std;
#include "log.h"

#define MAX_LOG_SIZE (1024 * 1024 * 1)

string __get_pragram_name()
{
	string strPath;
	strPath.assign(260, 0);
	GetModuleFileNameA(NULL, (LPCH)strPath.data(), 260);
	int iPos = strPath.rfind('\\');
	string  strFile = strPath.substr(0, iPos + 1) + "jcu_talk." + "log";
	return strFile;
}


FILE *__get_log_file(string log_path)
{
	FILE *fd;

	//WideCharToMultiByte(CP_ACP, 0, log_path.GetBuffer(), -1, buf, sizeof(buf), 0, 0);
	fd = fopen(log_path.c_str(), "a+");
	
	return fd;
}

int __get_file_size(FILE *file)
{
	int size;

	if (!file)
	{
		return (-1);
	}
	
	size = filelength(fileno(file));

	return size;
}

void __write_log_file(const char *buf, int len)
{
	int ret;
	FILE *log_path = NULL;
	int log_size = 0;

	if (!(log_path = __get_log_file(__get_pragram_name())))
	{
		goto _error;
	}
	
	if ((log_size = __get_file_size(log_path)) < 0)
	{
		goto _error;
	}

	if (log_size >= MAX_LOG_SIZE)
	{
		ret = fseek(log_path, 0, SEEK_SET);
		if (ret < 0)
		{
			goto _error;
		}
	}

	fwrite(buf, 1, len, log_path);
	fclose(log_path);

	return;

_error:
	if (log_path)
	{
		fclose(log_path);
	}
	return;
}

void __log_out(const char *fmt, va_list ap)
{
	int ret, len;
	char buf[1024] = {0};
	SYSTEMTIME st;

	GetLocalTime(&st);
	len = sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d.%06d]", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	if (len < 0)
	{
		return;
	}

	ret = vsnprintf((char *)buf + len, sizeof(buf) - 1 - len, fmt, ap);
	if (ret > 0)
	{
		__write_log_file(buf, ret + len);
	}
}

void log_out(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	__log_out(fmt, ap);
	va_end(ap);
}