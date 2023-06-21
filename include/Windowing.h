#pragma once

#include "Log.h"
#include "Common.h"
#include "Event.h"

#include "util/error_check.h"

// imgui is going to be the UI library for everything in the game
// so hard commit to tying it up with the window

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/implot.h"

#include "SDL.h"
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
	int FullScreen = 0;
	bool VSync = false;
};

struct Window
{
private:
	SDL_Window*   m_window;
	SDL_GLContext m_opengl;
	ImGuiContext* m_imgui;

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

	WindowConfig GetConfig() const;
	SDL_Window* GetSDLWindow() const;
	SDL_GLContext GetGLContext() const;
	ImGuiContext* GetImGuiContext() const;

	void Init();
	void InitUI();
	void Dnit();
	
	void PumpEvents();
	void EndFrame();
	
	// imgui renderer

	void BeginImgui();
	void EndImgui();
	
	// for now this works, but shouldn't be here
	// don't call this function to make removing it ezpz
	Window& SetViewport(int width, int height);

	Window& SetSize(int width, int height, bool center = true);
	Window& SetTitle(const std::string& title);
	Window& SetFullscreen(int mode); // 0 windowed, 1 borderless, 2 fullscreen
	Window& SetVSync(bool vsync);

	Window& SetEventQueue(EventQueue* queue);

	// yes moves
	Window(Window&& move) noexcept;
	Window& operator=(Window&& move) noexcept;
	
	//  no copy
	Window(const Window& copy) = delete;
	Window& operator=(const Window& copy) = delete;

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

	WindowConfig GetConfig() const;

	WindowRef& SetTitle(const std::string& title);

	WindowRef& ResizeViewport(int width, int height);
	WindowRef& Resize(int width, int height, bool center = true);

	WindowRef& SetFullscreen(int mode); // 0 windowed, 1 borderless, 2 fullscreen
	WindowRef& SetVSync(bool vsync);
};