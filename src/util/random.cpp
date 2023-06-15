#include "util/random.h"
#include <random>

static std::mt19937 g_random;
static std::uniform_int_distribution<int> dist(0, INT_MAX);

void rand_seed(int seed) {
	g_random.seed(seed);
}

int _rand() {
	return dist(g_random);
}

int rand_i() {
	return _rand();
}

float rand_f() {
	return _rand() / (float)dist.max();
}

bool rand_b() {
	return _rand() % 2 == 0;
}

int rand_im(int max) {
	if (max == 0)
		return 0;

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

int rand_ima(int min, int addition) {
	return min + rand_im(addition);
}

float rand_fma(float min, float addition) {
	return min + rand_fm(addition);
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

vec2 rand_2fma(float minX, float additionX, float minY, float additionY) {
	return vec2(rand_fma(minX, additionX), rand_fma(minY, additionY));
}

vec2 rand_2fc(float extentX, float extentY) {
	return vec2(rand_fc(extentX), rand_fc(extentY));
}

vec2 rand_2fn(float radius) {
	return on_unit(rand_fm(w2PI)) * radius;
}

vec2 rand_outside_box(float extentX, float extentY, float paddingX, float paddingY)
{
	// this seems complex because if you treat the corners as a part of one of the
	// side sections, the probability is off
	
	vec2 insidePadding;

	// push to edge based on sign

	float areaHorizontal = 2 * extentX  * paddingY;
	float areaVertical   = 2 * extentY  * paddingX;
	float areaCorner     =     paddingX * paddingY;

	float pickArea = rand_fm(areaHorizontal + areaVertical + areaCorner);

	if (pickArea < areaHorizontal)
	{
		insidePadding = rand_2fc(extentX, paddingY);

		bool top = insidePadding.y > 0;

		if (top) insidePadding.y += extentY;
		else     insidePadding.y -= extentY;
	}

	else if (pickArea < (areaHorizontal + areaVertical))
	{
		insidePadding = rand_2fc(paddingX, extentY);

		bool right = insidePadding.x > 0;

		if (right) insidePadding.x += extentX;
		else       insidePadding.x -= extentX;
	}

	else 
	{
		insidePadding = rand_2fc(paddingX, paddingY);

		bool right = insidePadding.x > 0;
		bool top = insidePadding.y > 0;

		if (right) insidePadding.x += extentX;
		else       insidePadding.x -= extentX;

		if (top) insidePadding.y += extentY;
		else     insidePadding.y -= extentY;
	}

	return insidePadding;
}