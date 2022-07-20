#pragma once

#include "Windowing.h"

Window::Window()
	: m_window (nullptr)
	, m_opengl (nullptr)
	, m_events (nullptr)
{}

Window::Window(
	const WindowConfig& config,
	event_queue* events
)
	: m_events (events)
	, m_config (config)
	, m_window (nullptr)
	, m_opengl (nullptr)
{
	if (s_first) s_first = false;
	else assert(false && "Only a single window has been tested");
}

Window::~Window()
{
	if (!m_window) return;
	Dnit();
}

int                Window::Width()      const { return m_config.Width; }
int                Window::Height()     const { return m_config.Height; }
vec2               Window::Dimensions() const { return vec2(Width(), Height()); }
const std::string& Window::Title()      const { return m_config.Title; }

void Window::Init()
{
	m_first_glsl_version = Window::Init_Video();

	m_window = SDL_CreateWindow(m_config.Title.c_str(), 0, 0, m_config.Width, m_config.Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	m_opengl = SDL_GL_CreateContext(m_window);

	SDL_GL_MakeCurrent(m_window, m_opengl);
	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	Resize(m_config.Width, m_config.Height);

	glClearColor(1.f, 1.f, 1.f, 1.f);

	// sand breaks because this isnt setup
	glEnable(GL_DEPTH_TEST);

	// enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// vsync

	SDL_GL_SetSwapInterval(1);
}

void Window::InitUI()
{
	Init_Imgui(m_first_glsl_version);
}

void Window::Dnit()
{
	Dnit_Imgui();
	SDL_GL_DeleteContext(m_opengl);
	SDL_DestroyWindow(m_window);
	m_window = nullptr;
}

void Window::PumpEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		printf("event: %d\n", event.type);

		ImGui_ImplSDL2_ProcessEvent(&event); // does this need to be between frames?

		if (!m_events)
		{
			if (event.type == SDL_QUIT)
			{
				SDL_Quit(); // allow for quit without events
			}

			continue; // pump but do nothing, maybe log
		} 

		switch (event.type)
		{
			case SDL_QUIT: 
			{
				m_events->send(event_Shutdown {});
				break;
			}
			case SDL_MOUSEBUTTONUP: // can these be combined?
			case SDL_MOUSEBUTTONDOWN:
			{
				bool pressed = event.button.state == SDL_PRESSED;

				m_events->send(event_Mouse {
					event.button.x,                                                           // position as (0, width/height)
					event.button.y,
					(                    event.button.x  / (float)m_config.Width)  * 2 - 1,   // position as (-1, +1)
					((m_config.Height -  event.button.y) / (float)m_config.Height) * 2 - 1,
					0.f,                                                                      // velocity
					0.f,
					bool( pressed * (event.button.button == SDL_BUTTON_LEFT) ),
					bool( pressed * (event.button.button == SDL_BUTTON_MIDDLE) ),
					bool( pressed * (event.button.button == SDL_BUTTON_RIGHT) ),
					bool( pressed * (event.button.button == SDL_BUTTON_X1) ),
					bool( pressed * (event.button.button == SDL_BUTTON_X2) ),
					event.button.clicks
				});	
				break;
			}
			case SDL_MOUSEMOTION:
			{
				m_events->send(event_Mouse { 
					event.motion.x,                                                          // position as (0, width/height)
					event.motion.y,
					(                   event.motion.x  / (float)m_config.Width)  * 2 - 1,   // position as (-1, +1)
					((m_config.Height - event.button.y) / (float)m_config.Height) * 2 - 1,
					(event.motion.xrel / (float)m_config.Width)  * 2 - 1,                    // velocity
					(event.motion.yrel / (float)m_config.Height) * 2 - 1,
					bool(event.motion.state & SDL_BUTTON_LMASK),
					bool(event.motion.state & SDL_BUTTON_MMASK),
					bool(event.motion.state & SDL_BUTTON_RMASK),
					bool(event.motion.state & SDL_BUTTON_X1MASK),
					bool(event.motion.state & SDL_BUTTON_X2MASK),
					0
				});
				break;
			}
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						ResizeViewport(event.window.data1, event.window.data2);
						m_config.Width  = event.window.data1;
						m_config.Height = event.window.data2;

						m_events->send(event_WindowResize { m_config.Width, m_config.Height });
						break;
					}
				}
				break;
			}
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				m_events->send(event_Key {
					event.key.keysym.scancode,
					(char)event.key.keysym.sym,
					(bool)event.key.state,
					(int)event.key.repeat,
					bool(event.key.keysym.mod & KMOD_SHIFT), // doesn't handle right/left
					bool(event.key.keysym.mod & KMOD_CTRL),
					bool(event.key.keysym.mod & KMOD_ALT)
				});
				break;
			}

			//case SDL_CONTROLLERBUTTONDOWN:
			//case SDL_CONTROLLERBUTTONUP:
			//	printf("controller button %d %d\n", event.cbutton.button, event.cbutton.state);
			//	break;

			//case SDL_CONTROLLERAXISMOTION:
			//	printf("controller axis %d %f\n", event.caxis.axis, event.caxis.value);
			//	break;

			//case SDL_CONTROLLERDEVICEADDED:
			//	printf("controller plugged in\n");
			//	break;

			//case SDL_CONTROLLERDEVICEREMOVED:
			//	printf("controller unplugged\n");
			//	break;
		}
	}
}

void Window::ResizeViewport(int width, int height)
{
	m_config.Width = width;
	m_config.Height = height;

	if (m_opengl) // if opengl isnt init, then when window is first opened it will take the m_config size
	{
		gl(glViewport(0, 0, m_config.Width, m_config.Height));
	}
}

void Window::Resize(int width, int height, bool center)
{
	if (m_window) // only if open
	{
		SDL_SetWindowSize(m_window, width, height); // high DPI screens need some other functions?
		if (center) SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	ResizeViewport(width, height);
}

void Window::SetTitle(const std::string& title)
{
	SDL_SetWindowTitle(m_window, title.c_str());
}

void Window::EndFrame()
{
	SDL_GL_SwapWindow(m_window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//#ifdef IW_PLATFORM_WINDOWS
//		ImGui::UpdatePlatformWindows();
//#endif
}
	
// imgui renderer

void Window::BeginImgui()
{
	// might want to allow user to configure this
	// but 99% of the time you want to draw imgui 
	// to the screen not some backbuffer
		
	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_window);
    ImGui::NewFrame();
}

void Window::EndImgui()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef IW_PLATFORM_WINDOWS
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
#endif

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_GL_MakeCurrent(m_window, m_opengl);
	}
}

Window::Window(Window&& move) noexcept
	: m_window (move.m_window)
	, m_opengl (move.m_opengl)
	, m_events (move.m_events)
	, m_config (move.m_config)
{
	move.m_window = nullptr;
	move.m_opengl = nullptr;
	move.m_events = nullptr;
}
Window& Window::operator=(Window&& move) noexcept
{
	m_window = move.m_window;
	m_opengl = move.m_opengl;
	m_events = move.m_events;
	m_config = move.m_config;
	move.m_window = nullptr;
	move.m_opengl = nullptr;
	move.m_events = nullptr;
	return *this;
}

const char* Window::Init_Video()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	// set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	// is this needed?

#ifdef __APPLE__
	SDL_GL_SetAttribute( // required on Mac OS
		SDL_GL_CONTEXT_FLAGS,
		SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
	);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	return "#version 150";
#elif __linux__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	return "#version 150";
#elif _WIN32
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	return "#version 130";
#endif
}

void Window::Init_Imgui(const char* glsl_version)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(m_window, m_opengl);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void Window::Dnit_Imgui()
{
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
 	ImGui::DestroyContext();
	ImPlot::DestroyContext();
}