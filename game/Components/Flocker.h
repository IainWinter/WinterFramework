#pragma once

struct Flocker
{
	float maxSpeed = 15;
	float maxForceFactor = 10;
	float desiredSeparation = 5.0f;
	float neighborDist = 5;

	Flocker& SetMaxSpeed         (float maxSpeed)          { this->maxSpeed         = maxSpeed;           return *this; }
	Flocker& SetMaxForceFactor   (float maxForceFactor)    { this->maxForceFactor    = maxForceFactor;    return *this; }
	Flocker& SetdesiredSeparation(float desiredSeparation) { this->desiredSeparation = desiredSeparation; return *this; }
	Flocker& SetNeighborDistanace(float neighborDistance)  { this->neighborDist      = desiredSeparation; return *this; }
};