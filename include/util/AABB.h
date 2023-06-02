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

    vec2 center() const;
    bool fits(const aabb2D& other) const;

    float width() const;
    float height() const;
    vec2 dimensions() const;

    static aabb2D from_points   (const std::vector<vec2>& points);
    static aabb2D from_transform(const Transform2D& points);
};
