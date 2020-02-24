#include"threadcache.h"

void* Threadcache::Allocate(size_t size)//由此处的size%8可以计算出自由链表的下标
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
void Threadcache::Deallocate(void* ptr, size_t size)//归还内存
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
		NextObj(cur) = next;//吧next作为下一个指针的值，传给NextObj

		cur = next;//
	}
	NextObj(cur) = nullptr;

	void* head = NextObj(start); //因为此处只取了一个，也就是start，所以往自由链表挂的时候，需要start的下一个作为头挂进去
	void* tail = cur;
	
	_freeLists[index].PushRange(head, tail);

	return start;
}
