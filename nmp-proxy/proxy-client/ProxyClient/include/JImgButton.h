#pragma once

JString GetModulePath(HMODULE hModule);

BOOL JLoadImageFile(const CString& strFile,CImage& img);
BOOL JLoadImage(const char* szName,CImage& img);

// CImgButton
#define CAPTURE_LBUTTONUP		300
#define NM_CMD_LBUTTONDOWN		(30000)	//控件按下WM_NOTIFY消息code，返回CAPTURE_LBUTTONUP表示处理该消息并捕获NM_CMD_LBUTTONUP
#define NM_CMD_LBUTTONUP		(30001)


class CImgButton : public CButton
{
	//DECLARE_DYNAMIC(CImgButton)
public:
	CImgButton(int nTxtHeight = 12);
	virtual ~CImgButton();

	void SetImage(const char* szImgNameNormal , const char* szImgNameSelect  = NULL, int nJpgLeft = 0 , int nJpgTop = 0);
	void SetImage(CImage* pImgNormal , CImage* pImgelect  = NULL, int nJpgLeft = 0 , int nJpgTop = 0);
	void SetRgn(COLORREF crTolerance = RGB(0,0,0));

	void SetToolTip(CToolTipCtrl* pToolTip , LPCTSTR lpszTip = NULL);

	long GetImgWidth();
	long GetImgHeight();

	void SetTextPlace(int nX,int nY);
	void SetTextColor(COLORREF crText,COLORREF crSelText = -1);

	void AdjustButtonPlace(int nX, int nY);
protected:
	CImage*	m_pImgOuterNormal;
	CImage*	m_pImgOuterSelect;
	CImage	m_imgNormal;
	CImage	m_imgSelect;
	int		m_nJpgLeft;
	int		m_nJpgTop;
	HRGN	m_hRgn;
	int		m_nAdjustX;
	int		m_nAdjustY;

	BOOL	m_bMoseHover;
	int		m_nXPlace;
	int		m_nYPlace;
	COLORREF	m_crText;
	COLORREF	m_crSelText;
	CFont	m_font;

	CToolTipCtrl*		m_pToolTip;
	CString				m_strTip;
	BOOL				m_bCapture;
protected:
	CImage* GetImgNormal();
	CImage* GetImgSelect();
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};


