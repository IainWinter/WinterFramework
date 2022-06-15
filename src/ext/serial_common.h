#pragma once

#include "Serial.h"
#include "Common.h"

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
		meta::describe< vec2>().name( "vec2").member<& vec2::x>("x").member<& vec2::y>("y");
		meta::describe<ivec2>().name("ivec2").member<&ivec2::x>("x").member<&ivec2::y>("y");
		meta::describe<bvec2>().name("bvec2").member<&bvec2::x>("x").member<&bvec2::y>("y");

		meta::describe< vec3>().name( "vec3").member<& vec3::x>("x").member<& vec3::y>("y");
		meta::describe<ivec3>().name("ivec3").member<&ivec3::x>("x").member<&ivec3::y>("y");
		meta::describe<bvec3>().name("bvec3").member<&bvec3::x>("x").member<&bvec3::y>("y");

		meta::describe< vec4>().name( "vec4").member<& vec4::x>("x").member<& vec4::y>("y");
		meta::describe<ivec4>().name("ivec4").member<&ivec4::x>("x").member<&ivec4::y>("y");
		meta::describe<bvec4>().name("bvec4").member<&bvec4::x>("x").member<&bvec4::y>("y");

		// mats would be custom types
	}


	// typenames, todo: add all of the common ones

	template<> std::string type_name(tag<int>)   { return "int"; }
	template<> std::string type_name(tag<float>) { return "float"; }
	template<> std::string type_name(tag<std::string>) { return "std::string"; }

	template<typename _i>
	std::string type_name(tag<std::vector<_i>>)
	{
		std::stringstream ss;
		ss << "std::vector<" << type_name(tag<_i>{}) << ">"; 
		return ss.str();
	}
}