// DataSafe.cpp : 实现文件
//

#include "stdafx.h"
#include "network.h"
#include "DataSafe.h"

#define MY_TIMER 1000
// CDataSafe 对话框
extern CManage gManage;

IMPLEMENT_DYNAMIC(CDataSafe, CMyBlackDlg)

CDataSafe::CDataSafe(UINT IDD1, CWnd* pParent /*=NULL*/)
	: CMyBlackDlg(IDD1, pParent)
	, m_hThreadBg(NULL)
	, m_hThreadUl(NULL)
	, m_hTreadBgExit(FALSE)
	, m_hTreadUlExit(FALSE)
{	
	m_bProcessUl = FALSE;
	m_bProcessBg = FALSE;
}

CDataSafe::~CDataSafe()
{
	//KillTimer(m_dl_timer);
}

void CDataSafe::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BACKUP, m_backup_btn);
	DDX_Control(pDX, IDC_UPLOAD, m_upload_btn);
	DDX_Control(pDX, IDC_PROGRESS_BG, m_ProgressBg);
	DDX_Control(pDX, IDC_PROGRESS_UL, m_ProgressUl);
}


BEGIN_MESSAGE_MAP(CDataSafe, CDialog)
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_CTLCOLOR()
	ON_COMMAND(CLOSE_BTN, OnClose)
	ON_BN_CLICKED(IDC_BACKUP, OnBnClickedBackup)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_UPLOAD, OnBnClickedUpload)
END_MESSAGE_MAP()


// CDataSafe 消息处理程序

LRESULT CDataSafe::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CMyBlackDlg::OnNcHitTest(point);
}

void CDataSafe::OnNcPaint()
{
	CMyBlackDlg::OnNcPaint();
}

HBRUSH CDataSafe::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CMyBlackDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

BOOL CDataSafe::OnInitDialog()
{
	CMyBlackDlg::OnInitDialog();
	m_dl_timer = SetTimer(MY_TIMER, 1000, NULL);  // 1s调用一次

	m_backup_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));
	m_upload_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_backup_btn.SetTextPos(CPoint(12,4));
	m_upload_btn.SetTextPos(CPoint(12,4));

	m_ProgressBg.SetRange(0, 100);
	m_ProgressUl.SetRange(0, 100);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDataSafe::OnClose()
{
	//if (m_bProcessUl)
	//{
	//	MessageBox(_T("数据正在进行恢复"), NULL, MB_OK);
	//	return;
	//}

	//if (m_bProcessBg)
	//{
	//	MessageBox(_T("数据正在进行备份"), NULL, MB_OK);
	//	return;
	//}

	if (m_bProcessUl && m_hThreadBg)
	{
		m_hTreadBgExit = TRUE;
		MessageBox(_T("取消数据备份"), NULL, MB_OK);
		CannelBackup();
	}

	if (m_bProcessBg && m_hThreadUl)
	{
		m_hTreadUlExit = TRUE;
		MessageBox(_T("取消数据恢复"), NULL, MB_OK);
		CannelUpload();
	}

	EndDialog(2);
}

CString GetBackupFileName()
{
	CString strFileName;

	SYSTEMTIME curTime;;
	GetLocalTime(&curTime);
	strFileName.Format(_T("proxyclient%4d%02d%02d%02d%02d%02d.dat"), curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond);

	return strFileName;
}

int CDataSafe::SetDataSavePath(CString &path)
{
	
	CFileDialog dlg(FALSE, _T("*.*"), GetBackupFileName(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("所有类型"), this);
	
	CString strModuleBaseName;
	GetModuleFileName(NULL, strModuleBaseName.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strModuleBaseName.ReleaseBuffer();
	int iPos = strModuleBaseName.ReverseFind(_T('\\'));
	CString strFile = strModuleBaseName.Left(iPos + 1);

	dlg.m_ofn.lpstrInitialDir = strFile;
	if (dlg.DoModal() == IDOK)  //按键是否按下
	{
		CStdioFile file;
		CString ss = dlg.GetPathName();
		BOOL res = file.Open(ss, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		if(!res)
		{
			MessageBox(ss + _T("打开失败！"));
			return -1;
		}
		else
		{
			path = ss;  // 获取保存文件路径
			file.Close();
			return 0;
		}
	}

	TRACE1("CFileDialog  %d\n", GetLastError());

	return -2;
}

DWORD WINAPI BackupDataPromptThread(LPVOID lpParameter)
{
	CDataSafe *pThis = (CDataSafe *)lpParameter;

	pThis->BackupDataPromptThread1(pThis);
	return 0;
}

void CDataSafe::BackupDataPromptThread1(void *pThis)
{
	//if (MessageBox(_T("数据正在进行备份中..."), NULL, MB_OK) != MB_OK);
}

void* WINAPI BackupDataThread(void* lpParameter)
{
	CDataSafe *pThis = (CDataSafe *)lpParameter;

	pThis->BackupDataThread1(pThis);
	return NULL;
}

void CDataSafe::BackupDataThread1(void *pThis)
{
	int ret;
	CString port, magic, fileSize;
	CString filePath;
	int nPort;
	int nMagic;
	int nFileSize;

	ret = gManage.DownloadData(port, magic, fileSize);
	if (ret < 0)
	{
		MessageBox(_T("备份数据失败"), NULL, MB_OK);
		goto _error;
	}

	ret = SetDataSavePath(filePath);
	if (ret == -1)
	{
		MessageBox(_T("获取文件路径失败"), NULL, MB_OK);
		goto _error;
	}
	else if (ret == -2)
	{
		goto _error;
	}

	TRACE("%s\n", filePath);

	nPort = StringToLong(StringUnicodeToAscii((LPCTSTR)port).c_str());
	nMagic = StringToLong(StringUnicodeToAscii((LPCTSTR)magic).c_str());
	nFileSize = StringToLong(StringUnicodeToAscii((LPCTSTR)fileSize).c_str());

	RecvAndSaveFile(nPort, nMagic, nFileSize, filePath);

_error:
	m_bProcessBg = FALSE;
	return;
}

int	
CDataSafe::CannelBackup() // 取消备份
{
	if (m_hThreadBg)
	{
		WaitThread(m_hThreadBg);
	}

	return 0;
}
int	
CDataSafe::CannelUpload() //取消上传
{
	if (m_hThreadUl)
	{
		WaitThread(m_hThreadUl);
	}

	return 0;
}

void CDataSafe::OnBnClickedBackup()
{	
	if (m_bProcessBg)
	{
		MessageBox(_T("数据正在备份中"), NULL, MB_OK);
		return;
	}

	if (m_bProcessUl)
	{
		MessageBox(_T("数据正在恢复中"), NULL, MB_OK);
		return;
	}

	m_hThreadBg = CreateThreadEx(BackupDataThread, this);
	if (!m_hThreadBg)
	{
		MessageBox(_T("创建备份线程失败"), NULL, MB_OK);
		return;
	}

	m_bProcessBg = TRUE;
}

void CDataSafe::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nIDEvent == MY_TIMER)
	{
	
	}

	CMyBlackDlg::OnTimer(nIDEvent);
}

#define MAX_BUFFER_SIZE (1 * 1024)

int CDataSafe::RecvAndSaveFile(int port, int magic, int fileSize, CString path)
{
	int wRet;
	int fd, ret;
	ULONGLONG total = 0;
	int nMaigc = 0;
	char buf[MAX_BUFFER_SIZE] = {0};

	FILE *file = fopen(StringUnicodeToAscii((LPCTSTR)path).c_str(), "wb");
	if (!file)
	{
		MessageBox(_T("创建本地文件失败"), NULL, MB_OK);
		goto _error;
	}

	fd = socket_connect(gManage.m_IP, port);
	if (fd < 0)
	{
		MessageBox(_T("连接备份服务器失败"), NULL, MB_OK);
		goto _error;
	}

	magic = htonl(magic);
	ret = socket_write(fd, (char *)&magic, 4);
	if (ret != 4)
	{
		TRACE("socket_write magic error, ret:%d", ret);
		goto _error;
	}
	
	for (;;)
	{
		if (m_hTreadBgExit  == TRUE)
		{
			break;
		}

		if (total >= fileSize)
		{
			if (total > fileSize)
			{
				MessageBox(_T("数据大小异常"), NULL, MB_OK);
				goto _finish;			
			}
			m_ProgressBg.SetPos(100);
			MessageBox(_T("备份成功"), NULL, MB_OK);
			
			goto _finish;
		}
		ret = socket_read(fd, buf, MAX_BUFFER_SIZE);
		if (ret < 0)
		{
			MessageBox(_T("备份数据失败"), NULL, MB_OK);
			goto _error;
		}

		total += ret;
		wRet = fwrite(buf, 1, ret, file);
		if (wRet != ret)
		{
			MessageBox(_T("写入文件失败"), NULL, MB_OK);
			goto _error;
		}
		// 设置进度
		m_ProgressBg.SetPos((int)(total  * 100 / fileSize));
	}
	
	return 0;

_error:
	if (fd > 0)
		closesocket(fd);
	
	if (file)
		fclose(file);

	m_bProcessBg = FALSE;
	return -1;

_finish:
	closesocket(fd);
	if (file)
		fclose(file);
	m_ProgressBg.SetPos(0);
	m_bProcessBg = FALSE;
	return 0;
}

int CDataSafe::UploadAndSendFile(int port, int magic, int fileSize, CString path)
{
	int fd = 0, ret;
	ULONGLONG total = 0;
	char buf[MAX_BUFFER_SIZE] = {0};
	FILE *file = NULL;

	if (fileSize <= 0)
	{
		MessageBox(_T("异常的文件大小"), NULL, MB_OK);
		goto _error;
	}

	file = fopen(StringUnicodeToAscii((LPCTSTR)path).c_str(), "rb");
	if (!file)
	{
		MessageBox(_T("读取备份文件失败"), NULL, MB_OK);
		goto _error;
	}

	fd = socket_connect(gManage.m_IP, port);
	if (fd < 0)
	{
		MessageBox(_T("连接上传服务器失败"), NULL, MB_OK);
		goto _error;
	}
	
	/* 连接成功后, 设置为非阻塞 */
	//if(socket_set_noblock(fd) < 0)
	//{
	//	goto _error;
	//}

	magic = htonl(magic);
	ret = socket_write(fd, (char *)&magic, 4);
	if (ret != 4)
	{
		TRACE("socket_write magic error, ret:%d", ret);
		goto _error;
	}

	for (;;)
	{
		if (m_hTreadUlExit  == TRUE)
		{
			break;
		}

		ret = fread(buf, 1, MAX_BUFFER_SIZE, file);
		if (ret != MAX_BUFFER_SIZE)
		{
			if (feof(file))
			{
				
			}
			else
			{
				MessageBox(_T("读取数据失败"), NULL, MB_OK);
				goto _error;
			}
		}

		ret = socket_write(fd, buf, ret);
		if (ret < 0)
		{
			MessageBox(_T("上传数据失败"), NULL, MB_OK);
			goto _error;
		}
		
		total += ret;
		m_ProgressUl.SetPos((int)(total * 100 / fileSize));
		if (total == fileSize)
		{
			m_ProgressUl.SetPos(100);
			MessageBox(_T("恢复成功, 请重新登录客户端"), NULL, MB_OK);
			closesocket(fd);
			if (file)
				fclose(file);
			
			m_ProgressUl.SetPos(0);
			m_bProcessUl = FALSE;
			return 0;
		}
	}

_error:
	if (fd > 0)
		closesocket(fd);

	if (file)
		fclose(file);
	
	m_bProcessUl = FALSE;	
	return -1;
}

int CDataSafe::GetFileLength(CString file)
{
	CFile *pFile;
	ULONGLONG fileLength;
	
	pFile = new CFile(file, CFile::modeRead | CFile::shareDenyNone);
	if (!pFile)
	{
		MessageBox(_T("打开文件失败"), NULL, MB_OK);
		return -1 ;
	}

	fileLength = pFile->GetLength();
	if (pFile)
	{
		pFile->Close();
	}

	return (int)fileLength;
}

//DWORD WINAPI UploadDataThread(LPVOID lpParameter)
void * WINAPI UploadDataThread(void *lpParameter)
{
	CDataSafe *pThis = (CDataSafe *)lpParameter;
	
	pThis->UploadDataThread1(pThis);
	return 0;
}

void CDataSafe::UploadDataThread1(void *pThis)
{
	int ret;
	CFile  file;
	CString filePath;
	CString fileSize, port, magic;
	ULONGLONG fileLength;
	
	ret = GetDataPath(filePath);
	if (ret == -1)
	{
		MessageBox(_T("获取文件地址失败"), NULL, MB_OK);
		goto _error;
	}
	else if (ret == -2)
	{
		goto _error;
	}

	fileLength = GetFileLength(filePath);
	if (fileLength < 0)
	{
		MessageBox(_T("获取文件大小失败"), NULL, MB_OK);
		goto _error;	
	}

	fileSize.Format(_T("%d"), fileLength); // 加4个字节的magic长度
	ret = gManage.UploadData(fileSize, port, magic);
	if (ret < 0)
	{
		MessageBox(_T("恢复文件失败"), NULL, MB_OK);
		goto _error;
	}

	ret = UploadAndSendFile(StringToLong(StringUnicodeToAscii((LPCTSTR)port).c_str()), StringToLong(StringUnicodeToAscii((LPCTSTR)magic).c_str()), (int)fileLength, filePath);
	if (ret < 0)
	{
		MessageBox(_T("发送文件失败"), NULL, MB_OK);
		goto _error;
	}

_error:
	m_bProcessUl = FALSE;
	return;
}

void CDataSafe::OnBnClickedUpload()
{
	if (m_bProcessBg)
	{
		MessageBox(_T("数据正在备份中"), NULL, MB_OK);
		return;
	}

	if (m_bProcessUl)
	{
		MessageBox(_T("数据正在恢复中"), NULL, MB_OK);
		return;
	}

	m_hThreadUl = CreateThreadEx(UploadDataThread, this);
	if (!m_hThreadUl)
	{
		MessageBox(_T("创建恢复线程失败"), NULL, MB_OK);
		return;
	}
	m_bProcessUl = TRUE;
}

int CDataSafe::GetDataPath(CString& path)
{
	CFileDialog dlg(TRUE, _T("*.*"), _T("*.*"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("所有类型"), this);
	
	CString strModulePath;
	GetModuleFileName(NULL, strModulePath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	strModulePath.ReleaseBuffer();
	int iPos = strModulePath.ReverseFind(_T('\\'));
	CString strPath = strModulePath.Left(iPos + 1);
	dlg.m_ofn.lpstrInitialDir = strPath;
	if (dlg.DoModal() == IDOK)  //按键是否按下
	{
		CStdioFile file;
		CString ss = dlg.GetPathName();
		BOOL res = file.Open(ss, CFile::modeRead | CFile::shareDenyNone | CFile::typeText);
		if(!res)
		{
			MessageBox(ss + _T("打开失败！"));
			return -1;
		}
		else
		{
			path = ss;  // 获取保存文件路径
			file.Close();
			return 0;
		}
	}

	return -2;
}