#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <memory>
#include <utility>

template<typename _t>
using r = std::shared_ptr<_t>;

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

float get_rand(float x) { return x * rand() / (float)RAND_MAX; }
int get_rand(int x) { return rand() % x; }

using u32 = uint32_t; // redundant with glm
using u8 = uint8_t;
using f32 = float;

struct Color
{
	union { u32 as_u32; struct { u8 r, g, b, a; }; };
	Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}
	Color(u32 rgba) : as_u32(rgba) {}
	Color() : r(255), g(255), b(255), a(255) {}

	float rf() const { return r / (float)255.f; }
	float gf() const { return g / (float)255.f; }
	float bf() const { return b / (float)255.f; }
	float af() const { return a / (float)255.f; }

	glm::vec4 as_v4() const
	{
		return { rf(), gf(), bf(), af() };
	}

	static Color rand()
	{
		return Color(get_rand(255), get_rand(255), get_rand(255), get_rand(255));	
	}

	static Color from32(int bits32)
	{
		return Color(0x000000ff & bits32, 0x0000ff00 & bits32, 0x00ff0000 & bits32, 0xff000000 & bits32);
	}
};

struct Transform2D
{
	float x, y, z, sx, sy, r;

	Transform2D()
		:  x(0), y(0), z(0), sx(1), sy(1), r(0)
	{}

	Transform2D(
		int x, int y, int z, int sx, int sy, int r
	)
		:  x(x), y(y), z(z), sx(sx), sy(sy), r(r)
	{}

	glm::mat4 World() const
	{
		glm::mat4 world = glm::mat4(1.f);
		world = glm::translate(world, glm::vec3(x, y, z));
		world = glm::rotate(world, r, glm::vec3(0, 0, 1.f));
		world = glm::scale(world, glm::vec3(sx, sy, 1.f));

		return world;
	}
};

float lerp(float a, float b, float w)
{
	return a + w * (b - a);
}

float clamp(float x, float min, float max)
{
	     if (x < min) x = min;
	else if (x > max) x = max;
	return x; 
}

template<typename _t>
std::pair<_t, _t> get_xy(const _t& index, const _t& width)
{
	return { index % width, index / width };
}

// map asset path

std::string _p(const std::string& filename)
{
	return "../assets/" + filename;
}
