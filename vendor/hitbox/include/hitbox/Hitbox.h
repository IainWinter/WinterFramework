#pragma once

#include "glm/vec2.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"

using namespace glm;

#include <utility>
#include <vector>
#include <functional>

struct HitboxBounds
{
	vec2 Min;
	vec2 Max;

	HitboxBounds()
	{
		Min = vec2(-FLT_MAX, -FLT_MAX);
		Max = vec2( FLT_MAX,  FLT_MAX);
	}

	float Width()  const { return Max.x - Min.x; }
	float Height() const { return Max.y - Min.y; }
    
    vec2 Dims() const { return vec2(Width(), Height()); }
};

struct Hitbox
{
	std::vector<std::vector<vec2>> polygons;
	HitboxBounds bounds;
};

bool IsPolygonConvex(const std::vector<vec2>& polygon);
float GetPolygonArea(const std::vector<vec2>& polygon);
HitboxBounds MakeHitboxBounds(const std::vector<vec2>& points);

std::vector<vec2> MakeContour(const bool* mask_grid, int width, int height);
std::vector<vec2> MakePolygon(const std::vector<vec2>& contour, const HitboxBounds& boundingBox, int accuracy);

std::vector<std::vector<vec2>> MakeTriangles(const std::vector<vec2>& polygon);
std::vector<vec2> FlattenTriangleList(const std::vector<std::vector<vec2>>& triangles);

std::vector<std::vector<vec2>> CombineTriangles(const std::vector<vec2>& polygon);
std::vector<std::vector<vec2>> CombineTriangles(const std::vector<std::vector<vec2>>& triangles);

std::pair<std::vector<std::vector<vec2>>, HitboxBounds> MakeHitbox(int accuracy, int width, int height, const std::function<bool(int, int)>& solid);
