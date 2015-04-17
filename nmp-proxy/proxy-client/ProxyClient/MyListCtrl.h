/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail:
 *			
 */


#pragma once


// CMyListCtrl

class CMyListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMyListCtrl)

public:
	CMyListCtrl();
	virtual ~CMyListCtrl();

	void SetRowHeigt(int nHeight) ;
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );

protected:


	void DrawItem(LPDRAWITEMSTRUCT lpMeasureItemStruct);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	DECLARE_MESSAGE_MAP()

protected:
	int m_nItemHeight;
};


