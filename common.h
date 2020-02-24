#pragma once

#include<iostream>
using namespace std;

const size_t MAX_SIZE = 64 * 1024;//用枚举替代宏，便于以后调试
const size_t NFRER_LIST = MAX_SIZE / 8;

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
	}

	void PushRange(void* head, void* tail)
	{
		NextObj(tail) = _freelist;
		_freelist = head;
	}

	void* Pop()//取内存
	{
		//头删
		void* obj = _freelist;
		_freelist = NextObj(obj);

		return obj;
	}
	bool Empty()
	{
		return _freelist == nullptr;
	}
private:
	void* _freelist = nullptr;
};


class SizeClass //计算大小的类
{
public:
	static size_t ListIndex(size_t size) //
	{
		if (size % 8 == 0)\
		{
			return size / 8 - 1;
		}
		else
			return size / 8;
	}
};

#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PAGE_ID;
#endif


struct Span
{
	int _pagid;//页号
	int _pagesize;//页的数量

	int _usecount;//内存块对象使用计数
	void* _freelist;

	//size_t objsize;
	Span* _next;
	Span* _prev;
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

private:
	Span* _head;//带头双向链表
};