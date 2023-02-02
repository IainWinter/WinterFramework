#pragma once

#include "Defines.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_gamecontroller.h"

#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <tuple>
#include <string>

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

/*

	Common Components

*/

struct Entity;

enum ParentFlags : int
{
	pNone = 0b0000,
	pPosition = 0b0001,
	pScale = 0b0010,
	pRotation = 0b0100,
	pZ = 0b1000,
	pAll = 0b1111
};

inline ParentFlags operator|(ParentFlags a, ParentFlags b)
{
	return static_cast<ParentFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct Transform2D
{
	vec2 position;
	vec2 scale;
	float rotation;
	float z;

	ParentFlags parent = pAll;

public:
	Transform2D();
	Transform2D(float x, float y, float z = 0.f, float sx = 1.f, float sy = 1.f, float r = 0.f);
	Transform2D(vec2 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f, float z = 0.f);
	Transform2D(vec3 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f);

	Transform2D& SetPosition(vec2 position);
	Transform2D& SetScale   (vec2 scale);
	Transform2D& SetRotation(float rotation);
	Transform2D& SetZIndex  (float z);
	Transform2D& SetParent  (ParentFlags parent);

	Transform2D  operator* (const Transform2D& other) const;
	Transform2D& operator*=(const Transform2D& other);

	mat4 World()   const;
	vec2 Forward() const;
	vec2 Right()   const;
};

template<typename _t>
struct Last
{
	const _t& Get() const { return last; }
	void UpdateLast(const _t& value) { last = value; }
private:
	_t last;
};

struct Transform
{
	vec3 position;
	vec3 scale;
	quat rotation;

	Transform();
	Transform(vec3 position, vec3 scale = vec3(1.f, 1.f, 1.f), quat rotation = quat(1.f, 0.f, 0.f, 0.f));
	Transform(const Transform2D& transform2D);

	Transform& SetPosition(vec3 position);
	Transform& SetScale   (vec3 scale);
	Transform& SetRotation(quat rotation);
	Transform& SetRotation(vec3 rotation);

	Transform  operator* (const Transform& other) const;
	Transform& operator*=(const Transform& other);

	mat4 World()   const;
	//vec3 Forward() const; // todo
	//vec3 Right()   const;

	// doesnt have interpolation yet...

	// might want to make this just a mode of a transform, aka have a flag for 2d or 3d mode
	// that will make the code cleaner, but for now this works. Its only used in the line renderer for the camera gizmo
};

struct Color
{
	union { 
		u32 as_u32; 
		struct { u8 r, g, b, a; };
	};
	
	// Sets the color white (255, 255, 255, 255)
	Color();

	Color(u8 r, u8 g, u8 b, u8 a = 255);
	Color(u32 rgba);

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

struct aabb2D
{
	vec2 min;
	vec2 max;

	aabb2D();
	aabb2D(vec2 min, vec2 max);
	aabb2D(vec2 position, vec2 scale, float rotation);
	aabb2D(const std::vector<vec2>& points);

	vec2 center() const;
	bool fits(const aabb2D& other) const;

	float width() const;
	float height() const;
	vec2 Dimensions() const;

	static aabb2D from_points   (const std::vector<vec2>& points);
	static aabb2D from_transform(const Transform2D& points);
};

/*

	Math helpers

*/

float get_rand  (float x); // return a random number between (0, x)
float get_randc (float x); // return a random number between (-x/2, x/2)
int   get_rand  (int x);   // return a random integer between (0, x)
vec2  get_rand  (float x, float y);
vec2  get_randc (float x, float y);
vec2  get_randn (float scale);
vec2  get_randnc(float scale);

vec2 get_rand_jitter(vec2 x, float jitter);

template<typename _t>
_t get_rand(const std::vector<_t>& x)
{
	return x.size() == 0 ? _t() : x[get_rand((int)x.size())];
}

float lerp(float        a, float        b, float w);
vec2  lerp(const vec2&  a, const vec2&  b, float w);
vec3  lerp(const vec3&  a, const vec3&  b, float w);
vec4  lerp(const vec4&  a, const vec4&  b, float w);
Color lerp(const Color& a, const Color& b, float w);

// do a lerp per value
// casts to a float array
template<typename _t>
_t lerpf(const _t& a, const _t& b, float w)
{
	float* af = (float*)&a;
	float* bf = (float*)&b;

	size_t length = sizeof(_t) / sizeof(float);

	_t out;
	float* of = (float*)&out;

	for (size_t i = 0; i < length; i++)
	{
		of[i] = lerp(af[i], bf[i], w);
	}

	return out;
}

float clamp(float x, float min, float max);
vec2  clamp(vec2 x, const vec2& min, const vec2& max);
vec3  clamp(vec3 x, const vec3& min, const vec3& max);
vec4  clamp(vec4 x, const vec4& min, const vec4& max);

float max(const vec2& v);
float min(const vec2& v);

vec2 safe_normalize(const vec2& p);
vec2 rotate(const vec2& v, float a);
vec2 limit(const vec2& x, float max);

vec2 turn_towards(const vec2& current, const vec2& target, float strength);

vec2 on_unit(float a);
vec2 right(vec2 v);

float pow4(float x);
float angle(vec2 v);

template<typename _t>
std::pair<_t, _t> get_xy(const _t& index, const _t& width)
{
	return { index % width, index / width };
}

/*

	Pointer helpers

*/

template<typename _t>
_t value_or(void* ptr, const _t& defaultValue)
{
	return ptr ? *(_t*)ptr : defaultValue;
}

template<typename _t>
_t value_or(void* ptr, size_t index, const _t& defaultValue)
{
	return ptr ? *(_t*)ptr + index : defaultValue;
}

/*

	std helpers

*/

template<typename _t, typename _int_t>
void pop_erase(std::vector<_t>& list, _int_t* index)
{
	list.at((size_t)*index) = std::move(list.back());
	list.pop_back();
	*index -= 1;
}

template<typename _t, typename... _i>
std::vector<_t> list(const _t& first, const _i&... others)
{
	return std::vector<_t>{ first, others... };
}

template<typename _t, typename _f>
bool contains(const _t& list, const _f& value)
{
	return std::find(list.begin(), list.end(), value) != list.end();
}

template<typename _t> 
using r = std::shared_ptr<_t>;

template<typename _t>
using wr = std::weak_ptr<_t>;

template<typename _t, typename... _args> 
r<_t> mkr(_args&&... args)
{ 
	return std::make_shared<_t>(std::forward<_args>(args)...); 
}

template<typename _t>
float* f(_t& vec)
{
	return (float*)&vec;
}

template<auto val>
using constant = std::integral_constant<decltype(val), val>;

template<typename... _t>
using t = std::tuple<_t...>;

struct pair_hash
{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

template <typename _map>
bool map_compare(const _map& lhs, const _map& rhs)
{
	// https://stackoverflow.com/a/8473603/6772365

	// No predicate needed because there is operator== for pairs already.
	return lhs.size() == rhs.size()
		&& std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/*

	List Helpers

*/

// holds a reference to a contiguous array that can be iterated over
// in range based for-loops
template<typename _t>
struct ArrayView
{
private:
	_t* m_begin;
	_t* m_end;

public:
	ArrayView()
		: m_begin (nullptr)
		, m_end   (nullptr)
	{}

	ArrayView(_t* begin, _t* end)
		: m_begin (begin)
		, m_end   (end)
	{}

	template<typename _std_contiguous>
	ArrayView(const _std_contiguous& arr)
		: m_begin ((_t*)arr.data())
		, m_end   ((_t*)arr.data() + arr.size())
	{}

	_t& at(size_t i) { return m_begin[i]; }
	const _t& at(size_t i) const { return m_begin[i]; }

	size_t size() const { return std::distance(m_begin, m_end); }

	_t* begin() { return m_begin; }
	_t* end() { return m_end; }

	const _t* begin() const { return m_begin; }
	const _t* end() const { return m_end; }

	_t* data() { return m_begin; }
	const _t* data() const { return m_begin; }
};

/*

	Events

*/

struct event_Shutdown
{

};

struct event_WindowResize
{
	int width, height;
};

/*

	Input common

*/

//using KeyboardInput = SDL_Scancode; // use sdl because it has everything already

enum KeyboardInput
{
	KEY_INVALID          = 0,
	KEY_A                = SDL_SCANCODE_A,
	KEY_B                = SDL_SCANCODE_B,
	KEY_C                = SDL_SCANCODE_C,
	KEY_D                = SDL_SCANCODE_D,
	KEY_E                = SDL_SCANCODE_E,
	KEY_F                = SDL_SCANCODE_F,
	KEY_G                = SDL_SCANCODE_G,
	KEY_H                = SDL_SCANCODE_H,
	KEY_I                = SDL_SCANCODE_I,
	KEY_J                = SDL_SCANCODE_J,
	KEY_K                = SDL_SCANCODE_K,
	KEY_L                = SDL_SCANCODE_L,
	KEY_M                = SDL_SCANCODE_M,
	KEY_N                = SDL_SCANCODE_N,
	KEY_O                = SDL_SCANCODE_O,
	KEY_P                = SDL_SCANCODE_P,
	KEY_Q                = SDL_SCANCODE_Q,
	KEY_R                = SDL_SCANCODE_R,
	KEY_S                = SDL_SCANCODE_S,
	KEY_T                = SDL_SCANCODE_T,
	KEY_U                = SDL_SCANCODE_U,
	KEY_V                = SDL_SCANCODE_V,
	KEY_W                = SDL_SCANCODE_W,
	KEY_X                = SDL_SCANCODE_X,
	KEY_Y                = SDL_SCANCODE_Y,
	KEY_Z                = SDL_SCANCODE_Z,
	KEY_1                = SDL_SCANCODE_1,
	KEY_2                = SDL_SCANCODE_2,
	KEY_3                = SDL_SCANCODE_3,
	KEY_4                = SDL_SCANCODE_4,
	KEY_5                = SDL_SCANCODE_5,
	KEY_6                = SDL_SCANCODE_6,
	KEY_7                = SDL_SCANCODE_7,
	KEY_8                = SDL_SCANCODE_8,
	KEY_9                = SDL_SCANCODE_9,
	KEY_0                = SDL_SCANCODE_0,
	KEY_Return           = SDL_SCANCODE_RETURN,
	KEY_Escape           = SDL_SCANCODE_ESCAPE,
	KEY_Backspace        = SDL_SCANCODE_BACKSPACE,
	KEY_Tab              = SDL_SCANCODE_TAB,
	KEY_Space            = SDL_SCANCODE_SPACE,
	KEY_Minus            = SDL_SCANCODE_MINUS,
	KEY_Equals           = SDL_SCANCODE_EQUALS,
	KEY_Bracket_Left     = SDL_SCANCODE_LEFTBRACKET,
	KEY_Bracket_Right    = SDL_SCANCODE_RIGHTBRACKET,
	KEY_Backslash        = SDL_SCANCODE_BACKSLASH,
	KEY_SemiColon        = SDL_SCANCODE_SEMICOLON,
	KEY_Apostrophe       = SDL_SCANCODE_APOSTROPHE,
	KEY_Grave            = SDL_SCANCODE_GRAVE,
	KEY_Comma            = SDL_SCANCODE_COMMA,
	KEY_Period           = SDL_SCANCODE_PERIOD,
	KEY_Slash            = SDL_SCANCODE_SLASH,
	KEY_Control_Left     = SDL_SCANCODE_LCTRL,
	KEY_Shift_Left       = SDL_SCANCODE_LSHIFT,
	KEY_Alt_Left         = SDL_SCANCODE_LALT,
	KEY_GUI_Left         = SDL_SCANCODE_LGUI,
	KEY_Control_Right    = SDL_SCANCODE_RCTRL,
	KEY_Shift_Right      = SDL_SCANCODE_RSHIFT,
	KEY_Alt_Right        = SDL_SCANCODE_RALT,
	KEY_GUI_Right        = SDL_SCANCODE_RGUI,
	KEY_Right            = SDL_SCANCODE_RIGHT,
	KEY_Left             = SDL_SCANCODE_LEFT,
	KEY_Down             = SDL_SCANCODE_DOWN,
	KEY_Up               = SDL_SCANCODE_UP,
	KEY_Lock_Caps        = SDL_SCANCODE_CAPSLOCK,
	KEY_Lock_Scroll      = SDL_SCANCODE_SCROLLLOCK,
	KEY_Lock_Number      = SDL_SCANCODE_NUMLOCKCLEAR,
	KEY_PrintScreen      = SDL_SCANCODE_PRINTSCREEN,
	KEY_Pause            = SDL_SCANCODE_PAUSE,
	KEY_Insert           = SDL_SCANCODE_INSERT,
	KEY_Home             = SDL_SCANCODE_HOME,
	KEY_Page_Up          = SDL_SCANCODE_PAGEUP,
	KEY_Page_Down        = SDL_SCANCODE_PAGEDOWN,
	KEY_Delete           = SDL_SCANCODE_DELETE,
	KEY_End              = SDL_SCANCODE_END,
	KEY_Keypad_Divide    = SDL_SCANCODE_KP_DIVIDE,
	KEY_Keypad_Multiply  = SDL_SCANCODE_KP_MULTIPLY,
	KEY_Keypad_Minus     = SDL_SCANCODE_KP_MINUS,
	KEY_Keypad_Plus      = SDL_SCANCODE_KP_PLUS,
	KEY_Keypad_Enter     = SDL_SCANCODE_KP_ENTER,
	KEY_Keypad_1         = SDL_SCANCODE_KP_1,
	KEY_Keypad_2         = SDL_SCANCODE_KP_2,
	KEY_Keypad_3         = SDL_SCANCODE_KP_3,
	KEY_Keypad_4         = SDL_SCANCODE_KP_4,
	KEY_Keypad_5         = SDL_SCANCODE_KP_5,
	KEY_Keypad_6         = SDL_SCANCODE_KP_6,
	KEY_Keypad_7         = SDL_SCANCODE_KP_7,
	KEY_Keypad_8         = SDL_SCANCODE_KP_8,
	KEY_Keypad_9         = SDL_SCANCODE_KP_9,
	KEY_Keypad_0         = SDL_SCANCODE_KP_0,
	KEY_Keypad_Period    = SDL_SCANCODE_KP_PERIOD,
	KEY_Keypad_Equals    = SDL_SCANCODE_KP_EQUALS,
	KEY_Function_1       = SDL_SCANCODE_F1,
	KEY_Function_2       = SDL_SCANCODE_F2,
	KEY_Function_3       = SDL_SCANCODE_F3,
	KEY_Function_4       = SDL_SCANCODE_F4,
	KEY_Function_5       = SDL_SCANCODE_F5,
	KEY_Function_6       = SDL_SCANCODE_F6,
	KEY_Function_7       = SDL_SCANCODE_F7,
	KEY_Function_8       = SDL_SCANCODE_F8,
	KEY_Function_9       = SDL_SCANCODE_F9,
	KEY_Function_10      = SDL_SCANCODE_F10,
	KEY_Function_11      = SDL_SCANCODE_F11,
	KEY_Function_12      = SDL_SCANCODE_F12,
	KEY_Function_13      = SDL_SCANCODE_F13,
	KEY_Function_14      = SDL_SCANCODE_F14,
	KEY_Function_15      = SDL_SCANCODE_F15,
	KEY_Function_16      = SDL_SCANCODE_F16,
	KEY_Function_17      = SDL_SCANCODE_F17,
	KEY_Function_18      = SDL_SCANCODE_F18,
	KEY_Function_19      = SDL_SCANCODE_F19,
	KEY_Function_20      = SDL_SCANCODE_F20,
	KEY_Function_21      = SDL_SCANCODE_F21,
	KEY_Function_22      = SDL_SCANCODE_F22,
	KEY_Function_23      = SDL_SCANCODE_F23,
	KEY_Function_24      = SDL_SCANCODE_F24
};

enum MouseInput
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	MOUSE_X1,
	MOUSE_X2,

	// these are in viewport space

	MOUSE_POS_X,
	MOUSE_POS_Y,

	MOUSE_VEL_X,
	MOUSE_VEL_Y,

	MOUSE_VEL_WHEEL_X,
	MOUSE_VEL_WHEEL_Y,

	// these are sent by the Window, EngineLoop handles breaking it into x,y/vx,vy
	MOUSE_INPUT_COUNT,

	MOUSE_VEL_POS,
	MOUSE_VEL_WHEEL,
};

// easier to combine the controller axis/buttons into a single enum
enum ControllerInput
{
	cAXIS_INVALID   = -10,
	cBUTTON_INVALID = -1,

    cBUTTON_A,
    cBUTTON_B,
    cBUTTON_X,
    cBUTTON_Y,
    cBUTTON_BACK,
    cBUTTON_GUIDE,
    cBUTTON_START,
    cBUTTON_LEFTSTICK,
    cBUTTON_RIGHTSTICK,
    cBUTTON_LEFTSHOULDER,
    cBUTTON_RIGHTSHOULDER,
    cBUTTON_DPAD_UP,
    cBUTTON_DPAD_DOWN,
    cBUTTON_DPAD_LEFT,
    cBUTTON_DPAD_RIGHT,
    cBUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    cBUTTON_PADDLE1,  /* Xbox Elite paddle P1 */
    cBUTTON_PADDLE2,  /* Xbox Elite paddle P3 */
    cBUTTON_PADDLE3,  /* Xbox Elite paddle P2 */
    cBUTTON_PADDLE4,  /* Xbox Elite paddle P4 */
    cBUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    
	cBUTTON_MAX,

	cAXIS_LEFTX,
	cAXIS_LEFTY,
	cAXIS_RIGHTX,
	cAXIS_RIGHTY,
	cAXIS_TRIGGERLEFT,
	cAXIS_TRIGGERRIGHT,

	cAXIS_MAX
};

ControllerInput MapSDLGameControllerButton(SDL_GameControllerButton button);
ControllerInput MapSDLGameControllerAxis(SDL_GameControllerAxis axis);

// Convert from an enum to a code that represents an index into a combined range containing all Keyboard/Mouse/Controller inputs
int GetInputCode(KeyboardInput scancode);
int GetInputCode(MouseInput input);
int GetInputCode(ControllerInput input);

// Return a string with the name of the input code, if name starts with '.' it should not be used. On input code, returns "."
const char* GetInputCodeName(int code);

struct event_Key
{
	KeyboardInput keycode;
	char key;

	bool state;
	int repeat;

	bool key_shift;
	bool key_ctrl;
	bool key_alt;
};

struct event_Mouse
{
	int pixel_x, pixel_y;
	float screen_x, screen_y;
	float vel_x, vel_y;

	MouseInput mousecode;

	bool button_left;
	bool button_middle;
	bool button_right;
	bool button_x1;
	bool button_x2;

	int button_repeat;

	// If this is true, this mouse event is from the mouse wheel being scrolled
	// vel_x and vel_y hold the direction of scrolling
	bool is_wheel;
};

struct event_Controller
{
	int controllerId;
	ControllerInput input;
	float value;
};

/*

	Tools

*/

std::string GetPremake5Command();
std::string GetBuildCommand(const std::string& solutionFilename);
