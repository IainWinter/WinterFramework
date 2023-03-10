#pragma once

#include <cstddef>

class allocator 
{
private:
	// allows for setting a block size to allocate fixed size byte arrays
	// if not larger than 1, a block is just a normal byte
	// can only set block size once
	size_t m_block_size = 1;

public:
	void set_block_size(size_t block_size);
	size_t get_block_size() const;

	virtual char* alloc_bytes(size_t size) = 0;
	virtual void free_bytes(void* address, size_t size) = 0;
	
	virtual bool has_space(size_t size) const = 0;
	virtual bool has_allocated(void* address) const = 0;
	
	virtual bool contains(void* address) const = 0;
	virtual bool contains_block(size_t index) const = 0;

	virtual size_t get_block_index(void* address) const = 0;
	virtual void*  get_block_address(size_t index) const = 0;

	virtual void reset() = 0;
	virtual size_t capacity() const = 0;

	virtual bool is_empty() const = 0;

	size_t block_capacity() const;

	char* alloc_block();
	void free_block(void* address);

	template<typename _t>
	_t* alloc(size_t count = 1)
	{
		char* address = alloc_bytes(sizeof(_t) * count);
		
		// don't construct
		//for (size_t i = 0; i < count; i++)
		//{
		//	new (address + sizeof(_t) * i) _t();
		//}

		return (_t*)address;
	}

	template<typename _t, typename... _args>
	_t* construct(_args&&... args)
	{
		_t* data = alloc<_t>();
		new (data) _t(args...);
		return data;
	}

	template<typename _t>
	void free(_t* address, size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			(address + i)->~_t();
		}

		free_bytes(address, sizeof(_t) * count);
	}
};

class allocator_iterator
{
public:
	virtual char* get_bytes() const = 0;
	virtual bool more() const = 0;
	virtual void next() = 0;

	template<typename _t>
	_t* get() const { return (_t*)get_bytes(); }
};
