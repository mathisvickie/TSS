
// TSSDlg.h : header file
//

#pragma once
#include <vector>

#define loop(v, m) for(UINT (v) = 0; (v) < (m); (v)++)
#define safe_delete(p) if(p) { delete (p); (p) = nullptr; }

enum
{
	WM_DRAW_IMAGE = WM_USER + 1,
	WM_DRAW_HISTOGRAM,
	WM_HISTOGRAM_READY,
	WM_PICTURE_READY
};

class CStaticImage : public CStatic
{
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};
class CStaticHist : public CStatic
{
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};

class CTSSFile
{
public:
	CTSSFile(CString p):m_Path(p)
	{
		m_Name = m_Path.Mid(m_Path.ReverseFind('\\') + 1);
	}
	CString m_Path;
	CString m_Name;
	Gdiplus::Image* m_pImg = nullptr;
	Gdiplus::Graphics* m_pGfx = nullptr;
	Gdiplus::Bitmap* m_pBmp = nullptr;
	HANDLE m_hHistThread = nullptr;
	class CHistThreadReturn* m_pHTR = nullptr;
	HANDLE m_hPicThreadR = nullptr;
	HANDLE m_hPicThreadG = nullptr;
	HANDLE m_hPicThreadB = nullptr;
	HANDLE m_hPicThreadRG = nullptr;
	HANDLE m_hPicThreadRB = nullptr;
	HANDLE m_hPicThreadGB = nullptr;
	Gdiplus::Image* m_pImgR = nullptr;
	Gdiplus::Image* m_pImgG = nullptr;
	Gdiplus::Image* m_pImgB = nullptr;
	Gdiplus::Image* m_pImgRG = nullptr;
	Gdiplus::Image* m_pImgRB = nullptr;
	Gdiplus::Image* m_pImgGB = nullptr;
};

class CHistThreadParam
{
public:
	CHistThreadParam(CString p, HWND hw, Gdiplus::Bitmap* pb, Gdiplus::BitmapData* pbd, UINT* pp, UINT s, UINT mx, UINT my, volatile PHANDLE ph)
		:m_Path(p), m_hWnd(hw), m_pBmp(pb), m_pBmpData(pbd), m_pPixels(pp), m_Stride(s), m_MaxX(mx), m_MaxY(my), m_vphThread(ph) { }
	CString m_Path;
	HWND m_hWnd;
	Gdiplus::Bitmap* m_pBmp;
	Gdiplus::BitmapData* m_pBmpData;
	UINT* m_pPixels;
	UINT m_Stride;
	UINT m_MaxX;
	UINT m_MaxY;
	volatile PHANDLE m_vphThread;
};

class CHistThreadReturn
{
public:
	CHistThreadReturn(CString p):m_Path(p)
	{
		loop(_, 256)
		{
			m_R.push_back(0);
			m_G.push_back(0);
			m_B.push_back(0);
		}
	}
	CString m_Path;
	std::vector<UINT> m_R;
	std::vector<UINT> m_G;
	std::vector<UINT> m_B;
};

class CPicThreadParam
{
public:
	CPicThreadParam(CString p, HWND hw, BOOL r, BOOL g, BOOL b, Gdiplus::Image* pi, volatile PHANDLE ph)
		:m_Path(p), m_hWnd(hw), m_bR(r), m_bG(g), m_bB(b), m_pImg(pi), m_vphThread(ph) { }
	CString m_Path;
	HWND m_hWnd;
	BOOL m_bR;
	BOOL m_bG;
	BOOL m_bB;
	Gdiplus::Image* m_pImg;
	volatile PHANDLE m_vphThread;
};

class CPicThreadReturn
{
public:
	CPicThreadReturn(class CPicThreadParam* p):m_Path(p->m_Path), m_bR(p->m_bR), m_bG(p->m_bG), m_bB(p->m_bB), m_pImgFx(p->m_pImg) { }
	CString m_Path;
	BOOL m_bR;
	BOOL m_bG;
	BOOL m_bB;
	Gdiplus::Image* m_pImgFx;
};

// CTSSDlg dialog
class CTSSDlg : public CDialogEx
{
// Construction
public:
	CTSSDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TSS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHelpAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnFileSave();
	afx_msg void OnFileExit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CRect m_Rect;
	CListCtrl m_FileList;
	CRect m_FileListRect;
	CStaticHist m_Hist;
	CRect m_HistRect;
	CStaticImage m_Pic;
	CRect m_PicRect;
	void KillThread(PHANDLE phThread, DWORD dwExitCode);
	
	afx_msg LRESULT OnDrawImage(WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI CalcPicThread(LPVOID lpParam);
	static void WINAPI RemovePicChannel(Gdiplus::Image** ppImg, BOOL bR, BOOL bG, BOOL bB);
	afx_msg LRESULT OnMsgPicReady(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnDrawHist(WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI CalcHistThread(LPVOID lpParam);
	afx_msg LRESULT OnMsgHistReady(WPARAM wParam, LPARAM lParam);
	Gdiplus::Pen* m_pPenHistR = nullptr;
	Gdiplus::Pen* m_pPenHistG = nullptr;
	Gdiplus::Pen* m_pPenHistB = nullptr;

	std::vector<CTSSFile> m_Files;
	BOOL IsFileOpen(CTSSFile* pFile);
	BOOL PreTranslateMessage(PMSG pMsg);
	afx_msg void OnLvnItemchangedListFile(NMHDR* pNMHDR, LRESULT* pResult);
	int m_SelectedItem = -1;
	
	void OnCheckBoxClick(UINT nIDCheckItem, BOOL* bState);
	afx_msg void OnHistogramR();
	afx_msg void OnHistogramG();
	afx_msg void OnHistogramB();
	void CheckImageChannels(void);
	afx_msg void OnImageR();
	afx_msg void OnImageG();
	afx_msg void OnImageB();
	BOOL m_bHistR = TRUE;
	BOOL m_bHistG = TRUE;
	BOOL m_bHistB = TRUE;
	BOOL m_bPicR = TRUE;
	BOOL m_bPicG = TRUE;
	BOOL m_bPicB = TRUE;
	afx_msg void OnImageOriginal();
};