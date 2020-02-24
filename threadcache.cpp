#include"threadcache.h"

void* Threadcache::Allocate(size_t size)//�ɴ˴���size%8���Լ��������������±�
{
	size_t index = SizeClass::ListIndex(size);
	FreeList& freeList = _freeLists[index];

	if (!freeList.Empty())
	{
		return freeList.Pop();
	}
	else
	{
		size_t num = 20;
		return FetchFromCentralCache(index);
	}
}
void Threadcache::Deallocate(void* ptr, size_t size)//�黹�ڴ�
{
	size_t index = SizeClass::ListIndex(size);
	FreeList& freelist = _freeLists[index];
	freelist.push(ptr);
}

void* Threadcache::FetchFromCentralCache(size_t index)
{
	size_t num = 20;
	size_t size = (index + 1) * 8;
	char* start = (char*)malloc(size*num);
	char* cur = start;

	for (size_t i = 0; i < num-1; ++i)
	{
		char* next = cur + size;
		NextObj(cur) = next;//��next��Ϊ��һ��ָ���ֵ������NextObj

		cur = next;//
	}
	NextObj(cur) = nullptr;

	void* head = NextObj(start); //��Ϊ�˴�ֻȡ��һ����Ҳ����start����������������ҵ�ʱ����Ҫstart����һ����Ϊͷ�ҽ�ȥ
	void* tail = cur;
	
	_freeLists[index].PushRange(head, tail);

	return start;
}
