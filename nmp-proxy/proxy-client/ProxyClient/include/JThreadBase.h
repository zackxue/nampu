#ifndef __JTHREADBASE_H__
#define __JTHREADBASE_H__

class CJThreadBase
{
public:
	CJThreadBase(int nServiceCount = 1);
	virtual ~CJThreadBase();

	virtual long Start();
	virtual void Stop();
	virtual void OnService(int nID) = 0;
	BOOL IsClose(DWORD dwMilSec = 0);

protected:
	static void* WINAPI ThreadService(void* pParam);

private:
	int		m_nServiceCount;
	HANDLE	*m_arhThread;
	CWinEvent m_EventSingle;
};

#endif