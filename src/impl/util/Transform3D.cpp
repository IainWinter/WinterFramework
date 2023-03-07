#include "util/Transform3D.h"

Transform::Transform()
    : position (0, 0, 0)
    , scale    (1, 1, 1)
    , rotation (1, 0, 0, 0)
{}

Transform::Transform(vec3 position, vec3 scale, quat rotation)
    : position (position)
    , scale    (scale)
    , rotation (rotation)
{}

// Rotation doesnt really work
Transform::Transform(const Transform2D& transform2D)
    : position (transform2D.position, transform2D.z)
    , scale    (transform2D.scale, 1.f)
    , rotation (vec3(0, 0, transform2D.rotation)) // rolls backwards sometimes
{}

Transform& Transform::SetPosition(vec3 position) { this->position = position; return *this; }
Transform& Transform::SetScale   (vec3 scale)    { this->scale    = scale;    return *this; }
Transform& Transform::SetRotation(quat rotation) { this->rotation = rotation; return *this; }
Transform& Transform::SetRotation(vec3 rotation) { this->rotation = rotation; return *this; }

Transform Transform::operator*(const Transform& other) const
{
    return Transform(
        position + other.position,
        scale * other.scale,
        rotation * other.rotation
    );
}

Transform& Transform::operator*=(const Transform& other)
{
    return *this = *this * other;
}

mat4 Transform::World() const
{
    mat4 world = mat4(1.f);

    world = glm::translate(world, position);
    world = glm::rotate   (world, angle(rotation), axis(rotation));
    world = glm::scale    (world, scale);

    return world;
}
