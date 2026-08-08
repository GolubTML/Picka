#include "ModLoader.h"
#include "LuaAPI/LuaHook.h"
#include "LuaAPI/LuaBridge.h"
#include "log.h"
#include "libs/nlohmann/json.hpp"
#include <istream>

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
    
    LuaBridge::RegisterAPI(L, this);

    LOGI("Initilized Lua!");
}

void ModLoader::closeLua()
{
    loadedMods.clear();

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
    LOGW("Trying find mods in folder %s ...", modsPath.c_str());
    M_LOGW("Trying find mods in folder %s ...", modsPath.c_str());

    if (!std::filesystem::exists(modsPath))
    {
        LOGI("Cannot find folder, creating new one!");
        M_LOGI("Cannot find folder, creating new one!");
        
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
                M_LOGI("Found mod: %s", entry.path().filename().c_str()); // for 2 loggers

                loadMain(mainScript);
            }
        }
    }
}

void ModLoader::loadModConfig(ModContext& mod)
{
    std::filesystem::path configPath = mod.folderPath / "config.json";

    mod.name = mod.id;
    mod.author = "Unknown";
    mod.version = "1.0";

    if (!std::filesystem::exists(configPath))
    {
        M_LOGW("No config.json for mod %s, using folder name as fallback", mod.id.c_str());
        return;
    }

    std::ifstream file(configPath);
    if (!file.is_open())
    {
        M_LOGE("Cannot open config.json file for mod %s", mod.id.c_str());
        return;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        mod.name = j.value("name", mod.id);
        mod.author = j.value("author", std::string("Unknown"));
        mod.version = j.value("version", std::string("1.0"));
    }
    catch (const nlohmann::json::exception e)
    {
        LOGE("Failed to parse config.json for mod %s: %s", mod.id.c_str(), e.what());
    }
}

void ModLoader::resetAll()
{
    LOGI("Hot reload for mods!");
    M_LOGI("Hot reload for mods!");

    loadedMods.clear();

    initLua();
    loadAll();
}

bool ModLoader::loadMain(const std::filesystem::path& scriptPath)
{
    ModContext mod;
    mod.id = scriptPath.parent_path().filename().string();
    mod.folderPath = scriptPath.parent_path().string();

    loadModConfig(mod);

    LOGI("Loaded mod: id=%s name=%s version=%s", mod.id.c_str(), mod.name.c_str(), mod.version.c_str());
    
    loadedMods.push_back(mod);
    
    std::string pathCommand = "package.path = package.path .. \";" + mod.folderPath.string() + "/?.lua;" + mod.folderPath.string() + "/?/init.lua\"";
    LOGI("Setting LUA path: %s", pathCommand.c_str());
    M_LOGI("Setting LUA path: %s", pathCommand.c_str());

    if (luaL_dostring(L, pathCommand.c_str()) != LUA_OK) 
    {
        LOGE("PATH ERROR: %s", lua_tostring(L, -1));
        M_LOGE("PATH ERROR: %s", lua_tostring(L, -1));
        
        lua_pop(L, 1);
    }

    if (luaL_dofile(L, scriptPath.c_str()) != LUA_OK) 
    {
        LOGE("LUA ERROR: %s", lua_tostring(L, -1));
        M_LOGE("LUA ERROR: %s", lua_tostring(L, -1));

        lua_pop(L, 1);
        return false;
    }

    return true;
}