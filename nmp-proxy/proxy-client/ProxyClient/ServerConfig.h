/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail: 服务器配置的对话框　
 *			
 */


#pragma once
#include "MyBlackDlg.h"

// CServerConfig 对话框

class CServerConfig : public CMyBlackDlg
{
	DECLARE_DYNAMIC(CServerConfig)

public:
	CServerConfig(UINT IDD1 =IDD_SERVERCONFIG ,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServerConfig();

// 对话框数据
	//Genum { IDD = IDD_SERVERCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnEnChangeEdit2();
protected:
	virtual void OnOK();
public:
	CSkinBtn m_cancel_btn;
	CSkinBtn m_ok_btn;
	CString ip2,port1,port2;
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 void OnClose();
	 virtual BOOL OnInitDialog();
};
