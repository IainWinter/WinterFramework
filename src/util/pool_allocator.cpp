#include "util/pool_allocator.h"

pool_allocator::pool_allocator()
	: m_page_size_hint (0)
	, m_next_page_size (0)
	, m_next_page_mult (2)
{}

pool_allocator::pool_allocator(
	size_t page_expansion
)
	: m_page_size_hint (0)
	, m_next_page_size (0)
	, m_next_page_mult (page_expansion)
{}

pool_allocator::pool_allocator(
	size_t page_size,
	size_t page_expansion
)
	: m_page_size_hint (page_size)
	, m_next_page_size (page_size)
	, m_next_page_mult (page_expansion)
{}

pool_allocator::~pool_allocator()
{
	for (auto [_, page] : m_pages)
	{
		delete page;
	}
}

const std::vector<std::pair<size_t, linear_allocator*>>& pool_allocator::get_pages() const
{
	return m_pages;
}

size_t pool_allocator::get_page_size_hint() const
{
	return m_page_size_hint;
}

size_t pool_allocator::get_next_page_size() const
{
	return m_next_page_size;
}

size_t pool_allocator::get_next_page_mult() const
{
	return m_next_page_mult;
}

char* pool_allocator::alloc_bytes(size_t size)
{
	linear_allocator* page = nullptr;
	for (auto [_, p] : m_pages)
	{
		if (p->has_space(size)) 
		{
			page = p;
			break;
		}
	}

	if (!page)
	{
		m_next_page_size = size > m_next_page_size ? size : m_next_page_size;

		page = new linear_allocator(m_next_page_size);
		page->set_block_size(get_block_size());

		size_t start_block_index = m_pages.size() == 0 ? 0 : m_pages.back().first + m_pages.back().second->block_capacity();
		m_pages.emplace_back((int)start_block_index, page);

		assert(m_next_page_mult > 0 && "Next page expansion is invalid");
		m_next_page_size *= m_next_page_mult;
	}

	return page->alloc_bytes(size);
}

void pool_allocator::free_bytes(void* address, size_t size)
{
	auto itr = m_pages.begin(); // to not include algorithm for std::find
	linear_allocator* page = nullptr;
	for (; itr != m_pages.end(); ++itr)
	{
		if ((*itr).second->contains(address)) 
		{
			page = (*itr).second;
			break;
		}
	}

	assert(page && "Address not in pool");

	page->free_bytes(address, size);
		
	// if (page->is_empty()) 
	// {
	// 	delete page;
	// 	assert(m_next_page_mult > 0 && "Next page expansion is invalid");
	// 	m_next_page_size /= m_next_page_mult;
	// 	m_pages.erase(itr);                           // see above
	// }
}

bool pool_allocator::has_space(size_t size) const
{
	for (auto [_, page] : m_pages) if (page->has_space(size)) return true;
	return false;
}

bool pool_allocator::has_allocated(void* address) const
{
	for (auto [_, page] : m_pages) if (page->has_allocated(address)) return true;
	return false;
}

bool pool_allocator::contains(void* address) const
{
	for (auto [_, page] : m_pages) if (page->contains(address)) return true;
	return false;
}

bool pool_allocator::contains_block(size_t index) const
{
	for (auto [_, page] : m_pages) if (page->contains_block(index)) return true;
	return false;
}

size_t pool_allocator::get_block_index(void* address) const
{
	for (auto [page_index_start, page] : m_pages)
	{
		if (page->contains(address))
		{
			return page_index_start + page->get_block_index(address);
		}
	}

	assert(false && "Address is not in allocator");
	throw nullptr;
}

void* pool_allocator::get_block_address(size_t index) const
{
	for (auto [page_index_start, page] : m_pages)
	{
		if (page->contains_block(index - page_index_start)) // valid if negative or overflow
		{
			return page->get_block_address(index - page_index_start);
		}
	}

	assert(false && "Address is not in allocator");
	throw nullptr;
}

void pool_allocator::reset()
{
	for (auto [_, page] : m_pages)
	{
		delete page;
	}

	m_pages.clear();
	m_next_page_size = m_page_size_hint;
}

size_t pool_allocator::capacity() const
{
	size_t size = 0;
	for (auto [_, page] : m_pages) size += page->capacity();
	return size;
}

bool pool_allocator::is_empty() const
{
	for (auto [_, page] : m_pages) if (!page->is_empty()) return false;
	return true;
}

pool_iterator::pool_iterator()
{
	m_current = m_pages.begin();
}

pool_iterator::pool_iterator(const pool_allocator& alloc)
{
	m_pages.reserve(alloc.get_pages().size());
	for (const auto& [_, page] : alloc.get_pages())
	{
		linear_iterator itr = linear_iterator(*page);
		//assert(itr.more());
		if (!itr.more()) continue;
		m_pages.push_back(itr);
	}

	m_current = m_pages.begin();
}

char* pool_iterator::get_bytes() const
{
	return m_current->get_bytes();
}

bool pool_iterator::more() const
{
	return m_current != m_pages.end() && m_current->more();
}

void pool_iterator::next()
{
	m_current->next();
	if (!m_current->more()) ++m_current;
}
