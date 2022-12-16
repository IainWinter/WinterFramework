#pragma once

#include "ext/serial/serial.h"
#include "util/graph.h"

#include "ext/AssetStore.h"
#include "ext/rendering/TextureAtlas.h"
#include "ext/rendering/Particle.h"

//
//	dependancy graph of known types
//
class serial_dependency
{
private:
	graph<meta::type*> m_types;

public:
	serial_dependency(meta::serial_context* ctx)
	{
		for (const auto& [id, type] : ctx->known_info)
		{
			m_types.add_node(type);
		}

		// connect all members to their classes

		for (const auto& [id, type] : ctx->known_info)
		{
			for (meta::type* member : type->get_members())
			{
				m_types.add_edge(type, member->get_type());
			}
		}
	}

	int order(meta::type* type) const
	{
		return m_types.depth(type);
	}
};