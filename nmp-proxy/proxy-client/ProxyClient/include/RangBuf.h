#pragma once

class CRangBuf
{
public:
					CRangBuf(void);
	virtual			~CRangBuf(void);

	long			Init(void* pBuf, long lSize);
	void			Uninit();
	long			GetSize();
	long			GetUsedSize();
	long			GetRemainSize();
	long			Push(const void* pData, long lSize);
	long			Pop(void* pData, long lSize);

	long			MoveData(CRangBuf* pQueueBuf);

private:
	CMutexEx		m_mtLock;

	BYTE*			m_pRealBuf;
	long			m_lRealSize;
	BOOL			m_bOuterBuffer;

	BYTE*			m_pBuf;
	long			m_lSize;
	long			*m_plBegin;
	long			*m_plEnd;
};
