#include "engine/io.h"
#include "engine/script.h"

#include <iostream>

// this gets used for its ability to relink pdb files in windows dlls
#define CR_HOST
#include "cr/cr.h"

std::unordered_map<std::string, void*> dlls;
int pdbId = 0;

std::filesystem::path GetHotPath(const std::string& filepath)
{
    return "./hot" / std::filesystem::path(filepath).filename();
}

std::string GetExt(const std::string& path)
{
    std::filesystem::path p(path);
    std::string name = p.filename().string();
    
    std::stringstream ss;
    ss << p.parent_path().string() << "/";
    
#ifdef _WIN32
    ss << name << ".dll";
#endif
    
#ifdef __APPLE__
    ss << "lib" << name << ".dylib";
#endif
    
#ifdef __linux__
    assert(false && "make this");
#endif
    
    return ss.str();
}

std::string CopyToHotDll(const std::string& filepath)
{
    using namespace std::filesystem;

    // get the name of the file and copy it to ./hot
    // on windows, copy the

    create_directories("./hot");

    // if we are on windows, we need to do a text replace in the dll to change the link from the old pdb to the new one
    
    path src = path(filepath);
    path dst = GetHotPath(filepath);

    // this is what we would do on linux and mac
    copy_file(src, dst, copy_options::overwrite_existing);

    // then passed here is all windows
#ifdef WIN32

    // append a unique id to the pdb because devenv never releases the file handle
    
    path src_pdb = src.parent_path() / src.filename().replace_extension(".pdb");

    if (exists(src_pdb))
    {
        std::stringstream ss; ss << "." << pdbId << ".pdb";
        path dst_pdb = "./hot/" / src_pdb.filename().replace_extension(ss.str());
    
        // simple copy for pdb
        copy_file(src_pdb, dst_pdb, copy_options::overwrite_existing);

        // replace pdb link in dll
        std::string _;
        cr_pdb_replace(dst.string(), dst_pdb.string(), _);

        pdbId += 1;
    }
#endif

    return dst.string();
}

void CleanHotDlls()
{
    for (auto [str, dll] : dlls)
    {
        SDL_UnloadObject(dll);
    }

    std::filesystem::remove_all("./hot");
}

void* wLoadLibrary(const std::string& path_)
{
    std::string path = GetExt(path_);
    
    auto itr = dlls.find(path);
    if (itr != dlls.end())
    {
        SDL_UnloadObject(itr->second);

        itr = dlls.end(); // setup for reload
        dlls.erase(path);
    }

    std::string hot = CopyToHotDll(path);

    if (itr == dlls.end())
    {
        void* lib = SDL_LoadObject(hot.c_str());

        if (!lib) // exit on failed load
        {
            log_io("e~Failed to load library. %s", path.c_str());
            return nullptr;
        }

        itr = dlls.emplace(path, lib).first;
    }

    return itr->second;
}

template<typename _r, typename... _a>
using funcptr = _r(*)(_a...);

template<typename _r, typename... _a>
funcptr<_r, _a...> wLoadFunction(void* lib, const std::string& path)
{
    using fptr = funcptr<_r, _a...>;
    fptr load = (fptr) SDL_LoadFunction(lib, path.c_str());

    if (!load)
    {
        log_io("e~Failed to load function from library. %s", path.c_str());
        return nullptr;
    }

    return load;
}

bool PassContextsToDll(void* lib, Application& app)
{
    ScriptingContext ctx;
    ctx.ctx_imgui  = app.GetWindow().GetImGuiContext();
    ctx.ctx_opengl = app.GetWindow().GetGLContext();
    ctx.ctx_window = app.GetWindow().GetSDLWindow();
    ctx.ctx_time   = Time::GetContext();
    ctx.ctx_render = Render::GetContext();

    auto fn = wLoadFunction<void, ScriptingContext>(lib, "PassContexts");
    
    if (fn) fn(ctx);
    else log_io("e~Failed to pass context to library");

    return !!fn;
}

void CallOnAllSystems(void* lib, World* world, const WorldFileData& data, const std::string& what)
{
    for (const std::string& system : data.systems)
    {
        std::stringstream ss;
        ss << what << system;

        // soft fail

        auto load = wLoadFunction<void, World*>(lib, ss.str());
        
        if (load) load(world);
        else log_io("e~Failed to call function '%s' in library", ss.str().c_str());
    }
}

// this will attach new systems and refresh ones that exists already
// see wSystemExport
void LoadSystemsOnWorld(void* lib, World* world, const WorldFileData& data)
{
    CallOnAllSystems(lib, world, data, "w_add_to_world_");
}

// this will attach new systems and refresh ones that exists already
// see wSystemExport
void FreeSystemsOnWorld(void* lib, World* world, const WorldFileData& data)
{
    CallOnAllSystems(lib, world, data, "w_remove_from_world_");
}

World* LoadWorldFromData(Application& app, const WorldFileData& data)
{
    // If an old world exists, then we will modify its state

    World* old = app.GetWorld(data.name.c_str());

    // Delete the systems through the old dll, then recreate in the new.
    // Systems shold hold no state, so this should be as valid as not changing any state

    if (old) // assumes dll
    {
        void* oldlib = dlls[GetExt(data.dll)];
        FreeSystemsOnWorld(oldlib, old, data);

        // serialize entity world
        // there is def a way to write then read to ram

        //std::string tempfile = "./hot/temp_entities.json";

        std::stringstream ss;

        json_writer writer(ss);
        WriteWorld(old, writer);

        // debug print json to console
        //ss << "\n";
        //log_io("World json: %s", ss.str().c_str());

        ss.seekp(0);
        
        json_reader reader(ss);
        ReadWorld(old, reader);

        // recreate entity world
    }

    // Load the dll containing the systems attached to this world

    void* lib = wLoadLibrary(data.dll);

    if (!lib) // exit on no lib
    {
        return nullptr;
    }

    bool success = PassContextsToDll(lib, app);

    if (!success) // exit on no context pass
    {
        return nullptr;
    }

    // Create the world, we need this for the factory functions
    //                   because the world needs their type info
    
    World* world = old ? old : app.CreateWorld();

    LoadSystemsOnWorld(lib, world, data);

    world->SetName(data.name.c_str());

    return world;
}
