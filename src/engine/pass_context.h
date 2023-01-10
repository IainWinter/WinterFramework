#pragma once

//
//
//  Only include this file in a single cpp file, 
//      almost always module.cpp
//
//

#include "engine/script.h"

//
// Module state
//

std::unordered_set<const char*> systems;

// fwd so PassContexts can call this
void ModuleMain();

wAPI void PassContexts(ScriptingContext ctx)
{
    // Load GL functions for this dll
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // set GL context
    SDL_GL_MakeCurrent(ctx.ctx_window, ctx.ctx_opengl);

    ImGui::SetCurrentContext(ctx.ctx_imgui);
    Time::SetCurrentContext(ctx.ctx_time);
    Render::SetCurrentContext(ctx.ctx_render);
    Asset::SetCurrentContext(ctx.ctx_asset);
    File::SetCurrentContext(ctx.ctx_file);
    Input::SetCurrentContext(ctx.ctx_input);
    meta::SetCurrentContext(ctx.ctx_meta);
    wlog::SetCurrentContext(ctx.ctx_log);

    ModuleMain();
}

// you own this memory
// delete with 'delete[]'

wAPI StringArray GetAvalibleSystemNames()
{
    StringArray arr = NewStringArray(systems.size());

    size_t i = 0;
    for (const char* n : systems)
    {
        arr.strings[i++] = n;
    }

    return arr;
}

// todo: remove this

#define RegisterComponent(component) meta::describe<component>().name(#component).prop("is_component", true)
