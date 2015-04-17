/************************************************************************ 
* 文件名：    UserModify1.h 
* 文件描述：  修改用户密码的对话框之一
* 创建人：    XiaoHe, 2013年03月2日
* 版本号：    1.0 
************************************************************************/ 

#pragma once
#include "skinbtn.h"
#include "drawbutton.h"


// CUserModify1 对话框

class CUserModify1 : public CDialog
{
	DECLARE_DYNAMIC(CUserModify1)

public:
	CUserModify1(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserModify1();

// 对话框数据
	enum { IDD = IDD_USER_MODIFY1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	void DrawRangeImage(int i, CDC *pDC, CRect rc);
	CString m_name;
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedClose();
	CSkinBtn m_close_btn;
	CSkinBtn m_cancel_btn;
	CSkinBtn m_ok_btn;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
