#pragma once

#include <string>
#include "ext/Time.h"

// using quat = vec4;

struct NameComponent
{
	std::string Name;
};

struct DestroyInTime
{
	float InSeconds;
};