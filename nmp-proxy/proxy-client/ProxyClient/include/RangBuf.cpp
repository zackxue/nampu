#include "PublicFuncImpl.h"
#include "RangBuf.h"

CRangBuf::CRangBuf(void)
{
	m_lRealSize = 0;
	m_pRealBuf = NULL;
	m_bOuterBuffer = FALSE;		

	m_plBegin = NULL;
	m_plEnd = NULL;
	m_pBuf = NULL;
	m_lSize = 0;
}

CRangBuf::~CRangBuf(void)
{
	if(!m_bOuterBuffer && m_pRealBuf)
	{
		delete [] m_pRealBuf;
		m_pRealBuf = NULL;
		m_pBuf = NULL;
	}
}

long CRangBuf::Init(void *pBuf, long lSize)
{
	if(lSize < (sizeof(long)*2 + 1))
	{
		return -1;
	}

	m_lRealSize = lSize;
	if(pBuf == NULL)
	{
		//Ôö´ó»º³å,2¸ölong
		m_lRealSize += sizeof(long)*2;

		m_pRealBuf = new BYTE[m_lRealSize];
		memset(m_pRealBuf,0,m_lRealSize);
		m_bOuterBuffer = FALSE;
	}
	else
	{
		m_pRealBuf = (BYTE*)pBuf;
		m_bOuterBuffer = TRUE;
	}

	m_plBegin = (long*)m_pRealBuf;
	m_plEnd = (long*)(m_pRealBuf + sizeof(long));
	m_pBuf = m_pRealBuf + sizeof(long)*2;
	m_lSize = m_lRealSize - sizeof(long)*2;

	return 0;
}

long CRangBuf::GetSize()
{
	return m_lSize;
}

long CRangBuf::GetUsedSize()
{
	m_mtLock.Lock();

	long lUsedSize = 0;
	if(*m_plBegin <= *m_plEnd)
	{
		lUsedSize = *m_plEnd - *m_plBegin;
	}
	else
	{
		lUsedSize = m_lSize - *m_plBegin + *m_plEnd;
	}

	m_mtLock.Unlock();

	return lUsedSize;
}

long CRangBuf::GetRemainSize()
{
	m_mtLock.Lock();

	long lRemainSize = 0;
	if(*m_plBegin <= *m_plEnd)
	{
		lRemainSize = m_lSize - (*m_plEnd - *m_plBegin) - 1;
	}
	else
	{
		lRemainSize = *m_plBegin - *m_plEnd - 1;
	}

	m_mtLock.Unlock();

	return lRemainSize;
}

long CRangBuf::Push(const void* pData , long lSize)
{
	m_mtLock.Lock();

	if(*m_plBegin <= *m_plEnd)
	{
		int iCopySize = min(lSize,m_lSize - (*m_plEnd - *m_plBegin) - 1);

		if(iCopySize + *m_plEnd < m_lSize)
		{
			memcpy(m_pBuf + *m_plEnd , pData , iCopySize);
			*m_plEnd += iCopySize;
			if(*m_plEnd >= m_lSize)
			{
				*m_plEnd = 0;
			}
		}
		else
		{
			int iR = m_lSize - *m_plEnd;
			memcpy(m_pBuf + *m_plEnd, pData, iR);
			memcpy(m_pBuf, (BYTE*)pData + iR, iCopySize - iR);
			*m_plEnd = iCopySize - iR;
		}

		m_mtLock.Unlock();
		return iCopySize;

	}	
	else
	{
		int iCopySize = min(lSize, *m_plBegin - *m_plEnd - 1);

		if(iCopySize > 0)
		{
			memcpy(m_pBuf + *m_plEnd, pData, iCopySize);
			*m_plEnd += iCopySize;
		}

		m_mtLock.Unlock();
		return iCopySize;
	}
}

long CRangBuf::Pop(void* pData, long lSize)
{
	m_mtLock.Lock();

	if(*m_plBegin <= *m_plEnd)
	{
		int iCopySize = min(lSize, *m_plEnd - *m_plBegin);

		if(iCopySize > 0)
		{
			memcpy( pData, m_pBuf + *m_plBegin, iCopySize);
			*m_plBegin += iCopySize;
		}

		m_mtLock.Unlock();
		return iCopySize;
	}
	else
	{
		int iCopySize = min(lSize, m_lSize - (*m_plBegin - *m_plEnd));

		if(*m_plBegin + iCopySize < m_lSize)
		{
			memcpy(pData, m_pBuf + *m_plBegin, iCopySize);
			*m_plBegin += iCopySize;

			if(*m_plBegin >= m_lSize)
			{
				*m_plBegin = 0;
			}
		}
		else
		{
			int iR = m_lSize - *m_plBegin;
			memcpy( pData, m_pBuf + *m_plBegin, iR);
			memcpy( (BYTE*)pData + iR, m_pBuf, iCopySize - iR);
			*m_plBegin = iCopySize - iR;
		}

		m_mtLock.Unlock();
		return iCopySize;
	}
}

long CRangBuf::MoveData(CRangBuf* pQueueBuf)
{
	m_mtLock.Lock();

	long lSize = pQueueBuf->Pop(m_pBuf, m_lSize-1);
	*m_plBegin = 0;
	*m_plEnd = lSize;

	m_mtLock.Unlock();

	return lSize;
}
