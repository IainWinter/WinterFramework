#include "ext/serial/serial.h"

namespace meta
{
    //
    //  read/writers
    //

    void write_string(serial_writer* writer, const std::string& instance)
    {
        writer->write_length(instance.size());
        writer->write_string(instance.data(), instance.size());
    }

    void read_string(serial_reader* reader, std::string& instance)
    {
        instance.resize(reader->read_length());
        reader->read_string(instance.data(), instance.size());
    }

    void write_any(serial_writer* serial, const any& value)
    {
        serial->class_begin(get_class<any>());
        serial->write_member(get_class<id_type>(), "type", &value.type()->info()->m_id);
        serial->class_delim();
        serial->write_member(value.type(), "data", value.data());
        serial->class_end();
    }

    void read_any(serial_reader* serial, any& value)
    {
        id_type id;

        serial->class_begin(get_class<any>());
        serial->read_member(get_class<id_type>(), &id);

        serial->class_delim();

        value = get_registered_type(id)->construct();

        serial->read_member(value.type(), value.data());
        serial->class_end();
    }

	//
	//	context
	//

	serial_context* ctx;

	void create_context()
	{
		destroy_context();
		ctx = new serial_context();

		describe<any>()
			.custom_write(&write_any)
			.custom_read(&read_any);
        
        describe<std::string>()
            .custom_write(&write_string)
            .custom_read(&read_string);
	}

	void destroy_context()
	{
		delete ctx;
	}

	serial_context* get_context()
	{
		return ctx;
	}

	void set_current_context(serial_context* context)
	{
		ctx = context;
	}

	bool has_registered_type(id_type id)
	{
		return ctx->known_info.find(id) != ctx->known_info.end();
	}

	void register_type(id_type type_id, type* type)
	{
		if (has_registered_type(type_id))
		{
			delete ctx->known_info[type_id];
		}

		ctx->known_info[type_id] = type;
	}

	type* get_registered_type(id_type type_id)
	{
		return ctx->known_info[type_id];
	}
}
