#include "ext/serial/serial.h"

namespace meta
{
	//
	//	context
	//

	serial_context* ctx;

	void create_context()
	{
		destroy_context();
		ctx = new serial_context();
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

//
//	//
//	// walker
//	//
//
//	walker::walker(type* type, void* instance)
//		: m_type     (type)
//		, m_instance (instance)
//	{
//		for (meta::type* child : type->get_members())
//		{
//			m_children[child->member_name()] = new walker(child, child->walk_ptr(instance));
//		}
//	}
//
//	walker::~walker()
//	{
//		for (const auto& [_, child] : m_children)
//		{
//			delete child;
//		}
//	}
//
//	void* walker::value() const
//	{
//		return m_instance;
//	}
//
//	const walker& walker::get(const std::string& name) const
//	{
//		return *m_children.at(name);
//	}
//
//	const walker& walker::operator[](const std::string& name) const
//	{
//		return get(name);
//	}
//
//	void walker::walk(std::function<void(type*, void*)> walker) const
//	{
//		walker(m_type, m_instance);
//
//		for (const auto& [_, child] : m_children)
//		{
//			child->walk(walker);
//		}
//	}
}