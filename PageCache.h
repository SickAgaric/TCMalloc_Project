#pragma once

#include"common.h"

class PageCache
{
public:
	Span* NewSpan(size_t num);
	//上一次还内存
	void ReleaseSpanToPageCache(Span* span);
	//返回id号所对应的地址
	Span* GetIdToSpan(PAGE_ID id);
private:
	SpanList _spanLists[MAX_PAGES];
	std::map<PAGE_ID, Span*> _idSpanMap;
};

static PageCache __pageCache;