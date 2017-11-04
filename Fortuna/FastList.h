/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// This is a singly linked list implementation which is designed to be fast -
// Note that memory is not released unless specifically requested
#pragma once

#include "CMclCritSec.h"

namespace CitadelSoftwareInc {

bool TestFastList();		// method to test out the fast list

	/*! \brief Fast singly linked list used to hold vector contents in non continguous memory, 
	*          also contains erase which writes over bytes of each value with rand() % 256
	*
	*/

template <typename T>
class FastList
{
private:

	template <typename T>
	struct ListItem
	{
		ListItem(const T& t)
			:
		m_t(t),
		m_pNext(NULL)
		{}

	ListItem<T>* m_pNext;	// next item in list
	T m_t;					// the data
	};

public:
	FastList()
		:
	m_pHead(NULL),
	m_pLast(NULL),
	m_size(0)
	{}

	virtual ~FastList()
	{
		ListItem<T>* p = m_pHead;
		while (p)
		{
			ListItem<T>* pNext = p->m_pNext;
			delete p;
			p = pNext;
		}
	}

	unsigned int Add(const T& t)
	{
		m_critsec.Enter();
		if (!m_pHead)
		{
			m_pHead = new ListItem<T>(t);
			m_pLast = m_pHead;
			m_size = 1;
		}
		else if(m_pLast)
		{
			if (m_pLast->m_pNext)
			{
				m_pLast->m_pNext->m_t = t;
				m_pLast = m_pLast->m_pNext;
			}
			else
			{
				m_pLast->m_pNext = new ListItem<T>(t);
				m_pLast = m_pLast->m_pNext;
			}
			++m_size;
		}
		else
		{
			m_pLast = m_pHead;
			m_pLast->m_t = t;
			++m_size;
		}

		m_critsec.Leave();

		return m_size;
	}
	
	void Reset()
	{
		m_critsec.Enter();

		if (m_pLast)
		{
			ListItem<T> *p = m_pHead;
			while(p)
			{
				p->m_t = rand() % 256;		// overwrite the existing data with something random looking
				if (p == m_pLast)
					break;
				p = p->m_pNext;
			}
			m_pLast = NULL;
			m_size = 0;
		}

		m_critsec.Leave();
	}

	int ExtractAndErase(std::vector<T>& vec, const unsigned int modnum=256)
	{
		vec.clear();
		if (m_size == 0)
			return 0;

		m_critsec.Enter();

		unsigned int i=0;
		vec.resize(m_size);
		ListItem<T>* p = m_pHead;
		while (p)
		{
			assert(i<m_size);
			vec[i++] = p->m_t;
			p->m_t = rand() % modnum;		// overwrite the existing data with something random looking
			if (p == m_pLast)
				break;

			p = p->m_pNext;
		}

		assert(i == m_size);
		int retval = m_size;
		m_size = 0;
		m_pLast = NULL;

		m_critsec.Leave();

		return retval;
	}

	int Extract(std::vector<T>& vec)
	{
		vec.clear();

		if (m_size == 0)
			return 0;

		m_critsec.Enter();

		vec.resize(m_size);
		unsigned int i=0;
		ListItem<T>* p = m_pHead;
		while(p)
		{
			assert(i<m_size);
			vec[i++] = p->m_t;
			if (p == m_pLast)
				break;
			p = p->m_pNext;
		}

		assert(i == m_size);

		m_critsec.Leave();

		return m_size;
	}

	// add in the address of each list item to provide some data specific to this data structure
	int AddRandomData(vecuc& vData)
	{
		int count=0;
		unsigned char uc=0;

		m_critsec.Enter();

		ListItem<T>* p = m_pHead;
		while(p)
		{
			const unsigned char* pData = (unsigned char*)p;
			for (int i=0; i<sizeof(ListItem<T>*); ++i)
			{
				uc = pData[i];
				if (uc)
				{
					vData.push_back(uc);
					++count;
				}
			}

			p = p->m_pNext;
		}

		m_critsec.Leave();

		return count;
	}

	unsigned int Size() const
	{
		unsigned int ui = 0;

		m_critsec.Enter();
		ui = m_size;
		m_critsec.Leave();

		return ui;
	}

private:
		ListItem<T>* m_pHead;		// head of physical list; head can be non null but the list is empty if last is null
		ListItem<T>* m_pLast;		// last item in the list

		unsigned int m_size;		// actual size of active list
		mutable CMclCritSec m_critsec;		// critical section to protect access, mutable == ok to change in const method
};

}	// end namespace CitadelSoftwareInc