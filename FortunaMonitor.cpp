// FortunaMonitor.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "FortunaMonitor.h"


// FortunaMonitor dialog

IMPLEMENT_DYNCREATE(FortunaMonitor, CDHtmlDialog)

FortunaMonitor::FortunaMonitor(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(FortunaMonitor::IDD, FortunaMonitor::IDH, pParent)
{
}

FortunaMonitor::~FortunaMonitor()
{
}

void FortunaMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL FortunaMonitor::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(FortunaMonitor, CDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(FortunaMonitor)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// FortunaMonitor message handlers

HRESULT FortunaMonitor::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;  // return TRUE  unless you set the focus to a control
}

HRESULT FortunaMonitor::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // return TRUE  unless you set the focus to a control
}
