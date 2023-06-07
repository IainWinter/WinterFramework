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

void aabb2D::combine_with(const aabb2D& other)
{
    if (min.x > other.min.x) min.x = other.min.x;
    if (min.y > other.min.y) min.y = other.min.y;
    if (max.x < other.max.x) max.x = other.max.x;
    if (max.y < other.max.y) max.y = other.max.y;
}

void aabb2D::add_point(const vec2& point)
{
    if (min.x > point.x) min.x = point.x;
    if (min.y > point.y) min.y = point.y;
    if (max.x < point.x) max.x = point.x;
    if (max.y < point.y) max.y = point.y;
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

bool aabb2D::contains_point(const vec2& point) const
{
    return point.x > min.x
        && point.y > min.y
        && point.x < max.x
        && point.y < max.y;
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

aabb2D aabb2D::transform(vec2 position, float angle) const
{
    return from_points({
        position + rotate(vec2(max.x, max.y), angle),
        position + rotate(vec2(min.x, max.y), angle),
        position + rotate(vec2(max.x, min.y), angle),
        position + rotate(vec2(min.x, min.y), angle)
    });
}

aabb2D aabb2D::from_points(const std::vector<vec2>& points)
{
    aabb2D aabb;

    for (const vec2& p : points)
        aabb.add_point(p);
    
    return aabb;
}

aabb2D aabb2D::from_transform(const Transform2D& transform)
{
    return aabb2D(transform.position, transform.scale / 2.f, transform.rotation);
}
