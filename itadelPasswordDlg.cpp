// itadelPasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "itadelPasswordDlg.h"


// CitadelPasswordDlg dialog

IMPLEMENT_DYNCREATE(CitadelPasswordDlg, CDHtmlDialog)

CitadelPasswordDlg::CitadelPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CitadelPasswordDlg::IDD, CitadelPasswordDlg::IDH, pParent)
{
}

CitadelPasswordDlg::~CitadelPasswordDlg()
{
}

void CitadelPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CitadelPasswordDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CitadelPasswordDlg, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CitadelPasswordDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CitadelPasswordDlg message handlers

HRESULT CitadelPasswordDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;  // return TRUE  unless you set the focus to a control
}

HRESULT CitadelPasswordDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // return TRUE  unless you set the focus to a control
}
