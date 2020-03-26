#pragma once

#include<iostream>
#include<assert.h>
#include<windows.h>
#include<map>
#include<thread>
#include<mutex>

using std::cout;
using std::endl;


const size_t MAX_SIZE = 64 * 1024;//用枚举替代宏，便于以后调试
const size_t NFRER_LIST = MAX_SIZE / 8;
const size_t MAX_PAGES = 129;
const size_t PAGE_SHIFT = 12;//4k


inline void*& NextObj(void* obj)//此函数因为频繁的调用，所以使用内联函数
{
	//因为*(int*)p在32位下是4字节，在64位下是8字节
	//所以转换为*(void**)解引用之后就是void*，而指针的大小会随着系统为改变而改变
	return *((void**)obj);
}

class FreeList
{
public:
	void push(void* obj)//还内存
	{
		//头插
		NextObj(obj) = _freelist;//将obj指向链表的头
		_freelist = obj;//将头赋值为obj
		++_num;//统计数量
	}

	void* Pop()//取内存
	{
		//头删
		void* obj = _freelist;
		_freelist = NextObj(obj);
		--_num;

		return obj;
	}

	void PushRange(void* head, void* tail, size_t num)
	{
		NextObj(tail) = _freelist;
		_freelist = head;
		_num += num;
	}


	size_t PopRange(void*& start, void*& end, size_t num)
	{
		size_t actualNum = 0;
		void* prev = nullptr;
		void* cur = _freelist;
		for (; actualNum < num && cur != nullptr; ++actualNum)
		{
			prev = cur;
			cur = NextObj(cur);
		}

		start = _freelist;
		end = prev;
		_freelist = cur;

		_num -= actualNum;

		return actualNum;
	}
	size_t Num()
	{
		return _num;
	}
	
	bool Empty()
	{
		return _freelist == nullptr;
	}

	void Clear()
	{
		_freelist = nullptr;
		_num = 0;
	}
private:
	void* _freelist = nullptr;//默认构造函数初始化
	size_t _num = 0;
};


class SizeClass //计算大小的类
{
public:
	static size_t _ListIndex(size_t size, size_t alignment_shift) //链表下标
	{
		return ((size + (1 << alignment_shift) - 1) >> alignment_shift) - 1;		/*if (size % 8 == 0)
		{
			return size / 8 - 1;
		}
		else
			return size / 8;*/
	}

	static inline size_t Index(size_t size)
	{
		assert(size <= MAX_SIZE);
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (size <= 128){
			return _ListIndex(size, 3);//以8字节对齐算出链表下表
		}
		else if (size <= 1024){
			return _ListIndex(size - 128, 4) + group_array[0];
		}
		else if (size <= 8192){
			return _ListIndex(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 65536){
			return _ListIndex(size - 8192, 9) + group_array[2] + group_array[1] +
				group_array[0];
		}
		assert(false);
		return -1;
	}

	static inline size_t _RoundUp(size_t size, size_t alignment)//计算所取的字节数应该补齐到多少（9-16）~~计算后都是16
	{
		return (size + alignment - 1)&(~(alignment - 1));
		/*if (size % 8 != 0)
			return (size / 8 + 1) * 8;
		else
			return size;*/
	}

// 对齐大小计算
	static inline size_t RoundUp(size_t size)
	{
		assert(size <= MAX_SIZE);
		if (size <= 128){
			return _RoundUp(size, 8);
		}
		else if (size <= 1024){
			return _RoundUp(size, 16);
		}
		else if (size <= 8192){
			return _RoundUp(size, 128);
		}
		else if (size <= 65536){
			return _RoundUp(size, 512);
		}
		return -1;
	}

	//动态适应给出的个数，thread 向 central 要
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = (MAX_SIZE / size);
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;
		return num;
	}

	static size_t NumMovePage(size_t size)//一次向pagecache取几个
	{
		size_t num = NumMoveSize(size);
		size_t npage = num*size;
		
		npage >>= 12;
 		if (npage == 0)
			npage = 1;

		return npage;
	}
};

#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PAGE_ID;
#endif


struct Span
{
	PAGE_ID _pagid = 0;//页号
	PAGE_ID _pagesize = 0;//页的数量
	size_t _objSize = 0;//自由链表对象的大小
	int _usecount = 0;//内存块对象使用计数
	FreeList _freeList;
	//默认构造函数会使用缺省值进行初始化
 	//size_t objsize;
	Span* _next = nullptr;
	Span* _prev = nullptr;
};


class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;//左闭右开，所以返回头
	}

	void Insert(Span* pos, Span* newspan)//随机位置插入
	{
		Span* prev = pos->_prev;

		//prev newspan pos
		prev->_next = newspan;
		newspan->_prev = prev;

		pos->_prev = newspan;
		newspan->_next = pos;
	}

	void Erase(Span* pos)
	{
		assert(pos != _head);
		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}

	void PushFront(Span* newspan)//头插
	{
		Insert(_head->_next,newspan);
	}

	void PopFront()
	{
		Erase(_head->_next);
	}

	void PushBack(Span* newspan)
	{
		Insert(_head->_prev, newspan);
	}

	void PopBack()
	{
		Erase(_head->_prev);
	}

	bool Empty()
	{
		return Begin() == End();
	}

	void Lock()
	{
		_mut.lock();
	}

	void Unlock()
	{
		_mut.unlock();
	}

private:
	Span* _head;//带头双向链表
	std::mutex _mut;
};

inline static void* SystemAlloc(size_t num_page)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, num_page*(1 << PAGE_SHIFT),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	//linux
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr,0,MEM_RELEASE);
#else
#endif
}
