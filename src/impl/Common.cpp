#include "Common.h"

Transform2D::Transform2D()
	: position (0.f, 0.f)
	, scale    (1.f, 1.f)
	, rotation (0.f)
	, z        (0.f)
{
	UpdateLastFrameData();
}

Transform2D::Transform2D(
	float x, float y, float z, float sx, float sy, float r
)
	: position (x, y)
	, scale    (sx, sy)
	, rotation (r)
	, z        (z)
{
	UpdateLastFrameData();
}

Transform2D::Transform2D(
	vec2 position, vec2 scale, float rotation, float z
)
	: position (position)
	, scale    (scale)
	, rotation (rotation)
	, z        (0.f)
{
	UpdateLastFrameData();
}

Transform2D::Transform2D(
	vec3 position, vec2 scale, float rotation
)
	: position (position)
	, scale    (scale)
	, rotation (rotation)
	, z        (position.z)
{
	UpdateLastFrameData();
}

Transform2D& Transform2D::SetPosition(vec2 position)  { this->position = position; return *this; }
Transform2D& Transform2D::SetScale   (vec2 scale)     { this->scale    = scale;    return *this; }
Transform2D& Transform2D::SetRotation(float rotation) { this->rotation = rotation; return *this; }
Transform2D& Transform2D::SetZIndex  (float z)        { this->z        = z;        return *this; }

Transform2D Transform2D::operator*(const Transform2D& other) const
{
	return Transform2D(
		vec3(position, z) + vec3(other.position, other.z), 
		scale * other.scale, 
		rotation + other.rotation
	);
}

Transform2D& Transform2D::operator*=(const Transform2D& other)
{
	return *this = *this * other;
}

mat4 Transform2D::World() const
{
	float sr = sin(rotation);
	float cr = cos(rotation);

	return mat4
	(
		scale.x *  cr, scale.x * sr,   0,   0,
		scale.y * -sr, scale.y * cr,   0,   0,
			        0,            0,   1,   0,
			position.x,  position.y,   z,   1
	);
}

void Transform2D::UpdateLastFrameData()
{
	positionLast = position;
	scaleLast    = scale;
	rotationLast = rotation;
	zLast        = z;
}

Transform2D Transform2D::LastTransform() const
{
	return Transform2D(positionLast, scaleLast, rotationLast, zLast);
}

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

Color Color::from32(int bits32)
{
	return Color(0x000000ff & bits32, 0x0000ff00 & bits32, 0x00ff0000 & bits32, 0xff000000 & bits32);
}

Color Color::fromv4(const vec4& v4)
{
	vec4 v = clamp(v4, vec4(0.f, 0.f, 0.f, 0.f), vec4(1.f, 1.f, 1.f, 1.f));
	return Color(u8(255 * v.x), u8(255 * v.y), u8(255 * v.z), u8(255 * v.w));
}

aabb2D::aabb2D()
	: min ( FLT_MAX)
	, max (-FLT_MAX)
{}

aabb2D::aabb2D(
	vec2 min, vec2 max
)
	: min (min)
	, max (max)
{}

aabb2D::aabb2D(
	vec2 position, vec2 scale, float rotation
)
{
	*this = from_points({
		position + rotate(vec2( scale.x,  scale.y), rotation),
		position + rotate(vec2(-scale.x,  scale.y), rotation),
		position + rotate(vec2( scale.x, -scale.y), rotation),
		position + rotate(vec2(-scale.x, -scale.y), rotation)
	});
}

aabb2D::aabb2D(
	const std::vector<vec2>& points
)
{
	*this = from_points(points);
}

vec2 aabb2D::center() const
{
	return (min + max) / 2.f;
}

bool aabb2D::fits(const aabb2D& other) const
{
	return min.x < other.min.x
		&& min.y < other.min.y
		&& max.x > other.max.x
		&& max.y > other.max.y;
}

float aabb2D::width() const
{
	return max.x - min.x;
}

float aabb2D::height() const
{
	return max.y - min.y;
}

vec2 aabb2D::Dimensions() const
{
	return vec2(width(), height());
}

aabb2D aabb2D::from_points(const std::vector<vec2>& points)
{
	aabb2D aabb;

	for (const vec2& p : points)
	{
		if (aabb.min.x > p.x) aabb.min.x = p.x;
		if (aabb.min.y > p.y) aabb.min.y = p.y;
		if (aabb.max.x < p.x) aabb.max.x = p.x;
		if (aabb.max.y < p.y) aabb.max.y = p.y;
	}

	return aabb;
}

aabb2D aabb2D::from_transform(const Transform2D& transform)
{
	return aabb2D(transform.position, transform.scale, transform.rotation);
}

int   get_rand  (int x)            { return x == 0 ? 0 : rand() % x; }
float get_rand  (float x)          { return x * rand() / (float)RAND_MAX; }
float get_randc (float x)          { return x * rand() / (float)RAND_MAX - x / 2.f; }
vec2  get_rand  (float x, float y) { return vec2(get_rand (x), get_rand (y)); }
vec2  get_randc (float x, float y) { return vec2(get_randc(x), get_randc(y)); }
vec2  get_randn (float scale)      { return get_rand(scale) * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }
vec2  get_randnc(float scale)      { return           scale * safe_normalize(vec2(get_randc(1.f), get_randc(1.f))); }

float lerp(float        a, float        b, float w) { return a + w * (b - a); }
vec2  lerp(const vec2&  a, const vec2&  b, float w) { return a + w * (b - a); }
vec3  lerp(const vec3&  a, const vec3&  b, float w) { return a + w * (b - a); }
vec4  lerp(const vec4&  a, const vec4&  b, float w) { return a + w * (b - a); }
Color lerp(const Color& a, const Color& b, float w) { return Color::fromv4(lerp(a.as_v4(), b.as_v4(), w)); }

float max(const vec2& v) { return max(v.x, v.y); }
float min(const vec2& v) { return min(v.x, v.y); }

float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x; 
}

vec2 clamp(vec2 x, const vec2& min, const vec2& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	return x;
}

vec3 clamp(vec3 x, const vec3& min, const vec3& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	return x;
}

vec4 clamp(vec4 x, const vec4& min, const vec4& max)
{
	x.x = clamp(x.x, min.x, max.x);
	x.y = clamp(x.y, min.y, max.y);
	x.z = clamp(x.z, min.z, max.z);
	x.w = clamp(x.w, min.w, max.w);
	return x;
}

vec2 safe_normalize(const vec2& p)
{
	float n = sqrt((float)(p.x * p.x + p.y * p.y));
	return (n == 0) ? vec2(0.f, 0.f) : vec2(p.x / n, p.y / n);
}

vec2 rotate(const vec2& v, float a)
{
	float s = sin(a);
	float c = cos(a);
	return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

vec2 limit(const vec2& x, float max)
{
	float d = length(x);
	if (d > max) return x / d * max;
	return x;
}

vec2 turn_towards(const vec2& current, const vec2& target, float strength)
{
	vec2 nVel = safe_normalize(current);
	vec2 nDir = safe_normalize(target);
	vec2 delta = (nDir - nVel) * strength;

	return safe_normalize(current + delta) * length(current);
}

float pow4(float x)
{
	return x * x * x * x;
}

float angle(vec2 v)
{
	return atan2(v.y, v.x);
}