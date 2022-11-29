#include "ext/serial/serial_bin.h"

//
//		Writer
//

bin_writer::bin_writer(std::ostream& out)
	: serial_writer(out, true)
{}

void bin_writer::class_begin(meta::type* type) {}
void bin_writer::class_delim() {}
void bin_writer::class_end() {}

void bin_writer::member_begin(meta::type* type, const char* name) {}
void bin_writer::member_end() {}

void bin_writer::array_begin(meta::type* type, size_t length) { write_value(length); }
void bin_writer::array_delim() {}
void bin_writer::array_end() {}

void bin_writer::string_begin(size_t length) { write_value(length); }
void bin_writer::string_end() {}

//
//		Reader
//

bin_reader::bin_reader(std::istream& in)
    : meta::serial_reader(in, true)
{}

void bin_reader::class_begin(meta::type* type) {}
void bin_reader::class_delim() {}
void bin_reader::class_end() {}

void bin_reader::member_begin(meta::type* type, const char* name) {}
void bin_reader::member_end() {}

void bin_reader::array_begin(meta::type* type, size_t length) {}
void bin_reader::array_delim() {}
void bin_reader::array_end() {}

void bin_reader::string_begin(size_t length) {}
void bin_reader::string_end() {}

size_t bin_reader::read_length()
{
    size_t length;
    read_value(length);
    return length;
}

void bin_reader::read_bytes(char* bytes, size_t length)
{
    m_in.read(bytes, length);
}
