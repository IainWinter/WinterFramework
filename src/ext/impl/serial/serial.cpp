#include "ext/serial/serial.h"

namespace meta
{
	//
	//	serial_writer
	//

	void serial_writer::write_class(meta::type* type, const void* instance)
	{
		if (type->has_custom_write() || !type->has_members()) // custom write / no members
		{
			type->_serial_write(this, instance);
		}

		else // objects
		{
			class_begin(type);

			auto& members = type->get_members();
			for (int i = 0; i < members.size(); i++)
			{
				meta::type* member = members.at(i);

				write_member(member, member->walk_ptr(instance), member->member_name());

				if (i != members.size() - 1)
				{
					class_delim();
				}
			}

			class_end();
		}
	}

	void serial_writer::write_member(meta::type* type, const void* instance, const char* name)
	{
		member_begin(type, name);
		write_class(type, instance);
		member_end();
	}

	void serial_writer::write_array(meta::type* type, const void* instance, size_t length)
	{
		array_begin(type, length);

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

	void serial_writer::write_string(const char* string, size_t length)
	{
		string_begin(length);
		write_bytes(string, length);
		string_end();
	}

	pseudo_writer serial_writer::pseudo()
	{
		return pseudo_writer(this);
	}

	//
	//	serial_reader
	//

	void serial_reader::read_class(type* type, void* instance)
	{
		if (type->has_custom_read() || !type->has_members()) // custom read / no members
		{
			type->_serial_read(this, instance);
		}

		else // objects
		{
			class_begin(type);

			auto& members = type->get_members();
			for (int i = 0; i < members.size(); i++)
			{
				meta::type* member = members.at(i);

				read_member(member, member->walk_ptr(instance), member->member_name());

				if (i != members.size() - 1)
				{
					class_delim();
				}
			}

			class_end();
		}
	}

	void serial_reader::read_member(type* type, void* instance, const char* name)
	{
		member_begin(type, name);
		read_class(type, instance);
		member_end();
	}

	void serial_reader::read_array(type* type, void* instance, size_t length)
	{
		array_begin(type, length);

		for (int i =  0; i < length; i++)
		{
			read_class(type, (char*)instance + i * type->info()->m_size);

			if (i != length - 1)
			{
				array_delim();
			}
		}

		array_end();
	}

	void serial_reader::read_string(char* string, size_t length)
	{
		string_begin(length);
		read_bytes(string, length);
		string_end();
	}

	pseudo_reader serial_reader::pseudo()
	{
		return pseudo_reader(this);
	}

    //
    //	custom read/writers
    //

    void write_string(serial_writer* writer, const std::string& instance)
    {
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
        serial->write_member(get_class<id_type>(), &value.type()->info()->m_id, "type");
        serial->class_delim();
        serial->write_member(value.type(), value.data(), "data");
        serial->class_end();
    }

    void read_any(serial_reader* serial, any& value)
    {
        id_type id;

        serial->class_begin(get_class<any>());
        serial->read_member(get_class<id_type>(), &id, "type");

        serial->class_delim();

        value = get_registered_type(id)->construct();

        serial->read_member(value.type(), value.data(), "data");
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
			//	delete types made in _get_class. This seems like an odd place for delete
			//
			delete ctx->known_info[type_id];
		}

		ctx->known_info[type_id] = type;
	}

	type* get_registered_type(id_type type_id)
	{
		if (!has_registered_type(type_id))
		{
			return nullptr;
		}

		return ctx->known_info.at(type_id);
	}
}
