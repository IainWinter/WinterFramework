#pragma once

#include "util/math.h"

struct Color
{
	union { 
		u32 as_u32; 
		struct { u8 r, g, b, a; };
	};
	
	// Sets the color white (255, 255, 255, 255)
	Color();

	Color(u8 r, u8 g, u8 b, u8 a = 255);
	explicit Color(u32 rgba); // should make explicit

	float rf() const;
	float gf() const;
	float bf() const;
	float af() const;

	Color  operator* (float scalar) const;
	Color& operator*=(float scalar);

	Color  operator* (const Color& other) const;
	Color& operator*=(const Color& other);

	Color  operator+ (const Color& other) const;
	Color& operator+=(const Color& other);

	vec4 as_v4() const;
    
    operator vec4() const;
    operator u32() const;
    
	static Color rand();
	static Color rand(u8 alpha);
	static Color grey(u8 grey, u8 alpha = 255);
	static Color from32(int bits32);
	static Color fromv4(const vec4& v4);
};

inline u8 r8(u32 bits32);
inline u8 g8(u32 bits32);
inline u8 b8(u32 bits32);
inline u8 a8(u32 bits32);

Color lerp(const Color& a, const Color& b, float w);
