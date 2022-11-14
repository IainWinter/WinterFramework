#pragma once

#include "ext/serial/serial.h"
#include <stack>
#include <sstream>

// these are in cpp so user doesn't need json.h

struct json_writer : meta::serial_writer
{
	json_writer(std::ostream& out);

	void write_class (meta::type* type, const void* instance) override;
	void write_member(meta::type* type, const char* name, const void* instance) override;
	void write_array (meta::type* type, const void* instance, size_t length) override;
	void write_string(const char* string, size_t length) override;

	void write_length(size_t length) override;

	void class_begin(meta::type* type) override;
	void class_delim() override;
	void class_end() override;

	void array_begin() override;
	void array_delim() override;
	void array_end() override;

	void string_begin() override;
	void string_end() override;
};

// fwd json.h
struct json_value_s;
struct json_object_element_s;

struct json_reader : meta::serial_reader
{
	json_reader(std::istream& in);

	bool is_valid() const;

	void read_class (meta::type* type, void* instance) override;
	void read_member(meta::type* type, void* instance) override;
	void read_array (meta::type* type, void* instance, size_t length) override;
	void read_string(char* string, size_t length)override;
	
	size_t read_length() override;

	void class_begin(meta::type* type) override;
	void class_delim() override;
	void class_end()   override;
	
	void array_begin() override;
	void array_delim() override;
	void array_end()   override;
	
	void string_begin() override;
	void string_end()  override;

private:
	json_value_s* m_json;

	// reading object state

	struct obj_frame
	{
		meta::type* current_type;
		std::vector<meta::type*>::const_iterator current_member_type;
		json_object_element_s* current_member_json;
		int real_members_left;
	};

	std::stack<obj_frame> m_objs;

	void init_json();
};