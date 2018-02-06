#pragma once
#include <iostream>
const float cRatio = 1.3f;
using namespace std;
enum HolderState
{
	Space = 0,
	Available = 1,
	Allocated = 2
};

template <class T>
class Holder
{
	template <class T> friend class VectorList;

public:
	Holder() :
		_index(0),
		_mindex(0)
	{

	};

	Holder(HolderState state) :
		_state(state)
	{

	};

private:
	Holder *_prev;
	Holder *_next;

	HolderState _state;
	int _index;
	int _mindex;
	T* _data;

public:
	Holder& operator++()
	{
		return *_next;
	}
	Holder& operator--()
	{
		return _prev;
	}

	void assign(const Holder* H)
	{
		_data = H->_data;
		_state = H->_state;
	};

	void pull()
	{
		assign(this->_next);
		this->_next = this->_next->_next();
	}

	int get_state() const { return _state; };
	int offset() const { return _mindex - _index; };
	void set_state(HolderState s) 
	{
		_state = s;
		if (_state == HolderState::Available || _state == HolderState::Space)
		{
			_index = _mindex;
		}
	
	};
	T* value() const { return _data; };
	int index() const { return this->_index; };

	Holder* prev() const { return _prev; };
	Holder* next() const { return _next; };

	HolderState state() { return _state; };

	void copy_and_fill(const T& v)
	{
		_data = new T(v);
	}

};


template <class T>
class VectorList
{
public:
	typedef Holder<T>& Iterator;
	typedef void (*testCallback)(T& v);

private:
	testCallback call;
	int size;

	int allocated;

	int deleted;
	int interval;
	
	int assign;
	int aPointer;
	int capacity;

	Holder<T>* map;

public:
	VectorList(int capacity) :
		interval(4),
		assign(capacity + capacity / interval),
		size(0),
		deleted(0),
		aPointer(1),
		capacity(assign)
	{
		map = new Holder<T>[assign];
		init();
	};

	VectorList(int repeat, T& t) :
		interval(4),
		allocated(repeat),
		assign(repeat + (repeat / interval)),
		size(0),
		deleted(0),
		aPointer(1),
		capacity(assign * cRatio)
	{
		call = [](T& v)->void { v++; };
		map = new Holder<T>[capacity];
		init(t);
	};

	void test()
	{
		for (int i = 0; i < capacity; i++)
		{
			std::cout << map[i]._mindex;

			if (map[i].state() == HolderState::Allocated)
			{
				std::cout << " : " << map[i]._index << " , " << *map[i].value() << std::endl;
			}
			else if (map[i].state() == HolderState::Available)
			{
				std::cout << " : Available"<< std::endl;
			} 
			else if (map[i].state() == HolderState::Space)
			{
				std::cout << " : Space"<< std::endl;
			}


		}
	};

	void set_rel()
	{
		map[0]._prev = 0;
		map[0]._next = &map[1];

		for (int i = 1; i < capacity - 1 ; i++)
		{
			map[i]._prev = &map[i - 1];
			map[i]._next = &map[i+1];
		}

		map[capacity - 1]._prev = &map[capacity - 2];
		map[capacity - 1]._next = 0;
	}

	void init(T& v)
	{
		int count = 0; 
		int alloc = 0;
		int index = 0;

		for (index = 0; alloc < allocated; index++)
		{
			map[index]._mindex = index;

			if (count == 0)
			{
				count = interval;
				map[index].set_state(HolderState::Space);
				map[index]._index = alloc;
				continue;
			}
			
			map[index]._index = alloc++;
			map[index].set_state(HolderState::Allocated);
			map[index].copy_and_fill(v++);

			count--;
		
		}

		for (int i = index; i < capacity; i++)
		{
			map[i]._mindex = i;
			map[i]._index = alloc;
			map[i].set_state(HolderState::Available);
		}

		set_rel();
	}
	int search(int idx)
	{
		int bsi = idx;
		
		int off = map[idx].offset();
		int boff = 0;
		int si = idx + off;

		Holder<T>* smap = map;

		while (smap->state() == HolderState::Space)
		{
			smap++;
			si++;
			off = smap->offset();
		}

		do {
			if (map[si].state() == HolderState::Space)
			{
				si++;
				continue;
			}
			
			off = map[si].offset() - map[bsi].offset();
			
			if (!off)
				return bsi;

			bsi = si;
			si += off;
	
		} while (off);
		
		return -1;

	}

	void init()
	{
		for (int i = 0; i < assign; i++)
		{
			if (i % interval == 0)
			{
				map[i].set_state(HolderState::Space);
			}

			else
			{
				map[i].set_state(HolderState::Available);
			}
		};
	};


	void insert(Iterator it, T& v)
	{
		Iterator insertedIter = it;
		bool inserted = false;
		int pushCount = 0;
		int base = it->mindex();

		if (it.offset() == 0)
			rearrange();

		while (insertedIter)
		{
			if (insertedIter->state() == HolderState::Space)
			{
				inserted = true;
				break;
			}
			else
			{
				insertedIter = insertedIter->_prev;
				pushCount++;
			}
		}


		if (inserted)
		{
			map[base - pushCount].set_state(Allocated);
			int k = base - pushCount;
			for (int i = 0; i < pushCount; i++, k++)
			{
				map[k].pull();
			};

			map[base].set_state(Space);

		}

	};

	Iterator begin()
	{
		return map[0];
	}

	T* at(int i)
	{
		int sindex = search(i);
		return map[sindex].value();
	};

	T* at(Iterator it)
	{

	}


	void first() { int idx = search(0); };
	int search_test(int idx ) { 
		int s = search(idx);
		return map[s]._mindex - s;
	};

};

