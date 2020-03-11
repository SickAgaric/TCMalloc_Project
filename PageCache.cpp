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
			splitpage->_pagid = span->_pagid + span->_pagesize - numpage;//�ҵ�β��Ȼ����
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

	void* ptr = SystemAlloc(MAX_PAGES - 1);//��ϵͳ����128ҳ

	Span* maxspan = new Span;
	maxspan->_pagid = (PAGE_ID)(ptr) >> PAGE_SHIFT;//�˴���ptr��дΪmaxspan
	maxspan->_pagesize = MAX_PAGES - 1;

	for (PAGE_ID i = 0; i < maxspan->_pagesize; ++i)
	{
		_idSpanMap[maxspan->_pagid + i] = maxspan; //����ʼҳ��һֱӳ�䵽+=128��
	}

	_spanLists[maxspan->_pagesize].PushFront(maxspan);

	return NewSpan(numpage);//��һ�εݹ��������
}

void PageCache::ReleaseSpanToPageCache(Span* span)
 {
	// ��ǰ�ϲ�
	while (1)
	{
		PAGE_ID prevPageId = span->_pagid - 1;
		auto pit = _idSpanMap.find(prevPageId);
		// ǰ���ҳ������
		if (pit == _idSpanMap.end())
		{
			break;
		}

		// ˵��ǰһ��Ҳ����ʹ���У����ܺϲ�
		Span* prevSpan = pit->second;
		if (prevSpan->_usecount != 0)
		{
			break;
		}

		// �ϲ�
		span->_pagid = prevSpan->_pagid;
		span->_pagesize += prevSpan->_pagesize;
		for (PAGE_ID i = 0; i < prevSpan->_pagesize; ++i)
		{
			_idSpanMap[prevSpan->_pagid + i] = span;
		}


		_spanLists[prevSpan->_pagesize].Erase(prevSpan);
		delete prevSpan;
	}


	// ���ϲ�
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
	////��ǰ�ϲ�
	//while (1)
	//{
	//	PAGE_ID prevPageid = span->_pagid - 1;
	//	auto it = _idSpanMap.find(prevPageid);
	//	//���1ǰ���ҳ������
	//	if (it == _idSpanMap.end())
	//	{
	//		break;
	//	}
	//	//it->secondȡ�����ǽṹ�����Է��ص���Span*
	//	Span* prevSpan = it->second;
	//	if (prevSpan->_usecount != 0)//���2ǰһ��ҳ����ʹ�ã����ɺϲ�
	//	{
	//		break;
	//	}

	//	span->_pagid += prevSpan->_pagesize;//��ǰ���ҳ�ϲ�,ҳ�Ÿ���Ϊǰ���ҳ��
	//	span->_pagesize += prevSpan->_pagesize;//ҳ�Ĵ�С����

	//	for (PAGE_ID i = 0; i < prevSpan->_pagesize; i++)
	//	{
	//		_idSpanMap[prevSpan->_pagid + i] = span;
	//	}

	//	_spanLists[prevSpan->_pagesize].Erase(prevSpan);
	//	delete prevSpan;//�˴��������ѭ�����ص���ʼ���ٴο�ʼ��ǰ�ϲ���ֱ��break
	//}
	////���ϲ�
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
	//	delete nextSpan;//ͬ�ϱ��߼��������ϱ��ٴκϲ�
	//}

	//_spanLists[span->_pagesize].PushFront(span);//�Ѻϲ�֮���span�ҽ�ȥ
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