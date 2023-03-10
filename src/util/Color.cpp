#include "util/Color.h"

Color::Color() : r(255), g(255), b(255), a(255) {}
Color::Color(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}
Color::Color(u32 rgba) : as_u32(rgba) {}

float Color::rf() const { return r / (float)255.f; }
float Color::gf() const { return g / (float)255.f; }
float Color::bf() const { return b / (float)255.f; }
float Color::af() const { return a / (float)255.f; }

Color& Color::operator*=(float scalar)       { return *this = operator*(scalar); }
Color  Color::operator* (float scalar) const { return fromv4(as_v4() * scalar); }

Color& Color::operator*=(const Color& other) { return *this = operator*(other); }
Color  Color::operator* (const Color& other) const { return fromv4(as_v4() * other.as_v4()); }

Color& Color::operator+=(const Color& other) { return *this = operator+(other); }
Color  Color::operator+ (const Color& other) const { return fromv4(clamp(as_v4() + other.as_v4(), vec4(0.f), vec4(1.f))); }

vec4 Color::as_v4() const
{
	return { rf(), gf(), bf(), af() };
}

Color Color::rand()
{
	return Color::rand(get_rand(255));
}

Color Color::rand(u8 alpha)
{
	return Color(get_rand(255), get_rand(255), get_rand(255), alpha);
}

Color Color::grey(u8 grey, u8 alpha)
{
	return Color(grey, grey, grey, alpha);
}

Color Color::from32(int bits32)
{
	return Color(r8(bits32), g8(bits32), b8(bits32), a8(bits32));
}

Color Color::fromv4(const vec4& v4)
{
	vec4 v = clamp(v4, vec4(0.f, 0.f, 0.f, 0.f), vec4(1.f, 1.f, 1.f, 1.f));
	return Color(u8(255 * v.x), u8(255 * v.y), u8(255 * v.z), u8(255 * v.w));
}

u8 r8(u32 bits32) { return (u8)(bits32 & 0x000000ff); }
u8 g8(u32 bits32) { return (u8)(bits32 & 0x0000ff00); }
u8 b8(u32 bits32) { return (u8)(bits32 & 0x00ff0000); }
u8 a8(u32 bits32) { return (u8)(bits32 & 0xff000000); }

Color lerp(const Color& a, const Color& b, float w)
{ 
	return Color::fromv4(lerp(a.as_v4(), b.as_v4(), w));
}
