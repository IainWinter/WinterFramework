#pragma once

#include "Rendering.h"

Mesh GenerateCircle(int numberOfPoints, float radius = 1.f)
{
	assert(numberOfPoints >= 3 && "Number of points must be above 3");

	std::vector<vec2> points;

	float angle = 2 * pi<float>() / numberOfPoints;
	for (int i = 0; i < numberOfPoints; i++)
	{
		float a = angle * i;
		vec2 v = vec2(cos(a), sin(a)) * radius;
		points.push_back(v);
	}

	Mesh mesh;
	mesh.Add(Mesh::aPosition, points);
	mesh.topology = Mesh::tLoops;

	return mesh;
}