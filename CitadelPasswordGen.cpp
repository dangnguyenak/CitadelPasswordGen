// CitadelPasswordGen.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "CitadelPasswordGenDlg.h"

using namespace CitadelSoftwareInc;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCitadelPasswordGenApp

BEGIN_MESSAGE_MAP(CCitadelPasswordGenApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCitadelPasswordGenApp construction

CCitadelPasswordGenApp::CCitadelPasswordGenApp()
	:
	m_pFortuna(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CCitadelPasswordGenApp::~CCitadelPasswordGenApp()
{

}

// The one and only CCitadelPasswordGenApp object

CCitadelPasswordGenApp theApp;


// CCitadelPasswordGenApp initialization

BOOL CCitadelPasswordGenApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	m_pFortuna = new CitadelSoftwareInc::Fortuna();

	std::string sFileName("FortunaCSI.bin");

	// this password should be hidden somewhere...
	std::string sPassword("abc&*(ui^5:>kMMJ");
	bool bStatus = m_pFortuna->SetSeedFile(sFileName, sPassword);
	if (!bStatus)
	{
		AfxMessageBox("Set Seed File Failed", MB_OK);
		delete m_pFortuna;
		m_pFortuna = NULL;
		return FALSE;
	}

	CitadelSoftwareInc::Fortuna::FortunaErrors fError = CitadelSoftwareInc::Fortuna::eNoErrors;
	bStatus = m_pFortuna->ReadSeedFile(fError);

	m_pFortuna->StartThreads();

	CCitadelPasswordGenDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// shut down the threads, write out the seed file
	m_pFortuna->Stop();

	delete m_pFortuna;
	m_pFortuna = NULL;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
