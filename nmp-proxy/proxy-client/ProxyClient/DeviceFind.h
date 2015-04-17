#pragma once
#include "afxwin.h"
#include "skinbtn.h"
#include "drawbutton.h"

/*
 * @author: xiaohe
 * @date: 2003/03/04 
 * @detail: 查找设备的对话框  
 *
 */
// CDeviceFind 对话框

class CDeviceFind : public CDialog
{
	DECLARE_DYNAMIC(CDeviceFind)

public:
	CDeviceFind(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDeviceFind();

// 对话框数据
	enum { IDD = IDD_DEVICE_FIND };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	CComboBox m_factory_combox;
	CComboBox m_devicetype_combox;
	CComboBox m_version_combox;
	virtual BOOL OnInitDialog();
	afx_msg void OnNcPaint();
	void DrawRangeImage(int i, CDC *pDC, CRect rc);
	void ChangeWindowRgn(CDC* pDC);
	CBitmap bit[2];
	afx_msg LRESULT OnNcHitTest(CPoint point);
	CSkinBtn m_close_btn;
	afx_msg void OnBnClickedClose();
	CSkinBtn m_cancel_btn;
	CSkinBtn m_ok_btn;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
