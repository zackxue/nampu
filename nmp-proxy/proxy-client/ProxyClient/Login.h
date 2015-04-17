#pragma once

#include "resource.h"
#include "afxwin.h"
#include "drawbutton.h"
#include "stdafx.h"

/*
 * @Author: XiaoHe
 * @Date: 2003/03/04 
 * @Detail: 登陆对话框
 *			
 */

#include "MyEdit.h"


#define  UM_ERR  WM_USER+111
// CLogin 对话框

class CLogin : public CDialog
{
	DECLARE_DYNAMIC(CLogin)

public:
	CLogin(UINT IDD1 = IDD_LOGIN,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLogin();
	CString m_name;
	CString m_pwd;

	// 对话框数据
	enum { IDD = IDD_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	CString m_IP;
	CString m_port;
	//CBrush m_edit_brush;
	

	void Close();

	virtual BOOL OnInitDialog();
	CEdit m_edit;
	void WriteLoginInf(const CString &name,const CString &ip,const CString &port);
	void GetLoginInf(CString &name,CString &ip,CString &port);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	void ChangeWindowRgn(CDC* pDC);

	static void TimerProc(							   HWND hWnd,      // handle of CWnd that called SetTimer
		UINT nMsg,      // WM_TIMER
		UINT nIDEvent,   // timer identification
		DWORD dwTime);    // system time)
private:
	CBitmap m_bkbitmap;

public:

	afx_msg LRESULT OnResult(WPARAM wParam, LPARAM lParam); 
	CDrawButton m_Idok;
	CDrawButton m_Idcancel;
	//	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//	afx_msg BOOL OnNcActivate(BOOL bActive);
	//	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	//	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	//	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	//	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	//	afx_msg void OnNcPaint();
	//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	static DWORD WINAPI LoginThread(void* pParam);
	CSkinBtn m_ok_btn;
	CSkinBtn m_cancel_btn;
	CStatic m_waiting_static;
	CMyEdit m_name_edit;
	CMyEdit m_ip_edit;
	CMyEdit m_port_edit;
	CMyEdit m_pwd_edit;
};
