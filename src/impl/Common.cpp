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
	world = glm::rotate   (world, angle(rotation), axis(rotation));
	world = glm::scale    (world, scale);

	return world;
}

//vec3 Transform::Forward() const
//{
//}
//vec3 Transform::Right() const
//{
//}

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
