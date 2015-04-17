#include "PublicFuncImpl.h"
#include "JThreadBase.h"

CJThreadBase::CJThreadBase(int nServiceCount)
{
	m_nServiceCount = nServiceCount;
	m_arhThread = new HANDLE[nServiceCount];
	memset(m_arhThread, 0, sizeof(HANDLE)*nServiceCount);
}

CJThreadBase::~CJThreadBase()
{
	Stop();
	delete []m_arhThread;
}

long CJThreadBase::Start()
{
	m_EventSingle.Init(NULL);
	m_EventSingle.UnSingle();

	for(int i = 0; i < m_nServiceCount; i++)
	{
		void** pParam = new void*[2];
		pParam[0] = this;
		pParam[1] = (void*)i;
		m_arhThread[i] = CreateThreadEx(ThreadService, pParam);
		if(m_arhThread[i] == NULL)
		{
			return -1;
		}
	}
	return 0;
}

void CJThreadBase::Stop()
{
	m_EventSingle.Single();

	for(int i = 0; i < m_nServiceCount; i++)
	{
		if(m_arhThread[i] != NULL)
		{
			WaitThread(m_arhThread[i]);
			m_arhThread[i] = NULL;
		}
	}
}

void* WINAPI CJThreadBase::ThreadService(void* pParam)
{
	void** lpParam = (void**)pParam;
	CJThreadBase* pObject = (CJThreadBase*)lpParam[0];
	int nServiceId = (int)lpParam[1];
	delete []lpParam;

	pObject->OnService(nServiceId);

	return NULL;
}

BOOL CJThreadBase::IsClose(DWORD dwMilSec)
{
	return m_EventSingle.TrySingle(dwMilSec);
}