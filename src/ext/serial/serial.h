#pragma once

#include <vector>
#include <unordered_map>
#include <ostream>
#include <istream>
#include <assert.h>

#define SERIAL_ENTT_TIE_IN

#ifdef SERIAL_ENTT_TIE_IN
	#include "entt/meta/resolve.hpp"
	#include "entt/meta/factory.hpp"
	#include "entt/entity/registry.hpp"

	using namespace entt::literals;

#define REG_STORAGE 45634

	namespace meta
	{
		using id_type = entt::id_type;
	}
#else
	namespace meta
	{
		using id_type = size_t;
	}
#endif

namespace meta
{
	template<typename _t>
	_t* as(void* instance)
	{
		return (_t*)instance;
	}

	// put custom ping functions here

	template<typename _t>
	void ping(void* userdata, int message)
	{
#ifdef SERIAL_ENTT_TIE_IN
		
		switch (message)
		{
			case REG_STORAGE:
				as<entt::registry>(userdata)->storage<_t>();
				break;
		}

#endif
	}
}

// ! This doesnt support pointers !
// wrap in a std container and provide custom behaviour to your needs

namespace meta
{
	//
	//	template for getting type of auto template args
	//

	template <typename _m>
	struct _ptrtype // https://stackoverflow.com/a/22851790
	{
		template <typename _t, typename _internal>
		static _internal get_type(_internal _t::*);
		typedef decltype(get_type(static_cast<_m>(nullptr))) type;
	};

	template<auto _m>
	using ptrtype = typename _ptrtype<decltype(_m)>::type;

	//
	//	type info, gets cached in type / serial_context
	//

	struct type_info
	{
		const char* m_name;
		id_type m_id;

		size_t m_size;

		bool m_is_floating;
		bool m_is_integral;
		
		bool m_is_structure;

		bool m_has_custom_write;
		bool m_has_custom_read;
	};

	template<typename _t>
	type_info* _make_type_info(id_type id)
	{
		type_info* info = new type_info();
		info->m_name = typeid(_t).name();
		info->m_id = id;
		info->m_size = sizeof(_t);
		info->m_is_floating  = std::is_floating_point<_t>::value;
		info->m_is_integral  = std::is_integral<_t>::value;
		info->m_is_structure = !info->m_is_floating && !info->m_is_integral;

		info->m_has_custom_write = false;
		info->m_has_custom_read = false;

		return info;
	}

	template<>
	inline type_info* _make_type_info<void>(id_type id)
	{
		type_info* info = new type_info();
		info->m_name = "void";
		info->m_id = id;
		info->m_size = 0;
		info->m_is_floating = false;
		info->m_is_integral = false;
		info->m_is_structure = false;
		info->m_has_custom_write = false;
		info->m_has_custom_read = false;

		return info;
	}

	//
	// serial forward declare
	//

	class serial_writer;
	class serial_reader;
	struct any;

	template<typename _t> class describe;
	template<typename _t> class _class_type;
	template<typename _t, auto _m> class _class_member;

	// represents a type, treats all data the same, ie. an int/float are a class with 0 members
	// types with no members can have custom serializers so can still be saved/loaded
	//
	class type
	{
	protected:
		type_info* m_info;

	public:
		type(type_info* info)
			: m_info (info)
		{}

		// getters

		const type_info* info() const { return m_info; }
		const char* name() const { return m_info->m_name; }
		bool has_custom_write() const { return m_info->m_has_custom_write; }
		bool has_custom_read()  const { return m_info->m_has_custom_read; }

		virtual bool is_member() const = 0;

		// asserts m_has_members = true
		virtual const std::vector<type*>& get_members() const = 0;

		// asserts m_is_member = true
		virtual const char* member_name() const = 0;

		// walks a pointer forward to a member of the type
		// instance should be of the same type as the type
		virtual void* walk_ptr(const void* instance) const = 0;

		// calls serial_write with the correct templated type
		// instance should already be walked...
		virtual void _serial_write(serial_writer* serial, const void* instance) const = 0;

		// calls serial_read with the correct templated type
		// instance should already be walked...
		virtual void _serial_read(serial_reader* serial, void* instance) const = 0;
		
		// create a new instance of a class
		virtual any construct() const = 0;

		// if the type is a member, returns an any with the min value hint
		virtual const any& min() const = 0;

		// if the type is a member, returns an any with the max value hint
		virtual const any& max() const = 0;
		
		// if the type is a member, set the min value. value in any must be castable to the member type
		virtual void set_min(const any& value) = 0;

		// if the type is a member, set the max value. value in any must be castable to the member type
		virtual void set_max(const any& value) = 0;

		// this calls the function meta::ping specialized with the underlying type
		virtual void ping(void* userdata = nullptr, int message = 0) const = 0;

		// helpers

		bool has_members() const
		{
			return get_members().size() > 0;
		}
	};

	// return the type of the template arg _t
	// if a type has not been registers, it describes a new class with default options
	template<typename _t>
	type* get_class();

	//
	// simple any class
	//

	struct any_storage
	{
	protected:
		meta::type* m_type;
		bool m_owns;

	public:
		any_storage(meta::type* type, bool owns)
			: m_type (type)
			, m_owns (owns)
		{}

		virtual ~any_storage() {}

		virtual const void* data() const = 0;
		virtual void* data() = 0;
		virtual any_storage* copy() const = 0;
		virtual any_storage* new_copy() const = 0;
		virtual bool is_type(meta::type* type) const = 0;

		meta::type* type() const
		{
			return m_type;
		}

		bool is_ref() const
		{
			return m_owns;
		}
	};

	template<typename _t>
	struct any_storage_t : any_storage
	{
		_t* m_instance;

		any_storage_t(meta::type* type, bool owns, _t* instance)
			: any_storage (type, owns)
			, m_instance  (instance)
		{
			if (m_owns)
			{
				m_instance = new _t(*instance);
			}
		}

		~any_storage_t()
		{
			if (m_owns)
			{
				delete m_instance;
			}

			m_instance = nullptr;
		}

		const void* data() const override
		{
			return (const void*)m_instance;
		}

		void* data() override
		{
			return (void*)m_instance;
		}

		any_storage* copy() const override
		{
			return new any_storage_t<_t>(m_type, m_owns, m_instance);
		}

		any_storage* new_copy() const override
		{
			return new any_storage_t<_t>(m_type, true, m_instance);
		}

		bool is_type(meta::type* type) const override
		{
			return get_class<_t>()->info()->m_id == type->info()->m_id;
		}
	};

	template<>
	struct any_storage_t<void> : any_storage
	{
		void* m_instance;

		any_storage_t(meta::type* type, void* instance)
			: any_storage (type, false)
			, m_instance  (instance)
		{}

		const void* data() const override
		{
			return m_instance;
		}

		void* data() override
		{
			return m_instance;
		}

		any_storage* copy() const override
		{
			return new any_storage_t<void>(m_type, m_instance);
		}

		any_storage* new_copy() const override
		{
			throw nullptr; // this makes no sense for void*
		}

		bool is_type(meta::type* type) const override
		{
			return get_class<void>()->info()->m_id == type->info()->m_id;
		}
	};

	struct any
	{
		any_storage* m_storage;

		any()
			: m_storage (nullptr)
		{}

		//	makes a refernce to instance
		// 
		any(type* type, void* instance)
		{
			m_storage = new any_storage_t<void>(type, instance);
		}

		// makes a reference or a copy of instance based on 'owns'
		//
		template<typename _t>
		any(_t& instance, bool owns)
		{
			m_storage = new any_storage_t<_t>(get_class<_t>(), owns, &instance);
		}

		~any()
		{
			delete m_storage;
		}

		// copy and move

		any(const any& copy)
		{
			m_storage = copy.m_storage->copy();
		}

		any& operator=(const any& copy)
		{
			m_storage = copy.m_storage->copy();

			return *this;
		}

		any(any&& move) noexcept
		{
			m_storage = move.m_storage;
			move.m_storage = nullptr;
		}

		any& operator=(any&& move) noexcept
		{
			m_storage = move.m_storage;
			move.m_storage = nullptr;

			return *this;
		}

		void copy_own(const any& other)
		{
			m_storage = other.m_storage->new_copy();
		}

		type* type() const { return m_storage->type(); }
		void* data() const { return m_storage->data(); }
		id_type type_id() const { return type()->info()->m_id; }
		bool has_data() const { return !!m_storage; }

		template<typename _t>
		_t& as()
		{
			return *(_t*)m_storage->data();
		}

		template<typename _t>
		const _t& as() const
		{
			return *(const _t*)m_storage->data();
		}

		template<typename _t>
		bool is_type() const
		{
			return m_storage->is_type(get_class<_t>());
		}
	};

	//
	//	context for saving types. the main purpose of this is to pass it across dll bounds
	//

	struct serial_context
	{
		std::unordered_map<id_type, type*> known_info;
	};

	void create_context();
	void destroy_context();
	serial_context* get_context();
	void set_current_context(serial_context* context);

	bool has_registered_type(id_type type_id);
	void register_type(id_type type_id, type* type);
	type* get_registered_type(id_type type_id);

	//
	// serialization
	//

	class serial_writer
	{
	protected:
		std::ostream& m_out;
		const bool m_binary;

		//struct pointer_ref
		//{
		//	type* type;
		//	void* instance;
		//};

		//std::vector<pointer_ref> m_pointer_table;

	public:
		serial_writer(std::ostream& out, bool binary)
			: m_out    (out)
			, m_binary (binary)
		{}

		// public call this
		//
		template<typename _t>
		void write(const _t& value)
		{
			write_class(get_class<_t>(), (const void*)&value);
		}

		virtual void write_class (type* type, const void* instance) = 0;
		virtual void write_member(type* type, const char* name, const void* instance) = 0;
		virtual void write_array (type* type, const void* instance, size_t length) = 0;
		virtual void write_string(const char* string, size_t length) = 0;

		virtual void write_length(size_t length) = 0;

		virtual void class_begin(type* type) = 0;
		virtual void class_delim() = 0;
		virtual void class_end() = 0;

		virtual void array_begin() = 0;
		virtual void array_delim() = 0;
		virtual void array_end() = 0;

		virtual void string_begin() = 0;
		virtual void string_end() = 0;

		template<typename _t>
		void write_value(const _t& value)
		{
			if (m_binary)
			{
				write_string((const char*)&value, sizeof(_t));
			}

			else if constexpr (std::is_integral<_t>::value || std::is_floating_point<_t>::value) // should just test for << op
			{
				m_out << value;
			}

			// should assert
		}
	};

	class serial_reader
	{
	protected:
		std::istream& m_in;
		const bool m_binary;

	public:
		serial_reader(std::istream& in, bool binary)
			: m_in     (in)
			, m_binary (binary)
		{}

		template<typename _t>
		void read(_t& value)
		{
			read_class(meta::get_class<_t>(), &value);
		}

		virtual void read_class (type* type, void* instance) = 0;
		virtual void read_member(type* type, void* instance) = 0;
		virtual void read_array (type* type, void* instance, size_t length) = 0;
		virtual void read_string(char* string, size_t length) = 0;
		
		virtual size_t read_length() = 0;

		virtual void class_begin(type* type) = 0;
		virtual void class_delim() = 0;
		virtual void class_end() = 0;

		virtual void array_begin() = 0;
		virtual void array_delim() = 0;
		virtual void array_end() = 0;

		virtual void string_begin() = 0;
		virtual void string_end() = 0;

		//// this should recurse down the tree of type

		//virtual void read_type(type* type, void* instance) = 0;

		//// these are special conditions

		//// read a single member of a type,
		//// type must be a member and current
		//// iteration has to have correct context
		//virtual void read_member(type* type, void* instance) = 0;

		//virtual size_t read_length() = 0;
		//virtual void read_array(type* type, void* instance, size_t repeat) = 0;
		//virtual void read_string(char* str, size_t length) = 0;

		template<typename _t>
		void read_value(_t& value)
		{
			if (m_binary)
			{
				read_string((char*)&value, sizeof(_t));
			}

			else if constexpr (std::is_integral<_t>::value || std::is_floating_point<_t>::value) // should just test for << op
			{
				m_in >> value;
				// this cant be here because of json.h needs to read the values
				// reader seems to be able to handle the input in a way to never call this
			}

			// could assert for not reading
		}
	};

	// default behaviour is to just write the value
	// specialize these for custom types

	template<typename _t>
	void serial_write(serial_writer* serial, const _t& value)
	{
		// this has to write the value because of type erasure

		serial->write_value(value);
	}

	template<typename _t>
	void serial_read(serial_reader* serial, _t& value)
	{
		// this doesnt need to read the value because the
		// json reader know the type, and the bin reader doesnt care
		// about types

		// main reason is that the json tree needs to be walked
		// and if we were to read the value, it would read from the stream
		// which is not correct

		// seems a little funky to have these be different, but this is how the
		// logic seems to work out...

		serial->read(value);
	}

	// 
	// internal, you never have to directly use any of these classes
	//

	template<typename _t>
	class _class_type;

	// internal representation of a class member
	//
	template<typename _t, auto _m>
	class _class_member : public type
	{
	public:
		using _mtype = ptrtype<_m>;

	private:
		_class_type<_mtype>* m_class;
		const char* m_name;

		// mainly for imgui display these set the range
		// doesnt make sense for all types
		// these are stored as any to lazy init them
		any m_min;
		any m_max;

	public:
		_class_member(_class_type<_mtype>* member_class_type, const char* member_name)
			: type    (member_class_type->info())
			, m_class (member_class_type)
			, m_name  (member_name)
		{}

		bool is_member() const override
		{
			return true;
		}

		const std::vector<type*>& get_members() const override
		{
			return m_class->get_members();
		}

		const char* member_name() const override
		{
			return m_name;
		}

		void* walk_ptr(const void* instance) const override
		{
			return (void*) & (((const _t*)instance)->*_m);
		}

		void _serial_write(serial_writer* serial, const void* instance) const override
		{
			serial_write(serial, *(const _mtype*)instance);
		}

		void _serial_read(serial_reader* serial, void* instance) const override
		{
			serial_read(serial, *(_mtype*)instance);
		}

		any construct() const override
		{
			return m_min;
		}

		const any& min() const override
		{
			return m_min;
		}

		const any& max() const override
		{
			return m_max;
		}

		void set_min(const any& value) override
		{
			m_min.copy_own(value);
		}

		void set_max(const any& value) override
		{
			m_max.copy_own(value);
		}

		void ping(void* userdata = nullptr, int message = 0) const override
		{
			meta::ping<_mtype>(userdata, message);
		}
	};

	// internal representation of a class
	//
	template<typename _t>
	class _class_type : public type
	{
	private:
		std::vector<type*> m_members;

	public:
		_class_type(type_info* info)
			: type (info)
		{}

		~_class_type()
		{
			for (type* m : m_members) delete m;
		}

		// setters / getters for internal

		type_info* info() { return m_info; }
		void set_custom_write() { m_has_custom_write = true; }
		void set_custom_read()  { m_has_custom_read = true; }

		template<auto _m>
		void add_member(const char* name)
		{
			m_members.push_back(
				new _class_member<_t, _m>(_get_class<ptrtype<_m>>(), name)
			);
		}

		bool is_member() const override
		{
			return false;
		}

		const std::vector<type*>& get_members() const override
		{
			return m_members;
		}

		const char* member_name() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void* walk_ptr(const void* instance) const override
		{
			return (void*)instance;
		}

		void _serial_write(serial_writer* serial, const void* instance) const override
		{
			serial_write(serial, *(const _t*)instance);
		}

		void _serial_read(serial_reader* serial, void* instance) const override
		{
			serial_read(serial, *(_t*)instance);
		}

		any construct() const override
		{
			_t t = _t();
			return any(t, true);
		}

		const any& min() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		const any& max() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void set_min(const any& value) override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void set_max(const any& value) override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void ping(void* userdata = nullptr, int message = 0) const override
		{
			meta::ping<_t>(userdata, message);
		}
	};

	template<>
	class _class_type<void> : public type
	{
	public:
		_class_type(type_info* info)
			: type (info)
		{}

		// setters / getters for internal

		type_info* info() { return m_info; }

		bool is_member() const override { return false; }

		const std::vector<type*>& get_members() const override { return {}; }

		const char* member_name() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void* walk_ptr(const void* instance) const override
		{
			assert(false && "type was void");
			throw nullptr;
		}

		void _serial_write(serial_writer* serial, const void* instance) const override {}
		void _serial_read(serial_reader* serial, void* instance) const override {}
		any construct() const override { return any((type*)this, nullptr); }

		const any& min() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		const any& max() const override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void set_min(const any& value) override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void set_max(const any& value) override
		{
			assert(false && "type was not a member");
			throw nullptr;
		}

		void ping(void* userdata = nullptr, int message = 0) const override { }
	};

	template<typename _t>
	_class_type<_t>* _get_class()
	{
		id_type id = 0;

#ifdef SERIAL_ENTT_TIE_IN
		id = entt::type_id<_t>().hash();
#else
		id = typeid(_t).hash_code();
#endif

		if (!has_registered_type(id))
		{
			type* type = new _class_type<_t>(_make_type_info<_t>(id));
			register_type(id, type);
		}

		return (_class_type<_t>*)get_registered_type(id);
	}

	//
	//	public API
	//

	// a builder class that allows the registration of classes and their members
	//
	template<typename _t>
	class describe
	{
	private:
		_class_type<_t>* m_current = _get_class<_t>();

	public:
		// set the name of the class
		describe<_t>& name(const char* name)
		{
			m_current->info()->m_name = name;
			return *this;
		}

		// add a member to the class
		template<auto _m>
		describe<_t>& member(const char* name)
		{
			m_current->add_member<_m>(name);
			return *this;
		}

		// for the last member added, set the min value
		// assumes at least one member has been added
		template<typename _v>
		describe<_t>& min(_v value)
		{
			m_current->get_members().back()->set_min(any(value, false));
			return *this;
		}

		// for the last member added, set the max value
		// assumes at least one member has been added
		template<typename _v>
		describe<_t>& max(_v value)
		{
			m_current->get_members().back()->set_max(any(value, false));
			return *this;
		}

		// mark this class as having a custom writer,
		// this is needed for the serial_writers to pick up if the class has members
		// but doesnt want default serialization behaviour
		describe<_t>& has_custom_write()
		{
			m_current->info()->m_has_custom_write = true;
			return *this;
		}

		// mark this class as having a custom readear,
		// this is needed for the serial_readers to pick up if the class has members
		// but doesnt want default deserialization behaviour
		describe<_t>& has_custom_read()
		{
			m_current->info()->m_has_custom_read = true;
			return *this;
		}

		// return the type
		type* get() const
		{
			return m_current;
		}
	};

	template<typename _t>
	type* get_class()
	{
		return _get_class<_t>();
	}

	template<typename _t>
	const char* name()
	{
		return get_class<_t>()->name();
	}

	template<typename _t>
	id_type id()
	{
		return get_class<_t>()->info()->m_id;
	}

	template<typename _t>
	type_info* info()
	{
		return get_class<_t>()->m_info;
	}
}

// this is so you can call meta::from_entt(entt::type_id) -> meta::type*

#ifdef SERIAL_ENTT_TIE_IN

namespace meta
{
	inline type* from_entt(entt::id_type id)
	{
		return get_registered_type(id);
	}
}

#endif

//
//		default custom writers
//

#include <string>

namespace meta
{
	template<>
	inline void serial_write(serial_writer* writer, const std::string& instance)
	{
		writer->write_length(instance.size());
		writer->write_string(instance.data(), instance.size());
	}

	template<>
	inline void serial_read(serial_reader* reader, std::string& instance)
	{
		instance.resize(reader->read_length());
		reader->read_string(instance.data(), instance.size());
	}

	template<typename _t> void serial_write(serial_writer* writer, const std::vector<_t>& instance)
	{
		writer->write_length(instance.size());
		writer->write_array(meta::get_class<_t>(), (void*)instance.data(), instance.size());
	}

	template<typename _t> void serial_read(serial_reader* reader, std::vector<_t>& instance)
	{
		instance.resize(reader->read_length());
		reader->read_array(meta::get_class<_t>(), instance.data(), instance.size());
	}
	
	template<>
	inline void serial_write(serial_writer* serial, const any& value)
	{
		serial->class_begin(get_class<any>());
		serial->write_member(get_class<id_type>(), "type", &value.type()->info()->m_id);
		serial->class_delim();
		serial->write_member(value.type(), "data", value.data());
		serial->class_end();
	}

	template<>
	inline void serial_read(serial_reader* serial, any& value)
	{
		id_type id;

		serial->class_begin(get_class<any>());
		serial->read_member(get_class<id_type>(), &id);

		serial->class_delim();

		value = get_registered_type(id)->construct();

		serial->read_member(value.type(), value.data());
		serial->class_end();
	}
}