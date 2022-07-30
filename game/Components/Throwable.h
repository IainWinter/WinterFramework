#pragma once

#include "Entity.h"

struct Throwable
{
	int _pad;
};

struct Thrower
{
	EntityWith<Transform2D> target;
	Entity throwing;
	float grabRadius = 14.f;
	float throwingForce = 30.f;
																	     
	Thrower& SetTarget       (Entity target)       { this->target        = target;        return *this; }
	Thrower& SetThrowing     (Entity throwing)     { this->throwing      = throwing;      return *this; }
	Thrower& SetGrabRadius   (float grabRadius)    { this->grabRadius    = grabRadius;    return *this; }
	Thrower& SetThrowingForce(float throwingForce) { this->throwingForce = throwingForce; return *this; }
};