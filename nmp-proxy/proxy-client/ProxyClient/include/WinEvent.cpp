#include "PublicFuncImpl.h"
#include "WinEvent.h"

CWinEvent::CWinEvent(void)
{
	m_hEvent = NULL;
	m_bManualReset = TRUE;
}

CWinEvent::~CWinEvent(void)
{
}

long CWinEvent::Init(const char *szName, BOOL bManualReset)
{
	m_bManualReset = bManualReset;

	if(szName == NULL)
	{
		m_hEvent = CreateEvent(NULL, m_bManualReset, FALSE, NULL);
	}
	else
	{
		//ACL
		SECURITY_DESCRIPTOR 	sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

		SECURITY_ATTRIBUTES	sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = &sd;

		m_hEvent = CreateEvent(&sa, m_bManualReset, FALSE, StringAsciiToUnicode(szName).c_str());
	}

	return 0;
}

void CWinEvent::Single()
{
	SetEvent(m_hEvent);
}

void CWinEvent::UnSingle()
{
	ResetEvent(m_hEvent);
}

BOOL CWinEvent::TrySingle(DWORD dwTimeOut)
{
	return WaitForSingleObject(m_hEvent, dwTimeOut) == WAIT_OBJECT_0;
}

void CWinEvent::Release()
{
	if(m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}
