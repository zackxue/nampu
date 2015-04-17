#include "PublicFuncImpl.h"
#include "SerialLong.h"

CSerialLong::CSerialLong(void)
{
	m_lSerialValue = 0;
}

CSerialLong::~CSerialLong(void)
{
}

long CSerialLong::RequestSerialLong()
{
	long lValue = -1;

	m_LockMutex.Lock();
	
	while(lValue < 1)
	{
		lValue = ++m_lSerialValue;
		if(lValue < 1)
		{
			lValue = m_lSerialValue = 1;
		}
		if(m_setSerial.find(lValue) == m_setSerial.end())
		{
			m_setSerial.insert(lValue);
			break;
		}
	}

	m_LockMutex.Unlock();

	return lValue;
}

long CSerialLong::ReleaseSerialLong(long lValue)
{
	m_LockMutex.Lock();
	m_setSerial.erase(lValue);
	m_LockMutex.Unlock();

	return 0;
}

CSerialLong& CSerialLong::Instance()
{
	static CSerialLong inc;
	return inc;
}