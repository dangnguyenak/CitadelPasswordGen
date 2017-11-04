#pragma once
#include "afxwin.h"


// TestDialog2 dialog

class TestDialog2 : public CDialog
{
	DECLARE_DYNAMIC(TestDialog2)

public:
	TestDialog2(CWnd* pParent = NULL);   // standard constructor
	virtual ~TestDialog2();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEST2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton Testbutton;
	afx_msg void OnBnClickedButton1();
};
