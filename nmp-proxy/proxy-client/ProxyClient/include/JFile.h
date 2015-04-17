#pragma once

class CJFile
{
public:
	CJFile(void);
	virtual ~CJFile(void);

	long		Open(const char* szFileName, BOOL bWrite = TRUE);
	long		Write(const void* pData, long lLen);
	long		Read(void* pBuf, long lSize);
	void		Close();

	long		GetErrCode();

private:
	FILE*		m_pFile;
	long		m_lTimeOut;
	CMutexEx	m_lockRW;
	long		m_lErrCode;
};
