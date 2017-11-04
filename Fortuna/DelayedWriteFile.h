#pragma once

namespace CitadelSoftwareInc {

class DelayedWriteFile
{
public:
	DelayedWriteFile(std::string& sFileName, 
					 size_t size=1024*1024,
					 DWORD  dwWriteFreqSec=3600);

	virtual ~DelayedWriteFile(void);

	void Add(unsigned char uc);

	void Write();
	
private:
	std::string m_sFileName;
	std::vector<unsigned char> m_buffer;
	FILE* m_pFile;
	size_t m_TotalBytes;				// total bytes added to the file
	size_t m_CurrentBytes;				// current number of bytes in the buffer
	DWORD   m_WriteFreqSec;				// write frequency in seconds, 0 to disable
	DWORD   m_NextWriteTime;			// next write time in ms

private:
	DelayedWriteFile();
	DelayedWriteFile(const DelayedWriteFile& x);
	DelayedWriteFile& operator=(const DelayedWriteFile& x);
};

}	// end namespace CitadelSoftwareInc