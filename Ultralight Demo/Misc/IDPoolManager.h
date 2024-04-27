#pragma once
#include <PCH.h>

//This class allows for getting unique id's/reusing old id's to prevent overflow. ID's start at 0.
//I'm sure there are better ways to do this, but this was a easy/lazy solution that works.
template<typename T>
class IDPoolManager
{
public:
	IDPoolManager(T startingId = 0);
	T GetNextId();
	void StoreId(T id);
private:
	std::vector<T> m_StoredIDs;
	T m_NextHighestId = 0;
};