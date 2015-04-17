#include "PublicFuncImpl.h"
#include "MutexEx.h"

CMutexEx::CMutexEx(LPCTSTR lpszName)
{
	if(lpszName)
	{
		//ACL
		SECURITY_DESCRIPTOR 	sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

		SECURITY_ATTRIBUTES	sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = &sd;

		m_hMutex = ::CreateMutex(&sa,FALSE,lpszName);
	}
	else
	{
		m_hMutex = ::CreateMutex(NULL,FALSE,lpszName);
	}
	
}

CMutexEx::~CMutexEx(void)
{
	::CloseHandle(m_hMutex);
}

void CMutexEx::Lock(void)
{
	::WaitForSingleObject(m_hMutex,INFINITE);
}
void CMutexEx::Unlock(void)
{
	::ReleaseMutex(m_hMutex);
}

BOOL CMutexEx::TryLock(DWORD dwWaitMilSec)
{
	return ::WaitForSingleObject(m_hMutex,dwWaitMilSec) == WAIT_OBJECT_0;
}
