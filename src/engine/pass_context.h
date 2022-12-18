#pragma once

#include "engine/script.h"

void ModuleMain();

#define RegisterComponent(component) meta::describe<component>().name(#component).prop("is_component", true)

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
    meta::set_current_context(ctx.ctx_meta);

    ModuleMain();
}