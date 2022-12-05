#pragma once

#include "engine/script.h"

wAPI void PassContexts(ScriptingContext ctx)
{
    // Load GL functions for this dll
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // set GL context
    SDL_GL_MakeCurrent(ctx.ctx_window, ctx.ctx_opengl);

    ImGui::SetCurrentContext(ctx.ctx_imgui);
    Time::SetCurrentContext(ctx.ctx_time);
    Render::SetCurrentContext(ctx.ctx_render);
    meta::set_current_context(ctx.ctx_meta);

    Asset::SetCurrentContext(ctx.ctx_asset);
}
