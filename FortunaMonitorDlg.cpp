// FortunaMonitorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CitadelPasswordGen.h"
#include "FortunaMonitorDlg.h"

#include "Generator.h"
#include "PoolMgr.h"
#include "Pool.h"
#include "SourceMgr.h"
#include "Source.h"
#include "zlib.h"

using namespace CitadelSoftwareInc;

// CFortunaMonitorDlg dialog

IMPLEMENT_DYNAMIC(CFortunaMonitorDlg, CDialog)

CFortunaMonitorDlg::CFortunaMonitorDlg(CitadelSoftwareInc::Fortuna* pFortuna, CWnd* pParent /*=NULL*/)
	: 
	CDialog(CFortunaMonitorDlg::IDD, pParent),
	m_pFortuna(pFortuna),
	m_nTimer(0),
	m_vCacheData(),
	m_vCacheDataResult(),
	m_dwLastCompression(0)
{
	m_vCacheData.reserve(32*1024);
	m_vCacheDataResult.reserve(64*1024);
}

CFortunaMonitorDlg::~CFortunaMonitorDlg()
{
	// note: do NOT delete the Fortuna object
}

void CFortunaMonitorDlg::OnOK()
{
	BOOL bStatus =	KillTimer(m_nTimer);
	m_nTimer = 0;
	return CDialog::OnOK();
}

void CFortunaMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_GENCOUNTER, m_GenCounter);
	DDX_Control(pDX, IDC_EDIT_GENKEY, m_GenKey);
	DDX_Control(pDX, IDC_EDIT_GENRESEEDCNT, m_GenReseedCount);
	DDX_Control(pDX, IDC_EDIT_GENLASTRESEED, m_GenLastReseed);
	DDX_Control(pDX, IDC_LIST_POOLS, m_ListPools);
	DDX_Control(pDX, IDC_EDIT_LIST_TBYTES, m_PoolTotalBytes);
	DDX_Control(pDX, IDC_EDIT_POOL_COMPACTCOUNT, m_PoolCompactCount);
	DDX_Control(pDX, IDC_EDIT_POOL_RAW, m_PoolRaw);
	DDX_Control(pDX, IDC_EDIT_POOL_HASHSTATE, m_PoolHash);
	DDX_Control(pDX, IDC_LIST_SOURCES, m_ListSources);
	DDX_Control(pDX, IDC_EDIT_TOTALBYTES, m_SourceTotalBytes);
	DDX_Control(pDX, IDC_EDIT_SOURCECYCLES, m_SourceCycles);
	DDX_Control(pDX, IDC_EDIT_SOURCE_PEEK, m_SourcePeek);
	DDX_Control(pDX, IDC_CHECK_SOURCE_COMPRESS, m_BtnCompress);
	DDX_Control(pDX, IDC_EDIT1, m_EditSrcCompression);
	DDX_Control(pDX, IDC_EDIT_SRC_CACHEBYTES, m_SrcCacheSize);
}


BEGIN_MESSAGE_MAP(CFortunaMonitorDlg, CDialog)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST_POOLS, OnLbnSelchangeListPools)
	ON_LBN_SELCHANGE(IDC_LIST_SOURCES, OnLbnSelchangeListSources)
	ON_BN_CLICKED(IDC_CHECK_SOURCE_COMPRESS, OnBnClickedCheckSourceCompress)
END_MESSAGE_MAP()


// CFortunaMonitorDlg message handlers
BOOL CFortunaMonitorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nTimer = SetTimer(1, 1000, 0);

	UpdateValues();

	return TRUE; // return TRUE  unless you set the focus to a control
}

void CFortunaMonitorDlg::OnTimer(UINT nTimerID)
{
	UpdateValues();

	CDialog::OnTimer(nTimerID);
}

void CFortunaMonitorDlg::UpdateValues()
{
	UpdateGeneratorValues();
	UpdatePoolValues();
	UpdateSourceValues();
}

void CFortunaMonitorDlg::UpdateGeneratorValues()
{
	Fortuna *pFortuna = theApp.GetFortuna();

	// Generator
	Generator *pGenerator = pFortuna->GetGenerator();
	// the counter - this is encrypted using the key to generate a stream of bytes
	std::string gen_counter = pGenerator->GetCounter128();
	m_GenCounter.SetWindowText(gen_counter.c_str());

	// the encryption key
	std::string gen_key = pGenerator->GetKeyAsString();
	m_GenKey.SetWindowText(gen_key.c_str());

	// reseed count
	unsigned int gen_reseedCount = pGenerator->GetReseedCount();
	char buffer[256] = {'\0'};
	sprintf(buffer,"%u",gen_reseedCount);
	m_GenReseedCount.SetWindowText(buffer);

	// time since last reseed
	DWORD dwLastReseedTime = pGenerator->GetLastReseedTime();
	if (dwLastReseedTime)
	{
		DWORD dwCurr = GetTickCount();
		int ms = dwCurr - dwLastReseedTime;
		int sec = ms / 1000;
		ms -= sec*1000;

		int min = sec / 60;
		sec -= min*60;

		int hr = min / 60;
		min -= hr*60;

		if (hr == 0 && min == 0)
			sprintf(buffer,"%d s %d ms", sec, ms);
		else if (hr == 0)
			sprintf(buffer,"%d m %d s %d ms", min, sec, ms);
		else
			sprintf(buffer,"%d hr %d m %d s", hr, min, sec);
	}
	else
	{
		sprintf(buffer,"-");
	}

	m_GenLastReseed.SetWindowText(buffer);
}

void CFortunaMonitorDlg::UpdateSourceValues()
{
	Fortuna *pFortuna = theApp.GetFortuna();
	SourceMgr *pSourceMgr = pFortuna->GetSourceMgr();

	int sizeSourcePools = m_ListSources.GetCount();
	if (sizeSourcePools == 0)
	{
		pSourceMgr->GetSources(m_Sources);

		std::string sName;
		size_t numSources = m_Sources.size();
		Source *pSource = NULL;
		for (size_t i=0; i<numSources; ++i)
		{
			pSource = m_Sources[i];
			sName = pSource->GetName();
			if (sName.empty())
				m_ListSources.AddString("?");
			else
				m_ListSources.AddString(sName.c_str());
		}
	}

	UpdateSingleSourceValues();
}

void CFortunaMonitorDlg::UpdatePoolValues()
{
	Fortuna *pFortuna = theApp.GetFortuna();
	PoolMgr *pPoolMgr = pFortuna->GetPoolMgr();

	int sizeListPools = m_ListPools.GetCount();
	if (sizeListPools == 0)
	{
		// populate the list box
		pPoolMgr->GetPools(m_Pools);
		size_t numPools = m_Pools.size();

		std::string sName;
		Pool *pPool = NULL;
		for (size_t i=0; i<numPools; ++i)
		{
			pPool = m_Pools[i];
			sName = pPool->GetName();
			m_ListPools.AddString(sName.c_str());
		}
	}

	UpdateSinglePoolValues();
}

void CFortunaMonitorDlg::UpdateSingleSourceValues()
{
	int cursel = m_ListSources.GetCurSel();
	if (cursel == LB_ERR)
	{
		cursel = 0;
		m_ListSources.SetCurSel(0);
	}

	if (cursel >= 0 && cursel < (int)m_Sources.size())
	{
		Source *pSource = m_Sources[cursel];

		char buffer[256] = {'\0'};
		unsigned int temp = pSource->GetTotalBytesData();
		sprintf(buffer,"%u", temp);
		m_SourceTotalBytes.SetWindowText(buffer);

		temp = pSource->GetNumberOfCycles();
		sprintf(buffer,"%u", temp);
		m_SourceCycles.SetWindowText(buffer);

		std::string sPeek = pSource->GetPeekString();
		if (sPeek.empty())
			m_SourcePeek.SetWindowText("");
		else
			m_SourcePeek.SetWindowText(sPeek.c_str());

		int checkState = pSource->GetCacheDataActive() ? BST_CHECKED : BST_UNCHECKED;
		m_BtnCompress.SetCheck(checkState);

		if (checkState == BST_CHECKED)
			UpdateCompressionRatio(pSource);
	}
}

void CFortunaMonitorDlg::UpdateSinglePoolValues()
{
	int cursel = m_ListPools.GetCurSel();
	if (cursel == LB_ERR)
	{
		cursel = 0;
		m_ListPools.SetCurSel(0);
	}

	if (cursel >= 0 && cursel < 32)
	{
		Pool *pPool = m_Pools[cursel];
		unsigned int totalBytes = pPool->GetTotalBytes();
		char buffer[256] = {'\0'};
		sprintf(buffer,"%u", totalBytes);
		m_PoolTotalBytes.SetWindowText(buffer);

		unsigned int compactPoolCount = pPool->GetCompactPoolCount();
		sprintf(buffer,"%u", compactPoolCount);
		m_PoolCompactCount.SetWindowText(buffer);

		std::string rawPool = pPool->GetRawPool();
		if (rawPool.empty())
			m_PoolRaw.SetWindowText("");
		else
			m_PoolRaw.SetWindowText(rawPool.c_str());

		std::string hashPool = pPool->GetHashedPool();
		if (hashPool.empty())
			m_PoolHash.SetWindowText("");
		else
			m_PoolHash.SetWindowText(hashPool.c_str());
	}
}

// a new pool was selected in the pool list box
void CFortunaMonitorDlg::OnLbnSelchangeListPools()
{
	UpdateSinglePoolValues();	
}


void CFortunaMonitorDlg::OnLbnSelchangeListSources()
{
	m_EditSrcCompression.SetWindowText("");
	m_SrcCacheSize.SetWindowText("");
	m_dwLastCompression = 0;
	UpdateSingleSourceValues();
}

void CFortunaMonitorDlg::OnBnClickedCheckSourceCompress()
{
	int cursel = m_ListSources.GetCurSel();
	if (cursel == LB_ERR || cursel < 0 || cursel > (int)m_Sources.size())
		return;

	Source *pSource = m_Sources[cursel];

	int check = m_BtnCompress.GetCheck();
	if (check == BST_CHECKED)
		pSource->SetCacheDataActive(true);
	else
		pSource->SetCacheDataActive(false);
}

void CFortunaMonitorDlg::UpdateCompressionRatio(Source* pSource)
{
	const int numSecs = 3;		// update the compression value every n seconds
	DWORD dwCurrTime = GetTickCount();

	if (dwCurrTime - m_dwLastCompression < numSecs * 1000)
		return;

	pSource->GetCacheData(m_vCacheData);
	if (m_vCacheData.empty())
		return;

	m_vCacheDataResult.resize(m_vCacheData.size() * 2);
	uLongf destLen = (uLongf)m_vCacheDataResult.size();

	Bytef *dest = &m_vCacheDataResult[0];
	const Bytef *source = &m_vCacheData[0];
	uLong sourceLen = (uLongf)m_vCacheData.size();

//	int error = compress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen); 
	int error = compress (dest, &destLen, source, sourceLen); 

	if (error == Z_OK)
	{
		double ratio = (1. - double(destLen) / double(sourceLen)) * 100;
		char buffer[256] = {'\0'};
		sprintf(buffer,"%.1f %%", ratio);
		m_EditSrcCompression.SetWindowText(buffer);

		sprintf(buffer,"%u", m_vCacheData.size());
		m_SrcCacheSize.SetWindowText(buffer);
	}

	m_dwLastCompression = GetTickCount();
}
