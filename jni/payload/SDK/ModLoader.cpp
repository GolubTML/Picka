#include "ModLoader.h"
#include "log.h"

ModLoader::ModLoader(lua_State* L, std::string path) : L(L), modsPath(path) { }
ModLoader::~ModLoader() { delete L; }

void ModLoader::loadAll()
{
    LOGI("Trying find mods in folder %s ...", modsPath.c_str());

    if (!std::filesystem::exists(modsPath))
    {
        LOGI("Cannot find folder, creating new one!");
        std::filesystem::create_directories(modsPath);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(modsPath)) 
    {
        if (entry.is_directory()) 
        {
            std::filesystem::path mainScript = entry.path() / "main.lua";
            
            if (std::filesystem::exists(mainScript)) 
            {
                LOGI("Found mod: %s", entry.path().filename().c_str());
                loadMain(mainScript);
            }
        }
    }
}

bool ModLoader::loadMain(const std::filesystem::path& scriptPath)
{
    if (luaL_dofile(L, scriptPath.c_str()) != LUA_OK) 
    {
        LOGI("LUA ERROR: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}