#pragma once

#include "Defines.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <memory>
#include <utility>
#include <string>

template<typename _t> using r = std::shared_ptr<_t>;

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

// return a random number between (0, x)
inline float get_rand(float x) { return x * rand() / (float)RAND_MAX; }

// return a random number between (-x/2, x/2)
inline float get_randc(float x) { return x * rand() / (float)RAND_MAX - x / 2.f; }

// return a random integer between (0, x)
inline int get_rand(int x) { return rand() % x; }

inline vec2 get_rand (float x, float y) { return vec2(get_rand(x), get_rand(y)); }
inline vec2 get_randc(float x, float y) { return vec2(get_randc(x), get_randc(y)); }

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
	union {
		vec3 position3D;
		struct
		{
			vec2 position;
			float _z; // this is messy...
		};
		struct { float x, y, z; };
	};

	union {
		vec2 scale;
		struct { float sx, sy; };
	};

	union
	{
		float r;
		float rotation;
	};

	Transform2D()
		:  x(0), y(0), z(0), sx(1), sy(1), r(0)
	{}

	Transform2D(
		float x, float y, float z, float sx, float sy, float r
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

inline float lerp(float a, float b, float w) { return a + w * (b - a); }
inline vec2  lerp(vec2  a, vec2  b, float w) { return a + w * (b - a); }
inline vec2  lerp(vec3  a, vec3  b, float w) { return a + w * (b - a); }
inline vec2  lerp(vec4  a, vec4  b, float w) { return a + w * (b - a); }

inline float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x; 
}

inline vec2 clamp(const vec2& x, float max)
{
	float d = length(x);
	if (d > max) return x / d * max;
	return x;
}

inline vec2 clamp(vec2 x, const vec2& min, const vec2& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	return x;
}

inline vec2 safe_normalize(const vec2& p)
{
	float n = sqrt((float)(p.x * p.x + p.y * p.y));
	return (n == 0) ? vec2(0.f, 0.f) : vec2(p.x / n, p.y / n);
}

template<typename _t>
std::pair<_t, _t> get_xy(const _t& index, const _t& width)
{
	return { index % width, index / width };
}

// map asset path

inline std::string _p(const std::string& filename)
{
	return "../assets/" + filename;
}
