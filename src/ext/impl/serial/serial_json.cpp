#include "ext/serial/serial_json.h"
#include <cstring>


#include "json/json.h" // only reads

#ifdef SERIAL_JSON_LOG_DEBUG
#	include "Log.h"
#endif

//
//		Writer
//

json_writer::json_writer(std::ostream& out)
	: meta::serial_writer(out, false)
{}

void json_writer::write_class(meta::type* type, const void* instance)
{
	if (type->has_custom_write() || !type->has_members())
	{
		// either custom write, or write the value to the stream with <<

		type->_serial_write(this, instance);
	}

	else // json objects
	{
		class_begin(type);

		auto& members = type->get_members();
		for (int i = 0; i < members.size(); i++)
		{
			meta::type* member = members.at(i);

			write_member(member, member->member_name(), member->walk_ptr(instance));

			if (i != members.size() - 1)
			{
				class_delim();
			}
		}

		class_end();
	}
}

void json_writer::write_member(meta::type* type, const char* name, const void* instance)
{
	m_out << '"' << name << '"' << ':';
	write_class(type, instance);
}

void json_writer::write_array(meta::type* type, const void* instance, size_t length)
{
	array_begin();

	for (int i =  0; i < length; i++)
	{
		write_class(type, (char*)instance + i * type->info()->m_size);

		if (i != length - 1)
		{
			array_delim();
		}
	}

	array_end();
}

void json_writer::write_string(const char* string, size_t length)
{
	string_begin();

	for (size_t i = 0; i < length; i++)
	{
		m_out << string[i];
		if (string[i] == '\\') m_out << '\\'; // escape slash? figure out where to do this
	}

	string_end();
}

void json_writer::write_length(size_t length)
{
	// do nothing
}

void json_writer::class_begin(meta::type* type) { m_out << '{'; }
void json_writer::class_end()   { m_out << '}'; }
void json_writer::class_delim() { m_out << ','; }

void json_writer::array_begin() { m_out << '['; }
void json_writer::array_delim() { m_out << ','; }
void json_writer::array_end()   { m_out << ']'; }

void json_writer::string_begin() { m_out << '"'; }
void json_writer::string_end()   { m_out << '"'; }

//
//		Reader
//

json_reader::json_reader(std::istream& in)
	: meta::serial_reader (in, false)
	, m_json              (nullptr)
{
	init_json();
}

bool json_reader::is_valid() const
{
	return m_json != nullptr;
}

void json_reader::init_json()
{
	std::stringstream ss; ss << m_in.rdbuf();
	std::string str = ss.str(); // does this do a big copy?

	m_json = json_parse(str.c_str(), str.size());
}

void json_reader::read_class(meta::type* type, void* instance)
{
	if (!is_valid())
	{
		printf("Error with json reader, invaild json\n");
		return;
	}

	switch (m_json->type)
	{
		case json_type_object: // recurse if complex
		{
			class_begin(type);

			obj_frame& curr = m_objs.top();

			while (curr.current_member_json)
			{
				meta::type* curtype = *curr.current_member_type;
				read_member(curtype, curtype->walk_ptr(instance));
			}

			class_end();

			break;
		}
		case json_type_string: // custom handler for strings and arrays
		case json_type_array:
		{
			type->_serial_read(this, instance);
			break;
		}
		case json_type_false: // read value directly for base types, skip read_value
		case json_type_true:
		{
			*(bool*)instance = m_json->payload;
			break;
		}
		case json_type_number:
		{
			json_number_s* number = json_value_as_number(m_json);

			if (type->info()->m_is_floating)
			{
				// no switch bc types sizes could be equal

				size_t s = type->info()->m_size;

				     if (s == sizeof(      float)) *(      float*)instance = std::stof (number->number);
				else if (s == sizeof(     double)) *(     double*)instance = std::stod (number->number);
				else if (s == sizeof(long double)) *(long double*)instance = std::stold(number->number);
				
				else
				{
					assert(false && "json floating value had a invalid size");
				}
			}

			else
			{
				unsigned long long largest = std::stoull(number->number);
				memcpy(instance, &largest, type->info()->m_size);
			}

			break;
		}
		case json_type_null:
		default:
			break;
	}
}

void json_reader::read_member(meta::type* type, void* instance)
{
	obj_frame& curr = m_objs.top();
#ifdef SERIAL_JSON_LOG_DEBUG
	log_io("Read Json member: %s", curr.current_member_json->name->string);
#endif
	m_json = curr.current_member_json->value;

	type->_serial_read(this, instance);

	curr.current_member_json = curr.current_member_json->next;
	if (curr.real_members_left > 0)
	{
		++curr.current_member_type;
		--curr.real_members_left;
	}
}

void json_reader::read_array(meta::type* type, void* instance, size_t length)
{
#ifdef SERIAL_JSON_LOG_DEBUG
	log_io("Read Json array begin: %s x %d", type->name(), length);
#endif

	json_array_s* arr = json_value_as_array(m_json);
	
	assert(arr && "json type was not array");
	assert(arr->length == length && "json array length did not match reader length");

	json_array_element_s* element = arr->start;

	for (int i = 0 ; i < arr->length; i++)
	{
		m_json = element->value;
		element = element->next;
		type->_serial_read(this, (char*)instance + i * type->info()->m_size);
	}
}

void json_reader::read_string(char* string, size_t length)
{
	json_string_s* out = json_value_as_string(m_json);
	
	assert(out && "json type was not string");
	assert(out->string_size == length && "json string length did not match reader length");

	memcpy(string, out->string, out->string_size);
}

size_t json_reader::read_length() // returns the length of the current string or array
{
	if (m_json->type == json_type_array)
	{
		return json_value_as_array(m_json)->length;
	}

	if (m_json->type == json_type_string)
	{
		return json_value_as_string(m_json)->string_size;
	}

	assert(false && "json was not an array or string");
	return 0;
}

void json_reader::class_begin(meta::type* type)
{
#ifdef SERIAL_JSON_LOG_DEBUG
	log_io("Read Json class begin: %s", type->name());
#endif

	json_object_s* object = json_value_as_object(m_json);

	obj_frame f;
	f.current_type = type;

	assert(object && "json type want not object");
	assert(object->length == f.current_type->get_members().size()
		|| f.current_type->has_custom_read() && "json object length did not match reader length, or hasnt been marked with a custom read");

	f.current_member_type = f.current_type->get_members().begin();
	f.current_member_json = object->start;
	f.real_members_left = type->get_members().size();

	m_objs.push(f);
}

void json_reader::class_delim()
{
	// nothing
}

void json_reader::class_end()
{
#ifdef SERIAL_JSON_LOG_DEBUG
	log_io("Read Json class end: %s", m_objs.top().current_type->name());
#endif

	m_objs.pop();
}
	
void json_reader::array_begin()
{
#ifdef SERIAL_JSON_LOG_DEBUG
	log_io("Read Json array end");
#endif
}

void json_reader::array_delim()
{

}

void json_reader::array_end()
{

}
	
void json_reader::string_begin()
{

}

void json_reader::string_end()
{

}