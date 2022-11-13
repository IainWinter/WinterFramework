#pragma once

#include "Serial.h"
#include "Common.h"

#include "entt/meta/meta.hpp"
#include "entt/meta/factory.hpp"

#include <string>
#include <vector>

namespace meta
{
	inline void register_common_types()
	{
		// glm

		describe< vec2>().name( "vec2").member<& vec2::x>("x").member<& vec2::y>("y");
		describe<ivec2>().name("ivec2").member<&ivec2::x>("x").member<&ivec2::y>("y");
		describe<bvec2>().name("bvec2").member<&bvec2::x>("x").member<&bvec2::y>("y");

		describe< vec3>().name( "vec3").member<& vec3::x>("x").member<& vec3::y>("y").member<& vec3::z>("z");
		describe<ivec3>().name("ivec3").member<&ivec3::x>("x").member<&ivec3::y>("y").member<&ivec3::z>("z");
		describe<bvec3>().name("bvec3").member<&bvec3::x>("x").member<&bvec3::y>("y").member<&bvec3::z>("z");

		describe< vec4>().name( "vec4").member<& vec4::x>("x").member<& vec4::y>("y").member<& vec4::z>("z").member<& vec4::w>("w");
		describe<ivec4>().name("ivec4").member<&ivec4::x>("x").member<&ivec4::y>("y").member<&ivec4::z>("z").member<&ivec4::w>("w");
		describe<bvec4>().name("bvec4").member<&bvec4::x>("x").member<&bvec4::y>("y").member<&bvec4::z>("z").member<&bvec4::w>("w");

		describe<Transform2D>()
			.name("Transform2D")
			.member<&Transform2D::position>("position")
			.member<&Transform2D::z>("z")
			.member<&Transform2D::scale>("scale")
			.member<&Transform2D::rotation>("rotation");

		describe<Color>()
			.name("Color")
			.member<&Color::r>("r")
			.member<&Color::g>("g")
			.member<&Color::b>("b")
			.member<&Color::a>("a");

		describe<aabb2D>()
			.name("aabb2D")
			.member<&aabb2D::min>("min")
			.member<&aabb2D::max>("max");

		describe<EntityMeta>()
			.name("EntityMeta")
			.member<&EntityMeta::name>("name")/*
			.member<&EntityMeta::parent>("parent")*/;
	}

	//template<>
	//inline void serial_write(serial_writer* writer, const Entity& instance)
	//{
	//	writer->write_value(instance.raw_id());
	//}

	//template<>
	//inline void serial_read(serial_reader* reader, Entity& instance)
	//{
	//	u32 id;
	//	reader->read_value(id);
	//	instance = Entity((entt::entity)id, nullptr);
	//}
}