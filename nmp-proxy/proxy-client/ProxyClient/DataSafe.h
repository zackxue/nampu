#pragma once
#include "MyBlackDlg.h"

// CDataSafe 对话框

class CDataSafe : public CMyBlackDlg
{
	DECLARE_DYNAMIC(CDataSafe)

public:
	CDataSafe(UINT IDD1=IDD_DATA_SAFE, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDataSafe();

// 对话框数据
	enum { IDD = IDD_DATA_SAFE };
	
	void BackupDataThread1(void *pThis);
	void UploadDataThread1(void *pThis);
	void BackupDataPromptThread1(void *pThis);
protected:
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	int				SetDataSavePath(CString &path);
	int				GetDataPath(CString& path);
	int				GetFileLength(CString file);
	int				RecvAndSaveFile(int port, int magic, int fileSize, CString path);
	int				UploadAndSendFile(int port, int magic, int fileSize, CString path);
	int				CannelBackup(); // 取消备份 
	int				CannelUpload(); //取消上传
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void	OnNcPaint();
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void	OnBnClickedBackup();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnBnClickedUpload();

	virtual BOOL	OnInitDialog();
	void			OnClose();

	CSkinBtn			m_cancel_btn;
	CSkinBtn			m_ok_btn;
	CSkinBtn			m_backup_btn;
	CSkinBtn			m_upload_btn;
	UINT_PTR			m_dl_timer;
	CProgressCtrl		m_ProgressBg; // 备份进度
	CProgressCtrl		m_ProgressUl; // 恢复进度
	BOOL				m_bProcessUl;
	BOOL				m_bProcessBg;
	HANDLE				m_hThreadBg;
	HANDLE				m_hThreadUl;
	BOOL                m_hTreadBgExit;
	BOOL                m_hTreadUlExit;
};
