#pragma once

typedef void* (WINAPI ON_FUNC_THREAD)(LPVOID lpParam);

HANDLE JCreateThread(ON_FUNC_THREAD func, LPVOID lpParam);
BOOL JWaitThread(HANDLE hThread, DWORD dwTimeout = -1);
void JWaitCloseThread(HANDLE hThread);
void JCloseThread(HANDLE hThread);
//-------------------------------------------------------------

string JGetExePath();
string JGetExePathEx();
string GetPathByFunc(LPVOID pFuncAddr);
HMODULE LoadDynamicLib(const char* szName);
void* JAlloc(size_t size);
void JFree(void* pData);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define J_DECLEAR_NEW \
public:\
	static void* operator new (size_t size);\
	static void operator delete(void* pData);

#define J_DECLEAR_REF \
public:\
	virtual long AddRef();\
	virtual long Release();\
	static void* CreateInstance();\
private:\
	volatile long	m_lRef;

#define J_DECLEAR_BASE \
	J_DECLEAR_NEW\
	J_DECLEAR_REF


#define J_INIT_REF_VALUE m_lRef = 1

#define J_IMPLEMANT_NEW(class_name, interface_name) \
	void* class_name::operator new(size_t size)\
	{\
		return JAlloc(size);\
	}\
	void class_name::operator delete(void* pData)\
	{\
		return JFree(pData);\
	}

#define J_IMPLEMANT_REF(class_name, interface_name) \
	long class_name::AddRef()\
	{\
		::InterlockedExchangeAdd(&m_lRef, 1);\
		return m_lRef;\
	}\
	long class_name::Release()\
	{\
		long lRef = ::InterlockedExchangeAdd(&m_lRef, -1);\
		if(m_lRef < 1)\
		{\
			delete this;\
		}\
		return lRef;\
	}\
	void* class_name::CreateInstance()\
	{\
		return (void*)new class_name;\
	}

#define J_IMPLEMANT_BASE(class_name, interface_name) \
	J_IMPLEMANT_NEW(class_name, interface_name)\
	J_IMPLEMANT_REF(class_name, interface_name)


#define CREATE_OBJ(class_name) (class_name*)class_name::CreateInstance()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IJXJUnknown
{
public:
	virtual long			AddRef() = 0;
	virtual long			Release() = 0;
};

template<class T>
class JXJSmartPtr
{
public:
	JXJSmartPtr():m_pIns(NULL){}
	JXJSmartPtr(T* pIns, BOOL bAddRef = TRUE)
	{
		m_pIns = pIns;
		if(bAddRef && m_pIns)
		{
			m_pIns->AddRef();
		}
	}

	virtual ~JXJSmartPtr()
	{
		if(m_pIns)
		{
			m_pIns->Release();
			m_pIns = NULL;
		}
	}

	long Release()
	{
		T* pIns = (T*)InterlockedExchange((LONG*)&m_pIns, NULL);
		if(pIns)pIns->Release();
		return -1;
	}

	T* operator ->()
	{
		return m_pIns;
	}

	JXJSmartPtr& operator = (const JXJSmartPtr<T>& pIns)
	{
		Release();
		if(pIns->m_pIns)
		{
			pIns->m_pIns->AddRef();
		}
		m_pIns = pIns->m_pIns;
		return *this;
	}

	T* operator = (T* pIns)
	{
		Release();
		m_pIns = pIns;
		if(m_pIns)
		{
			m_pIns->AddRef();
		}
		return m_pIns;
	}

	operator T*()
	{
		return m_pIns;
	}

	T** operator &()
	{
		return &m_pIns;
	}
private:
	T*		m_pIns;
};