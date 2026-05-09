#pragma once

#include <filesystem>
#include <string>
#include "libs/Lua54/lua.hpp"

class ModLoader
{
private:
    lua_State* L = nullptr;
    std::string modsPath;

    void initLua();
    void closeLua();

    bool loadMain(const std::filesystem::path& scriptPath);
    void setupEnv(const std::string& modName); // for future

public:
    ModLoader(std::string path);
    ~ModLoader();

    void loadAll();
    void resetAll();

    lua_State* getLua() { return L; }
};