#pragma once

#include <filesystem>
#include <string>
#include "libs/Lua54/lua.hpp"

class ModLoader
{
private:
    lua_State* L;
    std::string modsPath;

    bool loadMain(const std::filesystem::path& scriptPath);
    void setupEnv(const std::string& modName); // for future

public:
    ModLoader(lua_State* L, std::string path);
    ~ModLoader();

    void loadAll();
    void resetAll();
};