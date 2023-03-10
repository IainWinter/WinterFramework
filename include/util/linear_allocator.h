#pragma once

#include "util/allocator.h"
#include "util/free_list.h"

class linear_allocator : public allocator
{
public:
	void* m_address;
private:
	free_list<size_t> m_freelist;

public:
	linear_allocator(size_t size);
	~linear_allocator();

	const free_list<size_t>& freelist() const;
	char* address(size_t offset) const;
	size_t distance(void* address) const;

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

// This iterator only supports iterating over blocks of memory
// asserts will fail if there is a non-uniform range of memory allocated

class linear_iterator : public allocator_iterator
{
private:
	size_t m_block_size;

	char* m_begin;
	char* m_end;
	char* m_current;

	free_list<size_t>::const_iterator m_range_limit_current;
	free_list<size_t>::const_iterator m_range_limit_end;

public:
	linear_iterator(const linear_allocator& alloc);

	char* get_bytes() const override;
	bool more() const override;
	void next() override;

	void advance_to_valid();
};