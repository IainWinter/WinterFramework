#pragma once

#include "Defines.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_gamecontroller.h"

#include <memory>
#include <utility>
#include <vector>
#include <functional>

// make glm the deafult math library
// no need to think about if something is glm or not because it should
// be ingrained into the framework at a core level
using namespace glm;

/*

	Common Components

*/

struct Entity;

struct Transform2D
{
	vec2 position;
	vec2 scale;
	float rotation;
	float z;

private:
	vec2 positionLast;
	vec2 scaleLast;
	float rotationLast;
	float zLast;

public:
	Transform2D();
	Transform2D(float x, float y, float z = 0.f, float sx = 1.f, float sy = 1.f, float r = 0.f);
	Transform2D(vec2 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f, float z = 0.f);
	Transform2D(vec3 position, vec2 scale = vec2(1.f, 1.f), float rotation = 0.f);

	Transform2D& SetPosition(vec2 position);
	Transform2D& SetScale   (vec2 scale);
	Transform2D& SetRotation(float rotation);
	Transform2D& SetZIndex  (float z);

	Transform2D  operator* (const Transform2D& other) const;
	Transform2D& operator*=(const Transform2D& other);

	mat4 World()   const;
	vec2 Forward() const;
	vec2 Right()   const;

	void UpdateLastFrameData();
	Transform2D LastTransform() const;
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

struct event_Mouse
{
	int pixel_x, pixel_y;
	float screen_x, screen_y;
	float vel_x, vel_y;

	bool button_left;
	bool button_middle;
	bool button_right;
	bool button_x1;
	bool button_x2;

	int button_repeat;
};

using KeyboardInput = SDL_Scancode; // use sdl because it has everything already

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

struct event_Controller
{
	int controllerId;
	ControllerInput input;
	float value;
};

