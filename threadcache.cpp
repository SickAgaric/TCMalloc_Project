#include"threadcache.h"
#include"centrlcache.h"

//如果不加Threadcache，就会导致链接不上，因为没有指明这个函数在那个类里面
void* Threadcache::Allocate(size_t size)//由此处的size%8可以计算出自由链表的下标
{
	size_t index = SizeClass::Index(size);
	FreeList& freeList = _freeLists[index];

	if (!freeList.Empty())
	{
		return freeList.Pop();
	}
	else
	{
		//没有像中心缓存取
		return FetchFromCentralCache(SizeClass::RoundUp(size));
	}
}
void Threadcache::Deallocate(void* ptr, size_t size)//归还内存
{
	size_t index = SizeClass::Index(size);
	FreeList& freelist = _freeLists[index];
	freelist.push(ptr);

	size_t num = SizeClass::NumMoveSize(size);
	if (freelist.Num() >= num)
	{
		ListLong(freelist, num , size); 
	}

}

void Threadcache::ListLong(FreeList& freelist, size_t num, size_t size)
{
	void* start = nullptr, *end = nullptr;
	freelist.PopRange(start, end, num);

	NextObj(end) = nullptr;//方式内存问题
	__centrlcache.ReleaseListToSpans(start, size);
}
//void* Threadcache::FetchFromCentralCache(size_t index)
//{
//	size_t num = 20;
//	size_t size = (index + 1) * 8;
//	char* start = (char*)malloc(size*num);
//	char* cur = start;
//
//	for (size_t i = 0; i < num-1; ++i)
//	{
//		char* next = cur + size;
//		NextObj(cur) = next;//吧next作为下一个指针的值，传给NextObj
//
//		cur = next;//
//	}
//	NextObj(cur) = nullptr;
//
//	void* head = NextObj(start); //因为此处只取了一个，也就是start，所以往自由链表挂的时候，需要start的下一个作为头挂进去
//	void* tail = cur;
//	
//	_freeLists[index].PushRange(head, tail);
//
//	return start;
//}

void* Threadcache::FetchFromCentralCache(size_t size)
{
	size_t num = SizeClass::NumMoveSize(size);//计算需要的个数

	void* start = nullptr, *end = nullptr;

	// 从中心缓存获取一定数量的对象给threadcache
	size_t realNum = __centrlcache.FetchRangeObj(start, end, num , size);

	if (realNum == 1)//返回一个时候直接返回
	{
		return start;
	}
	else//多个的时候需要返回第一个，把剩余的挂起来
	{
		size_t index = SizeClass::Index(size);
		FreeList& list = _freeLists[index];
		list.PushRange(NextObj(start), end,realNum - 1);

		return start;
	}
}
