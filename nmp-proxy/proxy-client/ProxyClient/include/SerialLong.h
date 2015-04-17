#ifndef __SERIALLONG_H__
#define __SERIALLONG_H__

class CSerialLong
{
public:
	CSerialLong(void);
	~CSerialLong(void);
public:
	//
	long RequestSerialLong();
	long ReleaseSerialLong(long lValue);

	static CSerialLong& Instance();
protected:
	CMutexEx		m_LockMutex;
	set<long>		m_setSerial;
	long			m_lSerialValue;
};

#endif