#pragma once

#include "util/math.h"

void rand_seed(int seed);

int rand_i();
float rand_f();
bool rand_b();

bool rand_bf(float odds);

int rand_im(int max);
float rand_fm(float max);

int rand_imm(int min, int max);
float rand_fmm(float min, float max);

int rand_ima(int min, int addition);
float rand_fma(float min, float addition);

int rand_ic(int extent);
float rand_fc(float extent);

vec2 rand_2f();
vec2 rand_2fm(float maxX, float maxY);
vec2 rand_2fma(float minX, float minY, float additionX, float additionY);
vec2 rand_2fmm(float minX, float maxX, float minY, float maxY);
vec2 rand_2fc(float extentX, float extentY);
vec2 rand_2fn(float radius);

vec2 rand_outside_box(float extentX, float extentY, float paddingX, float paddingY);

template<typename _enum>
_enum rand_e(_enum count) {
	return static_cast<_enum>(rand_im(count));
}

// maybe put in another file
#include <vector>

template<typename _t>
const _t& rand_item(const std::vector<_t>& vector) {
	return vector.at(rand_im((int)vector.size()));
}