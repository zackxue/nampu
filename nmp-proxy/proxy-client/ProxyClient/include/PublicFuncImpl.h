#ifndef __PUBLIC_FUNC_LIB__
#define __PUBLIC_FUNC_LIB__

#define _PUBLIC_

#include <windows.h>
#include <tchar.h>
#include <set>
#include <vector>
#include <list>
using namespace std;
#include <io.h>

#include "JString.h"
#include "WinEvent.h"
#include "MutexEx.h"
#include "RangBuf.h"
#include "SysInfo.h"
#include "LockMem.h"
#include "JThreadBase.h"
#include "SerialLong.h"
#include "JPrintBug.h"
#include "JLogFile.h"
#include "WinSysFunc.h"
#include "JFile.h"
#include "JBase64.h"

//文件最大长度
#define MAX_FILE_PATH		MAX_PATH+64

#ifdef _DEBUG
#pragma comment(lib,  "../lib/PublicFuncImpl_D.lib")
#else
#pragma comment(lib,  "../lib/PublicFuncImpl_R.lib")
#endif

#endif