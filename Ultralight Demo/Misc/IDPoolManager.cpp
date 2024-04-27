#include <PCH.h>
#include "IDPoolManager.h"

template<typename T>
IDPoolManager<T>::IDPoolManager(T startingId)
{
	m_NextHighestId = startingId;
}

template<typename T>
T IDPoolManager<T>::GetNextId()
{
	if (m_StoredIDs.size() > 0)
	{
		int32_t id = m_StoredIDs.back();
		m_StoredIDs.pop_back();
		return id;
	}
	int32_t id = m_NextHighestId;
	m_NextHighestId += 1;
	return id;
}

template<typename T>
void IDPoolManager<T>::StoreId(T id)
{
	m_StoredIDs.push_back(id);
}

template IDPoolManager<int32_t>;
template IDPoolManager<uint32_t>;