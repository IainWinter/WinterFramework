#include "util/math.h"

int   get_rand  (int x)            { return x == 0 ? 0 : rand() % x; }
float get_rand  (float x)          { return x * rand() / (float)RAND_MAX; }
float get_randc (float x)          { return x * rand() / (float)RAND_MAX - x / 2.f; }
vec2  get_rand  (float x, float y) { return vec2(get_rand (x), get_rand (y)); }
vec2  get_randc (float x, float y) { return vec2(get_randc(x), get_randc(y)); }
vec2  get_randn (float scale)      { return get_rand(scale) * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }
vec2  get_randnc(float scale)      { return           scale * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }

vec2 get_rand_jitter(vec2 x, float jitter)
{
	return x + get_randc(2.f, 2.f) * length(x) * jitter;
}

float lerp(float        a, float        b, float w) { return a + w * (b - a); }
vec2  lerp(const vec2&  a, const vec2&  b, float w) { return a + w * (b - a); }
vec3  lerp(const vec3&  a, const vec3&  b, float w) { return a + w * (b - a); }
vec4  lerp(const vec4&  a, const vec4&  b, float w) { return a + w * (b - a); }

float max(const vec2& v) { return max(v.x, v.y); }
float min(const vec2& v) { return min(v.x, v.y); }

float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x; 
}

vec2 clamp(vec2 x, const vec2& min, const vec2& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	return x;
}

vec3 clamp(vec3 x, const vec3& min, const vec3& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	return x;
}

vec4 clamp(vec4 x, const vec4& min, const vec4& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	x.w = clamp(x.w, min.w, max.w);
	return x;
}

vec2 safe_normalize(const vec2& p)
{
	float n = sqrt((float)(p.x * p.x + p.y * p.y));
	return (n == 0) ? vec2(0.f, 0.f) : vec2(p.x / n, p.y / n);
}

vec2 rotate(const vec2& v, float a)
{
	float s = sin(a);
	float c = cos(a);
	return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

vec2 limit(const vec2& x, float max)
{
	float d = length(x);
	if (d > max) return x / d * max;
	return x;
}

vec2 turn_towards(const vec2& current, const vec2& target, float strength)
{
	vec2 nVel = safe_normalize(current);
	vec2 nDir = safe_normalize(target);
	vec2 delta = (nDir - nVel) * strength;

	return safe_normalize(current + delta) * length(current);
}

vec2 on_unit(float a)
{
	return vec2(cos(a), sin(a));
}

vec2 right(vec2 v)
{
	return vec2(v.y, -v.x); // todo: verify this is right
}

float pow4(float x)
{
	return x * x * x * x;
}

float angle(vec2 v)
{
	return atan2(v.y, v.x);
}

float aspect(const vec2& v)
{
	return v.x / v.y;
}

bool fe(float a, float b, float e)
{
	return abs(a - b) < e;
}