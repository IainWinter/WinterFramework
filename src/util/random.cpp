#include "util/random.h"
#include <random>
#include <limits.h>

static std::mt19937 g_random;
static std::uniform_int_distribution<int> dist(0, INT_MAX);

int _rand() {
    return dist(g_random);
}

void rand_seed(int seed) {
	g_random.seed(seed);
}

int rand_i() {
    return _rand();
}

bool rand_b() {
    return _rand() % 2 == 0;
}

float rand_f() {
    return _rand() / (float)dist.max();
}

vec2 rand_2f() {
    return vec2(rand_f(), rand_f());
}

vec3 rand_3f() {
    return vec3(rand_f(), rand_f(), rand_f());
}

bool rand_bf(float odds) {
    return rand_f() < odds;
}

int rand_ic() {
    return rand_icm(1.f);
}

float rand_fc() {
    return rand_fcm(1.f);
}

vec2 rand_2fc() {
    return rand_2fcm(1.f, 1.f);
}

vec3 rand_3fc() {
    return rand_3fcm(1.f, 1.f, 1.f);
}

int rand_im(int max) {
    if (max == 0)
        return 0;

    return _rand() % max;
}

float rand_fm(float max) {
    return rand_f() * max;
}

vec2 rand_2fm(vec2 max) {
    return rand_2f() * max;
}

vec3 rand_3fm(vec3 max) {
    return rand_3f() * max;
}

vec2 rand_2fm(float maxX, float maxY) {
    return rand_2fm(vec2(maxX, maxY));
}

vec3 rand_3fm(float maxX, float maxY, float maxZ) {
    return rand_3fm(vec3(maxX, maxY, maxZ));
}

int rand_imm(int min, int max) {
    return rand_ima(min, max - min);
}

float rand_fmm(float min, float max) {
    return rand_fma(min, max - min);
}

vec2 rand_2fmm(vec2 min, vec2 max) {
    return rand_2fma(min, max - min);
}

vec3 rand_3fmm(vec3 min, vec3 max) {
    return rand_3fma(min, max - min);
}

vec2 rand_2fmm(float minX, float minY, float maxX, float maxY) {
    return rand_2fmm(vec2(minX, minY), vec2(maxX, maxY));
}

vec3 rand_3fmm(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    return rand_3fmm(vec3(minX, minY, minZ), vec3(maxX, maxY, maxZ));
}

int rand_ima(int min, int addition) {
    return min + rand_im(addition);
}

float rand_fma(float min, float addition) {
    return min + rand_fm(addition);
}

vec2 rand_2fma(vec2 min, vec2 addition) {
    return min + rand_2fm(addition);
}

vec3 rand_3fma(vec3 min, vec3 addition) {
    return min + rand_3fm(addition);
}

vec2 rand_2fma(float minX, float minY, float additionX, float additionY) {
    return rand_2fma(vec2(minX, minY), vec2(additionX, additionY));
}

vec3 rand_3fma(float minX, float minY, float minZ, float additionX, float additionY, float additionZ) {
    return rand_3fma(vec3(minX, minY, minZ), vec3(additionX, additionY, additionZ));
}

int rand_icm(int extent) {
    return -extent + rand_im(extent * 2);
}

float rand_fcm(float extent) {
    return -extent + rand_fm(extent * 2.f);
}

vec2 rand_2fcm(vec2 extent) {
    return -extent + rand_2fm(extent * 2.f);
}

vec3 rand_3fcm(vec3 extent) {
    return -extent + rand_3fm(extent * 2.f);
}

vec2 rand_2fcm(float extentX, float extentY) {
    return rand_2fcm(vec2(extentX, extentY));
}

vec3 rand_3fcm(float extentX, float extentY, float extentZ) {
    return rand_3fcm(vec3(extentX, extentY, extentZ));
}

vec2 rand_2fn(float radius) {
    return on_unit(rand_fmm(0, wPI / 2)) * radius;
}

vec3 rand_3fn(float radius) {
    return normalize(rand_3f()) * radius;
}

vec2 rand_2fcn(float radius) {
    return normalize(rand_2fc()) * radius;
}

vec3 rand_3fcn(float radius) {
    return normalize(rand_3fc()) * radius;
}

vec2 rand_2fcmn(float maxRadius) {
    return on_unit(rand_fm(w2PI)) * rand_fm(maxRadius);
}

vec3 rand_3fcmn(float maxRadius) {
    float theta = rand_fm(w2PI);
    float phi = acos(rand_fcm(1.f));

    return on_unit3(phi, theta) * rand_fm(maxRadius);
}

vec2 rand_2fcmmn(float minRadius, float maxRadius) {
    return on_unit(rand_fm(w2PI)) * rand_fmm(minRadius, maxRadius);
}

vec3 rand_3fcmmn(float minRadius, float maxRadius) {
    float theta = rand_fm(w2PI);
    float phi = acos(rand_fcm(1.f));

    return on_unit3(phi, theta) * rand_fmm(minRadius, maxRadius);
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
		insidePadding = rand_2fcm(extentX, paddingY);

		bool top = insidePadding.y > 0;

		if (top) insidePadding.y += extentY;
		else     insidePadding.y -= extentY;
	}

	else if (pickArea < (areaHorizontal + areaVertical))
	{
		insidePadding = rand_2fcm(paddingX, extentY);

		bool right = insidePadding.x > 0;

		if (right) insidePadding.x += extentX;
		else       insidePadding.x -= extentX;
	}

	else 
	{
		insidePadding = rand_2fcm(paddingX, paddingY);

		bool right = insidePadding.x > 0;
		bool top = insidePadding.y > 0;

		if (right) insidePadding.x += extentX;
		else       insidePadding.x -= extentX;

		if (top) insidePadding.y += extentY;
		else     insidePadding.y -= extentY;
	}

	return insidePadding;
}
