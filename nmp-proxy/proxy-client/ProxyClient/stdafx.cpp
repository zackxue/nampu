
// stdafx.cpp : 只包括标准包含文件的源文件
// ProxyClient3.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

long GetMsgNumber()
{
	return CSerialLong::Instance().RequestSerialLong();
}
void ReleaseNumber(long lNumber)
{
	CSerialLong::Instance().ReleaseSerialLong(lNumber);
}


