#include "ext/serial/serial.h"

#include "Log.h"

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
		std::string name = value.type()->info()->m_name; // could write just the bytes with a custom read/write

        serial->class_begin(get_class<any>());
        serial->write_member(get_class<std::string>(), &name, "name");
        serial->class_delim();
        serial->write_member(value.type(), value.data(), "data");
        serial->class_end();
    }

    void read_any(serial_reader* serial, any& value)
    {
        std::string name;

        serial->class_begin(get_class<any>());
        serial->read_member(get_class<std::string>(), &name, "name");

        serial->class_delim();

		if (has_registered_type(name.c_str())) // I think this screws the bin reader, not the json through. todo: put size of type to fix this
		{
			value = get_registered_type(name.c_str())->construct();
			serial->read_member(value.type(), value.data(), "data");
		}

        serial->class_end();
    }

	//
	//	context
	//

	wContextImpl(serial_context);

	void serial_context::Realloc(int location)
	{
		std::vector<id_type> removeThese;

		for (auto& [id, reg] : ctx->known_info)
		{
			if (location == reg.location)
			{
				continue;
			}

			// for now just delete and assume that they will be re-registered
			// this doesnt work because when calling a virtual function we get passed into the dll
			// stack

			// to get around this, use a custom allocator or alloc memory here

			removeThese.push_back(id);
		}

		for (const id_type& id : removeThese)
		{
			free_registered_type(id);
		}
	}

	void register_meta_types()
	{
		describe<any>()
			.custom_write(&write_any)
			.custom_read(&read_any);
        
        describe<std::string>()
            .custom_write(&write_string)
            .custom_read(&read_string);
	}

	void register_type(id_type type_id, type* type)
	{
		auto& reg = ctx->known_info[type_id];

		if (has_registered_type(type_id))
		{
			//	delete types made in _get_class. This seems like an odd place for delete
			//
			delete reg.type;
		}

		reg.type = type;
		reg.location = GetContextLocation();
	}

	bool has_registered_type(id_type id)
	{
		return ctx->known_info.find(id) != ctx->known_info.end();
	}

	type* get_registered_type(id_type type_id)
	{
		if (!has_registered_type(type_id))
		{
			return nullptr;
		}

		return ctx->known_info.at(type_id).type;
	}

	bool has_registered_type(const char* name)
	{
		return get_registered_type(name) != nullptr;
	}

	type* get_registered_type(const char* name)
	{
		for (const auto& [id, reg] : ctx->known_info)
		{
			if (strcmp(reg.type->name(), name) == 0)
			{
				return reg.type;
			}
		}

		return nullptr;
	}

	void free_registered_type(id_type type_id)
	{
		if (has_registered_type(type_id))
		{
			delete ctx->known_info[type_id].type;
			ctx->known_info.erase(type_id);
		}
	}
}
