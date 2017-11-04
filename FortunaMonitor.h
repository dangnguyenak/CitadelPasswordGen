#pragma once


// FortunaMonitor dialog

class FortunaMonitor : public CDHtmlDialog
{
	DECLARE_DYNCREATE(FortunaMonitor)

public:
	FortunaMonitor(CWnd* pParent = NULL);   // standard constructor
	virtual ~FortunaMonitor();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_FORTUNA, IDH = IDR_HTML_FORTUNAMONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
