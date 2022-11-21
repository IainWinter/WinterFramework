//#include "engine/io.h"
//#include "engine/script.h"
//#include "ext/serial/serial_common.h"
//
//#include <iostream>
//
//bool PassContextsToDll(void* lib, Application& app)
//{
//    ScriptingContext ctx;
//    ctx.ctx_imgui  = app.GetWindow().GetImGuiContext();
//    ctx.ctx_opengl = app.GetWindow().GetGLContext();
//    ctx.ctx_window = app.GetWindow().GetSDLWindow();
//    ctx.ctx_time   = Time::GetContext();
//    ctx.ctx_render = Render::GetContext();
//
//    auto fn = wLoadFunction<void, ScriptingContext>(lib, "PassContexts");
//    
//    if (fn) fn(ctx);
//    else log_io("e~Failed to pass context to library");
//
//    return !!fn;
//}
//
//void CallOnAllSystems(void* lib, World* world, const WorldFileData& data, const std::string& what)
//{
//    for (const std::string& system : data.systems)
//    {
//        std::stringstream ss;
//        ss << what << system;
//
//        // soft fail
//
//        auto load = wLoadFunction<void, World*>(lib, ss.str());
//        
//        if (load) load(world);
//        else log_io("e~Failed to call function '%s' in library", ss.str().c_str());
//    }
//}
//
//// this will attach new systems and refresh ones that exists already
//// see wSystemExport
//void LoadSystemsOnWorld(void* lib, World* world, const WorldFileData& data)
//{
//    CallOnAllSystems(lib, world, data, "w_add_to_world_");
//}
//
//// this will attach new systems and refresh ones that exists already
//// see wSystemExport
//void FreeSystemsOnWorld(void* lib, World* world, const WorldFileData& data)
//{
//    CallOnAllSystems(lib, world, data, "w_remove_from_world_");
//}
//
//World* LoadWorldFromData(Application& app, const WorldFileData& data)
//{
//    // If an old world exists, then we will modify its state
//
//    World* old = app.GetWorld(data.name.c_str());
//
//    // Delete the systems through the old dll, then recreate in the new.
//    // Systems shold hold no state, so this should be as valid as not changing any state
//
//    if (old) // assumes dll
//    {
//        void* oldlib = dlls[GetExt(data.dll)];
//        FreeSystemsOnWorld(oldlib, old, data);
//
//        // serialize entity world
//        // there is def a way to write then read to ram
//
//        //std::string tempfile = "./hot/temp_entities.json";
//
//        std::stringstream ss;
//
//        json_writer writer(ss);
//        WriteWorld(old, writer);
//
//        // debug print json to console
//        ss << "\n";
//        log_io("World json: %s", ss.str().c_str());
//
//        ss.seekp(0);
//        
//        json_reader reader(ss);
//        ReadWorld(old, reader);
//
//        // recreate entity world
//    }
//
//    // Load the dll containing the systems attached to this world
//
//    void* lib = wLoadLibrary(data.dll);
//
//    if (!lib) // exit on no lib
//    {
//        return nullptr;
//    }
//
//    bool success = PassContextsToDll(lib, app);
//
//    if (!success) // exit on no context pass
//    {
//        return nullptr;
//    }
//
//    // Create the world, we need this for the factory functions
//    //                   because the world needs their type info
//    
//    World* world = old ? old : app.CreateWorld();
//
//    LoadSystemsOnWorld(lib, world, data);
//
//    world->SetName(data.name.c_str());
//
//    return world;
//}
