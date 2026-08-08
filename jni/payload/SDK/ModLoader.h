#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "libs/Lua54/lua.hpp"

struct ModContext 
{
    std::string id; // id is just a folder name
    std::string name;
    std::string author;
    std::string version;

    std::filesystem::path folderPath;
};

class ModLoader
{
private:
    lua_State* L = nullptr;
    std::string modsPath;

    void initLua();
    void closeLua();

    bool loadMain(const std::filesystem::path& scriptPath);

public:
    ModLoader(std::string path);
    ~ModLoader();

    void loadAll();
    void loadModConfig(ModContext& mod);
    
    void resetAll();
    
    std::vector<ModContext> loadedMods;

    lua_State* getLua() { return L; }
    std::vector<ModContext>& getAllMods() { return loadedMods; }
};