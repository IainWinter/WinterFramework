#include "Common.h"
#include <filesystem>
#include <array>
#include <unordered_map>

Transform2D::Transform2D()
	: position (0.f, 0.f)
	, scale    (1.f, 1.f)
	, rotation (0.f)
	, z        (0.f)
{}

Transform2D::Transform2D(
	float x, float y, float z, float sx, float sy, float r
)
	: position (x, y)
	, scale    (sx, sy)
	, rotation (r)
	, z        (z)
{}

Transform2D::Transform2D(
	vec2 position, vec2 scale, float rotation, float z
)
	: position (position)
	, scale    (scale)
	, rotation (rotation)
	, z        (0.f)
{}

Transform2D::Transform2D(
	vec3 position, vec2 scale, float rotation
)
	: position (position)
	, scale    (scale)
	, rotation (rotation)
	, z        (position.z)
{}

Transform2D& Transform2D::SetPosition(vec2 position)      { this->position = position; return *this; }
Transform2D& Transform2D::SetScale   (vec2 scale)         { this->scale    = scale;    return *this; }
Transform2D& Transform2D::SetRotation(float rotation)     { this->rotation = rotation; return *this; }
Transform2D& Transform2D::SetZIndex  (float z)            { this->z        = z;        return *this; }
Transform2D& Transform2D::SetParent  (ParentFlags parent) { this->parent = parent;     return *this; }

Transform2D Transform2D::operator*(const Transform2D& other) const
{
	Transform2D out = other;

	// need to rotate position by parent angle to account for
	// rotation of parent before offset of parent
	out.position = rotate(other.position, rotation);

	if (other.parent & pPosition) out.position += position;
	if (other.parent & pScale)    out.scale    *= scale;
	if (other.parent & pRotation) out.rotation += rotation;

	return out;
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

vec2 Transform2D::Forward() const
{
	return on_unit(rotation);
}

vec2 Transform2D::Right() const
{
	return right(on_unit(rotation));
}

Transform::Transform()
	: position (0, 0, 0)
	, scale    (1, 1, 1)
	, rotation (1, 0, 0, 0)
{}

Transform::Transform(vec3 position, vec3 scale, quat rotation)
	: position (position)
	, scale    (scale)
	, rotation (rotation)
{}

// Rotation doesnt really work
Transform::Transform(const Transform2D& transform2D)
	: position (transform2D.position, transform2D.z)
	, scale    (transform2D.scale, 1.f)
	, rotation (vec3(0, 0, transform2D.rotation)) // rolls backwards sometimes
{}

Transform& Transform::SetPosition(vec3 position) { this->position = position; return *this; }
Transform& Transform::SetScale   (vec3 scale)    { this->scale    = scale;    return *this; }
Transform& Transform::SetRotation(quat rotation) { this->rotation = rotation; return *this; }
Transform& Transform::SetRotation(vec3 rotation) { this->rotation = rotation; return *this; }

Transform Transform::operator*(const Transform& other) const
{
	return Transform(
		position + other.position,
		scale * other.scale,
		rotation * other.rotation
	);
}

Transform& Transform::operator*=(const Transform& other)
{
	return *this = *this * other;
}

mat4 Transform::World() const
{
	mat4 world = mat4(1.f);

	world = glm::translate(world, position);
	world = glm::scale    (world, scale);
	world = glm::rotate   (world, angle(rotation), axis(rotation));

	return world;
}

//vec3 Transform::Forward() const
//{
//}
//vec3 Transform::Right() const
//{
//}

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

aabb2D::aabb2D(vec2 position, vec2 scale, float rotation)
{
	*this = from_points({
		position + rotate(vec2( scale.x,  scale.y), rotation),
		position + rotate(vec2(-scale.x,  scale.y), rotation),
		position + rotate(vec2( scale.x, -scale.y), rotation),
		position + rotate(vec2(-scale.x, -scale.y), rotation)
	});
}

aabb2D::aabb2D(const std::vector<vec2>& points)
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

vec2 get_rand_jitter(vec2 x, float jitter)
{
	return x + get_randc(2.f, 2.f) * length(x) * jitter;
}

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

vec2 on_unit(float a)
{
	return vec2(cos(a), sin(a));
}

vec2 right(vec2 v)
{
	return vec2(v.y, -v.x); // todo: verify this is right
}

float pow4(float x)
{
	return x * x * x * x;
}

float angle(vec2 v)
{
	return atan2(v.y, v.x);
}

float aspect(const vec2& v)
{
	return v.x / v.y;
}

ControllerInput MapSDLGameControllerButton(SDL_GameControllerButton button)
{
	if (button == SDL_CONTROLLER_BUTTON_INVALID)
	{
		return ControllerInput::cBUTTON_INVALID;
	}

	return (ControllerInput)button;
}

ControllerInput MapSDLGameControllerAxis(SDL_GameControllerAxis axis)
{
	if (axis == SDL_CONTROLLER_AXIS_INVALID)
	{
		return ControllerInput::cAXIS_INVALID;
	}

	return (ControllerInput)(1 + axis + ControllerInput::cBUTTON_MAX);
}

int GetInputCode(KeyboardInput scancode)
{
	return (int)scancode;
}

int GetInputCode(MouseInput input)
{
	return (int)input + SDL_NUM_SCANCODES;
}

int GetInputCode(ControllerInput input)
{
	return (int)input + SDL_NUM_SCANCODES + MOUSE_INPUT_COUNT;
}

const char* GetInputCodeName(int code)
{
	static std::unordered_map<int, const char*> names =
	{
		{ GetInputCode(KEY_A),               "Key A" },
		{ GetInputCode(KEY_B),               "Key B" },
		{ GetInputCode(KEY_C),               "Key C" },
		{ GetInputCode(KEY_D),               "Key D" },
		{ GetInputCode(KEY_E),               "Key E" },
		{ GetInputCode(KEY_F),               "Key F" },
		{ GetInputCode(KEY_G),               "Key G" },
		{ GetInputCode(KEY_H),               "Key H" },
		{ GetInputCode(KEY_I),               "Key I" },
		{ GetInputCode(KEY_J),               "Key J" },
		{ GetInputCode(KEY_K),               "Key K" },
		{ GetInputCode(KEY_L),               "Key L" },
		{ GetInputCode(KEY_M),               "Key M" },
		{ GetInputCode(KEY_N),               "Key N" },
		{ GetInputCode(KEY_O),               "Key O" },
		{ GetInputCode(KEY_P),               "Key P" },
		{ GetInputCode(KEY_Q),               "Key Q" },
		{ GetInputCode(KEY_R),               "Key R" },
		{ GetInputCode(KEY_S),               "Key S" },
		{ GetInputCode(KEY_T),               "Key T" },
		{ GetInputCode(KEY_U),               "Key U" },
		{ GetInputCode(KEY_V),               "Key V" },
		{ GetInputCode(KEY_W),               "Key W" },
		{ GetInputCode(KEY_X),               "Key X" },
		{ GetInputCode(KEY_Y),               "Key Y" },
		{ GetInputCode(KEY_Z),               "Key Z" },
		{ GetInputCode(KEY_1),               "Key 1" },
		{ GetInputCode(KEY_2),               "Key 2" },
		{ GetInputCode(KEY_3),               "Key 3" },
		{ GetInputCode(KEY_4),               "Key 4" },
		{ GetInputCode(KEY_5),               "Key 5" },
		{ GetInputCode(KEY_6),               "Key 6" },
		{ GetInputCode(KEY_7),               "Key 7" },
		{ GetInputCode(KEY_8),               "Key 8" },
		{ GetInputCode(KEY_9),               "Key 9" },
		{ GetInputCode(KEY_0),               "Key 0" },
		{ GetInputCode(KEY_Return),          "Key Return" },
		{ GetInputCode(KEY_Escape),          "Key Escape" },
		{ GetInputCode(KEY_Backspace),       "Key Backspace" },
		{ GetInputCode(KEY_Tab),             "Key Tab" },
		{ GetInputCode(KEY_Space),           "Key Space" },
		{ GetInputCode(KEY_Minus),           "Key Minus" },
		{ GetInputCode(KEY_Equals),          "Key Equals" },
		{ GetInputCode(KEY_Bracket_Left),    "Key Bracket Left" },
		{ GetInputCode(KEY_Bracket_Right),   "Key Bracket Right" },
		{ GetInputCode(KEY_Backslash),       "Key Backslash" },
		{ GetInputCode(KEY_SemiColon),       "Key SemiColon" },
		{ GetInputCode(KEY_Apostrophe),      "Key Apostrophe" },
		{ GetInputCode(KEY_Grave),           "Key Grave" },
		{ GetInputCode(KEY_Comma),           "Key Comma" },
		{ GetInputCode(KEY_Period),          "Key Period" },
		{ GetInputCode(KEY_Slash),           "Key Slash" },
		{ GetInputCode(KEY_Control_Left),    "Key Control Left" },
		{ GetInputCode(KEY_Shift_Left),      "Key Shift Left" },
		{ GetInputCode(KEY_Alt_Left),        "Key Alt Left" },
		{ GetInputCode(KEY_GUI_Left),        "Key GUI Left" },
		{ GetInputCode(KEY_Control_Right),   "Key Control Right" },
		{ GetInputCode(KEY_Shift_Right),     "Key Shift Right" },
		{ GetInputCode(KEY_Alt_Right),       "Key Alt Right" },
		{ GetInputCode(KEY_GUI_Right),       "Key GUI Right" },
		{ GetInputCode(KEY_Right),           "Key Right" },
		{ GetInputCode(KEY_Left),            "Key Left" },
		{ GetInputCode(KEY_Down),            "Key Down" },
		{ GetInputCode(KEY_Up),              "Key Up" },
		{ GetInputCode(KEY_Lock_Caps),       "Key Lock Caps" },
		{ GetInputCode(KEY_Lock_Scroll),     "Key Lock Scroll" },
		{ GetInputCode(KEY_Lock_Number),     "Key Lock Number" },
		{ GetInputCode(KEY_PrintScreen),     "Key Print Screen" },
		{ GetInputCode(KEY_Pause),           "Key Pause" },
		{ GetInputCode(KEY_Insert),          "Key Insert" },
		{ GetInputCode(KEY_Home),            "Key Home" },
		{ GetInputCode(KEY_Page_Up),         "Key Page_Up" },
		{ GetInputCode(KEY_Page_Down),       "Key Page_Down" },
		{ GetInputCode(KEY_Delete),          "Key Delete" },
		{ GetInputCode(KEY_End),             "Key End" },
		{ GetInputCode(KEY_Keypad_Divide),   "Key Keypad Divide" },
		{ GetInputCode(KEY_Keypad_Multiply), "Key Keypad Multiply" },
		{ GetInputCode(KEY_Keypad_Minus),    "Key Keypad Minus" },
		{ GetInputCode(KEY_Keypad_Plus),     "Key Keypad Plus" },
		{ GetInputCode(KEY_Keypad_Enter),    "Key Keypad Enter" },
		{ GetInputCode(KEY_Keypad_1),        "Key Keypad 1" },
		{ GetInputCode(KEY_Keypad_2),        "Key Keypad 2" },
		{ GetInputCode(KEY_Keypad_3),        "Key Keypad 3" },
		{ GetInputCode(KEY_Keypad_4),        "Key Keypad 4" },
		{ GetInputCode(KEY_Keypad_5),        "Key Keypad 5" },
		{ GetInputCode(KEY_Keypad_6),        "Key Keypad 6" },
		{ GetInputCode(KEY_Keypad_7),        "Key Keypad 7" },
		{ GetInputCode(KEY_Keypad_8),        "Key Keypad 8" },
		{ GetInputCode(KEY_Keypad_9),        "Key Keypad 9" },
		{ GetInputCode(KEY_Keypad_0),        "Key Keypad 0" },
		{ GetInputCode(KEY_Keypad_Period),   "Key Keypad Period" },
		{ GetInputCode(KEY_Keypad_Equals),   "Key Keypad Equals" },
		{ GetInputCode(KEY_Function_1),      "Key Function 1" },
		{ GetInputCode(KEY_Function_2),      "Key Function 2" },
		{ GetInputCode(KEY_Function_3),      "Key Function 3" },
		{ GetInputCode(KEY_Function_4),      "Key Function 4" },
		{ GetInputCode(KEY_Function_5),      "Key Function 5" },
		{ GetInputCode(KEY_Function_6),      "Key Function 6" },
		{ GetInputCode(KEY_Function_7),      "Key Function 7" },
		{ GetInputCode(KEY_Function_8),      "Key Function 8" },
		{ GetInputCode(KEY_Function_9),      "Key Function 9" },
		{ GetInputCode(KEY_Function_10),     "Key Function 10" },
		{ GetInputCode(KEY_Function_11),     "Key Function 11" },
		{ GetInputCode(KEY_Function_12),     "Key Function 12" },
		{ GetInputCode(KEY_Function_13),     "Key Function 13" },
		{ GetInputCode(KEY_Function_14),     "Key Function 14" },
		{ GetInputCode(KEY_Function_15),     "Key Function 15" },
		{ GetInputCode(KEY_Function_16),     "Key Function 16" },
		{ GetInputCode(KEY_Function_17),     "Key Function 17" },
		{ GetInputCode(KEY_Function_18),     "Key Function 18" },
		{ GetInputCode(KEY_Function_19),     "Key Function 19" },
		{ GetInputCode(KEY_Function_20),     "Key Function 20" },
		{ GetInputCode(KEY_Function_21),     "Key Function 21" },
		{ GetInputCode(KEY_Function_22),     "Key Function 22" },
		{ GetInputCode(KEY_Function_23),     "Key Function 23" },
		{ GetInputCode(KEY_Function_24),     "Key Function 24" },

		{ GetInputCode(MOUSE_LEFT),        "Mouse Left" },
		{ GetInputCode(MOUSE_MIDDLE),      "Mouse Middle" },
		{ GetInputCode(MOUSE_RIGHT),       "Mouse Right" },
		{ GetInputCode(MOUSE_X1),          "Mouse X1" },
		{ GetInputCode(MOUSE_X2),          "Mouse X2" },
		{ GetInputCode(MOUSE_POS_X),       "Mouse Position X" },
		{ GetInputCode(MOUSE_POS_Y),       "Mouse Position Y" },
		{ GetInputCode(MOUSE_VEL_X),       "Mouse Velocity X" },
		{ GetInputCode(MOUSE_VEL_Y),       "Mouse Velocity Y" },
		{ GetInputCode(MOUSE_VEL_WHEEL_X), "Mouse Wheel Velocity X" },
		{ GetInputCode(MOUSE_VEL_WHEEL_Y), "Mouse Wheel Velocity Y" },

		{ GetInputCode(cBUTTON_A),             "Button A" },
		{ GetInputCode(cBUTTON_B),             "Button B" },
		{ GetInputCode(cBUTTON_X),             "Button X" },
		{ GetInputCode(cBUTTON_Y),             "Button Y" },
		{ GetInputCode(cBUTTON_BACK),          "Button BACK" },
		{ GetInputCode(cBUTTON_GUIDE),         "Button GUIDE" },
		{ GetInputCode(cBUTTON_START),         "Button START" },
		{ GetInputCode(cBUTTON_LEFTSTICK),     "Button Left Stick" },
		{ GetInputCode(cBUTTON_RIGHTSTICK),    "Button Right Stick" },
		{ GetInputCode(cBUTTON_LEFTSHOULDER),  "Button Left Shoulder" },
		{ GetInputCode(cBUTTON_RIGHTSHOULDER), "Button Right Shoulder" },
		{ GetInputCode(cBUTTON_DPAD_UP),       "Button DPAD Up" },
		{ GetInputCode(cBUTTON_DPAD_DOWN),     "Button DPAD Down" },
		{ GetInputCode(cBUTTON_DPAD_LEFT),     "Button DPAD Left" },
		{ GetInputCode(cBUTTON_DPAD_RIGHT),    "Button DPAD Right" },
		{ GetInputCode(cBUTTON_MISC1),         "Button Misc 1" },
		{ GetInputCode(cBUTTON_PADDLE1),       "Button Paddle 1" },
		{ GetInputCode(cBUTTON_PADDLE2),       "Button Paddle 2" },
		{ GetInputCode(cBUTTON_PADDLE3),       "Button Paddle 3" },
		{ GetInputCode(cBUTTON_PADDLE4),       "Button Paddle 4" },
		{ GetInputCode(cBUTTON_TOUCHPAD),      "Button TOUCHPAD" },
		{ GetInputCode(cAXIS_LEFTX),           "Axis Left X" },
		{ GetInputCode(cAXIS_LEFTY),           "Axis Left Y" },
		{ GetInputCode(cAXIS_RIGHTX),          "Axis Right X" },
		{ GetInputCode(cAXIS_RIGHTY),          "Axis Right Y" },
		{ GetInputCode(cAXIS_TRIGGERLEFT),     "Axis Trigger Left" },
		{ GetInputCode(cAXIS_TRIGGERRIGHT),    "Axis Trigger Right" }
	};

	auto itr = names.find(code);

	if (itr == names.end())
	{
        return ".";
	}

    return itr->second;
}

std::string GetPremake5Command()
{
	std::filesystem::path tools(wTOOLS);
	std::filesystem::path premake;

#ifdef _WIN32
	premake = "premake5.exe";
#endif

#ifdef __APPLE__
	premake = "mac_premake5";
#endif

#ifdef __linux__
	premake = "premake5";
#endif

	return (tools / premake).make_preferred().string() + " " + wPREMAKE_BUILD;
}

std::string GetBuildCommand(const std::string& solutionFilename)
{
	std::string builCommand;

#ifdef _WIN32
	builCommand = "msbuild " + solutionFilename;
#endif

#ifdef __APPLE__
	builCommand = "needs impl" + solutionFilename;
#endif

#ifdef __linux__
	builCommand = "make";
#endif

	return builCommand;
}
