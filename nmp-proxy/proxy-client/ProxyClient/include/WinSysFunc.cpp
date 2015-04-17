#include "PublicFuncImpl.h"
#include "WinSysFunc.h"

HANDLE JCreateThread(ON_FUNC_THREAD func, LPVOID lpParam)
{
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, lpParam, NULL, NULL);
}

BOOL JWaitThread(HANDLE hThread, DWORD dwTimeout)
{
	return WaitForSingleObject(hThread, dwTimeout);
}

void JWaitCloseThread(HANDLE hThread)
{
	WaitForSingleObject(hThread, INFINITE);
	JCloseThread(hThread);
}

void JCloseThread(HANDLE hThread)
{
	CloseHandle(hThread);
}

////////////////////////////////////////////////////////////////////////////////////////////////
string JGetExePath()
{
	string strPath;
	char szExeFileName[1024];
	::GetModuleFileNameA(NULL, szExeFileName, sizeof(szExeFileName));
	strPath = szExeFileName;
	return strPath.substr(0, strPath.rfind('\\') + 1);
}

string GetPathByFunc(LPVOID pFuncAddr)
{
	MEMORY_BASIC_INFORMATION stMemInfo;
	memset(&stMemInfo, 0, sizeof(stMemInfo));
	::VirtualQuery(pFuncAddr, &stMemInfo, sizeof(stMemInfo));
	HMODULE hModule = (HMODULE)stMemInfo.AllocationBase;
	char szModulePath[1024];
	::GetModuleFileNameA(hModule, szModulePath, sizeof(szModulePath));
	return szModulePath;
}

HMODULE LoadDynamicLib(const char* szName)
{
	string strName(szName);
	long lPos = strName.rfind('\\');
	string strPath = strName.substr(0, lPos + 1);
	string strFileName = strName.substr(lPos + 1, -1);
	if(strFileName.find(".dll") == string::npos)
	{
		strName += ".dll";
	}
	wchar_t szCurrDir[1024];
	::GetCurrentDirectory(sizeof(szCurrDir), szCurrDir);

	::SetCurrentDirectory(StringAsciiToUnicode(strPath).c_str());
	HMODULE hKernel = ::LoadLibrary(_T("Kernel32.dll"));
	if(hKernel)
	{
		typedef BOOL (WINAPI *Func_SetDllDirectoryW)(LPCTSTR);
		Func_SetDllDirectoryW SetDllDirectoryW = (Func_SetDllDirectoryW)::GetProcAddress(hKernel, "SetDllDirectoryW");
		if(SetDllDirectoryW)
		{
			SetDllDirectoryW(StringAsciiToUnicode(strPath).c_str());
		}		
		FreeLibrary(hKernel);
	}

	HMODULE hLib = ::LoadLibrary(StringAsciiToUnicode(strName).c_str());
	::SetCurrentDirectory(szCurrDir);
	return hLib;
}

void* JAlloc(size_t size)
{
	return ::HeapAlloc(GetProcessHeap(), 0, size);
}
void JFree(void* pData)
{
	::HeapFree(GetProcessHeap(), 0, pData);
}