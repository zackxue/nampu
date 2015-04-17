
// ProxyClient3.h : ProxyClient3 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号


// CProxyClient3App:
// 有关此类的实现，请参阅 ProxyClient3.cpp
//

class CProxyClient3App : public CWinAppEx
{
public:
	CProxyClient3App();


// 重写
public:
	int m_heart_elapse;
	virtual BOOL InitInstance();
	virtual ~CProxyClient3App();
// 实现

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	
};

extern CProxyClient3App theApp;
