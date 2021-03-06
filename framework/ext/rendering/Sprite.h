#pragma once

#include "Rendering.h"

struct Sprite
{
	r<Texture> source;
	Color tint = Color(255, 255, 255, 255);

	// used to be just particles that had this, but everything can use it
	vec2 uvOffset = vec2(0.f, 0.f);
	vec2 uvScale  = vec2(1.f, 1.f);

	Sprite() : source(nullptr) {}
	Sprite(r<Texture> source) : source(source) {}
	Sprite(const Texture& sourceToCopy) : source(mkr<Texture>(sourceToCopy)) {}
	Texture& Get() { return *source; }

	Sprite& SetSource  (const r<Texture>& source) { this->source   = source; return *this; }
	Sprite& SetTint    (const Color&      tint)   { this->tint     = tint;   return *this; }
	Sprite& SetUvOffset(const vec2&       offset) { this->uvOffset = offset; return *this; }
	Sprite& SetUvScale (const vec2&       scale)  { this->uvScale  = scale;  return *this; }
};