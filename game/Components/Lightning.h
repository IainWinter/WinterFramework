#pragma once

#include "Entity.h"

struct Lightning
{
	Entity owner;
	Entity target;
	float maxDistance = 10;
	float arcDeviation = wPI / 6.f; // the random deviation on an arc to the target
};