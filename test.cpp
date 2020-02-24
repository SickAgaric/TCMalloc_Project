#pragma once

#include"common.h"
#include"threadcache.h"
#include<vector>

void UnitTreadCache()
{
	Threadcache tc;
	vector<void*> v;
	for (size_t i = 0; i < 21; ++i)
	{
		v.push_back(tc.Allocate(7));
	}
	
	for (size_t i = 0; i < v.size(); ++i)
	{
		printf("[%d]->%p\n", i, v[i]);
	}

	for (auto ptr : v)
	{
		tc.Deallocate(ptr, 7);
	}
}

int main()
{
	UnitTreadCache();
	system("pause");
	return 0;
}