
// TSSDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TSS.h"
#include "TSSDlg.h"
#include "afxdialogex.h"

#include "Library.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

void CStaticImage::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(WM_DRAW_IMAGE, (WPARAM)lpDrawItemStruct);
}
void CStaticHist::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(WM_DRAW_HISTOGRAM, (WPARAM)lpDrawItemStruct);
}

LRESULT CTSSDlg::OnDrawImage(WPARAM wParam, LPARAM lParam)
{
	/*
	LPDRAWITEMSTRUCT st = (LPDRAWITEMSTRUCT)wParam;
	CDC* pDC = CDC::FromHandle(st->hDC);
	pDC->TextOutW(100, 100, CString("Hello World!"));
	*/
	if (m_SelectedItem == -1) return S_OK;
	auto f = &m_Files[m_SelectedItem];
	if (!f->m_pImg) f->m_pImg = Gdiplus::Image::FromFile(f->m_Path);

	CRect ir;
	m_Pic.GetClientRect(&ir);
	INT bw = ir.Width();
	INT bh = ir.Height();
	INT iw = f->m_pImg->GetWidth();
	INT ih = f->m_pImg->GetHeight();
	INT x = 0;
	INT y = 0;
	INT w = 0;
	INT h = 0;

	if ((bw > iw && bh > ih) || (bw < iw && bh < ih))
	{
		if (ih > iw)
		{
			w = iw * bh / ih;
			h = bh;
			x = (bw - w) >> 1;

			if (w > bw)
			{
				w = bw;
				h = ih * bw / iw;
				x = 0;
				y = (bh - h) >> 1;
			}
		}
		else
		{
			w = bw;
			h = ih * bw / iw;
			y = (bh - h) >> 1;

			if (h > bh)
			{
				w = iw * bh / ih;
				h = bh;
				y = 0;
				x = (bw - w) >> 1;
			}
		}
	}
	else if (bw > iw && bh < ih)
	{
		w = iw * bh / ih;
		h = bh;
		x = (bw - w) >> 1;
	}
	else if (bw < iw && bh > ih)
	{
		w = bw;
		h = ih * bw / iw;
		y = (bh - h) >> 1;
	}
	auto pGraphics = Gdiplus::Graphics::FromHDC(reinterpret_cast<LPDRAWITEMSTRUCT>(wParam)->hDC);
	pGraphics->DrawImage(f->m_pImg, x, y, w, h);

	delete pGraphics;
	return S_OK;
}
LRESULT CTSSDlg::OnDrawHist(WPARAM wParam, LPARAM lParam)
{
	if (m_SelectedItem == -1) return S_OK;
	auto f = &m_Files[m_SelectedItem];
	if (!f->m_pImg) return S_OK;

	if (!f->m_pGfx && !f->m_pBmp && !f->m_bHistReady)
	{
		f->m_pBmp = new Gdiplus::Bitmap(f->m_pImg->GetWidth(), f->m_pImg->GetHeight(), f->m_pImg->GetPixelFormat());
		f->m_pGfx = new Gdiplus::Graphics(f->m_pBmp);

		f->m_pGfx->Clear(Gdiplus::Color::Transparent);
		f->m_pGfx->DrawImage(f->m_pImg, 0, 0, f->m_pImg->GetWidth(), f->m_pImg->GetHeight());

		Gdiplus::BitmapData bmp_data;
		f->m_pBmp->LockBits(&Gdiplus::Rect(0, 0, f->m_pBmp->GetWidth(), f->m_pBmp->GetHeight()), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmp_data);

		/*::CalcHist(reinterpret_cast<UINT*>(bmp_data.Scan0), (UINT)(bmp_data.Stride >= 0 ? bmp_data.Stride : -bmp_data.Stride) >> 2,
			f->m_pBmp->GetWidth(), f->m_pBmp->GetHeight(), &f->m_Red, &f->m_Green, &f->m_Blue);

		f->m_pBmp->UnlockBits(&bmp_data);*/
		
		SHistThreadParams* pThreadParams = reinterpret_cast<SHistThreadParams*>(::malloc(sizeof(SHistThreadParams)));
		pThreadParams->hwnd = GetParent()->GetSafeHwnd();
		pThreadParams->pixels = reinterpret_cast<UINT*>(bmp_data.Scan0);
		pThreadParams->stride = (UINT)(bmp_data.Stride >= 0 ? bmp_data.Stride : -bmp_data.Stride) >> 2;
		pThreadParams->x_max = f->m_pBmp->GetWidth();
		pThreadParams->y_max = f->m_pBmp->GetHeight();
		pThreadParams->red = &f->m_Red;
		pThreadParams->green = &f->m_Green;
		pThreadParams->blue = &f->m_Blue;
		pThreadParams->bReady = &f->m_bHistReady;

		::CloseHandle(::CreateThread(nullptr, NULL, ::CalcHistThread, pThreadParams, NULL, nullptr));
		return S_OK;
	}
	if (!f->m_bHistReady) return S_OK;
	auto pGraphics = Gdiplus::Graphics::FromHDC(reinterpret_cast<LPDRAWITEMSTRUCT>(wParam)->hDC);

	if (!m_pPenR) m_pPenR = new Gdiplus::Pen(Gdiplus::Color(255, 0, 0), 1.0);
	if (!m_pPenG) m_pPenG = new Gdiplus::Pen(Gdiplus::Color(0, 255, 0), 1.0);
	if (!m_pPenB) m_pPenB = new Gdiplus::Pen(Gdiplus::Color(0, 0, 255), 1.0);
	
	CRect hr;
	m_Hist.GetClientRect(&hr);
	auto w = static_cast<Gdiplus::REAL>(hr.Width());
	auto h = static_cast<Gdiplus::REAL>(hr.Height());
	auto xs = w / 255.f;
	auto ys = h / 255.f;
	
	loop(i, 256)
	{
		auto x = static_cast<Gdiplus::REAL>(i) * xs;

		if (m_Red) pGraphics->DrawLine(m_pPenR, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_Red[i]));
		if (m_Green) pGraphics->DrawLine(m_pPenG, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_Green[i]));
		if (m_Blue) pGraphics->DrawLine(m_pPenB, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_Blue[i]));
	}
	
	delete pGraphics;
	return S_OK;
}

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
	ON_COMMAND(ID_FILE_OPEN32771, &CTSSDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE32776, &CTSSDlg::OnFileClose)
	ON_WM_SIZE()
	ON_COMMAND(ID_HELP_ABOUT, &CTSSDlg::OnHelpAbout)
	ON_COMMAND(ID_FILE_EXIT32774, &CTSSDlg::OnFileExit)
	ON_COMMAND(ID_FILE_SAVE32773, &CTSSDlg::OnFileSave)
	ON_MESSAGE(WM_DRAW_IMAGE, &OnDrawImage)
	ON_MESSAGE(WM_DRAW_HISTOGRAM, &OnDrawHist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILE, &CTSSDlg::OnLvnItemchangedListFile)
	ON_COMMAND(ID_HISTOGRAM_R, &CTSSDlg::OnHistogramR)
	ON_COMMAND(ID_HISTOGRAM_G, &CTSSDlg::OnHistogramG)
	ON_COMMAND(ID_HISTOGRAM_B, &CTSSDlg::OnHistogramB)
END_MESSAGE_MAP()

// CTSSDlg message handlers

BOOL CTSSDlg::PreTranslateMessage(PMSG pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
	{
		CTSSDlg::OnFileClose();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

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
	m_Hist.GetWindowRect(&m_HistRect);
	ScreenToClient(&m_HistRect);
	m_Pic.GetWindowRect(&m_PicRect);

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


bool CTSSDlg::IsFileOpen(SFile* pFile)
{
	auto path = pFile->m_Path.GetBuffer();

	loop(i, m_Files.size())
	{
		auto buff = m_Files[i].m_Path.GetBuffer();

		if (!wcscmp(buff, path)) return true;
	}
	return false;
}

void CTSSDlg::OnFileOpen()
{
	CFileDialog fd(TRUE, nullptr, NULL, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, L"Image Files (*.png,*.bmp,*.jpg)|*.png; *.bmp; *.jpg|", this);
	if (fd.DoModal() != IDOK) return;

	POSITION pos(fd.GetStartPosition());
	while (pos)
	{
		SFile f;
		f.m_Path = fd.GetNextPathName(pos);
		f.m_Name = f.m_Path.Mid(f.m_Path.ReverseFind('\\') + 1);
		f.m_pImg = nullptr;
		f.m_pGfx = nullptr;
		f.m_pBmp = nullptr;

		loop(_, 256)
		{
			f.m_Red.push_back(0);
			f.m_Green.push_back(0);
			f.m_Blue.push_back(0);
		}
		f.m_bHistReady = FALSE;

		if (!IsFileOpen(&f))
		{
			m_Files.push_back(f);
			m_FileList.InsertItem(m_FileList.GetItemCount() + 1, f.m_Name);
		}
	}
}


void CTSSDlg::OnFileClose()
{
	if (m_SelectedItem == -1) return;

	loop(i, m_Files.size())
	{
		auto f = &m_Files[i];

		if (f->m_pGfx && f->m_pBmp && !f->m_bHistReady)
		{
			MessageBoxA(nullptr, "Please wait until all calculations are finished.", "Calculation in progress", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
			return;
		}
	}

	if (MessageBoxA(nullptr, "Do you want to really delete this item?", "Are you sure?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) != IDYES) return;

	safe_delete(m_Files[m_SelectedItem].m_pImg);
	safe_delete(m_Files[m_SelectedItem].m_pGfx);
	safe_delete(m_Files[m_SelectedItem].m_pBmp);
	m_Files.erase(m_Files.begin() + m_SelectedItem);

	m_FileList.DeleteItem(m_SelectedItem);
	m_FileList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	m_FileList.SetSelectionMark(0);
	Invalidate();
}


void CTSSDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (::IsWindow(m_FileList) && ::IsWindow(m_Hist) && ::IsWindow(m_Pic))
	{
		m_FileList.SetWindowPos(nullptr, 0, 0, m_FileListRect.Width(), m_FileListRect.Height() + cy - m_Rect.Height(), SWP_NOMOVE);
		m_Hist.SetWindowPos(nullptr, m_HistRect.left, m_HistRect.top + cy - m_Rect.Height(), 0, 0, SWP_NOSIZE);
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


void CTSSDlg::OnLvnItemchangedListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED))
	{
		m_SelectedItem = pNMLV->iItem;
		Invalidate();
	}
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uOldState & LVNI_SELECTED) && !(pNMLV->uNewState & LVNI_SELECTED))
	{
		m_SelectedItem = -1;
	}
	*pResult = 0;
}


void CTSSDlg::OnHistogramR()
{
	m_Red ^= 1;
	GetMenu()->CheckMenuItem(ID_HISTOGRAM_R, m_Red ? MF_CHECKED : MF_UNCHECKED);
	Invalidate();
}


void CTSSDlg::OnHistogramG()
{
	m_Green ^= 1;
	GetMenu()->CheckMenuItem(ID_HISTOGRAM_G, m_Green ? MF_CHECKED : MF_UNCHECKED);
	Invalidate();
}


void CTSSDlg::OnHistogramB()
{
	m_Blue ^= 1;
	GetMenu()->CheckMenuItem(ID_HISTOGRAM_B, m_Blue ? MF_CHECKED : MF_UNCHECKED);
	Invalidate();
}
