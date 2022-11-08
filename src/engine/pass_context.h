#pragma once

#include "engine/script.h"

wAPI void PassContexts(ScriptingContext ctx)
{
    // Load GL functions for this library
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // set GL context
    SDL_GL_MakeCurrent(ctx.ctx_window, ctx.ctx_opengl);

    // set ImGui context
    ImGui::SetCurrentContext(ctx.ctx_imgui);
    
    // set time context
    Time::SetCurrentContext(ctx.ctx_time);
}
