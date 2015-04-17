
/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail: MyListCtrl中的用户管理的界面 
 *			
 */

#pragma once
#include "afxcmn.h"
#include "DrawButton.h"
#include "afxwin.h"
#include "MyEdit.h"
#include "CustomListCtrl.h"

// CMainUser 对话框

class CMainUser : public CDialog
{
	DECLARE_DYNAMIC(CMainUser)

public:
	CCustomListCtrl m_UserList;
	CMainUser(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainUser();
	CImageList   m_ImageList; 

// 对话框数据
	enum { IDD = IDD_MAINUSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedAdd();
private:
	CDrawButton m_find_btn;
	CEdit m_find_edit;
	CDrawButton m_add_btn;
	CDrawButton m_del_btn;
	CDrawButton m_modify_btn;
public:
	int m_offset;
	afx_msg void OnBnClickedFind();
	int m_count;
	CString m_username;
	void ShowUser(void);
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedModify();
	int m_tag;
//	virtual BOOL PreTranslateMessage(MSG* pMsg);


	CDrawButton m_back_btn;
	CDrawButton m_next_btn;
	CDrawButton m_goto_btn;
	CEdit m_page_edit;
	CEdit m_showpage_static;
protected:
	virtual void OnOK();
public:
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedGoto();
	afx_msg void OnPaint();
	afx_msg void OnNMRClickUserlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUserUpdate();
	afx_msg void OnUserAdd();
	afx_msg void OnUserDel();
	afx_msg void OnUserModify();
	afx_msg void OnUserQuery();
	CImageList m_imagelist;
	virtual BOOL OnInitDialog();
	//CListCtrl m_UserList22;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CDrawButton m_first_btn;
	CDrawButton m_tail_btn;
	afx_msg void OnBnClickedFirst();
	afx_msg void OnBnClickedTail();
	void ShowPage(void);
	afx_msg void OnNMDblclkUserlist(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
