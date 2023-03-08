#pragma once

#include <vector>
#include <unordered_map>
#include <ostream>
#include <istream>
#include <assert.h>
#include <stack>

//#include "util/comparable_str.h"
#include "util/pool_allocator.h"
#include "util/context.h"
#include <string>

//#include "util/graph.h"

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
			// register storage for types not seen by entt
			case REG_STORAGE:
				as<entt::registry>(userdata)->storage<_t>();
				break;
		}

#endif
	}
}

// ! This doesn't support pointers !
// wrap in a std container and provide custom behavior to your needs

namespace meta
{
	//
	// serial forward declare
	//

	class serial_writer;
	class serial_reader;
	class pseudo_writer;
	class pseudo_reader;
	struct any;

	template<typename _t> class describe;
	template<typename _t> class _class_type;
	template<typename _t, auto _m> class _class_member;

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
	//	template for detecting if a function exists on type
	//

	using y = std::true_type;
	using n = std::false_type;

	template <class _t>
	struct has_custom_write
	{
	private:
		template <typename _c, _c> struct check;
		template <typename _c> static y test(check<void(_c::*)(serial_writer*) const, &_c::serial_write>*);
		template <typename>    static n test(...);
	public:
		static const bool value = std::is_same<decltype(test<_t>(0)), y>::value;
	};

	template <class _t>
	struct has_custom_read
	{
	private:
		template <typename _c, _c> struct check;
		template <typename _c> static y test(check<void(_c::*)(serial_reader*), &_c::serial_read>*);
		template <typename>    static n test(...);
	public:
		static const bool value = std::is_same<decltype(test<_t>(0)), y>::value;
	};

	// can these be combined?

	//
	//	type info, gets cached in type / serial_context
	//

	struct type_info
	{
		// store this in a string as types need to be realloced sometimes
		std::string m_name;
		id_type m_id;

		size_t m_size;

		bool m_is_floating;
		bool m_is_integral;
		
		bool m_is_complex;

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
		info->m_is_complex = std::is_class<_t>::value;

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
		info->m_is_complex = false;

		return info;
	}

	// represents a type, treats all data the same, ie. an int/float are a class with 0 members
	// types with no members can have custom serializes so can still be saved/loaded
	//
	class type
	{
	protected:
		type_info* m_info;

	public:
		type(type_info* info)
			: m_info (info)
		{}

        virtual ~type() {} // leave info alone
        
		// getters

		const type_info* info() const { return m_info; }
		const char* name() const { return m_info->m_name.c_str(); }
		id_type id() const { return m_info->m_id; }
		bool is_complex() const { return m_info->m_is_complex; }
		bool has_custom_write() const { return has_prop("custom_write") || has_generic_write(); }
		bool has_custom_read()  const { return has_prop("custom_read") || has_generic_read(); }
		bool has_generic_write() const { return has_prop("generic_write"); }
		bool has_generic_read()  const { return has_prop("generic_read"); }


		// return a copy of the type, this is for internal use, resets this
		virtual type* _realloc() = 0;

		// return the underlying type
		virtual type* get_type() const = 0;

		// return if type is a member of another class
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

		// construct in place
		virtual void construct(void* instance) const = 0;

		// if the type is a member, returns an any with the value of a named property
		virtual const any& prop(const std::string& name) const = 0;
		
		// if the type is a member, return if this member has a property
		virtual bool has_prop(const std::string& name) const = 0;

		// if the type is a member, set the named a property
		virtual void set_prop(const std::string& name, const any& value) = 0;

		// this calls the function meta::ping specialized with the underlying type
		virtual void ping(void* userdata = nullptr, int message = 0) const = 0;

		// copy the data into instance, type must be the same as instance. This is unsafe
		virtual void copy_to(void* to, const void* from) const = 0;

		// move the data into instance, type must be the same as instance. This is unsafe
		virtual void move_to(void* to, void* from) const = 0;

		// helpers

		bool has_members() const
		{
			return get_members().size() > 0;
		}

		//template<typename _t>
		//void set_prop(const std::string& name, const _t& value)
		//{
		//	set_prop(name, any(value));
		//}
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
		type* m_type;
		bool m_owns;

	public:
		any_storage()
			: m_type (nullptr)
			, m_owns (false)
		{}

		any_storage(type* type, bool owns)
			: m_type (type)
			, m_owns (owns)
		{}

		virtual ~any_storage() {}

		virtual const void* data() const = 0;
		virtual void* data() = 0;
		virtual std::shared_ptr<void> make_ref() const = 0;
		virtual any_storage* copy() const = 0;
		virtual any_storage* new_copy() const = 0;
		virtual void copy_to(void* instance) const = 0;
		virtual void move_to(void* instance) const = 0;
		virtual void copy_in(const void* instance) = 0;
		virtual void move_in(void* instance) = 0;
		virtual bool is_type(type* type) const = 0;

		type* get_type() const
		{
			return m_type;
		}

		bool is_ref() const
		{
			return m_owns;
		}
	};

	template<typename _t, bool _owns>
	struct any_storage_t : any_storage
	{
	private:
		_t* m_instance;

	public:
		any_storage_t()
			: m_instance (nullptr)
		{}

		any_storage_t(meta::type* type, _t* instance)
			: any_storage (type, _owns)
			, m_instance  (instance)
		{
			if constexpr (_owns)
			{
				m_instance = new _t(*instance);
			}
		}
		
		//any_storage_t(meta::type* type, const _t* instance)
		//	: any_storage (type, _owns)
		//	, m_instance  (instance)
		//{
		//	if constexpr (_owns)
		//	{
		//		m_instance = new _t(*instance);
		//	}
		//}

		~any_storage_t()
		{
			if constexpr (_owns)
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

		std::shared_ptr<void> make_ref() const override
		{
			return std::make_shared<_t>(*m_instance);
		}

		any_storage* copy() const override
		{
			return new any_storage_t<_t, _owns>(m_type, m_instance);
		}

		any_storage* new_copy() const override
		{
			return new any_storage_t<_t, true>(m_type, m_instance);
		}

		void copy_to(void* instance) const override
		{
			m_type->copy_to(instance, m_instance);
		}

		void move_to(void* instance) const override
		{
			m_type->move_to(instance, m_instance);
		}

		void copy_in(const void* instance) override
		{
			m_type->copy_to(m_instance, instance);
		}

		void move_in(void* instance) override
		{
			m_type->move_to(m_instance, instance);
		}

		bool is_type(meta::type* type) const override
		{
			return get_class<_t>()->info()->m_id == type->info()->m_id;
		}
	};

	template<>
	struct any_storage_t<void, false> : any_storage
	{
	private:
		void* m_instance;

	public:
		any_storage_t()
			: any_storage (nullptr, false)
			, m_instance  (nullptr)
		{}

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

		std::shared_ptr<void> make_ref() const override
		{
			throw nullptr;
		}

		any_storage* copy() const override
		{
			return new any_storage_t<void, false>(m_type, m_instance);
		}

		any_storage* new_copy() const override
		{
			throw nullptr; // this makes no sense for void*
		}

		void copy_to(void* instance) const override
		{
			throw nullptr;
		}

		void move_to(void* instance) const override
		{
			throw nullptr;
		}

		void copy_in(const void* instance) override
		{
			throw nullptr;
		}

		void move_in(void* instance) override
		{
			throw nullptr;
		}

		bool is_type(meta::type* type) const override
		{
			return get_class<void>()->info()->m_id == type->info()->m_id;
		}
	};

	struct any
	{
	private:
		any_storage* m_storage;

	public:
		any()
			: m_storage (nullptr)
		{}

        // should add a const reference 
        
		//	makes a reference to instance
		// 
		any(type* type, void* instance)
		{
			m_storage = new any_storage_t<void, false>(type, instance);
		}

		//	makes a reference to instance
		//
		template<typename _t>
		any(_t* instance)
		{
			m_storage = new any_storage_t<_t, false>(get_class<_t>(), instance);
		}

		// makes a copy of an instance
		//
		template<typename _t>
		any(const _t& instance)
		{
			m_storage = new any_storage_t<_t, true>(get_class<_t>(), const_cast<_t*>(&instance));
		}

		~any()
		{
			delete m_storage;
			m_storage = nullptr;
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

		std::shared_ptr<void> make_ref() const
		{
			return m_storage->make_ref();
		}

		void copy_own(const any& other)
		{
			delete m_storage;
			m_storage = other.m_storage->new_copy();
		}

		void copy_to(void* instance) const
		{
			m_storage->copy_to(instance);
		}

		void move_to(void* instance)
		{
			m_storage->move_to(instance);
		}

		void copy_in(const void* instance)
		{
			m_storage->copy_in(instance);
		}

		void move_in(void* instance)
		{
			m_storage->move_in(instance);
		}

		type* get_type() const { return m_storage->get_type(); }
		void* data() const { return m_storage->data(); }
		id_type type_id() const { return get_type()->info()->m_id; }
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

	struct serial_context : wContext
	{
		struct registered_type
		{
			type* rtype;
			int location;
		};

		std::unordered_map<id_type, registered_type> known_info;

		void Realloc(int location) override;
	};

	wContextDecl(serial_context);

	void register_meta_types();

	void register_type(id_type type_id, type* type);

	bool has_registered_type(id_type type_id);
	type* get_registered_type(id_type type_id);

	bool has_registered_type(const char* name);
	type* get_registered_type(const char* name);

	void free_registered_type(id_type type_id);

	//
	// serialization
	//

	class serial_writer
	{
	protected:
		std::ostream& m_out;
		const bool m_binary;

	public:
		serial_writer(std::ostream& out, bool binary)
			: m_out    (out)
			, m_binary (binary)
		{}

		// write any type to the serializer stream
		template<typename _t>
		void write(const _t& value)
		{
			write_class(get_class<_t>(), (const void*)&value);
		}

		// create a temporary writer for custom data
		pseudo_writer pseudo();

		//
		//	internal
		//

		void write_class (type* type, const void* instance);
		void write_member(type* type, const void* instance, const char* name);
		void write_array (type* type, const void* instance, size_t length);
		void write_string(const char* string, size_t length);

		virtual void class_begin(type* type) = 0;
		virtual void class_delim() = 0;
		virtual void class_end() = 0;

		virtual void member_begin(type* type, const char* name) = 0;
		virtual void member_end() = 0;

		virtual void array_begin(type* type, size_t length) = 0;
		virtual void array_delim() = 0;
		virtual void array_end() = 0;

		virtual void string_begin(size_t length) = 0;
		virtual void string_end() = 0;

		virtual void write_bytes(const char* bytes, size_t length) = 0;

		template<typename _t>
		void write_value(const _t& value)
		{
			if (m_binary)
			{
				m_out.write((const char*)&value, sizeof(_t));
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

		// read any type from the deserializer stream
		template<typename _t>
		void read(_t& value)
		{
			read_class(meta::get_class<_t>(), &value);
		}

		// create a temporary reader for custom data
		pseudo_reader pseudo();

		//
		//	internal
		//

		void read_class (type* type, void* instance);
		void read_member(type* type, void* instance, const char* name);
		void read_array (type* type, void* instance, size_t length);
		void read_string(char* string, size_t length);

		virtual void class_begin(type* type) = 0;
		virtual void class_delim() = 0;
		virtual void class_end() = 0;

		virtual void member_begin(type* type, const char* name) = 0;
		virtual void member_end() = 0;

		virtual void array_begin(type* type, size_t length) = 0;
		virtual void array_delim() = 0;
		virtual void array_end() = 0;

		virtual void string_begin(size_t length) = 0;
		virtual void string_end() = 0;
        
        virtual size_t read_length() = 0;
        virtual void read_bytes(char* bytes, size_t length) = 0;

		template<typename _t>
		void read_value(_t& value)
		{
			if (m_binary)
			{
				read_bytes((char*)&value, sizeof(_t));
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

	//
	// Syntax sugar for custom writers
	//
	class pseudo_writer
	{
	private:
		struct pseudo_writer_frame
		{
			bool m_first_member = true;
		};

		serial_writer* m_writer;
		std::stack<pseudo_writer_frame> m_frames;

	public:
		pseudo_writer(serial_writer* writer)
			: m_writer(writer)
		{}

		~pseudo_writer()
		{
			while (m_frames.size() > 0)
			{
				end();
			}
		}

		template<typename _t>
		pseudo_writer& begin()
		{
			m_frames.push(pseudo_writer_frame{});

			m_writer->class_begin(meta::get_class<_t>());
			return *this;
		}

		template<typename _t>
		pseudo_writer& member(const char* name, const _t& instance)
		{
			member_delim();

			m_writer->write_member(get_class<_t>(), &instance, name);
			return *this;
		}

		template<typename _t>
		pseudo_writer& custom(const char* name, const std::function<void()>& func)
		{
			member_delim();

			m_writer->member_begin(meta::get_class<_t>(), name);
			func();
			m_writer->member_end();

			return *this;
		}

		// Gets called in destructor
		// Only need to call if you want to be explicit or if 
		// there are multiple pseudo writers in scope
		void end()
		{
			m_writer->class_end();
			m_frames.pop();
		}

	private:
		void member_delim()
		{
			bool& first = m_frames.top().m_first_member;
			if (!first) m_writer->class_delim();
			first = false;
		}
	};

	//
	// Syntax sugar for custom readers
	//
	class pseudo_reader
	{
	private:
		struct pseudo_reader_frame
		{
			bool m_first_member = true;
		};

		serial_reader* m_reader;
		std::stack<pseudo_reader_frame> m_frames;

	public:
		pseudo_reader(serial_reader* reader)
			: m_reader (reader)
		{}

		~pseudo_reader()
		{
			while (m_frames.size() > 0)
			{
				end();
			}
		}

		template<typename _t>
		pseudo_reader& begin()
		{
			m_frames.push(pseudo_reader_frame{});

			m_reader->class_begin(meta::get_class<_t>());
			return *this;
		}

		template<typename _t>
		pseudo_reader& member(const char* name, _t& instance)
		{
			member_delim();

			m_reader->read_member(get_class<_t>(), &instance, name);
			return *this;
		}

		template<typename _t>
		pseudo_reader& custom(const char* name, const std::function<void()>& func)
		{
			member_delim();

			m_reader->member_begin(meta::get_class<_t>(), name);
			func();
			m_reader->member_end();

			return *this;
		}

		// Gets called in destructor
		// Only need to call if you want to be explicit or if 
		// there are multiple pseudo writers in scope
		void end()
		{
			m_reader->class_end();
			m_frames.pop();
		}

	private:
		void member_delim()
		{
			bool& first = m_frames.top().m_first_member;
			if (!first) m_reader->class_delim();
			first = false;
		}
	};

	//
	// default behavior is to just write the value
	// specialize these for custom types
	//

	template<typename _t>
	void serial_write(serial_writer* serial, const _t& value)
	{
		if constexpr (meta::has_custom_write<_t>::value)
		{
			value.serial_write(serial);
			return;
		}

        if constexpr (std::is_enum<_t>::value)
        {
            serial->write_value((size_t)value);
            return;
        }
        
		serial->write_value(value);
	}

	template<typename _t>
	void serial_read(serial_reader* serial, _t& value)
	{
		if constexpr (meta::has_custom_read<_t>::value)
		{
			value.serial_read(serial);
			return;
		}

        if constexpr (std::is_enum<_t>::value)
        {
			size_t largest = 0;
            serial->read_value(largest);
			value = (_t)largest;

            return;
        }
        
		serial->read_value(value);
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
		std::string m_name;
		std::unordered_map<std::string, any> m_props;

	public:
		_class_member(_class_type<_mtype>* member_class_type, const char* member_name)
			: type    (member_class_type->info())
			, m_class (member_class_type)
			, m_name  (member_name)
		{}
        
        virtual ~_class_member() {}

		type* _realloc() override
		{
			assert(false && "type was member");
			return nullptr;
		}

		meta::type* get_type() const override
		{
			return (type*)m_class;
		}

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
			return m_name.c_str();
		}

		void* walk_ptr(const void* instance) const override
		{
			return (void*) & (((const _t*)instance)->*_m);
		}

		void _serial_write(serial_writer* serial, const void* instance) const override
		{
			m_class->_serial_write(serial, instance);
		}

		void _serial_read(serial_reader* serial, void* instance) const override
		{
			m_class->_serial_read(serial, instance);
		}

		any construct() const override
		{
			return m_class->construct();
		}

		void construct(void* instance) const override
		{
			m_class->construct(instance);
		}

		const any& prop(const std::string& name) const override
		{
			return m_props.at(name);
		}

		bool has_prop(const std::string& name) const override
		{
			return m_props.find(name) != m_props.end();
		}

		void set_prop(const std::string& name, const any& value) override
		{
			m_props[name].copy_own(value);
		}

		void ping(void* userdata = nullptr, int message = 0) const override
		{
			meta::ping<_mtype>(userdata, message);
		}

		void copy_to(void* to, const void* from) const override
		{
			m_class->copy_to(to, from);
		}

		void move_to(void* to, void* from) const override
		{
			m_class->move_to(to, from);
		}
	};

    // fwd
    template<typename _t>
    _class_type<_t>* _get_class();

	// internal representation of a class
	//
	template<typename _t>
	class _class_type : public type
	{
	private:
		std::vector<type*> m_members;

		std::function<void(serial_writer*, const _t&)> m_write;
		std::function<void(serial_reader*,       _t&)> m_read;

		std::unordered_map<std::string, any> m_props;

	public:
		_class_type(type_info* info)
			: type (info)
		{}

		virtual ~_class_type()
		{
			for (type* m : m_members) delete m;
		}

		// setters / getters for internal

		type_info* info() { return m_info; }

		template<auto _m>
		void add_member(const char* name)
		{
			m_members.push_back(
				new _class_member<_t, _m>(_get_class<ptrtype<_m>>(), name)
			);
		}

		void set_custom_writer(const std::function<void(serial_writer*, const _t&)>& func)
		{
			set_prop("custom_write", true);
			m_write = func;
		}

		void set_custom_reader(const std::function<void(serial_reader*, _t&)>& func)
		{
			set_prop("custom_read", true);
			m_read = func;
		}

		void call_custom_write(serial_writer* serial, const void* instance) const
		{
			m_write(serial, *(_t*)instance);
		}

		void call_custom_read(serial_reader* serial, void* instance) const
		{
			m_read(serial, *(_t*)instance);
		}

		// interface
		
		// this doesn't work because the vtable causes this function to be called in the dlls
		// stack >:(
		type* _realloc() override
		{
			// make copy of info & this type
			type_info* info = new type_info(*m_info);
			_class_type* type = new _class_type(info);

			type->m_members = m_members;
			type->m_write = m_write;
			type->m_read = m_read;
			type->m_props = m_props;

			// reset this, mainly for ~type that deletes members

			m_members = {};
			m_write = {};
			m_read = {};
			m_props = {};

			// register type, this deletes the old one

			register_type(info->m_id, type);

			return type;
		}

		type* get_type() const override
		{
			return (type*)this;
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
			if (!has_custom_write() || has_generic_write())
			{
				serial_write(serial, *(const _t*)instance); // if write is generic, call through this template function
			}

			else
			{
				call_custom_write(serial, instance);
			}
		}

		void _serial_read(serial_reader* serial, void* instance) const override
		{
			if (!has_custom_read() || has_generic_read())
			{
				serial_read(serial, *(_t*)instance); // see above
			}

			else
			{
				call_custom_read(serial, instance);
			}
		}

		any construct() const override
		{
			_t t = _t();
			return any(t);
		}

		void construct(void* instance) const override
		{
			new (instance) _t();
		}

		const any& prop(const std::string& name) const override
		{
			return m_props.at(name);
		}

		bool has_prop(const std::string& name) const override
		{
			return m_props.find(name) != m_props.end();
		}

		void set_prop(const std::string& name, const any& value) override
		{
			m_props[name].copy_own(value);
		}

		void ping(void* userdata = nullptr, int message = 0) const override
		{
			meta::ping<_t>(userdata, message);
		}

		void copy_to(void* to, const void* from) const override
		{
			*(_t*)to = *(const _t*)from;
		}

		void move_to(void* to, void* from) const override
		{
			*(_t*)to = std::forward<_t>(*(_t*)from);
		}
	};

	// specialize void because of min/max/construct
	template<>
	class _class_type<void> : public type
	{
	public:
		_class_type(type_info* info)
			: type (info)
		{}

		type_info*                info()                                                                    { return m_info; }
		type*                     _realloc()                                                       override { assert(false && "type was void"); throw nullptr; }
		type*                     get_type()                                                 const override { return (type*)this; }
		bool                      is_member()                                                const override { return false; }
		const std::vector<type*>& get_members()                                              const override { assert(false && "type was void"); throw nullptr; }
		const char*               member_name()                                              const override { assert(false && "type was not a member"); throw nullptr; }
		void*                     walk_ptr(const void* instance)                             const override { assert(false && "type was void"); throw nullptr; }
		void                      _serial_write(serial_writer* serial, const void* instance) const override { assert(false && "type was void"); throw nullptr; }
		void                      _serial_read(serial_reader* serial, void* instance)        const override { assert(false && "type was void"); throw nullptr; }
		any                       construct()                                                const override { assert(false && "type was void"); throw nullptr; }
		void                      construct(void* instance)                                  const override { assert(false && "type was void"); throw nullptr; }
		const any&                prop(const std::string& name)                              const override { assert(false && "type was not a member"); throw nullptr; }
		bool                      has_prop(const std::string& name)                          const override { assert(false && "type was not a member"); throw nullptr; }
		void                      set_prop(const std::string& name, const any& value)              override { assert(false && "type was not a member"); throw nullptr; }
		void                      ping(void* userdata = nullptr, int message = 0)            const override { assert(false && "type was void"); throw nullptr; }
		void                      copy_to(void* to, const void* from)                        const override { assert(false && "type was void"); throw nullptr; }
		void                      move_to(void* to, void* from)                              const override { assert(false && "type was void"); throw nullptr; }
	};

	template<typename _t>
	id_type _get_id()
	{
#ifdef SERIAL_ENTT_TIE_IN
		return entt::type_id<_t>().hash();
#else
		return typeid(_t).hash_code();
#endif
	}

	// for function override
	template<typename _t>
	struct tag {};

	template<typename _t>
	void describer(type* type, tag<_t>)
	{
		// do nothing
	}

	template<typename _t>
	_class_type<_t>* _get_class()
	{
		id_type id = _get_id<_t>();

		if (!has_registered_type(id))
		{
			type* type = new _class_type<_t>(_make_type_info<_t>(id));
			register_type(id, type);

			describer(type, tag<_t>{});
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
		_class_type<_t>* m_current = nullptr;

	public:
		describe()
		{
			if (!has_registered_type(_get_id<_t>()))
			{
				m_current = _get_class<_t>();
			}
		}

		// set the name of the class
		describe<_t>& name(const char* name)
		{
			if (m_current) m_current->info()->m_name = name;
			return *this;
		}

		// add a member to the class
		template<auto _m>
		describe<_t>& member(const char* name)
		{
			if (m_current) m_current->template add_member<_m>(name);
			return *this;
		}

		// for the last member added, set a property
		template<typename _v>
		describe<_t>& prop(const std::string& name, const _v& value)
		{
			if (m_current) ((type*)m_current)->set_prop(name, value);
			return *this;
		}

		// for the last member added, set a property
		template<typename _v>
		describe<_t>& member_prop(const std::string& name, _v value)
		{
			if (m_current) m_current->get_members().back()->set_prop(name, any(value, false));
			return *this;
		}

		// mark this class as having a custom writer,
		// this is needed for the serial_writers to pick up if the class has members
		// but doesn't want default serialization behavior
		describe<_t>& custom_write(const std::function<void(serial_writer*, const _t&)>& func)
		{
			if (m_current) m_current->set_custom_writer(func);
			return *this;
		}

		// mark this class as having a custom reader,
		// this is needed for the serial_readers to pick up if the class has members
		// but doesn't want default deserialization behavior
		describe<_t>& custom_read(const std::function<void(serial_reader*, _t&)>& func)
		{
			if (m_current) m_current->set_custom_reader(func);
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
		return _get_id<_t>();
		//return get_class<_t>()->info()->m_id;
	}

	template<typename _t>
	type_info* info()
	{
		return get_class<_t>()->m_info;
	}

	template<typename _t>
	bool is_same(type* type)
	{
		return id<_t>() == type->info()->m_id;
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

// should move these to a file: serial_std.h 

#include <unordered_set>

namespace meta
{
	template<typename _s>
	void write_lienar(serial_writer* serial, const _s& lienar_container)
	{
		auto itr = lienar_container.begin();
		auto end = lienar_container.end();

		size_t length = std::distance(itr, end);

		serial->array_begin(meta::get_class<typename std::remove_const<typename _s::value_type >::type >(), length);
		
		for (size_t i = 0; i < length; i++)
		{
			serial->write(*itr++);

			if (i != length - 1)
			{
				serial->array_delim();
			}
		}

		serial->array_end();
	}

	template<typename _s>
	void read_linear(serial_reader* serial, _s& lienar_container)
	{
		size_t length = serial->read_length();
		serial->array_begin(meta::get_class<typename std::remove_const<typename _s::value_type>::type>(), length);

		lienar_container.clear();
		lienar_container.reserve(length);

		for (size_t i = 0; i < length; i++)
		{
			typename _s::value_type temp;
			serial->read(temp);

			lienar_container.insert(lienar_container.end(), std::move(temp));

			if (i != length - 1)
			{
				serial->array_delim();
			}
		}

		serial->array_end();
	}

	
	template<typename _s>
	void write_pairs(serial_writer* serial, const _s& pair_container)
	{
		auto itr = pair_container.begin();
		auto end = pair_container.end();

		size_t length = std::distance(itr, end);

		// this assumes a map...
		// could use pair_element<0, _s::value_type>
		// to remove const of map key type, create a temp copy

		using pair_t = std::pair<
			typename std::remove_const<typename _s::key_type>::type,
			typename std::remove_const<typename _s::mapped_type>::type
		>;

		serial->array_begin(meta::get_class<pair_t>(), length);
		
		for (size_t i = 0; i < length; i++)
		{
			pair_t temp = *itr;
			serial->write(temp);

			if (i != length - 1)
			{
				serial->array_delim();
			}

			itr++;
		}

		serial->array_end();
	}

	template<typename _s>
	void read_pairs(serial_reader* serial, _s& pair_container)
	{
		// see above
		using pair_t = std::pair<
			typename std::remove_const<typename _s::key_type>::type,
			typename std::remove_const<typename _s::mapped_type>::type
		>;

		size_t length = serial->read_length();
		serial->array_begin(meta::get_class<pair_t>(), length);

		pair_container.clear();
		pair_container.reserve(length);

		for (size_t i = 0; i < length; i++)
		{
			pair_t temp;
			serial->read(temp);

			pair_container.emplace(temp.first, temp.second);

			if (i != length - 1)
			{
				serial->array_delim();
			}
		}

		serial->array_end();
	}

	template<typename _t>
	void serial_write(serial_writer* serial, const std::vector<_t>& instance)
	{
		write_lienar(serial, instance);
	}

	template<typename _t>
	void serial_read(serial_reader* serial, std::vector<_t>& instance)
	{
		read_linear(serial, instance);
	}

	template<typename _t>
	void serial_write(serial_writer* serial, const std::unordered_set<_t>& instance)
	{
		write_lienar(serial, instance);
	}

	template<typename _t>
	void serial_read(serial_reader* serial, std::unordered_set<_t>& instance)
	{
		read_linear(serial, instance);
	}

	template<typename _k, typename _v>
	void serial_write(serial_writer* serial, const std::unordered_map<_k, _v>& instance)
	{
		write_pairs(serial, instance);
	}

	template<typename _k, typename _v>
	void serial_read(serial_reader* serial, std::unordered_map<_k, _v>& instance)
	{
		read_pairs(serial, instance);
	}

	template<typename _a, typename _b>
	void serial_write(serial_writer* serial, const std::pair<_a, _b>& instance)
	{
		if constexpr (!std::is_const<_a>::value && !std::is_const<_b>::value)
		{
			serial->class_begin(meta::get_class<std::pair<_a, _b>>());
			serial->write_member(meta::get_class<_a>(), &instance.first, "first");
			serial->class_delim();
			serial->write_member(meta::get_class<_b>(), &instance.second, "second");
			serial->class_end();
		}
	}

	template<typename _a, typename _b>
	void serial_read(serial_reader* serial, std::pair<_a, _b>& instance)
	{
		if constexpr (!std::is_const<_a>::value && !std::is_const<_b>::value)
		{
			serial->class_begin(meta::get_class<std::pair<_a, _b>>());
			serial->read_member(meta::get_class<_a>(), &instance.first, "first");
			serial->class_delim();
			serial->read_member(meta::get_class<_b>(), &instance.second, "second");
			serial->class_end();
		}
	}

	template<typename _t>
	void describer(type* type, tag<std::vector<_t>>)
	{
		type->set_prop("generic_write", true);
		type->set_prop("generic_read", true);
		type->set_prop("is_vector", true);
		type->set_prop("inner_type", id<_t>());
	}

	template<typename _t>
	void describer(type* type, tag<std::unordered_set<_t>>)
	{
		type->set_prop("generic_write", true);
		type->set_prop("generic_read", true);
		type->set_prop("is_unordered_set", true);
		type->set_prop("inner_type", id<_t>());
	}

	template<typename _k, typename _v>
	void describer(type* type, tag<std::unordered_map<_k, _v>>)
	{
		type->set_prop("generic_write", true);
		type->set_prop("generic_read", true);
		type->set_prop("is_unordered_map", true);
		type->set_prop("inner_type", id<std::pair<_k, _v>>());
	}

	template<typename _a, typename _b>
	void describer(type* type, tag<std::pair<_a, _b>>)
	{
		type->set_prop("generic_write", true);
		type->set_prop("generic_read", true);
		type->set_prop("is_pair", true);
		type->set_prop("inner_type_a", id<_a>());
		type->set_prop("inner_type_b", id<_b>());
	}
}
