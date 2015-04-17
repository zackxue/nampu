// DeviceFind.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "DeviceFind.h"
#include "COMDEF.H"


// CDeviceFind 对话框

extern CManage gManage;
IMPLEMENT_DYNAMIC(CDeviceFind, CDialog)

CDeviceFind::CDeviceFind(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceFind::IDD, pParent)
{

}

CDeviceFind::~CDeviceFind()
{
}

void CDeviceFind::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FACTORY, m_factory_combox);
	DDX_Control(pDX, IDC_DEVICETYPE, m_devicetype_combox);
	DDX_Control(pDX, IDC_VERSION, m_version_combox);
	DDX_Control(pDX, IDC_CLOSE, m_close_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDOK, m_ok_btn);
}


BEGIN_MESSAGE_MAP(CDeviceFind, CDialog)
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CLOSE, &CDeviceFind::OnBnClickedClose)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDeviceFind 消息处理程序
extern int gCleanList_tag;
void CDeviceFind::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	CString factoryname,devicetype,version;


	GetDlgItemText(IDC_FACTORY,factoryname);
	GetDlgItemText(IDC_DEVICETYPE,devicetype);
	GetDlgItemText(IDC_VERSION,version);
	//GetDeviceList(void*,CString factory = _T(""),int m = -1,CString k = _T(""),int offset = 0,int deviceId = -1);
	 int i ;
	 if(devicetype.IsEmpty())
	   {
		   i = -1;
	   }
	else{
	const char *test = gManage.CString2char(gManage.GetDeviceNum(devicetype));
	 i= StringToLong(test);
	 delete (void*)test;
	}
	gCleanList_tag = 0;
	for(int j = 0 ;j<TIMES;j++){
		gManage.GetDeviceList(this,factoryname,i,version,0+j*5);
	}
	
	CDialog::OnOK();
}

BOOL CDeviceFind::OnInitDialog()
{
	CDialog::OnInitDialog();

	factory *rLinkfactory = NULL;//厂家链表指针
	device *pLinkdevice = NULL;
	device *rLinkdevice = NULL;//设备链表指针
	sdkversion *pLinkversion = NULL;
	sdkversion *rLinkversion = NULL;//版本链表指针 



	rLinkfactory = gManage.m_pFactory;
	while(rLinkfactory){
		m_factory_combox.AddString(rLinkfactory->name);//向下拉列表框填相关内容
		rLinkfactory = rLinkfactory->next;
	}
	if (gManage.m_pFactory)
		pLinkdevice = gManage.m_pFactory->pDevice;
	rLinkdevice = pLinkdevice; 
	while(pLinkdevice){
		m_devicetype_combox.AddString(gManage.GetDeviceType(pLinkdevice->name));
		pLinkdevice = pLinkdevice->next;
	}
	if(rLinkdevice)
		pLinkversion = rLinkdevice->pVsersion; 
	while(pLinkversion){

		m_version_combox.AddString(pLinkversion->name);
		pLinkversion = pLinkversion->next;

	}

	

	HBITMAP hBitmap1 = NULL;
	HBITMAP hBitmap2 = NULL;
	hBitmap1 = (HBITMAP)::LoadImage(NULL,_T("images/top.bmp"), 
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
	hBitmap2 = (HBITMAP)::LoadImage(NULL,_T("images/band.bmp"), 
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
	if(hBitmap1 == NULL||hBitmap2 == NULL)
	{
		AfxMessageBox(_T("无法加载图片"));
		PostQuitMessage(0);
	}
	if (bit[0].m_hObject)
		bit[0].Detach();
	if (bit[1].m_hObject)
		bit[1].Detach();
	bit[0].Attach(hBitmap1);
	bit[1].Attach(hBitmap2);

	m_close_btn.SetImage(_T("images/cu_title_close.bmp"),_T("images/cu_sel_title_close.bmp"),
		_T("images/cu_sel_title_close.bmp"),_T("images/cu_sel_title_close.bmp"));


	m_ok_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_cancel_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_ok_btn.SetTextPos(CPoint(10,4));
	m_cancel_btn.SetTextPos(CPoint(10,4));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDeviceFind::DrawRangeImage(int i, CDC *pDC, CRect rc){



	CDC MemDC;
	BITMAP bm;
	bit[i].GetBitmap(&bm);

	int li_Width = bm.bmWidth;
	int li_Height = bm.bmHeight;


	MemDC.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = MemDC.SelectObject(&bit[i]);

	int x=rc.left;
	int y=rc.top;

	while (y < (rc.Height()+rc.top)) 
	{
		while(x < (rc.Width()+rc.left)) 
		{
			pDC->BitBlt(x, y, li_Width, li_Height, &MemDC, 0, 0, SRCCOPY);
			x += li_Width;
		}
		x = rc.left;
		y += li_Height;
	}

	//pDC->StretchBlt(x, y, rc.right - rc.left, rc.bottom - rc.top, &MemDC, 0, 0,2,2 ,SRCCOPY);
	MemDC.SelectObject(pOldBitmap);
	MemDC.DeleteDC();
}
void CDeviceFind::ChangeWindowRgn(CDC* pDC)
{
	COLORREF col = RGB(255,0,255);
	CRect rcClient;
	GetClientRect (rcClient);
	CRgn rgn;
	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

	SetWindowRgn (rgn, TRUE);
}

void CDeviceFind::OnNcPaint()
{
	CDialog::OnNcPaint();	
	CPaintDC dc(this);
	CDC szMemDC;
	CRect rcClient;
	CRect rc;
	GetClientRect(&rcClient);

	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_HEAD);
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	BITMAP bit;
	bitmap.GetBitmap(&bit);
	memdc.SelectObject(&bitmap);
	// rcClient.InflateRect(1,1,1,1);
	dc.StretchBlt(0,0,rcClient.Width(),rcClient.Height(),&memdc,0,0,bit.bmWidth,bit.bmHeight,SRCCOPY);

	rc = rcClient;
	rc.bottom = 31;
	szMemDC.CreateCompatibleDC(&dc);
	DrawRangeImage(0,&dc,rc);//top

	rc = rcClient;
	rc.right = 1;
	rc.top = rcClient.top - 31;
	DrawRangeImage(1,&dc,rc);//left

	rc = rcClient;
	rc.left = rcClient.right - 5;
	rc.top = rcClient.top - 31;
	DrawRangeImage(1,&dc,rc);//right

	rc = rcClient;
	rc.top = rcClient.bottom - 5;
	DrawRangeImage(0,&dc,rc);//bottom

	ChangeWindowRgn(&dc);


	dc.SetTextColor(RGB(255,255,240));
	dc.SetBkMode(TRANSPARENT);
	dc.TextOut(10,5,_T("设备查询"));
}

LRESULT CDeviceFind::OnNcHitTest(CPoint point)
{
	ScreenToClient(&point);

	RECT rtWindow;
	GetClientRect(&rtWindow);

	long wndHeight = rtWindow.bottom - rtWindow.top;
	long wndWidth = rtWindow.right - rtWindow.left;

	RECT rcW = {0,0,wndWidth,31};
	if(::PtInRect(&rcW,point))
	{  
		return HTCAPTION;  // 在拖动范围内
	}

	return CDialog::OnNcHitTest(point);
}

void CDeviceFind::OnBnClickedClose()
{

	OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}

HBRUSH CDeviceFind::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor==CTLCOLOR_STATIC)
	{  
		pDC->SetBkMode(TRANSPARENT);  
		//pDC->SetTextColor(RGB(68,129,2));
		return  (HBRUSH)GetStockObject(NULL_BRUSH); 

	}
	return hbr;
}
