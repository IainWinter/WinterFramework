#pragma once

//MIT License
//
//Copyright (c) 2017 Luc Sinet
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "glm/vec2.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"

using namespace glm;

#include <utility>
#include <vector>
#include <functional>
#include <limits.h>

struct HitboxBounds
{
	ivec2 Min;
	ivec2 Max;

	HitboxBounds()
	{
		Min = ivec2(-INT_MAX, -INT_MAX);
		Max = ivec2( INT_MAX,  INT_MAX);
	}

	float Width()  const { return Max.x - Min.x + 1; }
	float Height() const { return Max.y - Min.y + 1; }
    float Area() const { return Width() * Height(); }
    
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

Hitbox MakeHitbox(int accuracy, int width, int height, const std::function<bool(int, int)>& solid, bool combineTriangles = true);
