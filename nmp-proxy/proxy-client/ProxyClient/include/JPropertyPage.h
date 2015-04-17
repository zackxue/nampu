#pragma once

//#define IDD_JPROPERTPAGE		0xFF12

// CJPropertyPage 对话框

class CJPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CJPropertyPage)

public:
	CJPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0, DWORD dwSize = sizeof(PROPSHEETPAGE));
	virtual ~CJPropertyPage();

// 对话框数据
	//enum { IDD = IDD_JPROPERTPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
