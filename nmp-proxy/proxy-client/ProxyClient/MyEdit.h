/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail: 这是一个中间类，基于对CEdit 的封装，应用于”登陆“对话框中，输入“用户名”“密码”等的输入控件中
 *          绿色的3D效果的输入框
 *			
 */

#pragma once


// CMyEdit

class CMyEdit : public CEdit
{
	DECLARE_DYNAMIC(CMyEdit)

public:
	CMyEdit();
	virtual ~CMyEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnPaint();
	COLORREF m_Colour;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnPaint();
protected:
	virtual void PreSubclassWindow();
public:
	afx_msg void OnNcPaint();
};


