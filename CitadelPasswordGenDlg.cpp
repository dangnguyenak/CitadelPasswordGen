// CitadelPasswordGenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "CitadelPasswordGenDlg.h"
#include "FortunaMonitorDlg.h"

using namespace CitadelSoftwareInc;
#include "Fortuna.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCitadelPasswordGenDlg dialog



CCitadelPasswordGenDlg::CCitadelPasswordGenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCitadelPasswordGenDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCitadelPasswordGenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_az, m_azlower);
	DDX_Control(pDX, IDC_CHECK_AZCaps, m_azupper);
	DDX_Control(pDX, IDC_CHECK_09, m_digits);
	DDX_Control(pDX, IDC_CHECK_Punct, m_punct);
	DDX_Control(pDX, IDC_EDIT_SIZE, m_size);
	DDX_Control(pDX, IDC_BTN_GEN, m_generate);
	DDX_Control(pDX, IDC_EDIT_PWD, m_password);
	DDX_Control(pDX, IDC_SPIN1, m_spinner);
	DDX_Control(pDX, IDC_BUTTON_MONITOR, m_MonitorButton);
}

BEGIN_MESSAGE_MAP(CCitadelPasswordGenDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT_SIZE, OnEnChangeEditSize)
	ON_BN_CLICKED(IDC_BTN_GEN, OnBnClickedBtnGen)
	ON_BN_CLICKED(IDC_CHECK_az, OnBnClickedCheckaz)
	ON_BN_CLICKED(IDC_CHECK_AZCaps, OnBnClickedCheckAzcaps)
	ON_BN_CLICKED(IDC_CHECK_09, OnBnClickedCheck09)
	ON_BN_CLICKED(IDC_CHECK_Punct, OnBnClickedCheckPunct)
	ON_BN_CLICKED(IDC_BUTTON_MONITOR, OnBnClickedButtonMonitor)
END_MESSAGE_MAP()


// CCitadelPasswordGenDlg message handlers

BOOL CCitadelPasswordGenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_digits.SetCheck(BST_CHECKED);

	SetGenState();

	m_spinner.SetBuddy(&m_size);
	m_spinner.SetRange(1,999);
	m_spinner.SetPos(8);

	m_size.SetLimitText(3);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCitadelPasswordGenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCitadelPasswordGenDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCitadelPasswordGenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCitadelPasswordGenDlg::OnEnChangeEditSize()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CCitadelPasswordGenDlg::OnBnClickedBtnGen()
{
	bool bStatus = OneCheckbox();
	if (!bStatus)
	{
		AfxMessageBox("Please check at least one check box");
		return;
	}

	CString chars;

	if (m_digits.GetCheck())
		chars += CString("01");

	Fortuna *pFortuna = theApp.GetFortuna();

	unsigned int numChars = 8;

	CString csNumChars;
	m_size.GetWindowText(csNumChars);

	if (!csNumChars.IsEmpty())
		numChars = atoi(csNumChars);

	if (numChars < 1)
	{
		numChars = 8;
		m_size.SetWindowText("8");
	}

	unsigned int i=0;
	unsigned int temp=0;
	unsigned int offset = 0;
	unsigned int charCount = chars.GetLength();

	// shuffle the chars before extracting them
	std::vector<unsigned char> vBuffer;
	vBuffer.resize(charCount*2);
	bStatus = pFortuna->GetRandomBytes(&vBuffer[0], charCount*2);
	unsigned char uc1;
	unsigned char uc2;
	char tempchar;
	for (i=1; i<charCount; ++i)
	{
		uc1 = vBuffer[i] % charCount;
		uc2 = vBuffer[i+charCount] % charCount;
		
		if (uc1 != uc2)
		{
			tempchar = chars.GetAt(uc1);
			chars.SetAt(uc1, chars.GetAt(uc2));
			chars.SetAt(uc2, tempchar);
		}
	}

	//
	CStdioFile writeToFile;
	CFileException fileException;
	CString strFilePath = _T("data\\password.dat");

	CString password;

	// generate some random bytes, throw away those that are greater than or equal to the number of chars
	// so that each char has the same probability of being selected

	const unsigned int maxBytes = (( (numChars*2) / 16) + 1)*16;		// generate an even number of 16 byte blocks
	vBuffer.resize(maxBytes);
	bStatus = pFortuna->GetRandomBytes(&vBuffer[0], maxBytes);

	bool bFound = false;
	bStatus = true;
	unsigned int jByte = 0;
	for (i=0; i<numChars && bStatus; ++i)
	{
		bFound = false;
		while(!bFound && bStatus)
		{
			if (jByte >= maxBytes)
			{
				bStatus = pFortuna->GetRandomBytes(&vBuffer[0], maxBytes);
				jByte = 0;
			}

			uc1 = vBuffer[jByte++];
			if (uc1 < charCount)
			{
				password += chars.GetAt(uc1);
				bFound = true;
			}
		}
	}

	if (bStatus)
	{
		m_password.SetWindowText(password);
		if (writeToFile.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate), &fileException)
		{
			writeToFile.SeekToEnd();
			writeToFile.WriteString(_T(password + "\n"));
		}
		else
		{
			CString strErrorMsg;
			strErrorMsg.Format(_T("Can't open file %s , error : %u"), strFilePath, fileException.m_cause);
			AfxMessageBox(strErrorMsg);
		}
	}
	else
	{
		AfxMessageBox("Insufficient Entropy to generate secure random numbers", MB_OK);
	}
	writeToFile.Close();
}

void CCitadelPasswordGenDlg::OnBnClickedCheckaz()
{
	SetGenState();
}

void CCitadelPasswordGenDlg::OnBnClickedCheckAzcaps()
{
	SetGenState();
}

void CCitadelPasswordGenDlg::OnBnClickedCheck09()
{
	SetGenState();
}

void CCitadelPasswordGenDlg::OnBnClickedCheckPunct()
{
	SetGenState();
}


void CCitadelPasswordGenDlg::SetGenState()
{
	bool bStatus = OneCheckbox();

	if (bStatus)
	{
		m_generate.EnableWindow(TRUE);
	}
	else
	{
		m_generate.EnableWindow(FALSE);
	}

}

//! ensure that at least one checkbox is checked
bool CCitadelPasswordGenDlg::OneCheckbox()
{
	int i = m_digits.GetCheck() == BST_CHECKED;

	return i ? true : false;
}





void CCitadelPasswordGenDlg::OnBnClickedButtonMonitor()
{
	Fortuna* pFortuna = theApp.GetFortuna();

	// TODO: Add your control notification handler code here
	CFortunaMonitorDlg fm(pFortuna);

	fm.DoModal();

}
