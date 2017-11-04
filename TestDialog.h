#pragma once


// TestDialog dialog

class TestDialog : public CDHtmlDialog
{
	DECLARE_DYNCREATE(TestDialog)

public:
	TestDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TestDialog();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_DIALOG_TEST, IDH = IDR_HTML_TESTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
