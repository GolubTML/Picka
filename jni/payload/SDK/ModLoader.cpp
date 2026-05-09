#include "ModLoader.h"
#include "LuaAPI/LuaHook.h"
#include "LuaAPI/LuaBridge.h"
#include "log.h"

ModLoader::ModLoader(std::string path) : modsPath(path) 
{
    initLua();
}

ModLoader::~ModLoader() { closeLua(); }

void ModLoader::initLua()
{
    if (L) closeLua();

    L = luaL_newstate();
    luaL_openlibs(L);
    
    LuaBridge::RegisterAPI(L);

    LOGI("Initilized Lua!");
}

void ModLoader::closeLua()
{
    if (L)
    {
        LuaBridge::ClearAllHooks();

        lua_close(L);
        L = nullptr;
        LOGI("Close lua state!");
    }
}

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

void ModLoader::resetAll()
{
    LOGI("Hot reload for mods!");
    initLua();
    loadAll();
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