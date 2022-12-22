#pragma once

#include "util/allocator.h"
#include <assert.h>

void allocator::set_block_size(size_t block_size)
{
	assert(m_block_size == 1 && "Cannot set block size after it has been set. Only if was set to something above 1");
	assert(m_block_size != 0 && "Cannot set block size to 0");
	m_block_size = block_size;
}

size_t allocator::get_block_size() const
{
	return m_block_size;
}

size_t allocator::block_capacity() const
{
	return capacity() / m_block_size;
}

char* allocator::alloc_block()
{
	return alloc_bytes(m_block_size);
}

void allocator::free_block(void* address)
{
	free_bytes(address, m_block_size);
}