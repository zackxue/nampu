#ifndef __JLOCKMEM_H__
#define __JLOCKMEM_H__


#define MAX_THREAD_COUNT		1024				//最大线程数量

#define READ_STATUS			1
#define WRITE_STATUS		2

struct stMemBlockLock
{
	long	lWriteRequestCount;						//写请求数量
	long	lWriteCount;							//正在写数量
	long	lReadCount;								//正在读数量
	long	lThreadCount;							//操作线程数量
	DWORD	dwTheadId[MAX_THREAD_COUNT];			//操作线程ID
	long	lThreadWriteCount[MAX_THREAD_COUNT];	//线程重入写状态数量
	long	lThreadReadCount[MAX_THREAD_COUNT];		//线程重入读状态数量
	long	lThreadPreReadCount[MAX_THREAD_COUNT];	//线程由读转入写状态时的读状态数量
};

class CLockMem
{
public:
	CLockMem();
	virtual ~CLockMem();

public:
	long Init(const char* szName = NULL);
	BOOL LockRead(DWORD dwTimeout = INFINITE);
	BOOL LockWrite(DWORD dwTimeout = INFINITE);
	void UnlockRead(BOOL bReleaseResource = TRUE);
	void UnlockWrite(BOOL bReleaseResource = TRUE);

protected:
	void Close();
	void AddPlaceStatus(long lPlace, DWORD dwThreadId, long lStatus, long lCount = 1);
	void RemovePlaceStatus(long lPlace, BOOL bReleaseResource);
	long FindThreadPlace(DWORD dwThreadId);

	void ReLockRead(DWORD dwThreadId, long lReadCount);

	BOOL LockMemBlock(DWORD dwTimeout);
	void UnlockMemBlock();

private:
	HANDLE				m_hMutexMemBlock;
	HANDLE				m_hMapFile;

	stMemBlockLock*		m_pMemBlock;
};


#endif