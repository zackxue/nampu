#include "PublicFuncImpl.h"
#include "JFile.h"

CJFile::CJFile(void)
{
	m_pFile		= NULL;
	m_lTimeOut	= 3*1000;
	m_lErrCode	= 0;
}

CJFile::~CJFile(void)
{
	Close();
}

void CJFile::Close()
{
	if(m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

long CJFile::Open(const char* szFileName, BOOL bWrite)
{
	if(NULL == szFileName || strlen(szFileName) <= 0)
	{
		return -1;
	}

	m_lockRW.Lock();

	Close();

	if(bWrite)
	{
		m_pFile = fopen(szFileName, "wb");
	}
	else
	{
		int iRet = access(szFileName, 0);
		if(iRet != 0)
		{
			m_lErrCode = *_errno();
			m_lockRW.Unlock();
			return -2;
		}
		m_pFile = fopen(szFileName, "rb");
	}
	if(NULL == m_pFile)
	{
		m_lErrCode = *_errno();
		m_lockRW.Unlock();
		return -3;
	}

	fseek(m_pFile, 0, SEEK_SET);

	m_lockRW.Unlock();
	return 0;
}

long CJFile::Write(const void* pData, long lLen)
{
	m_lockRW.Lock();
	if(NULL == m_pFile)
	{
		m_lockRW.Unlock();
		return -1;
	}

	DWORD dwTime = GetTickCount();
	DWORD dwWrited = 0;
	int iErrTick = 0;
	while(dwWrited < (DWORD)lLen)
	{
		if(GetTickCount() - dwTime > (DWORD)m_lTimeOut)
		{
			break;
		}

		size_t uRet = fwrite((BYTE*)pData + dwWrited, 1, lLen - dwWrited, m_pFile);
		if(uRet > 0)
		{
			dwWrited += uRet;
			fflush(m_pFile);
		}
		else
		{
			iErrTick++;
			if(iErrTick > 3)
			{
				m_lErrCode = *_errno();
			}
		}
	}

	m_lockRW.Unlock();
	return dwWrited;
}

long CJFile::Read(void* pBuf, long lSize)
{
	m_lockRW.Lock();
	if(NULL == m_pFile)
	{
		m_lockRW.Unlock();
		return -1;
	}

	DWORD dwTime = GetTickCount();
	DWORD dwReded = 0;
	DWORD iErrTick = 0;
	while(dwReded < (DWORD)lSize)
	{
		if(GetTickCount() - dwTime > (DWORD)m_lTimeOut)
		{
			break;
		}

		size_t uRet = fread((BYTE*)pBuf + dwReded, 1, lSize - dwReded, m_pFile);
		if(uRet > 0)
		{
			dwReded += uRet;
		}
		else
		{
			iErrTick++;
			if(iErrTick > 3)
			{
				m_lErrCode = *_errno();
			}
		}
	}

	m_lockRW.Unlock();
	return dwReded;
}

long CJFile::GetErrCode()
{
	return m_lErrCode;
}
