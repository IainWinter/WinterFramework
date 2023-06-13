#include "util/random.h"
#include <random>

#include "glm/gtc/random.hpp"

static std::mt19937 random;
static std::uniform_int_distribution<int> dist(0, INT_MAX);

void rand_seed(int seed) {
	random.seed(seed);
}

int _rand() {
	return dist(random);
}

int rand_i() {
	return _rand();
}

float rand_f() {
	return _rand() / (float)dist.max();
}

int rand_im(int max) {
	return _rand() % max;
}

float rand_fm(float max) {
	return rand_f() * max;
}

int rand_imm(int min, int max) {
	return min + rand_im(max);
}

float rand_fmm(float min, float max) {
	return min + rand_fm(max);
}

int rand_ic(int extent) {
	return -extent + rand_im(extent * 2);
}

float rand_fc(float extent) {
	return -extent + rand_fm(extent * 2.f);
}

vec2 rand_2f() {
	return vec2(rand_f(), rand_f());
}

vec2 rand_2fm(float maxX, float maxY) {
	return vec2(rand_fm(maxX), rand_fm(maxY));
}

vec2 rand_2fmm(float minX, float maxX, float minY, float maxY) {
	return vec2(rand_fmm(minX, maxX), rand_fmm(minY, maxY));
}

vec2 rand_2fc(float extentX, float extentY) {
	return vec2(rand_fc(extentX), rand_fc(extentY));
}

vec2 rand_2fn(float radius) {
	return on_unit(rand_fm(w2PI)) * radius;
}

vec2 rand_outside_box(float extentX, float extentY, float paddingX, float paddingY)
{
	return vec2();
}
