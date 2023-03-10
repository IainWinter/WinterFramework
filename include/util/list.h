#pragma once

#include "util/pool_allocator.h"

// a vector that when resized, allocates a new page using a pool allocator
// this allows the storage of pointers to elements in the array without worry that
// they will be moved to another address on resize

template<typename _t>
struct list
{
private:
	pool_allocator m_storage;
	_t* m_back;

public:
	using iterator = pool_iterator;

	list()
	{
		m_storage.m_block_size = sizeof(_t);
	}

	void push_back(const _t& item)
	{
		_t* memory = (_t*)m_storage.alloc_block();
		*memory = item;
	}

	void emplace_back(_t&& item)
	{
		_t* memory = (_t*)m_storage.alloc_block();
		new (memory) _t(std::forward(item));
	}

	void pop_back()
	{
		m_storage.free_block(m_storage.);
	}
};