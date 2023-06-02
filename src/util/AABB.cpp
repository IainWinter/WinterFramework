#include "util/AABB.h"
#include <float.h>

aabb2D::aabb2D()
    : min ( FLT_MAX)
    , max (-FLT_MAX)
{}

aabb2D::aabb2D(vec2 min, vec2 max)
    : min (min)
    , max (max)
{}

aabb2D::aabb2D(vec2 position, vec2 scale, float rotation)
{
    *this = from_points({
        position + rotate(vec2( scale.x,  scale.y), rotation),
        position + rotate(vec2(-scale.x,  scale.y), rotation),
        position + rotate(vec2( scale.x, -scale.y), rotation),
        position + rotate(vec2(-scale.x, -scale.y), rotation)
    });
}

aabb2D::aabb2D(const std::vector<vec2>& points)
{
    *this = from_points(points);
}

vec2 aabb2D::center() const
{
    return (min + max) / 2.f;
}

bool aabb2D::fits(const aabb2D& other) const
{
    return min.x < other.min.x
        && min.y < other.min.y
        && max.x > other.max.x
        && max.y > other.max.y;
}

float aabb2D::width() const
{
    return max.x - min.x;
}

float aabb2D::height() const
{
    return max.y - min.y;
}

vec2 aabb2D::dimensions() const
{
    return vec2(width(), height());
}

aabb2D aabb2D::from_points(const std::vector<vec2>& points)
{
    aabb2D aabb;

    for (const vec2& p : points)
    {
        if (aabb.min.x > p.x) aabb.min.x = p.x;
        if (aabb.min.y > p.y) aabb.min.y = p.y;
        if (aabb.max.x < p.x) aabb.max.x = p.x;
        if (aabb.max.y < p.y) aabb.max.y = p.y;
    }

    return aabb;
}

aabb2D aabb2D::from_transform(const Transform2D& transform)
{
    return aabb2D(transform.position, transform.scale / 2.f, transform.rotation);
}
