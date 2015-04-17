#pragma once

class CMutexEx
{
public:
	CMutexEx(LPCTSTR lpszName = NULL);
	~CMutexEx(void);
public:
	void Lock(void);
	BOOL TryLock(DWORD dwWaitMilSec = 0);
	void Unlock(void);
protected:
	HANDLE m_hMutex;
};
