#ifndef __WINEVENT_H__
#define __WINEVENT_H__

class CWinEvent
{
public:
	CWinEvent(void);
	virtual ~CWinEvent(void);

public:
	long Init(const char* szName, BOOL bManualReset = TRUE);
	void Single();
	void UnSingle();
	BOOL TrySingle(DWORD dwTimeOut);
	void Release();

private:
	HANDLE			m_hEvent;
	BOOL			m_bManualReset;
};

//Ïß³Ìº¯Êý
typedef void* (WINAPI *FCN_THREAD_PROC)(void*);

inline HANDLE CreateThreadEx(FCN_THREAD_PROC proc, void* param)
{
	return ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)proc, param, NULL, NULL);
}

inline void WaitThread(HANDLE hThread)
{
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

inline void CloseThread(HANDLE hThread)
{
	CloseHandle(hThread);
}

#endif
