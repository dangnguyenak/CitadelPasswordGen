/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

/*!  \file NCColln.h
*    Templated collection class to store data non contiguously, uses a map internally.
*
*
*/


#pragma once
#pragma warning(disable: 4311) //  warning C4311: 'type cast' : pointer truncation from 'CitadelSoftwareInc::NCColln<T> *const ' to 'unsigned long'

#include "FastList.h"

namespace CitadelSoftwareInc {

/*! \brief Templated collection class to store data non continguously.  Uses a map internally.
*
*/
template<typename T>
class NCColln
{
	public:

		// nested class used to hold items in the linked list
		template<typename T>
		class ListItem
		{
		public:
			ListItem()
				:
			m_t(0),
			m_bInUse(false)
			{}

			ListItem(const T& t, const bool b)
				:
			m_t(t),
			m_bInUse(b)
			{}

			T m_t;
			bool m_bInUse;		// used to determine if the value is in use
		};
	public:
		NCColln()
			:
		m_map(),
		m_curOffset(0),
		m_type(eVector)
		{
			unsigned long l = (unsigned long)(this);
			m_curOffset = l % 256;

			if (m_type == eVector)
				m_vector.reserve(1024);
		}

		int Add(const T& t)
		{
			int size=0;
			if (m_type == eMap)
			{
				unsigned long l = (unsigned long)(&t);
				m_curOffset += (l % 256) + 1;
				m_map[m_curOffset] = t;
				size = (int)m_map.size();
			}
			else if (m_type == eVector)
			{
				m_vector.push_back(t);
				size = (int)m_vector.size();
			}
			else if(m_type == eList)
			{
				// look through the list for the first available listitem
				std::list<ListItem<T> >::iterator iter		= m_list.begin();
				std::list<ListItem<T> >::iterator iter_end	= m_list.end();

				size=1;
				bool bDone = false;
				for (; iter != iter_end; ++iter, ++size)
				{
					ListItem<T>& li = *iter;
					if (li.m_bInUse == false)
					{
						li.m_t = t;
						li.m_bInUse = true;
						bDone = true;
						break;
					}
				}

				if (!bDone)
				{
					m_list.push_back( ListItem<T>(t, true));
					++size;
				}
			}
			else if(m_type == eFastList)
			{
				m_fastlist.Add(t);
			}
			else
			{
				assert(0);
			}

			return size;
		}

		int Size() const 
			{
				if(m_type == eMap)
					return (int)m_map.size(); 
				else if(m_type == eVector)
					return (int)m_vector.size();
				else if(m_type == eList)
				{
					std::list<ListItem<T> >::const_iterator iter		= m_list.begin();
					std::list<ListItem<T> >::const_iterator iter_end	= m_list.end();

					int size=1;
					for (; iter != iter_end; ++iter, ++size)
					{
						const ListItem<T>& li = *iter;
						if (li.m_bInUse == false)
						{
							break;
						}
					}
					return size;
				}
				else if(m_type == eFastList)
				{
					return m_fastlist.Size();
				}

				assert(0);
				return 0;
			}

		void Reset()
		{
			if (m_type == eMap)
			{
				std::map<unsigned long, T>::iterator iter = m_map.begin();
				std::map<unsigned long, T>::iterator endIter = m_map.end();
				for (int i=0 ; iter != endIter; ++iter, ++i)
				{
					// TODO - replace 256 with the maximum value of T
					(*iter).second = (T)(rand() % 256);		// overwrite with something random looking
				}

				m_map.clear();
				m_curOffset = 0;
			}
			else if (m_type == eVector)
			{
				size_t count = m_vector.size();
				for (size_t i=0; i<count; ++i)
				{
					m_vector[i] = (T)(rand() % 256);
				}
				m_vector.clear();
			}
			else if(m_type == eList)
			{
				std::list<ListItem<T> >::iterator iter		= m_list.begin();
				std::list<ListItem<T> >::iterator iter_end	= m_list.end();

				for (; iter != iter_end; ++iter)
				{
					ListItem<T>& li = *iter;
					if (li.m_bInUse == false)
						break;
					else
					{
						li.m_t = (T)(rand() % 256);
						li.m_bInUse = false;
					}
				}
			}
			else if(m_type == eFastList)
			{
				m_fastlist.Reset();
			}
			else
			{
				assert(0);
			}
		}

		// Extract the data from the collection, replace with something random looking, and then release the memory
		int ExtractAndErase(std::vector<T>& vec, const unsigned int modnum)
		{
			size_t size = (size_t)Size();
			if (size == 0)
			{
				vec.clear();
				return 0;
			}

			vec.resize(size);

			if (m_type == eMap)
			{
				std::map<unsigned long, T>::iterator iter = m_map.begin();
				std::map<unsigned long, T>::iterator endIter = m_map.end();
				for (int i=0 ; iter != endIter; ++iter, ++i)
				{
					vec[i] = (*iter).second;					// extract the data
					(*iter).second = (T)(rand() % modnum);		// overwrite with something random looking
				}
			}
			else if(m_type == eVector)
			{
				size_t size = m_vector.size();
				for (size_t i=0; i<size; ++i)
				{
					vec[i] = m_vector[i];
					m_vector[i] = (T)(rand() % modnum);
				}
			}
			else if(m_type == eList)
			{
				std::list<ListItem<T> >::iterator iter		= m_list.begin();
				std::list<ListItem<T> >::iterator iter_end	= m_list.end();

				int i=0;
				for (; iter != iter_end; ++iter,++i)
				{
					ListItem<T>& li = *iter;
					if (li.m_bInUse == false)
						break;
					else
					{
						vec[i] = li.m_t;
						li.m_t = (T)(rand() % 256);
						li.m_bInUse = false;
					}
				}
			}
			else if(m_type == eFastList)
			{
				return m_fastlist.ExtractAndErase(vec);
			}
			else
			{
				assert(0);
			}

			Reset();

			return (int)size;
		}


		int Extract(std::vector<T>& vec)
		{
			int size = Size();
			if (size == 0)
			{
				vec.clear();
				return 0;
			}

			vec.resize(size);

			if (m_type == eMap)
			{
				std::map<unsigned long, T>::iterator iter = m_map.begin();
				std::map<unsigned long, T>::iterator endIter = m_map.end();
				for (int i=0 ; iter != endIter; ++iter, ++i)
				{
					vec[i] = (*iter).second;
				}
			}
			else if (m_type == eVector)
			{
				for (int i=0; i<size; ++i)
				{
					vec[i] = m_vector[i];
				}
			}
			else if(m_type == eList)
			{
				std::list<ListItem<T> >::iterator iter		= m_list.begin();
				std::list<ListItem<T> >::iterator iter_end	= m_list.end();

				int i=0;
				for (; iter != iter_end; ++iter,++i)
				{
					ListItem<T>& li = *iter;
					if (li.m_bInUse == false)
						break;
					else
					{
						vec[i] = li.m_t;
					}
				}
			}
			else if(m_type == eFastList)
			{
				return m_fastlist.Extract(vec);
			}
			else
			{
				assert(0);
			}

			return size;
		}

	private:
		enum CollnType {
			eMap,
			eVector,
			eList,
			eFastList};

		std::map<unsigned long, T> m_map;		// map to hold the data
		unsigned int m_curOffset;

		std::vector<T> m_vector;

		std::list<ListItem<T> > m_list;
		FastList<T> m_fastlist;

		CollnType m_type;
};

}	// end namespace CitadelSoftwareInc