#pragma once

#include "Defines.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <memory>
#include <utility>
#include <string>
#include <vector>

// Memory helpers

template<typename _t> using r = std::shared_ptr<_t>;
template<typename _t, typename... _args> r<_t> mkr(_args&&... args) { return std::make_shared<_t>(std::forward<_args>(args)...); }

//inline void* alloc_and_copy(void* ptrToCopy, int sizeInBytes)
//{
//	void* newptr = malloc(sizeInBytes);
//	
//	if (newptr == nullptr)
//	{
//		printf("failed to allocate memory\n");
//		assert(false);
//		return nullptr;
//	}
//
//	memcpy(newptr, ptrToCopy, sizeInBytes);
//	return newptr;
//}
//
//inline void* realloc_and_copy(void* ptrToRealloc, const void* ptrToCopy, int oldSize, int newSize)
//{
//	void* newptr = nullptr;
//
//	if (oldSize == newSize) newptr = ptrToRealloc;
//	else                    newptr = realloc(ptrToRealloc, newSize);
//
//	if (newptr)
//	{
//		memset(newptr, 0, newSize);
//
//		if (newSize > oldSize) memcpy(newptr, ptrToCopy, oldSize);
//		else                   memcpy(newptr, ptrToCopy, newSize);
//
//		return newptr;
//	}
//
//	printf("failed to realloc_and_copy\n");
//	assert(false);
//	return newptr;
//}

// Math helpers

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

#define wPI 3.1415926535f
#define w2PI wPI * 2.f

inline vec2 safe_normalize(const vec2& p)
{
	float n = sqrt((float)(p.x * p.x + p.y * p.y));
	return (n == 0) ? vec2(0.f, 0.f) : vec2(p.x / n, p.y / n);
}

// return a random number between (0, x)
inline float get_rand(float x) { return x * rand() / (float)RAND_MAX; }

// return a random number between (-x/2, x/2)
inline float get_randc(float x) { return x * rand() / (float)RAND_MAX - x / 2.f; }

// return a random integer between (0, x)
inline int get_rand(int x) { return rand() % x; }

inline vec2 get_rand  (float x, float y) { return vec2(get_rand(x), get_rand(y)); }
inline vec2 get_randc (float x, float y) { return vec2(get_randc(x), get_randc(y)); }
inline vec2 get_randn (float scale)      { return get_rand(scale) * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }
inline vec2 get_randnc(float scale)      { return           scale * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }

inline float lerp(float        a, float        b, float w) { return a + w * (b - a); }
inline vec2  lerp(const vec2&  a, const vec2&  b, float w) { return a + w * (b - a); }
inline vec3  lerp(const vec3&  a, const vec3&  b, float w) { return a + w * (b - a); }
inline vec4  lerp(const vec4&  a, const vec4&  b, float w) { return a + w * (b - a); }

inline float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x; 
}

inline vec2 clamp(vec2 x, const vec2& min, const vec2& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	return x;
}

inline vec3 clamp(vec3 x, const vec3& min, const vec3& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	return x;
}

inline vec4 clamp(vec4 x, const vec4& min, const vec4& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	x.w = clamp(x.w, min.w, max.w);
	return x;
}

inline vec2 limit(const vec2& x, float max)
{
	float d = length(x);
	if (d > max) return x / d * max;
	return x;
}

inline float max(const vec2& v)
{
	return max(v.x, v.y);
}

template<typename _t>
std::pair<_t, _t> get_xy(const _t& index, const _t& width)
{
	return { index % width, index / width };
}

// vector helpers

template<typename _t>
void pop_erase(std::vector<_t>& list, size_t index)
{
	list.at(index) = list.back();
	list.pop_back();
}

// asset

#ifndef ASSET_ROOT_PATH
#	define ASSET_ROOT_PATH "../assets/"
#endif

inline std::string _a(const std::string& path)
{
	return ASSET_ROOT_PATH + path;
}

// common components

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

	static Color fromv4(const vec4& v4)
	{
		vec4 v = clamp(v4, vec4(0.f, 0.f, 0.f, 0.f), vec4(1.f, 1.f, 1.f, 1.f));
		return Color(255 * v.x, 255 * v.y, 255 * v.z, 255 * v.w);
	}
};

inline Color lerp(const Color& a, const Color& b, float w) { return Color::fromv4(lerp(a.as_v4(), b.as_v4(), w)); }

struct Transform2D
{
	vec2 position;
	vec2 scale;
	float rotation;
	float z;

private:
	vec2 positionLast;
	vec2 scaleLast;
	float rotationLast;
	float zLast;

public:
	Transform2D()
		: position (0.f, 0.f)
		, scale    (1.f, 1.f)
		, rotation (0.f)
		, z        (0.f)
	{
		UpdateLastFrameData();
	}

	Transform2D(
		float x, float y, float z = 0.f, float sx = 1.f, float sy = 1.f, float r = 0.f
	)
		: position (x, y)
		, scale    (sx, sy)
		, rotation (r)
		, z        (z)
	{
		UpdateLastFrameData();
	}

	Transform2D(
		vec2 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f, float z = 0.f
	)
		: position (position)
		, scale    (scale)
		, rotation (rotation)
		, z        (0.f)
	{
		UpdateLastFrameData();
	}

	Transform2D(
		vec3 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f
	)
		: position (position)
		, scale    (scale)
		, rotation (rotation)
		, z        (position.z)
	{
		UpdateLastFrameData();
	}

	Transform2D& operator*=(const Transform2D& other)
	{
		return *this = *this * other;
	}

	Transform2D operator*(const Transform2D& other) const
	{
		return Transform2D(
			vec3(position, z) + vec3(other.position, other.z), 
			scale * other.scale, 
			rotation + other.rotation
		);
	}

	glm::mat4 World() const
	{
		glm::mat4 world = glm::mat4(1.f);
		world = glm::translate(world, vec3(position, z));
		world = glm::rotate   (world, rotation, vec3(0.f, 0.f, 1.f));
		world = glm::scale    (world, vec3(scale, 1.f));

		return world;
	}

	void UpdateLastFrameData()
	{
		positionLast = position;
		scaleLast    = scale;
		rotationLast = rotation;
		zLast        = z;
	}

	Transform2D LastTransform() const
	{
		return Transform2D(positionLast, scaleLast, rotationLast, zLast);
	}
};