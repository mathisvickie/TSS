
// TSSDlg.h : header file
//

#pragma once

class CStaticImage : public CStatic
{
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};
class CStaticHist : public CStatic
{
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
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
	afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
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
};