#include "PublicFuncImpl.h"
#include "SysInfo.h"

#include <Iphlpapi.h>
#pragma comment( lib, "Iphlpapi.lib" )

#include <Psapi.h>
#pragma comment( lib, "Psapi.lib" )


void GetMemoryStatus(DWORD& dwTotal , DWORD& dwUsed)
{
	MEMORYSTATUS	MemoryStatus;
	
	MemoryStatus.dwLength = sizeof(MemoryStatus);
	GlobalMemoryStatus(&MemoryStatus);

	dwTotal = MemoryStatus.dwTotalPhys / (1024*1024);				//单位 M
	DWORD dwAvailPhys = MemoryStatus.dwAvailPhys / (1024*1024);
	dwUsed = dwTotal - dwAvailPhys;
}


//CPU defined
#define SystemBasicInformation 0
#define SystemPerformanceInformation 2
#define SystemTimeInformation 3

#define Long2Double(X) ((double)((X).HighPart) * 4.294967296E9 + (double)((X).LowPart))

typedef struct
{
	DWORD dwUnknown1;
	ULONG uKeMaximumIncrement;
	ULONG uPageSize;
	ULONG uMmNumberOfPhysicalPages;
	ULONG uMmLowestPhysicalPage;
	ULONG uMmHighestPhysicalPage;
	ULONG uAllocationGranularity;
	PVOID pLowestUserAddress;
	PVOID pMmHighestUserAddress;
	ULONG uKeActiveProcessors;
	BYTE bKeNumberProcessors;
	BYTE bUnknown2;
	WORD wUnknown3;
}SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;
}SYSTEM_TIME_INFORMATION;

typedef struct
{
	LARGE_INTEGER liIdleTime;
	DWORD dwSpare[76];
}SYSTEM_PERFORMANCE_INFORMATION;
//CPU defined end 

typedef LONG (WINAPI* PROCNTQSI)(UINT, LPVOID, ULONG, PULONG);
PROCNTQSI	NtQuerySystemInfo;

BOOL GetCPUUsed(long& lUsed)
{
	SYSTEM_PERFORMANCE_INFORMATION	SystemPerInfo;
	SYSTEM_TIME_INFORMATION			SystemTimeInfo;
	SYSTEM_BASIC_INFORMATION		SystemBasicInfo;

	LARGE_INTEGER					liOldIdleTime = {0, 0};
	LARGE_INTEGER					liOldSystemTime = {0, 0};
	LONG							Status;

	double							dbIdleTime;
	double							dbSystemTime;

	HMODULE hModule = ::GetModuleHandle(_T("ntdll"));
	NtQuerySystemInfo = (PROCNTQSI)::GetProcAddress(hModule, "NtQuerySystemInformation");
	if(NtQuerySystemInfo == NULL)
	{		
		return FALSE;
	}

	// 获取系统进程数
	Status = NtQuerySystemInfo(SystemBasicInformation, &SystemBasicInfo, sizeof(SystemBasicInfo), NULL);
	if(Status != NO_ERROR)
	{
		//run_log("获取系统进程数失败!");
		return FALSE;
	}

	// 获取CPU时间
	for(int i = 0; i < 2; i++)
	{
		//Get New System Time
		Status = NtQuerySystemInfo(SystemTimeInformation, &SystemTimeInfo, sizeof(SystemTimeInfo), NULL);
		if(Status != NO_ERROR)
		{
			//run_log("获取系统时间失败");
			return FALSE;
		}

		//Get New CPU By Idle Time
		Status = NtQuerySystemInfo(SystemPerformanceInformation, &SystemPerInfo, sizeof(SystemPerInfo), NULL);
		if(Status != NO_ERROR)
		{
			//run_log("获取 CPU By Idle时间失败");
			return FALSE;
		}

		if(liOldIdleTime.QuadPart != 0)
		{
			dbIdleTime = Long2Double(SystemPerInfo.liIdleTime) - Long2Double(liOldIdleTime);
			dbSystemTime = Long2Double(SystemTimeInfo.liKeSystemTime) - Long2Double(liOldSystemTime);

			dbIdleTime = dbIdleTime / dbSystemTime;
			dbIdleTime = 100.0 - (dbIdleTime * 100.0) / (double)SystemBasicInfo.bKeNumberProcessors + 0.5;

			lUsed = (long)dbIdleTime;
			return TRUE;
		}

		liOldIdleTime = SystemPerInfo.liIdleTime;
		liOldSystemTime = SystemTimeInfo.liKeSystemTime;

		Sleep(2000);
	}

	return FALSE;
}

typedef struct _THREAD_INFO
{
LARGE_INTEGER CreateTime;
DWORD dwUnknown1;
DWORD dwStartAddress;
DWORD StartEIP;
DWORD dwOwnerPID;
DWORD dwThreadId;
DWORD dwCurrentPriority;
DWORD dwBasePriority;
DWORD dwContextSwitches;
DWORD Unknown;
DWORD WaitReason;

}THREADINFO, *PTHREADINFO;

typedef struct _UNICODE_STRING
{
 USHORT Length;
 USHORT MaxLength;
 PWSTR Buffer;
} UNICODE_STRING;

typedef struct _PROCESS_INFO
{
DWORD dwOffset;
DWORD dwThreadsCount;
DWORD dwUnused1[6];
LARGE_INTEGER CreateTime;
LARGE_INTEGER UserTime;
LARGE_INTEGER KernelTime;
UNICODE_STRING ProcessName;

DWORD dwBasePriority;
DWORD dwProcessID;
DWORD dwParentProcessId;
DWORD dwHandleCount;
DWORD dwUnused3[2];

DWORD dwVirtualBytesPeak;
DWORD dwVirtualBytes;
ULONG dwPageFaults;
DWORD dwWorkingSetPeak;
DWORD dwWorkingSet;
DWORD dwQuotaPeakPagedPoolUsage;
DWORD dwQuotaPagedPoolUsage;
DWORD dwQuotaPeakNonPagedPoolUsage;
DWORD dwQuotaNonPagedPoolUsage;
DWORD dwPageFileUsage;
DWORD dwPageFileUsagePeak;

DWORD dCommitCharge;
THREADINFO ThreadSysInfo[1];

} PROCESSINFO, *PPROCESSINFO;

BOOL GetProcessCPUUsed(DWORD dwPrcID,long& lUsed)
{
	lUsed = 0;
	PVOID pProcInfo = NULL;
	DWORD dwInfoSize = 0x20000;
	PPROCESSINFO pProcessInfo;
	DWORD dwWorkingSet;
	long ( __stdcall *NtQuerySystemInformation )( DWORD, PVOID, DWORD, DWORD );


	static __int64 LastTotalProcessCPUUsage = 0;
	static __int64 LastCurrentProcessCPUUsage = 0;

	int CurrentDelta = 0;
	int TotalDelta = 0;

	__int64 TotalProcessCPUUsage = 0;
	__int64 CurrentProcessCPUUsage = 0;

	/////////////////////////////////

	pProcInfo = (PVOID)(new byte[dwInfoSize]);

	NtQuerySystemInformation = (long(__stdcall*)(DWORD,PVOID,DWORD,DWORD))
		GetProcAddress( GetModuleHandle( _T("ntdll.dll" )),"NtQuerySystemInformation" );

	NtQuerySystemInformation(5,pProcInfo,dwInfoSize,0);

	pProcessInfo = (PPROCESSINFO)pProcInfo;

	do
	{
		TotalProcessCPUUsage += (__int64)pProcessInfo->KernelTime.QuadPart + (__int64)pProcessInfo->UserTime.QuadPart;

		if(pProcessInfo->dwProcessID == dwPrcID)
		{
			dwWorkingSet = pProcessInfo->dwWorkingSet;
			CurrentProcessCPUUsage += (__int64)pProcessInfo->KernelTime.QuadPart + (__int64)pProcessInfo->UserTime.QuadPart;
		}

		/////////
		if(pProcessInfo->dwOffset == 0)
		{
			break;
		}

		pProcessInfo = (PPROCESSINFO)((byte*)pProcessInfo + pProcessInfo->dwOffset);
	}
	while(true);

	TotalDelta = (int)(TotalProcessCPUUsage - LastTotalProcessCPUUsage);
	CurrentDelta = (int)(CurrentProcessCPUUsage - LastCurrentProcessCPUUsage);

	if(TotalDelta != 0)
		lUsed = 100 * CurrentDelta / TotalDelta;

	LastTotalProcessCPUUsage = TotalProcessCPUUsage;
	LastCurrentProcessCPUUsage = CurrentProcessCPUUsage;

	delete[] pProcInfo;
	return TRUE;
}


BOOL GetProcMemoryUsed(DWORD dwPrcID,long& lUsed)
{
	HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, dwPrcID );
    if (NULL == hProcess)
        return FALSE;

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
		lUsed = pmc.WorkingSetSize / (1024*1024);
		
		CloseHandle( hProcess );
        return TRUE;
    }

    CloseHandle( hProcess );
	return FALSE;

}

//获取CPU序列号
const char* GetCPUID()
{
	static BOOL bRead = FALSE;
	static char szCPUID[256] = {0};

	if(bRead)
	{
		return szCPUID;
	}

	BOOL bException = FALSE;
	unsigned long s1,s2;
	BYTE szCpu[16]  = { 0 };

	__try
	{
		_asm
		{
			mov eax, 0
			cpuid
			mov dword ptr szCpu[0], ebx
			mov dword ptr szCpu[4], edx
			mov dword ptr szCpu[8], ecx

			mov eax,1
			cpuid
			mov s1,edx
			mov s2,eax		
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bException = TRUE;
	}
	bRead = TRUE;
	if(bException)
	{
		return szCPUID;
	}

	//
	strcpy(szCPUID,(char*)szCpu);

	char szTmp[32] = {0};
	sprintf(szTmp," %08X_%08X",s1,s2);
	strcat(szCPUID,szTmp);

	return szCPUID;
}

#define FILE_DEVICE_SCSI 0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY ( ( FILE_DEVICE_SCSI << 16 ) + 0x0501 )
#define IOCTL_SCSI_MINIPORT 0x0004D008 // see NTDDSCSI.H for definition
#define IDENTIFY_BUFFER_SIZE 512
#define SENDIDLENGTH ( sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE )
#define IDE_ATAPI_IDENTIFY 0xA1 // Returns ID sector for ATAPI.
#define IDE_ATA_IDENTIFY 0xEC // Returns ID sector for ATA.
#define DFP_RECEIVE_DRIVE_DATA 0x0007c088

#ifndef _PUBLIC_

typedef struct _DRIVERSTATUS
{
BYTE bDriverError; // Error code from driver, or 0 if no error.
BYTE bIDEStatus; // Contents of IDE Error register.
// Only valid when bDriverError is SMART_IDE_ERROR.
BYTE bReserved[2]; // Reserved for future expansion.
DWORD dwReserved[2]; // Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
typedef struct _SENDCMDOUTPARAMS
{
 DWORD cBufferSize; // Size of bBuffer in bytes
 DRIVERSTATUS DriverStatus; // Driver status structure.
 BYTE bBuffer[1]; // Buffer of arbitrary length in which to store the data read from the // drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
typedef struct _IDEREGS
{
BYTE bFeaturesReg; // Used for specifying SMART "commands".
BYTE bSectorCountReg; // IDE sector count register
BYTE bSectorNumberReg; // IDE sector number register
BYTE bCylLowReg; // IDE low order cylinder value
BYTE bCylHighReg; // IDE high order cylinder value
BYTE bDriveHeadReg; // IDE drive/head register
BYTE bCommandReg; // Actual IDE command.
BYTE bReserved; // reserved for future use. Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;
typedef struct _SENDCMDINPARAMS
{
DWORD cBufferSize; // Buffer size in bytes
IDEREGS irDriveRegs; // Structure with drive register values.
BYTE bDriveNumber; // Physical drive number to send
// command to (0,1,2,3).
BYTE bReserved[3]; // Reserved for future expansion.
DWORD dwReserved[4]; // For future use.
BYTE bBuffer[1]; // Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#endif

typedef struct _GETVERSIONOUTPARAMS
{
BYTE bVersion; // Binary driver version.
BYTE bRevision; // Binary driver revision.
BYTE bReserved; // Not used.
BYTE bIDEDeviceMap; // Bit map of IDE devices.
DWORD fCapabilities; // Bit mask of driver capabilities.
DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;


typedef struct _IDSECTOR
{
USHORT wGenConfig;
USHORT wNumCyls;
USHORT wReserved;
USHORT wNumHeads;
USHORT wBytesPerTrack;
USHORT wBytesPerSector;
USHORT wSectorsPerTrack;
USHORT wVendorUnique[3];
CHAR sSerialNumber[20];
USHORT wBufferType;
USHORT wBufferSize;
USHORT wECCSize;
CHAR sFirmwareRev[8];
CHAR sModelNumber[40];
USHORT wMoreVendorUnique;
USHORT wDoubleWordIO;
USHORT wCapabilities;
USHORT wReserved1;
USHORT wPIOTiming;
USHORT wDMATiming;
USHORT wBS;
USHORT wNumCurrentCyls;
USHORT wNumCurrentHeads;
USHORT wNumCurrentSectorsPerTrack;
ULONG ulCurrentSectorCapacity;
USHORT wMultSectorStuff;
ULONG ulTotalAddressableSectors;
USHORT wSingleWordDMA;
USHORT wMultiWordDMA;
BYTE bReserved[128];
} IDSECTOR, *PIDSECTOR;

typedef struct _SRB_IO_CONTROL
{
ULONG HeaderLength;
UCHAR Signature[8];
ULONG Timeout;
ULONG ControlCode;
ULONG ReturnCode;
ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;



BOOL WinNTHDSerialNumAsScsiRead( BYTE* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen )
{
	BOOL bInfoLoaded = FALSE;

	for( int iController = 0; iController < 2; ++ iController )
	{
		HANDLE hScsiDriveIOCTL = 0;
		char szDriveName[256];

		// Try to get a handle to PhysicalDrive IOCTL, report failure
		// and exit if can't.
		sprintf( szDriveName, "\\\\.\\Scsi%d:", iController );
		// Windows NT, Windows 2000, any rights should do
		hScsiDriveIOCTL = CreateFileA( szDriveName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);
		// if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
		// printf ("Unable to open SCSI controller %d, error code: 0x%lX\n",
		// controller, GetLastError ());

		if( hScsiDriveIOCTL != INVALID_HANDLE_VALUE )
		{
			int iDrive = 0;
			for( iDrive = 0; iDrive < 2; ++ iDrive )
			{
				char szBuffer[sizeof( SRB_IO_CONTROL ) + SENDIDLENGTH] = { 0 };
				SRB_IO_CONTROL* p = ( SRB_IO_CONTROL* )szBuffer;
				SENDCMDINPARAMS* pin = ( SENDCMDINPARAMS* )( szBuffer + sizeof( SRB_IO_CONTROL ) );
				DWORD dwResult;
				p->HeaderLength = sizeof( SRB_IO_CONTROL );
				p->Timeout = 10000;
				p->Length = SENDIDLENGTH;
				p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
				strncpy( ( char* )p->Signature, "SCSIDISK", 8 );
				pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
				pin->bDriveNumber = iDrive;

				if( DeviceIoControl( hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,
					szBuffer,
					sizeof( SRB_IO_CONTROL ) + sizeof( SENDCMDINPARAMS ) - 1,
					szBuffer,
					sizeof( SRB_IO_CONTROL ) + SENDIDLENGTH,
					&dwResult, NULL ) )
				{
					SENDCMDOUTPARAMS* pOut = ( SENDCMDOUTPARAMS* )( szBuffer + sizeof( SRB_IO_CONTROL ) );
					IDSECTOR* pId = ( IDSECTOR* )( pOut->bBuffer );
					if( pId->sModelNumber[0] )
					{
						if( * puSerialLen + 20U <= uMaxSerialLen )
						{
							// 序列号
							CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )pId ) + 10, 20 );
							// Cut off the trailing blanks
							UINT i;
							for( i = 20; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i ){}
							* puSerialLen += i;
							// 型号
							CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )pId ) + 27, 40 );
							// Cut off the trailing blanks
							for( i = 40; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i ){}
							* puSerialLen += i;
							bInfoLoaded = TRUE;
						}
						else
						{
							::CloseHandle( hScsiDriveIOCTL );
							return bInfoLoaded;
						}
					}
				}
			}
			::CloseHandle( hScsiDriveIOCTL );
		}
	}
	return bInfoLoaded;
}

BOOL DoIdentify( HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 PDWORD lpcbBytesReturned )
{
    // Set up data structures for IDENTIFY command.
    pSCIP->cBufferSize                  = IDENTIFY_BUFFER_SIZE;
    pSCIP->irDriveRegs.bFeaturesReg     = 0;
    pSCIP->irDriveRegs.bSectorCountReg  = 1;
    pSCIP->irDriveRegs.bSectorNumberReg = 1;
    pSCIP->irDriveRegs.bCylLowReg       = 0;
    pSCIP->irDriveRegs.bCylHighReg      = 0;
    
    // calc the drive number.
    pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ( ( bDriveNum & 1 ) << 4 );
    // The command can either be IDE identify or ATAPI identify.
    pSCIP->irDriveRegs.bCommandReg = bIDCmd;
    pSCIP->bDriveNumber = bDriveNum;
    pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;
    
    return DeviceIoControl( hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
        ( LPVOID ) pSCIP,
        sizeof( SENDCMDINPARAMS ) - 1,
        ( LPVOID ) pSCOP,
        sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE - 1,
        lpcbBytesReturned, NULL );
}

BOOL WinNTHDSerialNumAsPhysicalRead( BYTE* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen )
{

#define  DFP_GET_VERSION          0x00074080

    BOOL bInfoLoaded = FALSE;
    for( UINT uDrive = 0; uDrive < 4; ++ uDrive )
    {
        HANDLE hPhysicalDriveIOCTL = 0;
        //  Try to get a handle to PhysicalDrive IOCTL, report failure
        //  and exit if can't.
        char szDriveName [256];
        sprintf( szDriveName, "\\\\.\\PhysicalDrive%d", uDrive );
        //  Windows NT, Windows 2000, must have admin rights
        hPhysicalDriveIOCTL = CreateFileA( szDriveName,
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
            OPEN_EXISTING, 0, NULL);
        if( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE )
        {
            GETVERSIONOUTPARAMS VersionParams = { 0 };
            DWORD               cbBytesReturned = 0;
            // Get the version, etc of PhysicalDrive IOCTL
            if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_GET_VERSION,
                NULL, 
                0,
                &VersionParams,
                sizeof( GETVERSIONOUTPARAMS ),
                &cbBytesReturned, NULL ) )
            {
                // If there is a IDE device at number "i" issue commands
                // to the device
                if( VersionParams.bIDEDeviceMap != 0 )
                {
                    BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
                    SENDCMDINPARAMS  scip = { 0 };
                    // Now, get the ID sector for all IDE devices in the system.
                    // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
                    // otherwise use the IDE_ATA_IDENTIFY command
                    bIDCmd = ( VersionParams.bIDEDeviceMap >> uDrive & 0x10 ) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
                    BYTE IdOutCmd[sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE - 1] = { 0 };
                    if( DoIdentify( hPhysicalDriveIOCTL, 
                        &scip, 
                        ( PSENDCMDOUTPARAMS )&IdOutCmd, 
                        ( BYTE )bIDCmd,
                        ( BYTE )uDrive,
                        &cbBytesReturned ) )
                    {
                        if( * puSerialLen + 20U <= uMaxSerialLen )
                        {
                            CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )( ( ( PSENDCMDOUTPARAMS )IdOutCmd )->bBuffer ) ) + 10, 20 );  // 序列号
                            // Cut off the trailing blanks
							UINT i;
                            for( i = 20; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )  {}
                            * puSerialLen += i;
                            CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )( ( ( PSENDCMDOUTPARAMS )IdOutCmd )->bBuffer ) ) + 27, 40 );  // 型号
                            // Cut off the trailing blanks
                            for( i = 40; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )  {}
                            * puSerialLen += i;
                            bInfoLoaded = TRUE;
                        }
                        else
                        {
                            ::CloseHandle( hPhysicalDriveIOCTL );
                            return bInfoLoaded;
                        }
                    }
                }
            }
            CloseHandle( hPhysicalDriveIOCTL );
        }
    }
    return bInfoLoaded;
}

const char* GetDiskID()
{
	static BOOL bRead = FALSE;
	static char szDiskID[4096] = {0};

	if(bRead)
	{
		return szDiskID;
	}

	bRead = TRUE;

	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &ovi );

	BYTE szSystemInfo[4096];
	UINT uSystemInfoLen = 0;

	if( ovi.dwPlatformId != VER_PLATFORM_WIN32_NT )
	{
		// Only Windows 2000, Windows XP, Windows Server 2003...
		return szDiskID;
	}
	else
	{
		if( !WinNTHDSerialNumAsPhysicalRead( szSystemInfo, &uSystemInfoLen, 1024 ) )
		{
			WinNTHDSerialNumAsScsiRead( szSystemInfo, &uSystemInfoLen, 1024 );
		}
	}

	int nTag = 0;
	for(int i = 0; i < (int)uSystemInfoLen;i++)
	{
		if(szSystemInfo[i] == 0)
		{
			break;
		}
		else if(szSystemInfo[i] != 32)
		{
			szDiskID[nTag++] = szSystemInfo[i];
		}
	}

	szDiskID[nTag] = 0;

	return szDiskID;
}

/////////////////////////////////////////////////////////////////////
BOOL GetNetAdaptersInfo(stAdaptersInfo* pInfo,int* nCount)
{
	int nRealCount = 0;

	IP_ADAPTER_INFO iai;
	ULONG uSize = 0;
	DWORD dwResult = GetAdaptersInfo( &iai, &uSize );
	if( dwResult == ERROR_BUFFER_OVERFLOW )
	{
		IP_ADAPTER_INFO* piai = ( IP_ADAPTER_INFO* )HeapAlloc( GetProcessHeap( ), 0, uSize );
		if( piai != NULL )
		{
			dwResult = GetAdaptersInfo( piai, &uSize );
			if( ERROR_SUCCESS == dwResult )
			{
				IP_ADAPTER_INFO* piai2 = piai;
				while( piai2 != NULL )
				{
					if(nRealCount >= *nCount)
					{
						HeapFree( GetProcessHeap( ), 0, piai );
						return FALSE;
					}

					//MAC
					piai2->Address;
					sprintf(pInfo[nRealCount].szMac,"%02X-%02X-%02X-%02X-%02X-%02X",
						piai2->Address[0],piai2->Address[1],piai2->Address[2],piai2->Address[3],piai2->Address[4],piai2->Address[5]);

					//IP
					pInfo[nRealCount].nIPCount = 0;
					for(IP_ADDR_STRING* pIP = &piai2->IpAddressList;pIP != NULL;pIP = pIP->Next)
					{
						if(pInfo[nRealCount].nIPCount >= 16)
						{
							break;
						}
						strncpy(pInfo[nRealCount].szIP[pInfo[nRealCount].nIPCount++],pIP->IpAddress.String,15);
					}
					

					//
					nRealCount++;
					piai2 = piai2->Next;
				}
			}
			else
			{
				HeapFree( GetProcessHeap( ), 0, piai );
				return FALSE;
			}
			HeapFree( GetProcessHeap( ), 0, piai );
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	

	*nCount = nRealCount;
	return TRUE;
}

DWORD GetdwNumberOfProcessors()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
}

LONG InterlockedAddLong(LONG volatile *lpAddend,LONG lAddValue)
{
	return ::InterlockedExchangeAdd(lpAddend,lAddValue) + lAddValue;
}

LONG InterlockedExchangeLong(LONG volatile *lpValue,LONG lNewValue)
{
    return ::InterlockedExchange(lpValue,lNewValue);
}

LONG InterlockedCompareExchangeLong(LONG volatile *lpValue,LONG lNewValue,LONG lCmpValue)
{
    return ::InterlockedCompareExchange(lpValue,lNewValue,lCmpValue);
}