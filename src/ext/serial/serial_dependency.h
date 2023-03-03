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
	serial_dependency(const meta::serial_context* ctx)
	{
		for (const auto& [id, reg] : ctx->known_info)
		{
			m_types.add_node(reg.rtype);
		}

		// connect all members to their classes

		for (const auto& [id, reg] : ctx->known_info)
		{
			for (meta::type* member : reg.rtype->get_members())
			{
				m_types.add_edge(reg.rtype, member->get_type());
			}
		}
	}

	int order(meta::type* type) const
	{
		return m_types.depth(type);
	}
};