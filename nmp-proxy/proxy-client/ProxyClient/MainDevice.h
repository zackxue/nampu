
/*
 * @Author: XiaoHe
 * @Date  : 2003-03-04 
 * @Detail: MyListCtrl中的设备管理的界面 
 *			
 */

#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "DrawButton.h"
#include "afxwin.h"
#include "skinbtn.h"
#include "MyListCtrl.h"
#include "Usermanage.h"
#include "stdafx.h"
#include "MutexEx.h"

// CMainDevice 对话框


class DeviceState
{
public:
	DeviceState(CString id, CString status, CString errCode)
		:m_id(id),m_status(status),m_errCode(errCode){}
	CString m_id;
	CString m_status;
	CString m_errCode;
};

class DeviceStateList
{
public:
	CList<DeviceState*>		m_list;
	CMutexEx                m_lock;
};

class CMainDevice : public CDialog
{
	DECLARE_DYNAMIC(CMainDevice)

public:
	CCustomListCtrl m_DeviceList;
	CMainDevice(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDevice();

// 对话框数据
	enum { IDD = IDD_MAINDEVICE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CDrawButton m_add_btn;
	CDrawButton m_del_btn;
	CDrawButton m_modify_btn;
	
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnBnClickedButton2();

	afx_msg void OnBnClickedAdd();
	int m_tag_firstpage;
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void OnOK();
public:
	//device_info m_device;
	CString m_factoryName;
	int m_machineType;
	CString m_sdkVersion;
	int m_offset; /* 通过m_offset来获取指定页码的设备列表 */



	bool m_showFlag;


	afx_msg void OnBnClickedFind();
	CDrawButton m_find_btn;
	int m_count;
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedModify();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	CDrawButton m_back_btn;
	CDrawButton m_next_btn;
	CDrawButton m_goto_btn;
	CEdit m_page_edit;
	CEdit m_showpage_static;

	CImageList m_imgelist;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedGoto();
	afx_msg void OnPaint();
	afx_msg void OnNMRClickDevicelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeviceUpdate();
	void ShowDeviceList(void);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CDrawButton m_first_btn;
	CDrawButton m_tail_btn;
	afx_msg void OnBnClickedFirst();
	afx_msg void OnBnClickedTail();
	void ShowPage(void);

	void SetDevceState(int  index,CString StateId,CString errCode);

	//设备状态回调;
	static BOOL WINAPI Call_back_deviceState(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam);
	afx_msg void OnDeviceAdd();
	afx_msg void OnDeviceDel();
	afx_msg void OnDeviceModify();
	afx_msg void OnDeviceQuery();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	int GetLimitDeviceId(CString &startDeviveId, CString &endDeviceId);


	UINT_PTR                m_Timer;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	DeviceStateList         m_DeviceStateList;
};
