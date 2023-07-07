#pragma once

#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"

#include "util/typedef.h"

#include <utility>

#define wPI 3.1415926535f
#define w2PI wPI * 2.f

// make glm the default math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

float get_rand  (float x); // return a random number between (0, x)
float get_randc (float x); // return a random number between (-x/2, x/2)
int   get_rand  (int x);   // return a random integer between (0, x)
vec2  get_rand  (float x, float y);
vec2  get_randc (float x, float y);
vec2  get_randn (float scale);
vec2  get_randnc(float scale);

vec2 get_rand_jitter(vec2 x, float jitter);

float lerpf(float        a, float        b, float w);
vec2  lerp (const vec2&  a, const vec2&  b, float w);
vec3  lerp (const vec3&  a, const vec3&  b, float w);
vec4  lerp (const vec4&  a, const vec4&  b, float w);

// do a lerp per value
// casts to a float array
template<typename _t>
_t lerpfa(const _t& a, const _t& b, float w)
{
	float* af = (float*)&a;
	float* bf = (float*)&b;

	size_t length = sizeof(_t) / sizeof(float);

	_t out;
	float* of = (float*)&out;

	for (size_t i = 0; i < length; i++)
	{
		of[i] = lerpf(af[i], bf[i], w);
	}

	return out;
}

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

vec2 on_unit(float a);
vec3 on_unit3(float phi, float theta);
vec2 right(vec2 v);
vec2 left(vec2 v);

float pow4(float x);
float angle(vec2 v);

float aspect(const vec2& v);

bool fe(float a, float b, float e = 0.0001f);

ivec2 get_xy(int index, int width);
std::pair<int, int> get_xyp(int index, int width);

template<typename _t>
_t value_or(void* ptr, const _t& defaultValue)
{
	return ptr ? *(_t*)ptr : defaultValue;
}

template<typename _t>
_t value_or(void* ptr, size_t index, const _t& defaultValue)
{
	return ptr ? *(_t*)ptr + index : defaultValue;
}
