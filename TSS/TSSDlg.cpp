
// TSSDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TSS.h"
#include "TSSDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
public:
	afx_msg void OnHelpAbout();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_COMMAND(ID_HELP_ABOUT, &CAboutDlg::OnHelpAbout)
END_MESSAGE_MAP()


// CTSSDlg dialog



CTSSDlg::CTSSDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TSS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTSSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_FileList);
	DDX_Control(pDX, IDC_STATIC_HIST, m_Hist);
	DDX_Control(pDX, IDC_STATIC_PIC, m_Pic);
}

BEGIN_MESSAGE_MAP(CTSSDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CTSSDlg::OnLvnItemchangedList1)
	ON_COMMAND(ID_FILE_OPEN32771, &CTSSDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE32776, &CTSSDlg::OnFileClose)
	ON_WM_SIZE()
	ON_COMMAND(ID_HELP_ABOUT, &CTSSDlg::OnHelpAbout)
	ON_COMMAND(ID_FILE_EXIT32774, &CTSSDlg::OnFileExit)
	ON_COMMAND(ID_FILE_SAVE32773, &CTSSDlg::OnFileSave)
END_MESSAGE_MAP()


// CTSSDlg message handlers

BOOL CTSSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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
	GetClientRect(&m_Rect);
	m_FileList.GetWindowRect(&m_FileListRect);
	m_Hist.GetClientRect(&m_HistRect);
	m_Pic.GetClientRect(&m_PicRect);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTSSDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTSSDlg::OnPaint()
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
HCURSOR CTSSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTSSDlg::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
}


void CTSSDlg::OnFileOpen()
{
	CFileDialog fd(TRUE);
	fd.DoModal();
	auto f = fd.GetFileName();
	if (f == "") return;
	CListCtrl* lc = (CListCtrl*)GetDlgItem(IDC_LIST_FILE);
	lc->InsertItem(0, f);
}


void CTSSDlg::OnFileClose()
{
	CListCtrl* lc = (CListCtrl*)GetDlgItem(IDC_LIST_FILE);
	auto pos =  lc->GetFirstSelectedItemPosition();
	if (!pos) return;
	lc->DeleteItem(lc->GetNextSelectedItem(pos));
}


void CTSSDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (::IsWindow(m_FileList) && ::IsWindow(m_Hist) && ::IsWindow(m_Pic))
	{
		m_FileList.SetWindowPos(nullptr, 0, 0, m_FileListRect.Width(), m_FileListRect.Height() + cy - m_Rect.Height(), SWP_NOMOVE);
		m_Hist.SetWindowPos(nullptr, 11, m_HistRect.Height() + cy - m_Rect.Height() + 40, 0, 0, SWP_NOSIZE);
		m_Pic.SetWindowPos(nullptr, 0, 0, m_PicRect.Width() + cx - m_Rect.Width(), m_PicRect.Height() + cy - m_Rect.Height(), SWP_NOMOVE);
	}
	Invalidate();
}


void CAboutDlg::OnHelpAbout(){}
void CTSSDlg::OnHelpAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CTSSDlg::OnFileExit()
{
	CTSSDlg::OnCancel();
}


void CTSSDlg::OnFileSave()
{
	// TODO: Add your command handler code here
}


void CStaticImage::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(1, (WPARAM)lpDrawItemStruct);
}
void CStaticHist::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(1, (WPARAM)lpDrawItemStruct);
}