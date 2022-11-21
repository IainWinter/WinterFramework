#pragma once

#include "app/System.h"
#include "ext/serial/serial_json.h"

#include "ext/serial/SerializeEntityWorld.h"

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

struct WorldFileData
{
    std::string dll;
    std::vector<std::string> systems;

    // loading options -- not sure if these would be in file or not

    std::string name;
    bool reloadAllState = true; // destroies all world data by creating a new world
    
    bool reloadSystems = true; //
};

inline void CleanHotDlls() {}

// Load a world from file data
// this creates the world in the application
inline World* LoadWorldFromData(Application& app, const WorldFileData& data) { return nullptr;  }
