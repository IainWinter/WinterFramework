#include "ext/serial/serial_walker.h"

namespace meta
{
	walker::walker(meta::type* type, void* instance)
		: m_type(type)
		, m_instance(instance)
	{
        init_children();
	}

	walker::~walker()
	{
		for (const auto& [_, child] : m_children)
		{
			delete child;
		}
	}

	void* walker::value() const
	{
		return m_instance;
	}

	type* walker::type() const
	{
		return m_type;
	}

	const walker& walker::get(const std::string& name) const
	{
		return *m_children.at(name);
	}

	const walker& walker::operator[](const std::string& name) const
	{
		return get(name);
	}

	void walker::walk(std::function<void(meta::type*, void*)> walker) const
	{
		walker(m_type, m_instance);

		for (const auto& [_, child] : m_children)
		{
			child->walk(walker);
		}
	}

    void walker::init_children()
    {
        for (meta::type* child : m_type->get_members())
        {
            m_children[child->member_name()] = new walker(child, child->walk_ptr(m_instance));
        }
    }
}
