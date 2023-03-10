#pragma once

#include "ext/serial/serial.h"
#include <stack>

struct json_writer : meta::serial_writer
{
	json_writer(std::ostream& out);

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

// fwd json.h
struct json_value_s;
struct json_array_element_s;
struct json_object_element_s;

struct json_reader : meta::serial_reader
{
public:
	json_reader(std::istream& in);

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
    
private:
    struct json_frame
    {
        meta::type* type;
        int what;

        json_frame(meta::type* type, json_value_s*          value)  : type(type), value (value),  what(0) {}
        json_frame(meta::type* type, json_array_element_s*  item)   : type(type), item  (item),   what(1) {}
        json_frame(meta::type* type, json_object_element_s* member) : type(type), member(member), what(2) {}

        union
        {
            json_value_s* value;
            json_array_element_s*  item;
            json_object_element_s* member;
        };

        json_value_s* get_value();
    };

    std::stack<json_frame> m_frames;
};
