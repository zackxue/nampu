#pragma once


#define IDC_BTN_OK				33210
#define IDC_BTN_CANCEL			33211

// CJPublicDlg 对话框

class CJPublicDlg : public CDialog
{
	DECLARE_DYNAMIC(CJPublicDlg)

public:
	CJPublicDlg(UINT nIDTemplate, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CJPublicDlg();

// 对话框数据
	//enum { IDD = IDD_JPUBLICDLG };

public:
	void SetSize(long lWidth, long lHeight);
	void SetFontColor(COLORREF crFont){m_crText = crFont;}
	void SetBKColor(COLORREF crBK){m_crBK = crBK;}
	void SetBtnText(const CString& strOK, const CString& strCancel);
	void ToBtnLeft(long lLeft = 50, long lBtnInterval = 20){m_lLeft = lLeft;m_lBtnInterval = lBtnInterval;}
	void SetWindowText(LPCTSTR lpText){m_strText = lpText;}
	void GetWindowText(CString& strText) const{strText = m_strText;}

protected:
	void HideSysBtn();

private:
	long			m_lWidth;
	long			m_lHeight;
	COLORREF		m_crBK;
	COLORREF		m_crText;
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

	CImgButton		m_btnOK;
	CImgButton		m_btnCancel;
	CString			m_strOKText;
	CString			m_strCancel;
	CString			m_strText;

	long			m_lBtnState;
	long			m_lLeft;
	long			m_lBtnInterval;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
protected:
	virtual void PreSubclassWindow();
};
