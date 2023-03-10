#pragma once

#include "Physics.h"

std::vector<vec2> GetBodyVertices(const Rigidbody2D& body);
vec2 GetClosestPointInLine(vec2 point, vec2 linePoint1, vec2 linePoint2);
vec2 GetClosestPoint(const Rigidbody2D& body, vec2 boxPoint);