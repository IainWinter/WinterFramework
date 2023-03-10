#pragma once

#include "util/linear_allocator.h"

class pool_allocator : public allocator
{
private:
	std::vector<std::pair<size_t, linear_allocator*>> m_pages; // block index start, page
	size_t m_page_size_hint;
	size_t m_next_page_size;
	size_t m_next_page_mult;

public:
	pool_allocator();
	pool_allocator(size_t page_expansion);
	pool_allocator(size_t page_size, size_t page_expansion);

	~pool_allocator();

	const std::vector<std::pair<size_t, linear_allocator*>>& get_pages() const;
	size_t get_page_size_hint() const;
	size_t get_next_page_size() const;
	size_t get_next_page_mult() const;

	char* alloc_bytes(size_t size) override;
	void free_bytes(void* address, size_t size) override;

	bool has_space(size_t size) const override;
	bool has_allocated(void* address) const override;

	bool contains(void* address) const override;
	bool contains_block(size_t index) const override;

	size_t get_block_index(void* address) const override;
	void* get_block_address(size_t index) const override;

	void reset() override;
	size_t capacity() const override;
	bool is_empty() const override;
};

// need a way for this to be const

class pool_iterator : public allocator_iterator
{
private:
	std::vector<linear_iterator> m_pages;
	std::vector<linear_iterator>::iterator m_current;

public:
	pool_iterator();
	pool_iterator(const pool_allocator& alloc);

	char* get_bytes() const override;
	bool more() const override;
	void next() override;
};
