#ifndef __JSTRING_H__
#define __JSTRING_H__

#ifndef WIN32
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <iconv.h>
#else
#include <time.h>
#pragma warning (disable: 4996)
#endif

#ifndef _STRING_
#include <string>
using namespace std;
#endif

#ifndef _VECTOR_
#include <vector>
using namespace std;
#endif

class JString : public string
{
public:
	JString(const char* sz = NULL):string(sz?sz:""){};
	JString(const JString& str):string(str){};
	JString(const string& str):string(str){};
	const JString& operator = (const char* sz){assign(sz?sz:"");return *this;};
	const JString& operator = (const JString& str){if(this != &str)assign(str);return *this;};
	const JString& operator = (const string& str){if(this != &str)assign(str);return *this;};
	operator char* (){return (char*)c_str();};
		
	void MakeLower();
	void MakeUpper();
	void TrimLeft(char trim=' ');
	int Compare(const JString& str) const;
	int CompareNoCase(const JString& str) const;
	void Replace(const JString& strOld , const JString& strNew);
	int Find(const JString& str,int iStart = 0) const;
	JString Left(int nCount) const;
	JString Right(int nCount) const;
	JString Mid(int nPlace, int nCount = -1) const;
	void Format(const char* szFormat, ...);
	void Decompose(const JString& split, vector<JString>& arStr) const;

	LONG ToLong() const;
	LONGLONG ToLongLong() const;
};

//编码转换函数
inline string StringAsciiToUtf8(const string& str);
inline string StringUtf8ToAscii(const string& str);

inline wstring StringAsciiToUnicode(const string& str);
inline wstring StringUtf8ToUnicode(const string& str);
inline string StringUnicodeToAscii(const wstring& str);
inline string StringUnicodeToUtf8(const wstring& str);

//整数转换函数
inline LONG StringToLong(const string& str);
inline LONGLONG	StingToLongLong(const string& str);
inline DWORD_PTR HexStringToPtr(const string& str);
inline string LongToString(LONG value);
inline string LongLongToString(LONGLONG value);
inline string PtrToHexString(DWORD_PTR value);


//时间转换函数
time_t	StringToTime(const string& strTime);
time_t	ShortStringToTime(const string& strTime);
string  TimeToString(const time_t& tm);
string  TimeToShortString(const time_t& tm);

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//重载字符串拷贝函数
inline char* strncpy(char* dest, const char* source, size_t count)
{
	//assert(dest);
	//assert(count > 0);

	if(source == NULL)
	{
		if(count > 0)
		{
			dest[0] = '\0';
		}
		return dest;
	}

	char* p = dest;
	for(int i = 0;i < (int)count;i++)
	{
		if((p[i] = source[i]) == '\0')
		{
			break;
		}
		
	}
	return dest;
}

inline string StringUnicodeToUtf8(const wstring& str)
{
	int nLen = WideCharToMultiByte(CP_UTF8,0,str.c_str(),-1,NULL,0,NULL,NULL);

	string strAscii;
	strAscii.assign(nLen - 1,0);
	WideCharToMultiByte(CP_UTF8,0,str.c_str(),-1,(LPSTR)strAscii.data(),nLen,NULL,NULL);
	return strAscii;
}

inline string StringUnicodeToAscii(const wstring& str)
{
	int nLen = WideCharToMultiByte(CP_ACP,0,str.c_str(),-1,NULL,0,NULL,NULL);

	string strAscii;
	strAscii.assign(nLen - 1,0);
	WideCharToMultiByte(CP_ACP,0,str.c_str(),-1,(LPSTR)strAscii.data(),nLen,NULL,NULL);
	return strAscii;
}

inline wstring StringAsciiToUnicode(const string& str)
{
	wstring wstr;
	int nLen = MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,NULL,0);

	wstr.assign(nLen - 1,0);
	MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,(LPWSTR)wstr.data(),nLen);
	return wstr;
}
inline wstring StringUtf8ToUnicode(const string& str)
{
	wstring wstr;
    int nLen = MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,NULL,0);

    wstr.assign(nLen - 1,0);
    MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,(LPWSTR)wstr.data(),nLen);
    return wstr;
}

inline string StringAsciiToUtf8(const string& str)
{
	wstring wstr;
    int nLen = MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,NULL,0);

    wstr.assign(nLen - 1,0);
    MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,(LPWSTR)wstr.data(),nLen);
	//
	string strUtf8;
	nLen = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,NULL,0,NULL,NULL);
	strUtf8.assign(nLen - 1,0);
	WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,(LPSTR)strUtf8.data(),nLen,NULL,NULL);
	return strUtf8;
}

inline string StringUtf8ToAscii(const string& str)
{
	wstring wstr;
    int nLen = MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,NULL,0);

    wstr.assign(nLen - 1,0);
    MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,(LPWSTR)wstr.data(),nLen);
	//
	string strAscii;
	nLen = WideCharToMultiByte(CP_ACP,0,wstr.c_str(),-1,NULL,0,NULL,NULL);
	strAscii.assign(nLen - 1,0);
	WideCharToMultiByte(CP_ACP,0,wstr.c_str(),-1,(LPSTR)strAscii.data(),nLen,NULL,NULL);
	return strAscii;
}

inline LONG StringToLong(const string& str)
{
	LONG value = 0;
	sscanf(str.c_str(),"%d",(int*)&value);
	return value;
}

inline LONGLONG	StingToLongLong(const string& str)
{
	LONGLONG value = 0;
	sscanf(str.c_str(),"%I64d",&value);
	return value;
}

inline DWORD_PTR HexStringToPtr(const string& str)
{
	DWORD_PTR value = 0;
	sscanf(str.c_str(),"%x",(unsigned int*)&value);
	return value;
}

inline string LongToString(LONG value)
{
	char sz[128] = {0};
	sprintf(sz,"%d",(int)value);
	return sz;
}

inline string LongLongToString(LONGLONG value)
{
	char sz[128] = {0};
	sprintf(sz,"%I64d",value);
	return sz;
}

inline string PtrToHexString(DWORD_PTR value)
{
	char sz[128] = {0};
	sprintf(sz,"%08x",(unsigned int)value);
	return sz;
}

inline time_t StringToTime(const string& strTime)
{
	struct tm tmLocal;

	sscanf(strTime.c_str(), "%4d-%2d-%2d %2d:%2d:%2d",     
                 &tmLocal.tm_year, 
                 &tmLocal.tm_mon, 
                 &tmLocal.tm_mday, 
                 &tmLocal.tm_hour, 
                 &tmLocal.tm_min,
                 &tmLocal.tm_sec);
           
    tmLocal.tm_year -= 1900;
    tmLocal.tm_mon--;
    tmLocal.tm_isdst = -1;

    return mktime(&tmLocal);
}

inline time_t ShortStringToTime(const string& strTime)
{
	struct tm tmLocal;

	sscanf(strTime.c_str(), "%4d%2d%2d%2d%2d%2d",     
                 &tmLocal.tm_year, 
                 &tmLocal.tm_mon, 
                 &tmLocal.tm_mday, 
                 &tmLocal.tm_hour, 
                 &tmLocal.tm_min,
                 &tmLocal.tm_sec);
           
    tmLocal.tm_year -= 1900;
    tmLocal.tm_mon--;
    tmLocal.tm_isdst = -1;

    return mktime(&tmLocal);
}

inline string TimeToString(const time_t& tm)
{
	struct tm tmLocal;
	tmLocal = *localtime(&tm);

	JString strTime;
	strTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		tmLocal.tm_year+1900, tmLocal.tm_mon+1, tmLocal.tm_mday,
		tmLocal.tm_hour, tmLocal.tm_min,tmLocal.tm_sec);
	return strTime;
}

inline string TimeToShortString(const time_t& tm)
{
	struct tm tmLocal;

	tmLocal = *localtime(&tm);

	JString strTime;
	strTime.Format("%04d%02d%02d%02d%02d%02d",
		tmLocal.tm_year+1900, tmLocal.tm_mon+1, tmLocal.tm_mday,
		tmLocal.tm_hour, tmLocal.tm_min,tmLocal.tm_sec);
	return strTime;
}

#endif
