#pragma once

#include <string>
#include <vector>

#include "app/System.h"

struct WorldFileData
{
    std::string dll;
    std::vector<std::string> systems;

    // loading options -- not sure if these would be in file or not

    std::string name;
    bool reloadAllState = true; // destroies all world data by creating a new world
    
    bool reloadSystems = true; //
};

void CleanHotDlls();

// Load a world from file data
// this creates the world in the application
World* LoadWorldFromData(Application& app, const WorldFileData& data);
