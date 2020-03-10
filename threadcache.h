#pragma once

#include"common.h"

class Threadcache
{
public:
	//申请内存和释放内存
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
	//从中心cache取内存
	void* FetchFromCentralCache(size_t index);
	//如果自由链表中对象超过一定长度
	void ListLong(FreeList& freelist, size_t num, size_t size);
private:
	FreeList _freeLists[NFRER_LIST];
};

_declspec (thread) static Threadcache* tlsThread = nullptr;