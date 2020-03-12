#pragma once

#include"threadcache.h"
#include"PageCache.h"

void* ConcurrencyMalloc(size_t size)
{
	if (tlsThread == nullptr)
	{
		tlsThread = new Threadcache;
	}

	if (size <= MAX_SIZE) // 1byte - 64kb
	{
		return tlsThread->Allocate(size);
	}
	else if (size <= (MAX_PAGES - 1) << PAGE_SHIFT) // 64kb - 128*4 kb
	{
		size_t align_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);//以一页对齐
		size_t pagenum = (align_size >> PAGE_SHIFT);//因为以4k对齐，所以需要右移
		Span* span = __pageCache.NewSpan(pagenum);
		span->_objSize = align_size;
		void* ptr = (void*)(span->_pagid << PAGE_SHIFT);
		return ptr;
	}
	else
	{
		size_t align_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);//以一页对齐
		size_t pagenum = (align_size >> PAGE_SHIFT);//因为以4k对齐，所以需要右移
		return SystemAlloc(pagenum);
	}
}

void ConcurrencyFree(void* ptr)
{
	size_t pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
	Span* span = __pageCache.GetIdToSpan(pageid);
	if (span == nullptr)//直接找系，所以为空统，没有映射
	{
		SystemFree(ptr);
		return;
	}
	size_t size = span->_objSize;
	if (size <= MAX_SIZE) // 1byte - 64kb
	{
		return tlsThread->Deallocate(ptr, size);
	}
	else if (size <= (MAX_PAGES - 1) << PAGE_SHIFT) // 64kb - 128*4 kb
	{
		__pageCache.ReleaseSpanToPageCache(span);
	}
}