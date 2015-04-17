
// MainFrm.h : CMainFrame 类的接口
//

/*
 * @Author: Microsoft
 * @Date  : 2003-03-04 
 * @Detail: 基于单文档模版的主程序对话框 
 *			
 */

#pragma once
#include "ChildView.h"

class CMainFrame : public CFrameWndEx
{
	
public:
	CMainFrame();
	CMFCStatusBar     m_wndStatusBar;
protected: 
	DECLARE_DYNAMIC(CMainFrame)
	

// 属性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CChildView    m_wndView;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNcPaint();
	int m_heart_elapse;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_timeout;
	void ShowTime(CString);
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


