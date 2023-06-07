#pragma once

#include "util/math.h"
#include "util/Transform.h"
#include <vector>

struct aabb2D
{
    vec2 min;
    vec2 max;

    aabb2D();
    aabb2D(vec2 min, vec2 max);
    aabb2D(vec2 position, vec2 scale, float rotation);
    aabb2D(const std::vector<vec2>& points);

    void combine_with(const aabb2D& other);
    void add_point(const vec2& point);

    vec2 center() const;

    bool fits(const aabb2D& other) const;
    bool contains_point(const vec2& point) const;

    float width() const;
    float height() const;
    vec2 dimensions() const;

    aabb2D transform(vec2 position, float angle) const;

    static aabb2D from_points   (const std::vector<vec2>& points);
    static aabb2D from_transform(const Transform2D& points);
};
