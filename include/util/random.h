#pragma once

#include "util/math.h"

void rand_seed(int seed);

int rand_i();
bool rand_b();
float rand_f();
vec2 rand_2f();
vec3 rand_3f();

bool rand_bf(float odds);

int rand_ic();
float rand_fc();
vec2 rand_2fc();
vec3 rand_3fc();

int rand_im(int max);
float rand_fm(float max);
vec2 rand_2fm(vec2 max);
vec3 rand_3fm(vec3 max);
vec2 rand_2fm(float maxX, float maxY);
vec3 rand_3fm(float maxX, float maxY, float maxZ);

int rand_imm(int min, int max);
float rand_fmm(float min, float max);
vec2 rand_2fmm(vec2 min, vec2 max);
vec3 rand_3fmm(vec3 min, vec3 max);
vec2 rand_2fmm(float minX, float minY, float maxX, float maxY);
vec3 rand_3fmm(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

int rand_ima(int min, int addition);
float rand_fma(float min, float addition);
vec2 rand_2fma(vec2 min, vec2 addition);
vec3 rand_3fma(vec3 min, vec3 addition);
vec2 rand_2fma(float minX, float minY, float additionX, float additionY);
vec3 rand_3fma(float minX, float minY, float minZ, float additionX, float additionY, float additionZ);

int rand_icm(int extent);
float rand_fcm(float extent);
vec2 rand_2fcm(vec2 extent);
vec3 rand_3fcm(vec3 extent);
vec2 rand_2fcm(float extentX, float extentY);
vec3 rand_3fcm(float extentX, float extentY, float extentZ);

vec2 rand_2fn(float radius);
vec3 rand_3fn(float radius);

vec2 rand_2fcn(float radius);
vec3 rand_3fcn(float radius);

vec2 rand_2fcmn(float maxRadius);
vec3 rand_3fcmn(float maxRadius);

vec2 rand_2fcmmn(float minRadius, float maxRadius);
vec3 rand_3fcmmn(float minRadius, float maxRadius);

vec2 rand_outside_box(float extentX, float extentY, float paddingX, float paddingY);

template<typename _enum>
_enum rand_e(_enum count) {
	return static_cast<_enum>(rand_im(count));
}

// might just want to template it...

struct RandomBool {
    float odds;
    operator bool() const { return get(); }
    bool get() const { return rand_bf(odds); }
};

struct RandomInt {
    int min;
    int max;
    operator int() const { return get(); }
    int get() const { return rand_imm(min, max); }
};

struct RandomFloat {
    float min;
    float max;
    operator float() const { return get(); }
    float get() const { return rand_fmm(min, max); }
};

struct RandomFloat2 {
    vec2 min;
    vec2 max;
    operator vec2() const { return get(); }
    vec2 get() const { return rand_2fmm(min, max); }
    vec2 get_circle() const { return rand_2fcmn(1) * (max - min); }
};

struct RandomFloat3 {
    vec3 min;
    vec3 max;
    operator vec3() const { return get(); }
    vec3 get() const { return rand_3fmm(min, max); }
    vec3 get_circle() const { return rand_3fcmn(1) * (max - min); }
};

struct RandomFloat4 {
    vec4 min;
    vec4 max;
    operator vec4() const { return get(); }
    vec4 get() const { return vec4(rand_2fmm(min.x, min.y, max.x, max.y), rand_2fmm(min.z, min.w, max.z, max.w)); }
};

// maybe put in another file
#include <vector>

template<typename _t>
const _t& rand_item(const std::vector<_t>& vector) {
	return vector.at(rand_im((int)vector.size()));
}
