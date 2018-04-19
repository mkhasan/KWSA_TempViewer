
// ViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Viewer.h"
#include "ViewerDlg.h"
#include "afxdialogex.h"
//#include <atlstr.h>  

#include "USB2SERIAL_W32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif






CString ComPortName;

bool quit = false;
int value = -1;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CViewerDlg dialog



CViewerDlg::CViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIEWER_DIALOG, pParent)
	, port(_T("COM7"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SHOW, &CViewerDlg::OnBnClickedBtnShow)
	ON_WM_TIMER()
	ON_STN_CLICKED(IDC_STATIC_TEMP, &CViewerDlg::OnStnClickedStaticTemp)
END_MESSAGE_MAP()


// CViewerDlg message handlers

BOOL CViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(300, _T("Arial Bold"));
	CStatic * m_Label = (CStatic *)GetDlgItem(IDC_MSG);
	m_Label->SetFont(m_Font1);

	m_Label->SetWindowTextW(_T(""));

	CFont *m_Font2 = new CFont;
	m_Font2->CreatePointFont(200, _T("Arial Bold"));

	GetDlgItem(IDC_STATIC_TEMP)->SetFont(m_Font2);
	GetDlgItem(IDC_STATIC_TEMP)->SetWindowTextW(_T(""));

	GetDlgItem(IDC_ID)->SetWindowTextW(_T("1234"));
	GetDlgItem(IDC_COM)->SetWindowTextW(port);
	
	OnBnClickedBtnShow();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CViewerDlg::OnBnClickedBtnShow()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BTN_SHOW)->EnableWindow(false);
	GetDlgItem(IDC_COM)->EnableWindow(false);

	hThread = NULL;
	//static DWORD   dwThread;
	
	GetDlgItem(IDC_COM)->GetWindowTextW(port);

	ComPortName = port;

	value = -1;
		 
	hThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		USB2SERIAL_W32,       // thread function name
		NULL,          // argument to thread function 
		0,                      // use default creation flags 
		NULL);   // returns the thread identifier 


	if (hThread == NULL) {

		AfxMessageBox(_T("Error"));
		exit(0);
	}

	SetTimer(1, 1000, NULL);


}


extern sensorRecord sensor;
void CViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	double val = -1.0;
	CString str;
	int i=0;
	if (nIDEvent == 1) {
		// handle timer event

		if (sensor.ownPtr != NULL) {
			if (SensorRread(sensor.ComNr, &val) == SENSOR_OK) {
				str.Format(_T("%.2f"), val);
				GetDlgItem(IDC_MSG)->SetWindowTextW(str);
			}
				
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CViewerDlg::OnStnClickedStaticTemp()
{
	// TODO: Add your control notification handler code here
}


void CAboutDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	quit = true;
	CDialogEx::OnCancel();
}
