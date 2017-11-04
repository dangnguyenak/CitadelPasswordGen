// TestDialog2.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "TestDialog2.h"


// TestDialog2 dialog

IMPLEMENT_DYNAMIC(TestDialog2, CDialog)
TestDialog2::TestDialog2(CWnd* pParent /*=NULL*/)
	: CDialog(TestDialog2::IDD, pParent)
{
}

TestDialog2::~TestDialog2()
{
}

void TestDialog2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, Testbutton);
}


BEGIN_MESSAGE_MAP(TestDialog2, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// TestDialog2 message handlers

void TestDialog2::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}
