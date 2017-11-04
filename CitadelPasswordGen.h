// CitadelPasswordGen.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CCitadelPasswordGenApp:
// See CitadelPasswordGen.cpp for the implementation of this class
//

#include "Fortuna.h"

class CCitadelPasswordGenApp : public CWinApp
{
public:
	CCitadelPasswordGenApp();
	virtual ~CCitadelPasswordGenApp();

// Overrides
	public:
		virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

	private:
		CitadelSoftwareInc::Fortuna* m_pFortuna;
	public:
		CitadelSoftwareInc::Fortuna* GetFortuna() { return m_pFortuna; }
};

extern CCitadelPasswordGenApp theApp;