#pragma once

#include"common.h"
#include"PageCache.h"
//约定num为个数，size为大小
class CentrlCache
{
public:
	// 从中心缓存获取一定数量的对象给thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t num, size_t size);
	// 将一定数量的对象释放到span跨度
	void ReleaseListToSpans(void* start,size_t size);
	// 从spalist 或者 page cache获取一个span
	Span* GetOneSpan(size_t size);

	static CentrlCache& GetInsatnce()
	{
		static CentrlCache in;
		return in;
	}
private:
	SpanList _spanLists[NFRER_LIST];

	CentrlCache()
	{}

	CentrlCache(const CentrlCache&) = delete;
};

//改为单例模式，达到全局只有一个__centrlCache
//static CentrlCache __centrlcache;//多个线程共享静态区