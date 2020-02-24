#pragma once

#include<iostream>
using namespace std;

const size_t MAX_SIZE = 64 * 1024;//��ö������꣬�����Ժ����
const size_t NFRER_LIST = MAX_SIZE / 8;

inline void*& NextObj(void* obj)//�˺�����ΪƵ���ĵ��ã�����ʹ����������
{
	//��Ϊ*(int*)p��32λ����4�ֽڣ���64λ����8�ֽ�
	//����ת��Ϊ*(void**)������֮�����void*����ָ��Ĵ�С������ϵͳΪ�ı���ı�
	return *((void**)obj);
}

class FreeList
{
public:
	void push(void* obj)//���ڴ�
	{
		//ͷ��
		NextObj(obj) = _freelist;//��objָ�������ͷ
		_freelist = obj;//��ͷ��ֵΪobj
	}

	void PushRange(void* head, void* tail)
	{
		NextObj(tail) = _freelist;
		_freelist = head;
	}

	void* Pop()//ȡ�ڴ�
	{
		//ͷɾ
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


class SizeClass //�����С����
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
	int _pagid;//ҳ��
	int _pagesize;//ҳ������

	int _usecount;//�ڴ�����ʹ�ü���
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

	void Insert(Span* pos, Span* newspan)//���λ�ò���
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

	void PushFront(Span* newspan)//ͷ��
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
	Span* _head;//��ͷ˫������
};