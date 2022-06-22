#pragma once

#include "Rendering.h"

struct Sprite
{
	r<Texture> source;
	Sprite() : source(nullptr) {}
	Sprite(r<Texture> source) : source(source) {}
	Sprite(const Texture& sourceToCopy) : source(mkr<Texture>(sourceToCopy)) {}
	Texture& Get() { return *source; }
};