
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
	CTSSFile(CString p)
		:m_Path(p), m_pImg(nullptr), m_pGfx(nullptr), m_pBmp(nullptr), m_hHistThread(nullptr), m_pHTR(nullptr), m_hPicThread(nullptr), m_pITR(nullptr)
	{
		m_Name = m_Path.Mid(m_Path.ReverseFind('\\') + 1);
	}
	CString m_Path;
	CString m_Name;
	Gdiplus::Image* m_pImg;
	Gdiplus::Graphics* m_pGfx;
	Gdiplus::Bitmap* m_pBmp;
	HANDLE m_hHistThread;
	class CHistThreadReturn* m_pHTR;
	HANDLE m_hPicThread;
	class CPicThreadReturn* m_pITR;
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
	CPicThreadParam(CString p, HWND hw, Gdiplus::Image* pi, volatile PHANDLE ph):m_Path(p), m_hWnd(hw), m_pImg(pi), m_vphThread(ph) {}
	CString m_Path;
	HWND m_hWnd;
	Gdiplus::Image* m_pImg;
	volatile PHANDLE m_vphThread;
};

class CPicThreadReturn
{
public:
	CPicThreadReturn(CString p):m_Path(p), m_pImgR(nullptr), m_pImgG(nullptr), m_pImgB(nullptr), m_pImgRG(nullptr), m_pImgRB(nullptr), m_pImgGB(nullptr) {}
	CString m_Path;
	Gdiplus::Image* m_pImgR;
	Gdiplus::Image* m_pImgG;
	Gdiplus::Image* m_pImgB;
	Gdiplus::Image* m_pImgRG;
	Gdiplus::Image* m_pImgRB;
	Gdiplus::Image* m_pImgGB;
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
	static void WINAPI RemovePicChannel(LPVOID pPic, bool bR, bool bG, bool bB);
	afx_msg LRESULT OnMsgPicReady(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnDrawHist(WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI CalcHistThread(LPVOID lpParam);
	afx_msg LRESULT OnMsgHistReady(WPARAM wParam, LPARAM lParam);
	Gdiplus::Pen* m_pPenHistR;
	Gdiplus::Pen* m_pPenHistG;
	Gdiplus::Pen* m_pPenHistB;

	std::vector<CTSSFile> m_Files;
	bool IsFileOpen(CTSSFile* pFile);
	BOOL PreTranslateMessage(PMSG pMsg);
	afx_msg void OnLvnItemchangedListFile(NMHDR* pNMHDR, LRESULT* pResult);
	int m_SelectedItem = -1;
	
	void OnCheckBoxClick(UINT nIDCheckItem, bool* bState);
	afx_msg void OnHistogramR();
	afx_msg void OnHistogramG();
	afx_msg void OnHistogramB();
	afx_msg void OnImageR();
	afx_msg void OnImageG();
	afx_msg void OnImageB();
	bool m_bHistR = 1;
	bool m_bHistG = 1;
	bool m_bHistB = 1;
	bool m_bPicR = 1;
	bool m_bPicG = 1;
	bool m_bPicB = 1;
};