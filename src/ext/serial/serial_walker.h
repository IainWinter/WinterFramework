#pragma once

#include "ext/serial/serial.h"
#include <functional>

namespace meta
{
	// creates a tree of pointers that have been walked to their byte offset for a type
	// can index on the name of the member like a map
	//
	struct walker
	{
		void* m_instance;
		type* m_type;
		std::unordered_map<std::string, walker*> m_children;

		walker(type* type, void* instance);
		~walker();

		void* value() const;

		const walker& get(const std::string& name) const;
		const walker& operator[](const std::string& name) const;
		void walk(std::function<void(type*, void*)> walker) const;
	};
}