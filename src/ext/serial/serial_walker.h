#pragma once

#include "ext/serial/serial.h"
#include <functional>

namespace meta
{
	// creates a tree of pointers that have been walked to their byte offset for a type
	// can index on the name of the member like a map
	//
	class walker
	{
	private:
		void* m_instance;
		meta::type* m_type;

	public:
		std::unordered_map<std::string, walker*> m_children;

		walker(meta::type* type, void* instance);
		~walker();

		void* value() const;
		meta::type* type() const;

		const walker& get(const std::string& name) const;
		const walker& operator[](const std::string& name) const;
		void walk(std::function<void(meta::type*, void*)> walker) const;
	};
}