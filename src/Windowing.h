#pragma once

#include "Log.h"
#include "Common.h"
#include "Event.h"

#include "util/error_check.h"

// imgui is going to be the UI library for everything in the game
// so hard commit to tieing it up with the window

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/implot.h"

#include "SDL2/SDL.h"
#include "glad/glad.h"

#include <string>

/*

	Window and UI

*/

struct WindowConfig
{
	std::string Title = "Welcome to Winter Framework";
	int Width = 640;
	int Height = 480;
};

struct Window
{
private:
	SDL_Window*   m_window;
	SDL_GLContext m_opengl;
	EventQueue*   m_events;

	WindowConfig m_config;

	// only supports first controller plugged in currently
	// would be super simple to make this a map on controller id
	// and send that id with the input events, but for now this is simpler
	SDL_GameController* m_controller;

	inline static bool s_first = true;
	const char* m_first_glsl_version = nullptr;

public:
	Window();
	Window(const WindowConfig& config, EventQueue* events = nullptr);
	~Window();

	int                Width()      const;
	int                Height()     const;
	vec2               Dimensions() const;
	const std::string& Title()      const;

	void Init();
	void InitUI();
	void Dnit();
	
	void PumpEvents();
	void EndFrame();
	
	// imgui renderer

	void BeginImgui();
	void EndImgui();
	
	// for now this works, but shouldnt be here
	// dont call this func to make removing it ezpz
	void ResizeViewport(int width, int height);

	void Resize(int width, int height, bool center = true);
	void SetTitle(const std::string& title);

	void SetFullscreen(int mode); // 0 windowed, 1 borderless, 2 fullscreen
	void SetVSync(bool vsync);

	// yes moves
	Window(Window&& move) noexcept;
	Window& operator=(Window&& move) noexcept;
	
	//  no copys
	Window(const Window& copy) = delete;
	Window& operator=(const Window& copy) = delete;

// init funcs

private:

	const char* Init_Video();
	void Init_Imgui(const char* glsl_version);
	void Dnit_Imgui();
};

// encapsulates the window so systems can read the props without access to engine functions
struct WindowRef
{
private:
	Window* m_window;

public:
	WindowRef(Window* window);

	int                Width()      const;
	int                Height()     const;
	vec2               Dimensions() const;
	const std::string& Title()      const;
	
	void SetTitle(const std::string& title);

	void ResizeViewport(int width, int height);
	void Resize(int width, int height, bool center = true);

	void SetFullscreen(int mode); // 0 windowed, 1 borderless, 2 fullscreen
	void SetVSync(bool vsync);
};