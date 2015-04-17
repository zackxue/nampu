#pragma once

#include "../include/IJDCULanguage.h"
#define INC_SHEET_WIDE			160				//加宽sheet
#define RIGHT_PAGE_WIDE			20				//缩进page

#define IDC_SHEET_BTN_OK		3200			//确定按钮
#define IDC_SHEET_BTN_CANCEL	3201			//取消按钮

//属性sheet按钮(IDOK,IDCANCEL,ID_APPLY_NOW,IDHELP)
// CJPropertySheet

class CJPropertySheet : public CPropertySheet
{
	//DECLARE_DYNAMIC(CJPropertySheet)

public:
	//CJPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CJPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CJPropertySheet();

public:
	void AddPage(CPropertyPage* pPage, HICON hIcon = NULL);
	void SetBtnText(LPCTSTR lpOKText, LPCTSTR lpCancelText);

protected:
	void WidenSheet(int nWide = INC_SHEET_WIDE);
	void MovePage(UINT uPage, int nToLeft = RIGHT_PAGE_WIDE);
	void DrawPageCaption(CString& strCaption, CDC* pDC, COLORREF crCaption);
	void DrawGradualChange(CDC* pDC, const CRect& rc, COLORREF crChange);
	void DrawCaptionText(CDC* pDC, CString& strText, const CRect& rc);
	void DrawGradualChangeEx(CDC* pDC, const CRect& rc, COLORREF crChange);

	void HideSysBtn();
protected:
	IJLanguage*		m_pJLanguage;
	CString			m_strTitle[1];
	CRect			m_rcPageClient;
	CListCtrlSheet	m_lstSheet;
	long			m_lSelectPage;
	CString			m_strActiveTitle;
	vector<HICON>	m_vtPageIcon;
	CFont			m_font;

	CImage			m_imgPropertyTitle;
	CImage			m_imgLeftBorder;
	CImage			m_imgRightBorder;
	CImage			m_imgBottomBorder;
	CImage			m_imgPageTitle;
	CImage			m_imgClose;
	CImage			m_imgSelClose;
	CImage			m_imgBtn;
	CImage			m_imgSelBtn;
	
	long			m_lBtnState;
	CString			m_strOKTitle;
	CString			m_strCancelTitle;

	CImgButton		m_btnOK;
	CImgButton		m_btnCancel;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg LRESULT OnListCtrlMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFunBtnOK();
	afx_msg void OnFunBtnCancel();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
};


