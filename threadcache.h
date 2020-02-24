#pragma once

#include"common.h"

class Threadcache
{
public:
	//�����ڴ���ͷ��ڴ�
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
	//������cacheȡ�ڴ�
	void* FetchFromCentralCache(size_t index);
private:
	FreeList _freeLists[NFRER_LIST];
};