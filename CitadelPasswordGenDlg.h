// CitadelPasswordGenDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CCitadelPasswordGenDlg dialog
class CCitadelPasswordGenDlg : public CDialog
{
// Construction
public:
	CCitadelPasswordGenDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CITADELPASSWORDGEN_DIALOG };

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
	afx_msg void OnEnChangeEditSize();
	afx_msg void OnBnClickedBtnGen();
	afx_msg void OnBnClickedCheckaz();
	CButton m_azlower;
	CButton m_azupper;
	CButton m_digits;
	CButton m_punct;
	CEdit m_size;
	CEdit m_password;
	CButton m_generate;
	CSpinButtonCtrl m_spinner;

private:
	//! ensure that at least one checkbox is checked
	bool OneCheckbox();
	void SetGenState();

public:

	afx_msg void OnBnClickedCheckAzcaps();
	afx_msg void OnBnClickedCheck09();
	afx_msg void OnBnClickedCheckPunct();


	CButton m_MonitorButton;
	afx_msg void OnBnClickedButtonMonitor();
};
