#include "util/Transform.h"

Transform2D::Transform2D()
    : position (0.f, 0.f)
    , scale    (1.f, 1.f)
    , rotation (0.f)
    , z        (0.f)
{}

Transform2D::Transform2D(
    float x, float y, float z, float sx, float sy, float r
)
    : position (x, y)
    , scale    (sx, sy)
    , rotation (r)
    , z        (z)
{}

Transform2D::Transform2D(
    vec2 position, vec2 scale, float rotation, float z
)
    : position (position)
    , scale    (scale)
    , rotation (rotation)
    , z        (z)
{}

Transform2D::Transform2D(
    vec3 position, vec2 scale, float rotation
)
    : position (position)
    , scale    (scale)
    , rotation (rotation)
    , z        (position.z)
{}

Transform2D& Transform2D::SetPosition(vec2 position)      { this->position = position; return *this; }
Transform2D& Transform2D::SetScale   (vec2 scale)         { this->scale    = scale;    return *this; }
Transform2D& Transform2D::SetRotation(float rotation)     { this->rotation = rotation; return *this; }
Transform2D& Transform2D::SetZIndex  (float z)            { this->z        = z;        return *this; }
Transform2D& Transform2D::SetParent  (ParentFlags parent) { this->parent = parent;     return *this; }

Transform2D Transform2D::operator*(const Transform2D& other) const
{
    Transform2D out = other;

    // need to rotate position by parent angle to account for
    // rotation of parent before offset of parent
    out.position = rotate(other.position, rotation);

    if (other.parent & pPosition) out.position += position;
    if (other.parent & pScale)    out.scale    *= scale;
    if (other.parent & pRotation) out.rotation += rotation;

    return out;
}

Transform2D& Transform2D::operator*=(const Transform2D& other)
{
    return *this = *this * other;
}

mat4 Transform2D::World() const
{
    float sr = sin(rotation);
    float cr = cos(rotation);

    return mat4
    (
        scale.x *  cr, scale.x * sr,   0,   0,
        scale.y * -sr, scale.y * cr,   0,   0,
                    0,            0,   1,   0,
            position.x,  position.y,   z,   1
    );
}

vec2 Transform2D::Forward() const
{
    return on_unit(rotation);
}

vec2 Transform2D::Right() const
{
    return right(on_unit(rotation));
}
