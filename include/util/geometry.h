#pragma once

#include "Common.h"
#include <type_traits>

vec2 TransformPoint(
	vec2& point,
	const Transform2D& transform)
{
	float c = cos(transform.rotation);
	float s = sin(transform.rotation);

	return vec2(
		(c * point.x - s * point.y) * transform.scale.x + transform.position.x,
		(s * point.x + c * point.y) * transform.scale.y + transform.position.y);
}

template<typename C, typename T = std::decay_t<decltype(*begin(std::declval<C>()))>>
void TransformPolygon(
	C& polygon,
	const Transform2D& transform)
{
	for (T& vert : polygon)
	{
		vert = TransformPoint(vert, transform);
	}
}