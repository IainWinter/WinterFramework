#pragma once

#include "util/math.h"
#include "glm/ext/matrix_float4x4.hpp"

enum ParentFlags : int
{
    pNone = 0b0000,
    pPosition = 0b0001,
    pScale = 0b0010,
    pRotation = 0b0100,
    pZ = 0b1000,
    pAll = 0b1111
};

inline ParentFlags operator|(ParentFlags a, ParentFlags b)
{
    return static_cast<ParentFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct Transform2D
{
    vec2 position;
    vec2 scale;
    float rotation;
    float z;

    ParentFlags parent = pAll;

public:
    Transform2D();
    Transform2D(float x, float y, float z = 0.f, float sx = 1.f, float sy = 1.f, float r = 0.f);
    Transform2D(vec2 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f, float z = 0.f);
    Transform2D(vec3 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f);

    Transform2D& SetPosition(vec2 position);
    Transform2D& SetScale   (vec2 scale);
    Transform2D& SetRotation(float rotation);
    Transform2D& SetZIndex  (float z);
    Transform2D& SetParent  (ParentFlags parent);

    Transform2D  operator* (const Transform2D& other) const;
    Transform2D& operator*=(const Transform2D& other);

    mat4 World()   const;
    vec2 Forward() const;
    vec2 Right()   const;
};
