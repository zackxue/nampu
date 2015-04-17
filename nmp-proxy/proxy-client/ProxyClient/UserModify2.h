#pragma once
#include "skinbtn.h"
#include "drawbutton.h"


// CUserModify2 对话框

class CUserModify2 : public CDialog
{
	DECLARE_DYNAMIC(CUserModify2)

public:
	CUserModify2(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserModify2();

// 对话框数据
	enum { IDD = IDD_USER_MODIFY2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	void DrawRangeImage(int i, CDC *pDC, CRect rc);
	afx_msg void OnNcPaint();
	afx_msg void OnBnClickedClose();
	CSkinBtn m_close_btn;
	CSkinBtn m_cancel_btn;
	CSkinBtn m_ok_btn;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
