#pragma once

#include "ext/serial/serial.h"

struct bin_writer : meta::serial_writer
{
	bin_writer(std::ostream& out);

	void class_begin(meta::type* type) override;
	void class_delim() override;
	void class_end() override;

	void member_begin(meta::type* type, const char* name) override;
	void member_end() override;

	void array_begin(meta::type* type, size_t length) override;
	void array_delim() override;
	void array_end() override;

	void string_begin(size_t length) override;
	void string_end() override;

    void write_bytes(const char* bytes, size_t length) override;
};

struct bin_reader : meta::serial_reader
{
    bin_reader(std::istream& in);
    
    void class_begin(meta::type* type) override;
    void class_delim() override;
    void class_end() override;
    
    void member_begin(meta::type* type, const char* name) override;
    void member_end() override;
    
    void array_begin(meta::type* type, size_t length) override;
    void array_delim() override;
    void array_end() override;
    
    void string_begin(size_t length) override;
    void string_end() override;
    
    size_t read_length() override;
    void read_bytes(char* bytes, size_t length) override;
};


//struct bin_reader : meta::serial_reader
//{ 
//	bin_reader(std::istream& in)
//		: meta::serial_reader (in, true)
//	{}
//
//	void read_type(meta::type* type, void* instance) override
//	{
//		auto& members = type->get_members();
//
//		if (members.size() > 0)
//		{
//			for (int i = 0; i < members.size(); i++)
//			{
//				meta::type* member = members.at(i);
//				read_type(member, member->walk_ptr(instance));
//			}
//		}
//
//		else
//		{
//			type->_serial_read(this, instance);
//		}
//	}
//
//	size_t read_length() override
//	{
//		size_t length = 0;
//		read_value<size_t>(length);
//		return length;
//	}
//
//	void read_array(meta::type* type, void* instance, size_t repeat) override
//	{
//		for (int i = 0; i < repeat; i++)
//		{
//			read_type(type, (char*)instance + i * type->info()->m_size);
//		}
//	}
//
//	void read_string(char* str, size_t length) override
//	{
//		m_in.read(str, sizeof(char) * length);
//	}
//};
