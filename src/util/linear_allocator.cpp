#include "util/linear_allocator.h"
#include <assert.h>

linear_allocator::linear_allocator(
	size_t size
)
	: m_address  (malloc(size))
	, m_freelist (0, size)
{
	assert(size > 0 && "allocator constructed with invalid size");
	assert(m_address && "allocator failed to alloc new page");
}

linear_allocator::~linear_allocator()
{
	::free(m_address);
}

const free_list<size_t>& linear_allocator::freelist() const
{
	return m_freelist; 
}

char* linear_allocator::address(size_t offset) const
{ 
	return (char*)m_address + offset; 
}

size_t linear_allocator::distance(void* address) const 
{ 
	return size_t((char*)address - (char*)m_address); 
}

char* linear_allocator::alloc_bytes(size_t size)
{
	return address(m_freelist.mark_first(size));
}

void linear_allocator::free_bytes(void* address, size_t size)
{
	m_freelist.unmark(distance(address), size);
}

bool linear_allocator::has_space(size_t size) const
{
	return m_freelist.has_space(size);
}

bool linear_allocator::has_allocated(void* address) const
{
	return contains(address) && m_freelist.is_marked(distance(address));
}

bool linear_allocator::contains(void* address) const
{
	return (char*)address >= (char*)m_address 
		&& (char*)address <  (char*)m_address + m_freelist.m_limit_size; 
}

bool linear_allocator::contains_block(size_t index) const
{
	return contains((char*)m_address + get_block_size() * index);
}

size_t linear_allocator::get_block_index(void* address) const
{
	assert(contains(address) && "Address is not owned by allocator");
	assert(((size_t)address - (size_t)m_address) % get_block_size() == 0 && "Address isn't the start of a block");

	return ((size_t)address - (size_t)m_address) / get_block_size();
}

void* linear_allocator::get_block_address(size_t index) const
{
	void* address = (char*)m_address + get_block_size() * index;
	assert(contains(address) && "Address is not owned by allocator");

	return address;
}

void linear_allocator::reset()
{
	m_freelist.reset();
}

size_t linear_allocator::capacity() const
{
	return m_freelist.m_limit_size;
}
	
bool linear_allocator::is_empty() const
{
	return m_freelist.has_space(m_freelist.m_limit_size);
}

linear_iterator::linear_iterator(
	const linear_allocator& alloc
)
	: m_block_size          (alloc.get_block_size())
	, m_begin               (alloc.address(0))
	, m_end                 (alloc.address(alloc.freelist().m_limit_size))
	, m_range_limit_current (alloc.freelist().begin())
	, m_range_limit_end     (alloc.freelist().end())
{
	assert(m_block_size > 0 && "Iterator needs to set block_size to be able to iterate it");

	m_current = m_begin;
	advance_to_valid();
}

char* linear_iterator::get_bytes() const
{
	assert(m_current < m_end && "past end of iterator");
	return m_current;
}

bool linear_iterator::more() const
{
	assert(m_current <= m_end && "past end of iterator");
	return m_current < m_end;
}

void linear_iterator::next()
{
	m_current += m_block_size;
	advance_to_valid();
}

void linear_iterator::advance_to_valid() // issue lies here
{
	if (m_range_limit_current != m_range_limit_end)
	{
		assert(m_range_limit_current->m_begin % m_block_size == 0 && "block size is inconsistent");
		
		if (m_current == m_begin + m_range_limit_current->m_begin)
		{
			m_current += m_range_limit_current->m_size;
			++m_range_limit_current;
		}
	}
}
