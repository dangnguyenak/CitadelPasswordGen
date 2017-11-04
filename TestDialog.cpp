// TestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "TestDialog.h"


// TestDialog dialog

IMPLEMENT_DYNCREATE(TestDialog, CDHtmlDialog)

TestDialog::TestDialog(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(TestDialog::IDD, TestDialog::IDH, pParent)
{
}

TestDialog::~TestDialog()
{
}

void TestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL TestDialog::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(TestDialog, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(TestDialog)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// TestDialog message handlers

HRESULT TestDialog::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;  // return TRUE  unless you set the focus to a control
}

HRESULT TestDialog::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // return TRUE  unless you set the focus to a control
}
