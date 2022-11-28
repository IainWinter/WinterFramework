#pragma once

#include "app/System.h"

#ifdef _WIN32
#   ifdef wEXPORT
#       define wAPI extern "C" __declspec(dllexport)
#   else
#       define wAPI __declspec(dllimport)
#   endif
#else
#   define wAPI extern "C"
#endif

#define wSystemExport(system_name)                            \
                                                              \
    wAPI void w_add_to_world_##system_name(r<World> world)      \
    {                                                         \
        world->CreateSystem<system_name>()                    \
             ->SetName(#system_name);                         \
    }                                                         \
                                                              \
    wAPI void w_remove_from_world_##system_name(r<World> world) \
    {                                                         \
        world->DestroySystem<system_name>();                  \
    }

struct ScriptingContext
{
    ImGuiContext* ctx_imgui;
    SDL_GLContext ctx_opengl;
    SDL_Window*   ctx_window;
    
    Time::TimeContext* ctx_time;
    Render::RenderContext* ctx_render;
    meta::serial_context* ctx_meta;
};