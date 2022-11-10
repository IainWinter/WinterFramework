#pragma once

#include "Serial.h"
#include "Common.h"

#include "entt/meta/meta.hpp"
#include "entt/meta/factory.hpp"

#include <string>
#include <vector>

namespace meta
{
	//
	// Strings
	//

	template<> void serial_write(serial_writer* writer, const std::string& instance)
	{
		writer->write_length(instance.size());
		writer->write_string(instance.data(), instance.size());
	}

	template<> void serial_read(serial_reader* reader, std::string& instance)
	{
		instance.resize(reader->read_length());
		reader->read_string(instance.data(), instance.size());
	}

	//
	// Vectors
	//

	// g++ doesnt find these
	// clang does find these

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

	//
	// glm types
	//

	void register_glm_types()
	{
		describe< vec2>().name( "vec2").member<& vec2::x>("x").member<& vec2::y>("y");
		describe<ivec2>().name("ivec2").member<&ivec2::x>("x").member<&ivec2::y>("y");
		describe<bvec2>().name("bvec2").member<&bvec2::x>("x").member<&bvec2::y>("y");

		//describe< vec3>().name( "vec3").member<& vec3::x>("x").member<& vec3::y>("y").member<& vec3::z>("z");
		//describe<ivec3>().name("ivec3").member<&ivec3::x>("x").member<&ivec3::y>("y").member<&ivec3::z>("z");
		//describe<bvec3>().name("bvec3").member<&bvec3::x>("x").member<&bvec3::y>("y").member<&bvec3::z>("z");

		//describe< vec4>().name( "vec4").member<& vec4::x>("x").member<& vec4::y>("y").member<& vec3::z>("z").member<& vec4::w>("w");
		//describe<ivec4>().name("ivec4").member<&ivec4::x>("x").member<&ivec4::y>("y").member<&ivec3::z>("z").member<&ivec4::w>("w");
		//describe<bvec4>().name("bvec4").member<&bvec4::x>("x").member<&bvec4::y>("y").member<&bvec3::z>("z").member<&bvec4::w>("w");

		// mats would be custom types
	}

	void register_common_types()
	{
		meta::type* t = describe<Transform2D>()
			.name("Transform2D")
			.member<&Transform2D::position>("position")
			.member<&Transform2D::z>("z")
			.member<&Transform2D::scale>("scale")
			.member<&Transform2D::rotation>("rotation")
			.get();

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
	}

	void register_all_types()
	{
		register_glm_types();
		register_common_types();
	}
}