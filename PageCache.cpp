#include"PageCache.h"

Span* PageCache::NewSpan(size_t numpage)
{
	if (!_spanLists[numpage].Empty())
	{
		Span* span = _spanLists[numpage].Begin();
		_spanLists[numpage].PopFront();
		return span;
	}

	for (size_t i = numpage + 1; i < MAX_PAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			Span* span = _spanLists[i].Begin();
			_spanLists[i].PopFront();

			Span* splitpage = new Span;
			splitpage->_pagid = span->_pagid + span->_pagesize - numpage;//找到尾，然后切
			splitpage->_pagesize = numpage;
			for (PAGE_ID i = 0; i < numpage; ++i)
			{
				_idSpanMap[splitpage->_pagid + i] = splitpage;
			}
			//splitpage->_pagid = span->_pagid + numpage;//
			//splitpage->_pagesize = span->_pagesize - numpage;

			//span->_pagesize = numpage;

			//_spanLists[splitpage->_pagesize].PushFront(splitpage);
			span->_pagesize -= numpage;

			_spanLists[span->_pagesize].PushFront(span);

			return splitpage;
		}
	}

	void* ptr = SystemAlloc(MAX_PAGES - 1);//向系统申请128页

	Span* maxspan = new Span;
	maxspan->_pagid = (PAGE_ID)(ptr) >> PAGE_SHIFT;//此处将ptr错写为maxspan
	maxspan->_pagesize = MAX_PAGES - 1;

	for (PAGE_ID i = 0; i < maxspan->_pagesize; ++i)
	{
		_idSpanMap[maxspan->_pagid + i] = maxspan; //从起始页，一直映射到+=128后
	}

	_spanLists[maxspan->_pagesize].PushFront(maxspan);

	return NewSpan(numpage);//再一次递归这个函数
}

void PageCache::ReleaseSpanToPageCache(Span* span)
 {
	// 向前合并
	while (1)
	{
		PAGE_ID prevPageId = span->_pagid - 1;
		auto pit = _idSpanMap.find(prevPageId);
		// 前面的页不存在
		if (pit == _idSpanMap.end())
		{
			break;
		}

		// 说明前一个也还在使用中，不能合并
		Span* prevSpan = pit->second;
		if (prevSpan->_usecount != 0)
		{
			break;
		}

		// 合并
		span->_pagid = prevSpan->_pagid;
		span->_pagesize += prevSpan->_pagesize;
		for (PAGE_ID i = 0; i < prevSpan->_pagesize; ++i)
		{
			_idSpanMap[prevSpan->_pagid + i] = span;
		}


		_spanLists[prevSpan->_pagesize].Erase(prevSpan);
		delete prevSpan;
	}


	// 向后合并
	while (1)
	{
		PAGE_ID nextPageId = span->_pagid + span->_pagesize;
		auto nextIt = _idSpanMap.find(nextPageId);
		if (nextIt == _idSpanMap.end())
		{
			break;
		}

		Span* nextSpan = nextIt->second;
		if (nextSpan->_usecount != 0)
		{
			break;
		}

		span->_pagesize += nextSpan->_pagesize;
		for (PAGE_ID i = 0; i < nextSpan->_pagesize; ++i)
		{
			_idSpanMap[nextSpan->_pagid + i] = span;
		}

		_spanLists[nextSpan->_pagesize].Erase(nextSpan);
		delete nextSpan;
	}

	_spanLists[span->_pagesize].PushFront(span);
	////向前合并
	//while (1)
	//{
	//	PAGE_ID prevPageid = span->_pagid - 1;
	//	auto it = _idSpanMap.find(prevPageid);
	//	//情况1前面的页不存在
	//	if (it == _idSpanMap.end())
	//	{
	//		break;
	//	}
	//	//it->second取到的是结构，所以返回的是Span*
	//	Span* prevSpan = it->second;
	//	if (prevSpan->_usecount != 0)//情况2前一个页正在使用，不可合并
	//	{
	//		break;
	//	}

	//	span->_pagid += prevSpan->_pagesize;//与前面的页合并,页号更改为前面的页号
	//	span->_pagesize += prevSpan->_pagesize;//页的大小增加

	//	for (PAGE_ID i = 0; i < prevSpan->_pagesize; i++)
	//	{
	//		_idSpanMap[prevSpan->_pagid + i] = span;
	//	}

	//	_spanLists[prevSpan->_pagesize].Erase(prevSpan);
	//	delete prevSpan;//此处不会结束循环，回到起始，再次开始向前合并，直到break
	//}
	////向后合并
	//while (1)
	//{
	//	PAGE_ID nextId = span->_pagid + span->_pagesize;
	//	auto it = _idSpanMap.find(nextId);

	//	if (it == _idSpanMap.end())
	//	{
	//		break;
	//	}

	//	Span* nextSpan = it->second;
	//	if (nextSpan->_usecount != 0)
	//	{
	//		break;
	//	}

	//	span->_pagid += nextSpan->_pagesize;

	//	for (PAGE_ID i = 0; i < nextSpan->_pagesize; ++i)
	//	{
	//		_idSpanMap[nextSpan->_pagid + i] = span;
	//	}

	//	_spanLists[nextSpan->_pagesize].Erase(nextSpan);
	//	delete nextSpan;//同上边逻辑，返回上边再次合并
	//}

	//_spanLists[span->_pagesize].PushFront(span);//把合并之后的span挂进去
}


Span* PageCache::GetIdToSpan(PAGE_ID id)
{
	//std::map<PAGE_ID, Span*>::iterator it = _idSpanMap.find(id);
	auto it = _idSpanMap.find(id);
	if (it != _idSpanMap.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}