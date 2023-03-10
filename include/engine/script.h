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

#define wSystemExport(system_name)                              \
                                                                \
    wAPI void w_add_to_world_##system_name(r<World> world)      \
    {                                                           \
        world->CreateSystem<system_name>()                      \
             ->SetName(#system_name);                           \
                                                                \
    }                                                           \
                                                                \
    wAPI void w_remove_from_world_##system_name(r<World> world) \
    {                                                           \
        world->DestroySystem<system_name>();                    \
    }

struct ScriptingContext
{
    ImGuiContext* ctx_imgui;
    SDL_GLContext ctx_opengl;
    SDL_Window* ctx_window;

    Time::TimeContext* ctx_time;
    Render::RenderContext* ctx_render;
    Asset::AssetContext* ctx_asset;
    File::FileContext* ctx_file;
    Input::InputContext* ctx_input;
    meta::serial_context* ctx_meta;
    wlog::log_context* ctx_log;
};

//
//  This is so module can pass string arrays using a C struct, ie not a vector
//

struct StringArray
{
    char const** strings;
    size_t length;
};

inline StringArray NewStringArray(size_t size)
{
    StringArray arr;
    arr.strings = new const char*[size];
    arr.length = size;

    return arr;
}

inline void DeleteStringArray(StringArray& arr)
{
    delete[] arr.strings;
}