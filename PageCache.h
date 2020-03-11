#pragma once

#include"common.h"

class PageCache
{
public:
	Span* NewSpan(size_t num);
	//��һ�λ��ڴ�
	void ReleaseSpanToPageCache(Span* span);
	//����id������Ӧ�ĵ�ַ
	Span* GetIdToSpan(PAGE_ID id);
private:
	SpanList _spanLists[MAX_PAGES];
	std::map<PAGE_ID, Span*> _idSpanMap;
};

static PageCache __pageCache;