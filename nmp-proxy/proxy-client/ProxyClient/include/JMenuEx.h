#pragma once

#define		LEFT_BORDER_MENUE			25				//左菜单
#define		SEPLIT_STATUS_MENUE			5				//分隔条

//自定义菜单项
typedef struct
{
	CString			strText;
	UINT			uID;
	UINT			uIndex;
	int				iLeftPos;
}JMENUITEM, *PJMENUITEM;

class JMenuEx : public CMenu
{
public:
	JMenuEx(void);
	virtual ~JMenuEx(void);

	void InitMenu();

	void SetLeftColor(COLORREF cr){m_crMenuLeft = cr;}
	void SetBKColor(COLORREF cr){m_crMenuBK = cr;}
	void SetSelectColor(COLORREF cr){m_crMenuSelect = cr;}
	void SetTextColor(COLORREF cr){m_crMenuText = cr;}
	void SetTextSelectColor(COLORREF cr){m_crMenuTextSelect = cr;}
	void SetSpliterbarColor(COLORREF cr){m_crSpliterbar = cr;}

	void ChangeItemType(CMenu* pMenu);
	void DrawMenuText(CDC *pDC, CRect &rect, CRect rcText, BOOL bSelected, BOOL bGrayed, PJMENUITEM pItem);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
public:
	BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL);
	BOOL TrackPopupMenu(UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect = 0);

private:
	COLORREF		m_crMenuLeft;
	COLORREF		m_crMenuBK;
	COLORREF		m_crMenuSelect;
	COLORREF		m_crMenuText;
	COLORREF		m_crMenuTextSelect;
	COLORREF		m_crSpliterbar;
	COLORREF		m_crCheck;

	CList<PJMENUITEM, PJMENUITEM>	m_lstMenuItem;
public:
	static CMap<UINT, UINT, UINT, UINT>	m_mpCheckItem;
};
