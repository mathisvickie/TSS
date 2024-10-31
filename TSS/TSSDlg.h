
// TSSDlg.h : header file
//

#pragma once
#include <vector>

#define loop(var, max) for(UINT (var) = 0; (var) < (max); (var)++)
#define safe_delete(p) if(p) delete (p);

enum
{
	WM_DRAW_IMAGE = WM_USER + 1,
	WM_DRAW_HISTOGRAM,
	WM_INVALIDATE
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
	CTSSFile(CString path):m_Path(path), m_pImg(nullptr), m_pGfx(nullptr), m_pBmp(nullptr), m_bHistReady(FALSE)
	{
		m_Name = m_Path.Mid(m_Path.ReverseFind('\\') + 1);

		loop(_, 256)
		{
			m_Red.push_back(0);
			m_Green.push_back(0);
			m_Blue.push_back(0);
		}
	}
	CString m_Path;
	CString m_Name;
	Gdiplus::Image* m_pImg;
	Gdiplus::Graphics* m_pGfx;
	Gdiplus::Bitmap* m_pBmp;
	std::vector<UINT> m_Red;
	std::vector<UINT> m_Green;
	std::vector<UINT> m_Blue;
	bool m_bHistReady;
};

class CHistThreadParams
{
public:
	CHistThreadParams(HWND hw, UINT* p, UINT s, UINT xm, UINT ym, std::vector<UINT>* r, std::vector<UINT>* g, std::vector<UINT>* b, bool* br)
		:hwnd(hw), pixels(p), stride(s), x_max(xm), y_max(ym), red(r), green(g), blue(b), bReady(br) { }
	HWND hwnd;
	UINT* pixels;
	UINT stride;
	UINT x_max;
	UINT y_max;
	std::vector<UINT>* red;
	std::vector<UINT>* green;
	std::vector<UINT>* blue;
	bool* bReady;
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
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListCtrl m_FileList;
	CRect m_Rect;
	CRect m_FileListRect;
	afx_msg void OnHelpAbout();
	CStaticHist m_Hist;
	CRect m_HistRect;
	CStaticImage m_Pic;
	CRect m_PicRect;
	afx_msg void OnFileExit();
	afx_msg void OnFileSave();
	afx_msg LRESULT OnDrawImage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDrawHist(WPARAM wParam, LPARAM lParam);
	std::vector<CTSSFile> m_Files;
	bool IsFileOpen(CTSSFile* pFile);
	BOOL PreTranslateMessage(PMSG pMsg);
	afx_msg void OnLvnItemchangedListFile(NMHDR* pNMHDR, LRESULT* pResult);
	int m_SelectedItem = -1;
	afx_msg void OnHistogramR();
	afx_msg void OnHistogramG();
	afx_msg void OnHistogramB();
	bool m_Red = 1;
	bool m_Green = 1;
	bool m_Blue = 1;
	Gdiplus::Pen* m_pPenR;
	Gdiplus::Pen* m_pPenG;
	Gdiplus::Pen* m_pPenB;
	static DWORD WINAPI CalcHistThread(LPVOID lpParam);
	afx_msg LRESULT OnMsgInvalidate(WPARAM wParam, LPARAM lParam);
};