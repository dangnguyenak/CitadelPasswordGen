#include "StdAfx.h"
#include "DelayedWriteFile.h"

namespace CitadelSoftwareInc {

DelayedWriteFile::DelayedWriteFile(std::string& sFileName, 
								   size_t size,
    		   					   DWORD  dwWriteFreqSec)
	:
	m_sFileName(sFileName),
	m_pFile(NULL),
	m_CurrentBytes(0),
	m_TotalBytes(0),
	m_WriteFreqSec(dwWriteFreqSec),
	m_NextWriteTime(0)
{
	m_buffer.reserve(size);

	if (m_WriteFreqSec)
	{
		m_NextWriteTime = GetTickCount() + m_WriteFreqSec*1000;	// next write time in Ticks
	}
}

DelayedWriteFile::~DelayedWriteFile(void)
{
	Write();
}

void DelayedWriteFile::Write()
{
	if (!m_buffer.empty())
	{
		m_pFile = fopen(m_sFileName.c_str(), "a+b");
		fwrite(&m_buffer[0], sizeof(unsigned char), m_buffer.size(), m_pFile);
		fclose(m_pFile);
		m_buffer.resize(0);
		m_pFile = NULL;
		m_CurrentBytes = 0;

		if (m_WriteFreqSec)
		{
			m_NextWriteTime = GetTickCount() + m_WriteFreqSec*1000;	// next write time in Ticks
		}
	}
}

void DelayedWriteFile::Add(unsigned char uc)
{
	++m_TotalBytes;
	size_t bufSize = m_buffer.size();
	size_t bufCapacity = m_buffer.capacity();
	if (bufSize == bufCapacity)
	{
		Write();
	}
	else if (m_WriteFreqSec)
	{
		DWORD dwCurrTime = GetTickCount();
		if (dwCurrTime >= m_NextWriteTime)
			Write();
	}

	m_buffer.push_back(uc);

}


}	// end namespace CitadelSoftwareInc