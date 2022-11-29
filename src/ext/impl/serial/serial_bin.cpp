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

