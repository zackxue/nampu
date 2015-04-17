#include "PublicFuncImpl.h"
#include "LockMem.h"

CLockMem::CLockMem()
{
	m_hMapFile = NULL;
	m_hMutexMemBlock = NULL;
	m_pMemBlock = NULL;
}

CLockMem::~CLockMem()
{
	Close();
}

long CLockMem::Init(const char* szName)
{
	Close();

	//ACL
	SECURITY_DESCRIPTOR 	sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	//
	char szNameTmp[1024] = {0};
	strcat(szNameTmp, szName);
	strcat(szNameTmp, "JXJWINLOCKMEM");
	m_hMutexMemBlock = ::CreateMutex(&sa, FALSE, szName ? StringAsciiToUnicode(szNameTmp).c_str() : NULL);
	if(m_hMutexMemBlock == NULL)
	{
		return -1;
	}

	LockMemBlock(INFINITE);

	strcat(szNameTmp, "JXJWINMAPMEM");
	m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, sizeof(stMemBlockLock), szName ? StringAsciiToUnicode(szNameTmp).c_str() : NULL);
	if(m_hMapFile == NULL)
	{
		UnlockMemBlock();
		return -2;
	}
	if(::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		m_pMemBlock = (stMemBlockLock*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(stMemBlockLock));
	}
	else
	{
		m_pMemBlock = (stMemBlockLock*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(stMemBlockLock));
		memset(m_pMemBlock, 0, sizeof(stMemBlockLock));
	}

	UnlockMemBlock();

	return 0;
}

void CLockMem::Close()
{
	if(m_pMemBlock != NULL)
	{
		UnmapViewOfFile(m_pMemBlock);
		m_pMemBlock = NULL;
	}
	if(m_hMapFile != NULL)
	{
		CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}
	if(m_hMutexMemBlock != NULL)
	{
		::CloseHandle(m_hMutexMemBlock);
		m_hMutexMemBlock = NULL;
	}
}

BOOL CLockMem::LockMemBlock(DWORD dwTimeout)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hMutexMemBlock,dwTimeout);
}

void CLockMem::UnlockMemBlock()
{
	::ReleaseMutex(m_hMutexMemBlock);
}

BOOL CLockMem::LockRead(DWORD dwTimeout)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	DWORD dwStartTick = GetTickCount();
	while((dwTimeout == INFINITE) || ((GetTickCount() - dwStartTick) <= dwTimeout))
	{
		if(LockMemBlock(10))
		{
			long lPlace = FindThreadPlace(dwThreadId);
			if(lPlace >= 0 && (m_pMemBlock->lThreadWriteCount[lPlace] > 0 || m_pMemBlock->lThreadReadCount[lPlace] > 0))
			{
				//已经获取读或写权限
				m_pMemBlock->lReadCount++;
				AddPlaceStatus(lPlace,dwThreadId, READ_STATUS);
				UnlockMemBlock();
				return TRUE;
			}
		
			if(m_pMemBlock->lThreadCount < MAX_THREAD_COUNT 
				&& m_pMemBlock->lWriteCount < 1 
				&& m_pMemBlock->lWriteRequestCount < 1)
			{
				m_pMemBlock->lReadCount++;
				AddPlaceStatus(lPlace, dwThreadId, READ_STATUS);
				UnlockMemBlock();
				return TRUE;
			}
			
			UnlockMemBlock();
		
			Sleep(1);
		}
	}

	return FALSE;
}

BOOL CLockMem::LockWrite(DWORD dwTimeout)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	BOOL bAddRequest = FALSE;
	BOOL bReadToWrite = FALSE;
	DWORD dwStartTick = GetTickCount();
	while((dwTimeout == INFINITE) || ((GetTickCount() - dwStartTick) <= dwTimeout))
	{
		if(LockMemBlock(10))
		{
			if(!bAddRequest)
			{
				m_pMemBlock->lWriteRequestCount++;
				bAddRequest = TRUE;
			}

			long lPlace = FindThreadPlace(dwThreadId);
			if(lPlace >= 0)
			{
				//已经获取写权限
				if(m_pMemBlock->lThreadWriteCount[lPlace] > 0)
				{
					m_pMemBlock->lWriteCount++;
					m_pMemBlock->lWriteRequestCount--;
					AddPlaceStatus(lPlace, dwThreadId, WRITE_STATUS);
					UnlockMemBlock();
					return TRUE;
				}

				//已经获取读权限
				if(m_pMemBlock->lThreadReadCount[lPlace] > 0)
				{
					//释放读权限，同时保存读权限数，申请写权限 
					m_pMemBlock->lThreadPreReadCount[lPlace] += m_pMemBlock->lThreadReadCount[lPlace];
					m_pMemBlock->lReadCount -= m_pMemBlock->lThreadReadCount[lPlace];
					m_pMemBlock->lThreadReadCount[lPlace] = 0;

					bReadToWrite = TRUE;
				}
			}

			if(m_pMemBlock->lThreadCount < MAX_THREAD_COUNT 
				&& m_pMemBlock->lReadCount < 1 
				&& m_pMemBlock->lWriteCount < 1)
			{
				m_pMemBlock->lWriteCount++;
				m_pMemBlock->lWriteRequestCount--;
				AddPlaceStatus(lPlace, dwThreadId, WRITE_STATUS);
				UnlockMemBlock();
				return TRUE;
			}
			
			UnlockMemBlock();
			Sleep(1);
		}
	}

	if(bAddRequest)
	{
		LockMemBlock(INFINITE);
		m_pMemBlock->lWriteRequestCount--;
		UnlockMemBlock();
	}

	if(bReadToWrite)
	{
		//重新申请读
		LockMemBlock(INFINITE);
		long lPlace = FindThreadPlace(dwThreadId);
		long lReReadCount = m_pMemBlock->lThreadPreReadCount[lPlace];
		m_pMemBlock->lThreadPreReadCount[lPlace] = 0;
		UnlockMemBlock();
		ReLockRead(dwThreadId,lReReadCount);
	}

	return FALSE;
}

void CLockMem::ReLockRead(DWORD dwThreadId,long lReadCount)
{
	for(;;)
	{
		if(LockMemBlock(10))
		{
			long lPlace = FindThreadPlace(dwThreadId);
			if(lPlace >= 0 && (m_pMemBlock->lThreadWriteCount[lPlace] > 0 || m_pMemBlock->lThreadReadCount[lPlace] > 0))
			{
				//已经获取读或写权限
				m_pMemBlock->lReadCount += lReadCount;
				AddPlaceStatus(lPlace, dwThreadId,READ_STATUS, lReadCount);
				UnlockMemBlock();
				return;
			}
		
			if(m_pMemBlock->lThreadCount < MAX_THREAD_COUNT 
				&& m_pMemBlock->lWriteCount < 1 
				&& m_pMemBlock->lWriteRequestCount < 1)
			{
				m_pMemBlock->lReadCount += lReadCount;
				AddPlaceStatus(lPlace, dwThreadId, READ_STATUS, lReadCount);
				UnlockMemBlock();
				return;
			}
			
			UnlockMemBlock();
		
			Sleep(1);
		}
	}
}

void CLockMem::UnlockRead(BOOL bReleaseResource)
{
	DWORD dwThreadId = ::GetCurrentThreadId();

	LockMemBlock(INFINITE);

	long lPlace = FindThreadPlace(dwThreadId);
	if(lPlace >= 0)
	{		
		m_pMemBlock->lThreadReadCount[lPlace]--;
		m_pMemBlock->lReadCount--;

		if(m_pMemBlock->lThreadReadCount[lPlace] < 1 && m_pMemBlock->lThreadWriteCount[lPlace] < 1)
		{
			RemovePlaceStatus(lPlace, bReleaseResource);
		}
	}

	UnlockMemBlock();
}

void CLockMem::UnlockWrite(BOOL bReleaseResource)
{
	DWORD dwThreadId = ::GetCurrentThreadId();

	LockMemBlock(INFINITE);

	long lPlace = FindThreadPlace(dwThreadId);
	if(lPlace >= 0)
	{		
		m_pMemBlock->lThreadWriteCount[lPlace]--;
		m_pMemBlock->lWriteCount--;	

		if(m_pMemBlock->lThreadReadCount[lPlace] < 1 && m_pMemBlock->lThreadWriteCount[lPlace] < 1)
		{
			if(m_pMemBlock->lThreadPreReadCount[lPlace] > 0)
			{
				long lReReadCount = m_pMemBlock->lThreadPreReadCount[lPlace];
				m_pMemBlock->lThreadPreReadCount[lPlace] = 0;
				UnlockMemBlock();
				//重新申请读
				ReLockRead(dwThreadId, lReReadCount);
				return;
			}
			else
			{
				RemovePlaceStatus(lPlace, bReleaseResource);
			}
		}
	}		

	UnlockMemBlock();
}

void CLockMem::AddPlaceStatus(long lPlace, DWORD dwThreadId, long lStatus, long lCount)
{
	if(lPlace >= 0)
	{
		if(lStatus == READ_STATUS)
		{
			m_pMemBlock->lThreadReadCount[lPlace] += lCount;
		}
		else
		{
			m_pMemBlock->lThreadWriteCount[lPlace] += lCount;
		}		
	}
	else
	{
		lPlace = -lPlace-1;
		//移动后面的数据
		for(int i = m_pMemBlock->lThreadCount - 1; i >= lPlace; i--)
		{
			m_pMemBlock->dwTheadId[i+1] =  m_pMemBlock->dwTheadId[i];
			m_pMemBlock->lThreadWriteCount[i+1] =  m_pMemBlock->lThreadWriteCount[i];
			m_pMemBlock->lThreadReadCount[i+1] =  m_pMemBlock->lThreadReadCount[i];
			m_pMemBlock->lThreadPreReadCount[i+1] =  m_pMemBlock->lThreadPreReadCount[i];
		}
		m_pMemBlock->lThreadCount++;
		m_pMemBlock->dwTheadId[lPlace] = dwThreadId;

		if(lStatus == READ_STATUS)
		{
			m_pMemBlock->lThreadReadCount[lPlace] = lCount;
			m_pMemBlock->lThreadWriteCount[lPlace] = 0;
			m_pMemBlock->lThreadPreReadCount[lPlace] = 0;
		}
		else
		{
			m_pMemBlock->lThreadReadCount[lPlace] = 0;
			m_pMemBlock->lThreadWriteCount[lPlace] = lCount;
			m_pMemBlock->lThreadPreReadCount[lPlace] = 0;
		}		
	}
}

void CLockMem::RemovePlaceStatus(long lPlace,BOOL bReleaseResource)
{
	if(lPlace < 0)
	{
		return;
	}

	if(bReleaseResource)
	{
		//移动后面的数据
		for(int i = lPlace+1; i < m_pMemBlock->lThreadCount; i++)
		{
			m_pMemBlock->dwTheadId[i-1] =  m_pMemBlock->dwTheadId[i];
			m_pMemBlock->lThreadWriteCount[i-1] =  m_pMemBlock->lThreadWriteCount[i];
			m_pMemBlock->lThreadReadCount[i-1] =  m_pMemBlock->lThreadReadCount[i];
			m_pMemBlock->lThreadPreReadCount[i-1] =  m_pMemBlock->lThreadPreReadCount[i];
		}
		m_pMemBlock->lThreadCount--;
	}
	else
	{
		//m_pMemBlock->dwTheadId[lPlace] =  m_pMemBlock->dwTheadId[lPlace];
		m_pMemBlock->lThreadWriteCount[lPlace] =  0;
		m_pMemBlock->lThreadReadCount[lPlace] =  0;
		m_pMemBlock->lThreadPreReadCount[lPlace] = 0;
	}
}

long CLockMem::FindThreadPlace(DWORD dwThreadId)
{
	//二分查找
	long lLeft = 0;
	long lRight = m_pMemBlock->lThreadCount - 1;
	while(lLeft <= lRight)
	{
		long lMid = (lLeft + lRight)/2;
		if(m_pMemBlock->dwTheadId[lMid] == dwThreadId)
		{
			return lMid;
		}
		else if(m_pMemBlock->dwTheadId[lMid] < dwThreadId)
		{
			lLeft = lMid + 1;
		}
		else
		{
			lRight = lMid - 1;
		}
	}

	return -lLeft-1;
}