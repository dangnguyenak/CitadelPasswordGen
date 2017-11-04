/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

#pragma once

#include "CMclEvent.h"

#include "FastList.h"

namespace CitadelSoftwareInc {

class Generator;
class PoolMgr;
class SourceMgr;
class Pool;
class Source;
class SeedFile;

	/*! \brief Top Level Object for the PRNG, users create and interact with this to generate PRN's
	*
	*/

class Fortuna
{
	public:
		enum FortunaErrors{
				eNoErrors,
				eCannotOpenSeedFile,
				eSeedFileNotSpecified,
				eErrorReadingSeedFile,
				eBadVersionNumberInSeedFile,
				eHMacFailed
		};

	public:
		Fortuna();
		~Fortuna();

		bool Stop();

		HANDLE GetShutdownEventHandle() const { return m_ShutdownEvent.GetHandle(); }

		bool StartThreads();

		Pool *GetPool(const int num);
		Source *GetSource(const int num);

		bool GetRandomChar( unsigned char&  uchar);
		bool GetRandomShort(unsigned short& ushort);
		bool GetRandomInt(  unsigned int&   uint);

		bool GetRandomBytes(unsigned char* pData, unsigned int numBytes);

		// set the seedfile name and the password used to encrypt the seed file
		bool SetSeedFile(const std::string& sFileName, std::string& sPassword);

		// get the encryption key for the seed file
		bool GetSeedFileKey(vecuc& vKey);

		void GetSeedFilename(std::string& sFileName) {sFileName = m_sSeedFileName; }

		bool ReadSeedFile(FortunaErrors& fError);


#ifdef FORTUNAMONITOR
	public:
#else
	private:
#endif
		Generator* GetGenerator() const {return m_pGenerator; }
		PoolMgr*   GetPoolMgr() const { return m_pPoolMgr; }
		SourceMgr* GetSourceMgr() const { return m_pSourceMgr; }

	private:
		Generator *m_pGenerator;
		PoolMgr   *m_pPoolMgr;
		SourceMgr *m_pSourceMgr;
		SeedFile  *m_pSeedFile;

		std::vector<Pool*> m_vPools;

		FastList<unsigned char> m_SeedFileKey;	// encryption key for seed file
		FastList<unsigned char> m_RandomKey;	// Random key used to encrypt the seedfilekey
		const std::string m_sSalt;				// salt used to prevent dictionary attacks

		std::string m_sSeedFileName;

		// manual reset event that becomes signalled when the object is shutting down
		CMclEvent m_ShutdownEvent;

		bool m_bWinsock;						// true-> Winsock is available

	friend Generator;
	friend PoolMgr;
	friend SourceMgr;
	friend SeedFile;

	private:
		bool CheckHMac(dequc& vState);

	private:
		Fortuna(const Fortuna& x);					// don't allow copy ctor
		Fortuna& operator=(const Fortuna& x);		// don't allow operator =

};

}	// end namespace CitadelSoftwareInc
