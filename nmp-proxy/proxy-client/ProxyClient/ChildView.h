
// ChildView.h : CChildView 类的接口
//

/*
 * @author: xiaohe
 * @date: 2003/03/04 
 * @detail: 该窗口类是用户登陆成功以后，展现给用户的窗口，用于搜索，查询等设备操作.  
 *
 */

#pragma once

#include "maindevice.h"
#include "mainuser.h"
#include "skinbtn.h"
#include "afxwin.h"


// CChildView 窗口

class CChildView : public CWnd
{
// 构造
public:
	CChildView();

// 属性
public:

// 操作
public:

// 重写
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CChildView();

	// 生成的消息映射函数
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
private:
 	CSkinBtn m_DeviceBtn;
 	CSkinBtn m_UserBtn;
	CSkinBtn m_config_btn;
	CSkinBtn m_logout_btn;
	CSkinBtn m_data_btn;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMainDevice();
	afx_msg void OnMainUser();
	CMainDevice m_MainDevice;
	CMainUser m_MainUser;
	S_Prameter *m_prameter;
	
	int m_timeout;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_heart_elapse;
	
	void OnLogout(void);
	void OnConfig(void);
	void OnData(void);
};

