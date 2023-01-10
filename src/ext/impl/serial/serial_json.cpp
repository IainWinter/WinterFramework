#include "ext/serial/serial_json.h"
#include "json/json.h"
#include <string.h>
#include <sstream>

// for debug
#include "Log.h"

//
//		Writer
//

json_writer::json_writer(std::ostream& out)
	: meta::serial_writer(out, false)
{}

void json_writer::class_begin(meta::type* type) { m_out << '{'; }
void json_writer::class_delim()                 { m_out << ','; }
void json_writer::class_end()                   { m_out << '}'; }

void json_writer::member_begin(meta::type* type, const char* name) { m_out << "\"" << name << "\":"; }
void json_writer::member_end()                                     { }

void json_writer::array_begin(meta::type* type, size_t length) { m_out << '['; }
void json_writer::array_delim()                                { m_out << ','; }
void json_writer::array_end()                                  { m_out << ']'; }

void json_writer::string_begin(size_t length) { m_out << '"'; }
void json_writer::string_end()                { m_out << '"'; }

void json_writer::write_bytes(const char* bytes, size_t length)
{
	// this changes the length in begin_string, but json doesnt care about the reported length
	// the reader uses json.h's length, which doesnt include escaped chars
	// should investigate this fully

	for (size_t i = 0; i < length; i++)
	{
		m_out << bytes[i];
		if (bytes[i] == '\\') m_out << '\\'; // escape slash? figure out where to do this
	}
}

//
//		Reader
//

json_value_s* json_reader::json_frame::get_value()
{
	switch (what)
	{
		case 0: return value;
		case 1: return item->value;
		case 2: return member->value;
	}
}

json_reader::json_reader(std::istream& in)
	: meta::serial_reader (in, true)
	//, m_json              (nullptr)
{
	std::stringstream ss; ss << m_in.rdbuf();
	std::string str = ss.str(); // big copy?
	
	m_frames.emplace(nullptr, json_parse(str.c_str(), str.size()));
}

void json_reader::class_begin(meta::type* type)
{
	m_frames.emplace(type, json_value_as_object(m_frames.top().get_value())->start);
}

void json_reader::class_delim()
{
	// increment object frame
	json_frame& frame = m_frames.top();
	frame.member = frame.member->next;
}

void json_reader::class_end()
{
	// remove object frame
	m_frames.pop();
}

void json_reader::member_begin(meta::type* type, const char* name)
{
	m_frames.emplace(type, m_frames.top().get_value());
}

void json_reader::member_end()
{
	// remove member frame
	m_frames.pop();
}

void json_reader::array_begin(meta::type* type, size_t length)
{
	m_frames.emplace(type, json_value_as_array(m_frames.top().get_value())->start);
}

void json_reader::array_delim()
{
	// increment array frame
	json_frame& frame = m_frames.top();
	frame.item = frame.item->next;
}

void json_reader::array_end()
{
	// remove array frame
	m_frames.pop();
}

void json_reader::string_begin(size_t length)
{
	// strings are handled in read_bytes because json_reader is set to binary mode
	// should change the name of the mode
}

void json_reader::string_end()
{
}

size_t json_reader::read_length()
{
	json_value_s* jvalue = m_frames.top().get_value();

	switch (jvalue->type)
	{
		case json_type_e::json_type_string: return json_value_as_string(jvalue)->string_size;
		case json_type_e::json_type_array:  return json_value_as_array (jvalue)->length;
	}

	assert(false && "json type has no size");
	return 0;
}

void json_reader::read_bytes(char* bytes, size_t length)
{
	json_value_s* jvalue = m_frames.top().get_value();

	switch (jvalue->type)
	{
		case json_type_e::json_type_string:
		{
			json_string_s* jstring = json_value_as_string(jvalue);
			memcpy(bytes, jstring->string, length);

			log_io("JSON: read string %s", jstring->string);
			break;
		}

		case json_type_e::json_type_number:
		{
			json_number_s* jnumber = json_value_as_number(jvalue);
			meta::type* type = m_frames.top().type;

			if (type->info()->m_is_floating)
			{		
				     if (meta::is_same<     float> (type)) *(      float*)bytes = std::stof (jnumber->number);
				else if (meta::is_same<     double>(type)) *(     double*)bytes = std::stod (jnumber->number);
				else if (meta::is_same<long double>(type)) *(long double*)bytes = std::stold(jnumber->number);
							
				else
				{
					assert(false && "json floating value had a invalid size");
				}
			}
			
			else
			{
				unsigned long long largest = std::stoull(jnumber->number);
				memcpy(bytes, &largest, type->info()->m_size);
			}

			log_io("JSON: read number %s", jnumber->number);
			break;
		}

		case json_type_e::json_type_false:
		{
			*bytes = (char)false;

			log_io("JSON: read false");
			break;
		}

		case json_type_e::json_type_true:
		{
			*bytes = (char)true;

			log_io("JSON: read true");
			break;
		}

		//case json_type_e::json_type_object: // why was this nessesary?
		//{
		//	meta::type* type = m_frames.top().type;
		//	read_class(type, bytes);

		//	log_io("JSON: read object");
		//	break;
		//}

		default:
		{
			assert(false && "json type wasnt valid");
		}
	}
}
