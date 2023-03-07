#pragma once

#include "util/Transform.h"
#include "glm/gtx/quaternion.hpp"

struct Transform
{
    vec3 position;
    vec3 scale;
    quat rotation;

    Transform();
    Transform(vec3 position, vec3 scale = vec3(1.f, 1.f, 1.f), quat rotation = quat(1.f, 0.f, 0.f, 0.f));
    Transform(const Transform2D& transform2D);

    Transform& SetPosition(vec3 position);
    Transform& SetScale   (vec3 scale);
    Transform& SetRotation(quat rotation);
    Transform& SetRotation(vec3 rotation);

    Transform  operator* (const Transform& other) const;
    Transform& operator*=(const Transform& other);

    mat4 World()   const;
    //vec3 Forward() const; // todo
    //vec3 Right()   const;

    // doesnt have interpolation yet...

    // might want to make this just a mode of a transform, aka have a flag for 2d or 3d mode
    // that will make the code cleaner, but for now this works. Its only used in the line renderer for the camera gizmo
};
