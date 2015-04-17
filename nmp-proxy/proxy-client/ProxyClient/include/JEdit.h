#pragma once


// CJEdit

enum Mask
{
	MASK_HOUR12       = 0,
	MASK_HOUR24       = 1,
	MASK_HOURFREE     = 2,
	MASK_IPADDRESS    = 3,
	MASK_DATEDDMMYYYY = 4,
	MASK_DATEMMDDYYYY = 5,
	MASK_DATEYYYYDDMM = 6,
	MASK_DATEYYYYMMDD = 7,
	MASK_FREEMASK     = 8
};

class CJEdit : public CEdit
{
	//DECLARE_DYNAMIC(CJEdit)
public:
	CJEdit();
	virtual ~CJEdit();

public:
	void SetToolTip(LPCTSTR	lpMsg);
	void SetMask(CString mszMask, CString mszShowMask, Mask enTypeMask = MASK_FREEMASK);
	void ValidMask(UINT nChar);
	int	GetNextPos(int start);
	BOOL IsValidChar(UINT nChar, int nStartPos);
	BOOL ValSpecialKey(int nStartPos, int nEndPos);
	BOOL IsPosMask(int StartPos);
	void DeleteString(int nStartPos, int nEndPos);
	void AjustaCadena(int nStartPos, int nEndPos);
	int	NumCharNoMask();
	int FindLasCharR();
	void DeleteAndAjust(int nStartPos, int nEndPos);
	int DifCharReal(int start, int fin);
	BOOL IsValPos(UINT nChar, int pos);

private:
	CToolTipCtrl m_tooltipCtrl;
	CString		m_szMask;
	CString		m_szShowMask;
	Mask		m_enMask;
	CString		StrToUse;
	CString		m_cadResult;
	int			m_KeySpecial;
	COLORREF	m_crFace;
	COLORREF	m_crCaption;

protected:
	DECLARE_MESSAGE_MAP()
protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnNcPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSysColorChange();
};


