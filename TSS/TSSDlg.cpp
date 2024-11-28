
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
	safe_delete(pDC);
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
	Gdiplus::Image* pImg = nullptr;

	if (m_bPicR && !m_bPicG && !m_bPicB) pImg = f->m_pImgR;
	if (!m_bPicR && m_bPicG && !m_bPicB) pImg = f->m_pImgG;
	if (!m_bPicR && !m_bPicG && m_bPicB) pImg = f->m_pImgB;

	if (m_bPicR && m_bPicG && !m_bPicB) pImg = f->m_pImgRG;
	if (m_bPicR && !m_bPicG && m_bPicB) pImg = f->m_pImgRB;
	if (!m_bPicR && m_bPicG && m_bPicB) pImg = f->m_pImgGB;

	if (!pImg) pImg = f->m_pImg;
	if (m_bPicR || m_bPicG || m_bPicB) pGraphics->DrawImage(pImg, x, y, w, h);

	safe_delete(pGraphics);
	return S_OK;
}
void WINAPI CTSSDlg::RemovePicChannel(Gdiplus::Image** ppImg, BOOL bR, BOOL bG, BOOL bB)
{
	UINT mask = 0xFF000000 + 0xFF0000 * bR + 0xFF00 * bG + 0xFF * bB;

	auto pBmp = new Gdiplus::Bitmap(ppImg[0]->GetWidth(), ppImg[0]->GetHeight(), ppImg[0]->GetPixelFormat());
	auto pGfx = new Gdiplus::Graphics(pBmp);
	
	pGfx->Clear(Gdiplus::Color::Transparent);
	pGfx->DrawImage(ppImg[0], 0, 0, ppImg[0]->GetWidth(), ppImg[0]->GetHeight());

	Gdiplus::BitmapData bmp_data;
	pBmp->LockBits(&Gdiplus::Rect(0, 0, pBmp->GetWidth(), pBmp->GetHeight()),
		Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bmp_data);

	auto pixels = reinterpret_cast<UINT*>(bmp_data.Scan0);
	auto stride = (UINT)(bmp_data.Stride >= 0 ? bmp_data.Stride : -bmp_data.Stride) >> 2;
	auto x_max = pBmp->GetWidth();
	auto y_max = pBmp->GetHeight();

	loop(y, y_max)
	{
		UINT ys = y * stride;

		loop(x, x_max)
		{
			pixels[ys + x] &= mask;
		}
	}
	pBmp->UnlockBits(&bmp_data);

	safe_delete(ppImg[0]);
	ppImg[0] = static_cast<Gdiplus::Image*>(pBmp);
	safe_delete(pGfx);
}
DWORD WINAPI CTSSDlg::CalcPicThread(LPVOID lpParam)
{
	auto p = reinterpret_cast<CPicThreadParam*>(lpParam);
	auto pRet = new CPicThreadReturn(p);

	CTSSDlg::RemovePicChannel(&pRet->m_pImgFx, p->m_bR, p->m_bG, p->m_bB);

	while (!p->m_vphThread) ::SwitchToThread();
	::CloseHandle(p->m_vphThread[0]);
	p->m_vphThread[0] = nullptr;

	::SendMessageW(p->m_hWnd, WM_PICTURE_READY, (WPARAM)pRet, 0);
	safe_delete(p);
	return NULL;
}
LRESULT CTSSDlg::OnMsgPicReady(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	auto p = reinterpret_cast<CPicThreadReturn*>(wParam);
	auto path = p->m_Path.GetBuffer();

	loop(i, m_Files.size())
	{
		auto f = &m_Files[i];

		if (!wcscmp(f->m_Path.GetBuffer(), path))
		{
			if ((!p->m_bR && !p->m_bG && !p->m_bB) || (p->m_bR && p->m_bG && p->m_bB)) break;

			if (p->m_bR && !p->m_bG && !p->m_bB) f->m_pImgR = p->m_pImgFx;
			if (!p->m_bR && p->m_bG && !p->m_bB) f->m_pImgG = p->m_pImgFx;
			if (!p->m_bR && !p->m_bG && p->m_bB) f->m_pImgB = p->m_pImgFx;

			if (p->m_bR && p->m_bG && !p->m_bB) f->m_pImgRG = p->m_pImgFx;
			if (p->m_bR && !p->m_bG && p->m_bB) f->m_pImgRB = p->m_pImgFx;
			if (!p->m_bR && p->m_bG && p->m_bB) f->m_pImgGB = p->m_pImgFx;

			safe_delete(p);
			Invalidate();
			return S_OK;
		}
	}
	safe_delete(p->m_pImgFx);
	safe_delete(p);
	return S_OK;
}

LRESULT CTSSDlg::OnDrawHist(WPARAM wParam, LPARAM lParam)
{
	if (m_SelectedItem == -1) return S_OK;
	auto f = &m_Files[m_SelectedItem];
	if (!f->m_pImg) return S_OK;

	if (!f->m_pGfx && !f->m_pBmp && !f->m_hHistThread && !f->m_pHTR)
	{
		f->m_pBmp = new Gdiplus::Bitmap(f->m_pImg->GetWidth(), f->m_pImg->GetHeight(), f->m_pImg->GetPixelFormat());
		f->m_pGfx = new Gdiplus::Graphics(f->m_pBmp);

		f->m_pGfx->Clear(Gdiplus::Color::Transparent);
		f->m_pGfx->DrawImage(f->m_pImg, 0, 0, f->m_pImg->GetWidth(), f->m_pImg->GetHeight());

		auto pBmpData = new Gdiplus::BitmapData();
		f->m_pBmp->LockBits(&Gdiplus::Rect(0, 0, f->m_pBmp->GetWidth(), f->m_pBmp->GetHeight()), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, pBmpData);

		auto pThreadParam = new CHistThreadParam(f->m_Path, GetSafeHwnd(), f->m_pBmp, pBmpData, reinterpret_cast<UINT*>(pBmpData->Scan0),
			(UINT)(pBmpData->Stride >= 0 ? pBmpData->Stride : -pBmpData->Stride) >> 2, f->m_pBmp->GetWidth(), f->m_pBmp->GetHeight(), nullptr);

		f->m_hHistThread = ::CreateThread(nullptr, NULL, CTSSDlg::CalcHistThread, pThreadParam, NULL, nullptr);
		pThreadParam->m_vphThread = &f->m_hHistThread;

		return S_OK;
	}
	if (f->m_hHistThread || !f->m_pHTR) return S_OK;
	auto pGraphics = Gdiplus::Graphics::FromHDC(reinterpret_cast<LPDRAWITEMSTRUCT>(wParam)->hDC);

	if (!m_pPenHistR) m_pPenHistR = new Gdiplus::Pen(Gdiplus::Color(255, 0, 0), 1.0);
	if (!m_pPenHistG) m_pPenHistG = new Gdiplus::Pen(Gdiplus::Color(0, 255, 0), 1.0);
	if (!m_pPenHistB) m_pPenHistB = new Gdiplus::Pen(Gdiplus::Color(0, 0, 255), 1.0);
	
	CRect hr;
	m_Hist.GetClientRect(&hr);
	auto w = static_cast<Gdiplus::REAL>(hr.Width());
	auto h = static_cast<Gdiplus::REAL>(hr.Height());
	auto xs = w / 255.f;
	auto ys = h / 255.f;
	
	loop(i, 256)
	{
		auto x = static_cast<Gdiplus::REAL>(i) * xs;

		if (m_bHistR) pGraphics->DrawLine(m_pPenHistR, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_pHTR->m_R[i]));
		if (m_bHistG) pGraphics->DrawLine(m_pPenHistG, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_pHTR->m_G[i]));
		if (m_bHistB) pGraphics->DrawLine(m_pPenHistB, x, h, x, h - ys * static_cast<Gdiplus::REAL>(f->m_pHTR->m_B[i]));
	}
	
	safe_delete(pGraphics);
	return S_OK;
}
DWORD WINAPI CTSSDlg::CalcHistThread(LPVOID lpParam)
{
	auto p = reinterpret_cast<CHistThreadParam*>(lpParam);
	auto pRet = new CHistThreadReturn(p->m_Path);

	::CalcHist(p->m_pPixels, p->m_Stride, p->m_MaxX, p->m_MaxY, &pRet->m_R, &pRet->m_G, &pRet->m_B);
	p->m_pBmp->UnlockBits(p->m_pBmpData);
	safe_delete(p->m_pBmpData);

	while (!p->m_vphThread) ::SwitchToThread();
	::CloseHandle(p->m_vphThread[0]);
	p->m_vphThread[0] = nullptr;
	
	::SendMessageW(p->m_hWnd, WM_HISTOGRAM_READY, (WPARAM)pRet, 0);
	safe_delete(p);
	return NULL;
}
LRESULT CTSSDlg::OnMsgHistReady(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	auto p = reinterpret_cast<CHistThreadReturn*>(wParam);
	auto path = p->m_Path.GetBuffer();

	loop(i, m_Files.size())
	{
		auto f = &m_Files[i];

		if (!wcscmp(f->m_Path.GetBuffer(), path))
		{
			f->m_pHTR = p;
			safe_delete(f->m_pGfx);
			safe_delete(f->m_pBmp);

			Invalidate();
			return S_OK;
		}
	}
	safe_delete(p);
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
	ON_MESSAGE(WM_DRAW_IMAGE, &CTSSDlg::OnDrawImage)
	ON_MESSAGE(WM_DRAW_HISTOGRAM, &CTSSDlg::OnDrawHist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILE, &CTSSDlg::OnLvnItemchangedListFile)
	ON_COMMAND(ID_HISTOGRAM_R, &CTSSDlg::OnHistogramR)
	ON_COMMAND(ID_HISTOGRAM_G, &CTSSDlg::OnHistogramG)
	ON_COMMAND(ID_HISTOGRAM_B, &CTSSDlg::OnHistogramB)
	ON_MESSAGE(WM_HISTOGRAM_READY, &CTSSDlg::OnMsgHistReady)
	ON_MESSAGE(WM_PICTURE_READY, &CTSSDlg::OnMsgPicReady)
	ON_COMMAND(ID_IMAGE_R, &CTSSDlg::OnImageR)
	ON_COMMAND(ID_IMAGE_G, &CTSSDlg::OnImageG)
	ON_COMMAND(ID_IMAGE_B, &CTSSDlg::OnImageB)
	ON_COMMAND(ID_IMAGE_ORIGINAL, &CTSSDlg::OnImageOriginal)
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


BOOL CTSSDlg::IsFileOpen(CTSSFile* pFile)
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
		CTSSFile f(fd.GetNextPathName(pos));

		if (!IsFileOpen(&f))
		{
			m_Files.push_back(f);
			m_FileList.InsertItem(m_FileList.GetItemCount() + 1, f.m_Name);
		}
	}
}

void CTSSDlg::KillThread(PHANDLE phThread, DWORD dwExitCode)
{
	if (phThread[0] == nullptr) return;
	::SuspendThread(phThread[0]);
	::TerminateThread(phThread[0], dwExitCode);
	::WaitForSingleObject(phThread[0], INFINITE);
	::CloseHandle(phThread[0]);
	phThread[0] = nullptr;
}
void CTSSDlg::OnFileClose()
{
	if (m_SelectedItem == -1) return;
	auto f = &m_Files[m_SelectedItem];
	if (MessageBoxA(nullptr, "Do you want to really delete this item?", "Are you sure?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) != IDYES) return;
	
	KillThread(&f->m_hHistThread, 9);
	KillThread(&f->m_hPicThreadR, 10);
	KillThread(&f->m_hPicThreadG, 11);
	KillThread(&f->m_hPicThreadB, 12);
	KillThread(&f->m_hPicThreadRG, 13);
	KillThread(&f->m_hPicThreadRB, 14);
	KillThread(&f->m_hPicThreadGB, 15);

	safe_delete(f->m_pImg);
	safe_delete(f->m_pGfx);
	safe_delete(f->m_pBmp);
	safe_delete(f->m_pHTR);
	safe_delete(f->m_pImgR);
	safe_delete(f->m_pImgG);
	safe_delete(f->m_pImgB);
	safe_delete(f->m_pImgRG);
	safe_delete(f->m_pImgRB);
	safe_delete(f->m_pImgGB);

	m_Files.erase(m_Files.begin() + m_SelectedItem);

	m_FileList.DeleteItem(m_SelectedItem);
	m_FileList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	m_FileList.SetSelectionMark(0);

	OnImageOriginal();
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

		CheckImageChannels();
		Invalidate();
	}
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uOldState & LVNI_SELECTED) && !(pNMLV->uNewState & LVNI_SELECTED))
	{
		m_SelectedItem = -1;
		Invalidate();
	}
	*pResult = 0;
}


void CTSSDlg::OnCheckBoxClick(UINT nIDCheckItem, BOOL* bState)
{
	bState[0] ^= TRUE;
	GetMenu()->CheckMenuItem(nIDCheckItem, bState[0] ? MF_CHECKED : MF_UNCHECKED);
	Invalidate();
}

void CTSSDlg::OnHistogramR() { OnCheckBoxClick(ID_HISTOGRAM_R, &m_bHistR); }
void CTSSDlg::OnHistogramG() { OnCheckBoxClick(ID_HISTOGRAM_G, &m_bHistG); }
void CTSSDlg::OnHistogramB() { OnCheckBoxClick(ID_HISTOGRAM_B, &m_bHistB); }

void CTSSDlg::OnImageR() { OnCheckBoxClick(ID_IMAGE_R, &m_bPicR); CheckImageChannels(); }
void CTSSDlg::OnImageG() { OnCheckBoxClick(ID_IMAGE_G, &m_bPicG); CheckImageChannels(); }
void CTSSDlg::OnImageB() { OnCheckBoxClick(ID_IMAGE_B, &m_bPicB); CheckImageChannels(); }

void CTSSDlg::CheckImageChannels(void)
{
	if (m_SelectedItem == -1) return;
	auto f = &m_Files[m_SelectedItem];

	if ((m_bPicR && !m_bPicG && !m_bPicB && !f->m_pImgR && !f->m_hPicThreadR) ||
		(!m_bPicR && m_bPicG && !m_bPicB && !f->m_pImgG && !f->m_hPicThreadG) ||
		(!m_bPicR && !m_bPicG && m_bPicB && !f->m_pImgB && !f->m_hPicThreadB) ||
		(m_bPicR && m_bPicG && !m_bPicB && !f->m_pImgRG && !f->m_hPicThreadRG) ||
		(m_bPicR && !m_bPicG && m_bPicB && !f->m_pImgRB && !f->m_hPicThreadRB) ||
		(!m_bPicR && m_bPicG && m_bPicB && !f->m_pImgGB && !f->m_hPicThreadGB))
	{
		if (!f->m_pImg) f->m_pImg = Gdiplus::Image::FromFile(f->m_Path);
		auto pThreadParam = new CPicThreadParam(f->m_Path, GetSafeHwnd(), m_bPicR, m_bPicG, m_bPicB, f->m_pImg->Clone(), nullptr);

		auto hThread = ::CreateThread(nullptr, NULL, CTSSDlg::CalcPicThread, pThreadParam, NULL, nullptr);
		
		if (m_bPicR && !m_bPicG && !m_bPicB) { f->m_hPicThreadR = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadR; }
		if (!m_bPicR && m_bPicG && !m_bPicB) { f->m_hPicThreadG = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadG; }
		if (!m_bPicR && !m_bPicG && m_bPicB) { f->m_hPicThreadB = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadB; }

		if (m_bPicR && m_bPicG && !m_bPicB) { f->m_hPicThreadRG = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadRG; }
		if (m_bPicR && !m_bPicG && m_bPicB) { f->m_hPicThreadRB = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadRB; }
		if (!m_bPicR && m_bPicG && m_bPicB) { f->m_hPicThreadGB = hThread; pThreadParam->m_vphThread = &f->m_hPicThreadGB; }
	}
}

void CTSSDlg::OnImageOriginal()
{
	if (!m_bPicR) OnCheckBoxClick(ID_IMAGE_R, &m_bPicR);
	if (!m_bPicG) OnCheckBoxClick(ID_IMAGE_G, &m_bPicG);
	if (!m_bPicB) OnCheckBoxClick(ID_IMAGE_B, &m_bPicB);
}
