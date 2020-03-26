#include"centrlcache.h"
#include"PageCache.h"

Span* CentrlCache::GetOneSpan(size_t size)
{
	size_t index = SizeClass::Index(size);
	SpanList& spanlist = _spanLists[index];

	Span* it = spanlist.Begin();
	while (it != spanlist.End())
	{
		if (!it->_freeList.Empty())
		{
			return it;
		}
		else
		{
			it = it->_next;
		}
	}

	size_t numpage = SizeClass::NumMovePage(size);
	Span* span = PageCache::GetInsatnce().NewSpan(numpage);

	char* start = (char*)(span->_pagid << PAGE_SHIFT);
	char* end = start + (span->_pagesize << PAGE_SHIFT);

	while (start < end)
	{
		char* obj = start;
		start += size;

		span->_freeList.push(obj);
	}
	span->_objSize = size;
	spanlist.PushFront(span);

	return span;
}
size_t CentrlCache::FetchRangeObj(void*& start, void*& end, size_t num, size_t size)
{
	size_t index = SizeClass::Index(size);
	SpanList& spanlist = _spanLists[index];
	spanlist.Lock();

	Span* span = GetOneSpan(size);
	FreeList& freelist = span->_freeList;
	size_t realNum = freelist.PopRange(start, end , num);//返回实际挂上去的数量
	span->_usecount += realNum;

	spanlist.Unlock();

	return realNum;
}
//将还回来的内存，挂到对应的span上
void CentrlCache::ReleaseListToSpans(void* start,size_t size)
{
	size_t index = SizeClass::Index(size);
	SpanList& spanlist = _spanLists[index];
	spanlist.Lock();

	while (start)
	{
		void* next = NextObj(start);
		PAGE_ID id = (PAGE_ID)start >> PAGE_SHIFT;
		Span* span = PageCache::GetInsatnce().GetIdToSpan(id);
		span->_freeList.push(start);
		span->_usecount--;//因为是尾插，所以要保存start下一个的地址

		//当span的内存全部还回来之后，将span还给pagechche进行合并
		if (span->_usecount == 0)
		{
			size_t index = SizeClass::Index(span->_objSize);
			_spanLists[index].Erase(span);
			span->_freeList.Clear();

			PageCache::GetInsatnce().ReleaseSpanToPageCache(span);
		}

		start = next;
	}
	spanlist.Unlock();
}