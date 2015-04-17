/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail: 这是一个中间类，基于对CDialog的封装，重绘了对话框的标题栏和对话框的边框, 重绘后，边框是黑色 
 *			
 */


#pragma once

#ifndef _PROXYCLIENT_531 
#define CLOSE_BTN  WM_USER+6
#endif

// CMyBlackDlg 对话框

class CMyBlackDlg : public CDialog
{
	DECLARE_DYNAMIC(CMyBlackDlg)

public:
	CMyBlackDlg(UINT IDD1,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMyBlackDlg();

// 对话框数据
	//enum { IDD = xxxx };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_title;
	CBitmap bit[2];
	CSkinBtn m_close_btn;
	void DrawRangeImage(int i, CDC *pDC, CRect rc);
	void ChangeWindowRgn(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
	void SetTitle(CString str);
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//	afx_msg void OnSize(UINT nType, int cx, int cy);
};
