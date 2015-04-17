#pragma once


#define WM_LSTCTR_MSG			(WM_USER + 123)				//自定义消息(LPARAM-表示ST_LSTCTR_MSG类型)

//
#define WM_LSTCTR_CLICK			1							//单击消息
#define WM_LSTCTR_RCLICK		2							//右键消息

typedef struct
{
	UINT		uMsg;				//消息
	POINT		pt;					//光标位置
	int			nSelectItem;		//选中项
	LRESULT		lResult;			//是否处理, 0-处理, 其它值-不处理
}ST_LSTCTR_MSG, *PST_LSTCTR_MSG;

// CListCtrlSheet

class CListCtrlSheet : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlSheet)

public:
	CListCtrlSheet();
	virtual ~CListCtrlSheet();

public:
	BOOL Create(DWORD dwStyle, const RECT& rc, CWnd* pParent, UINT uID);
	BOOL AddItemText(int nItem, LPWSTR szText);
	BOOL AddItemIcon(int nItem, HICON hIcon);
	HWND SetOwnWnd(HWND hWnd);
	UINT SetOwnMsg(UINT uMsg);
	HICON GetItemIcon(int nItem);

protected:
	void InitList();
	void DrawGradientLine(CDC* pDC, COLORREF crLine, POINT ptStart, POINT ptEnd);
protected:
	CImageList			m_ImgList;
	COLORREF			m_crTextBKSelect;
	COLORREF			m_crSplitItem;
	CFont				m_ftList;

	HWND				m_hWndRecMsg;
	UINT				m_uMsg;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


