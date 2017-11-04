#pragma once


// CFortunaMonitorDlg dialog
#include "Fortuna.h"
#include "afxwin.h"

class CitadelSoftwareInc::Pool;

class CFortunaMonitorDlg : public CDialog
{
	DECLARE_DYNAMIC(CFortunaMonitorDlg)

public:
	CFortunaMonitorDlg(CitadelSoftwareInc::Fortuna* pFortuna, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFortunaMonitorDlg();

// Dialog Data
	enum { IDD = IDD_FORTUNA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CitadelSoftwareInc::Fortuna* m_pFortuna;

	virtual void OnOK();
	afx_msg void OnLbnSelchangeListPools();
	afx_msg void OnTimer(UINT nTimerID);
	UINT_PTR m_nTimer;		// timer handle

	DECLARE_MESSAGE_MAP()

	std::vector<CitadelSoftwareInc::Pool*> m_Pools;
	std::vector<CitadelSoftwareInc::Source*> m_Sources;

private:
	void UpdateValues();
	void UpdateGeneratorValues();
	void UpdatePoolValues();
	void UpdateSinglePoolValues();
	void UpdateSourceValues();
	void UpdateSingleSourceValues();
	void UpdateCompressionRatio(CitadelSoftwareInc::Source* pSource);

	std::vector<unsigned char> m_vCacheData;						// input to the compression algorithm
	std::vector<unsigned char> m_vCacheDataResult;				// output from the compression algorithm

private:
	CFortunaMonitorDlg();
	CFortunaMonitorDlg(const CFortunaMonitorDlg& x);
	CFortunaMonitorDlg& operator=(CFortunaMonitorDlg& x);
public:
	CEdit m_GenCounter;
	CEdit m_GenKey;
	CEdit m_GenReseedCount;
	CEdit m_GenLastReseed;

	CListBox m_ListPools;

	CEdit m_PoolTotalBytes;
	CEdit m_PoolCompactCount;
	CEdit m_PoolRaw;
	CEdit m_PoolHash;
	CListBox m_ListSources;
	CEdit m_SourceTotalBytes;
	CEdit m_SourceCycles;
	afx_msg void OnLbnSelchangeListSources();
	CEdit m_SourcePeek;
	// check box to enable the compression test
	CButton m_BtnCompress;
	CEdit m_EditSrcCompression;
	DWORD m_dwLastCompression;
	afx_msg void OnBnClickedCheckSourceCompress();
	CEdit m_SrcCacheSize;
};
