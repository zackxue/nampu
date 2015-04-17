/************************************************************************ 
* 文件名：    UserAddDlg.h   
* 文件描述：  增加用户的对话框
* 创建人：    XiaoHe, 2013年03月2日
* 版本号：    1.0 
************************************************************************/ 


#pragma once
#include "skinbtn.h"


// CUserAddDlg 对话框

class CUserAddDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserAddDlg)

public:
	CUserAddDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserAddDlg();

	// 对话框数据
	enum { IDD = IDD_USER_ADD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CSkinBtn m_idok_btn;
public:
	CSkinBtn m_idcancel_btn;
	CString m_name;
	CString m_pwd;
	CString m_pwd_confirm;
//	afx_msg void OnBnClickedOk();
protected:
//	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	void OnClose(void);
	void DrawRangeImage(int i, CDC *pDC, CRect rc);

protected:
	virtual void OnOK();
public:
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedClose();
	CSkinBtn m_close_btn;
	void* m_pParent;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
};
