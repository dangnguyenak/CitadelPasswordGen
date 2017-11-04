#pragma once


// CitadelPasswordDlg dialog

class CitadelPasswordDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CitadelPasswordDlg)

public:
	CitadelPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CitadelPasswordDlg();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_CITADELPASSWORDGEN_DIALOG, IDH = IDR_HTML_ITADELPASSWORDDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
