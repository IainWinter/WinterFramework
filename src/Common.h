#pragma once

#include "Defines.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <memory>
#include <utility>
#include <vector>
#include <functional>

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

// common components

struct Entity;

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
	Transform2D();
	Transform2D(float x, float y, float z = 0.f, float sx = 1.f, float sy = 1.f, float r = 0.f);
	Transform2D(vec2 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f, float z = 0.f);
	Transform2D(vec3 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f);

	Transform2D& SetPosition(vec2 position);
	Transform2D& SetScale   (vec2 scale);
	Transform2D& SetRotation(float rotation);
	Transform2D& SetZIndex  (float z);

	Transform2D  operator* (const Transform2D& other) const;
	Transform2D& operator*=(const Transform2D& other);

	mat4 World() const;

	void UpdateLastFrameData();
	Transform2D LastTransform() const;
};

struct Color
{
	union { 
		u32 as_u32; 
		struct { u8 r, g, b, a; };
	};
	
	// Sets the color white (255, 255, 255, 255)
	Color();

	Color(u8 r, u8 g, u8 b, u8 a = 255);
	Color(u32 rgba);

	float rf() const;
	float gf() const;
	float bf() const;
	float af() const;

	Color  operator* (float scalar) const;
	Color& operator*=(float scalar);

	Color  operator* (const Color& other) const;
	Color& operator*=(const Color& other);

	Color  operator+ (const Color& other) const;
	Color& operator+=(const Color& other);

	vec4 as_v4() const;

	static Color rand();
	static Color rand(u8 alpha);
	static Color from32(int bits32);
	static Color fromv4(const vec4& v4);
};

struct DestroyInTime
{
	float InSeconds;
};

struct aabb2D
{
	vec2 min;
	vec2 max;

	aabb2D();
	aabb2D(vec2 min, vec2 max);
	aabb2D(vec2 position, vec2 scale, float rotation);
	aabb2D(const std::vector<vec2>& points);

	vec2 center() const;
	bool fits(const aabb2D& other) const;

	float width() const;
	float height() const;
	vec2 Dimensions() const;

	static aabb2D from_points   (const std::vector<vec2>& points);
	static aabb2D from_transform(const Transform2D& points);
};

// Math helpers

float get_rand  (float x); // return a random number between (0, x)
float get_randc (float x); // return a random number between (-x/2, x/2)
int   get_rand  (int x); // return a random integer between (0, x)
vec2  get_rand  (float x, float y);
vec2  get_randc (float x, float y);
vec2  get_randn (float scale);
vec2  get_randnc(float scale);

float lerp(float        a, float        b, float w);
vec2  lerp(const vec2&  a, const vec2&  b, float w);
vec3  lerp(const vec3&  a, const vec3&  b, float w);
vec4  lerp(const vec4&  a, const vec4&  b, float w);
Color lerp(const Color& a, const Color& b, float w);

float clamp(float x, float min, float max);
vec2  clamp(vec2 x, const vec2& min, const vec2& max);
vec3  clamp(vec3 x, const vec3& min, const vec3& max);
vec4  clamp(vec4 x, const vec4& min, const vec4& max);

float max(const vec2& v);
float min(const vec2& v);

vec2 safe_normalize(const vec2& p);
vec2 rotate(const vec2& v, float a);
vec2 limit(const vec2& x, float max);

vec2 turn_towards(const vec2& current, const vec2& target, float strength);

float pow4(float x);
float angle(vec2 v);

template<typename _t>
std::pair<_t, _t> get_xy(const _t& index, const _t& width)
{
	return { index % width, index / width };
}

// vector helpers

template<typename _t, typename _int_t>
void pop_erase(std::vector<_t>& list, _int_t* index)
{
	list.at((size_t)*index) = std::move(list.back());
	list.pop_back();
	*index -= 1;
}

// Memory helpers

template<typename _t> 
using r = std::shared_ptr<_t>;

template<typename _t>
using wr = std::weak_ptr<_t>;

template<typename _t, typename... _args> 
r<_t> mkr(_args&&... args)
{ 
	return std::make_shared<_t>(std::forward<_args>(args)...); 
}

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