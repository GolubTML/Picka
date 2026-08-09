#include "lua_mod.h"

#include "../../ModLoader.h"

namespace API
{
    ModLoader* g_ModLoader = nullptr;

    ModLoader* getModLoader() { return g_ModLoader; }

    static ModContext* resolveCallerMod(lua_State* L)
    {
        lua_Debug ar;

        if (!lua_getstack(L, 1, &ar)) return nullptr;
        lua_getinfo(L, "S", &ar);

        std::string source = ar.source ? ar.source : "";
        if (!source.empty() && source[0] == '@') source = source.substr(1);
        if (source.empty()) return nullptr;

        std::filesystem::path scriptPath = std::filesystem::weakly_canonical(source);

        for (auto& mod : g_ModLoader->getAllMods())
        {
            auto rel = std::filesystem::relative(scriptPath, mod.folderPath);

            if (!rel.empty() && rel.native().find("..", 0) != 0)
                return &mod;
        }

        return nullptr;
    }

    int lua_getModName(lua_State* L)
    {
        ModContext* mod = resolveCallerMod(L);
        if (!mod)
        {
            lua_pushnil(L);
            return 0;
        } 
        else lua_pushstring(L, mod->name.c_str());
        return 1;
    }

    int lua_getModAuthor(lua_State* L)
    {
        ModContext* mod = resolveCallerMod(L);
        if (!mod)
        {
            lua_pushnil(L);
            return 0;
        } 
        else lua_pushstring(L, mod->author.c_str());
        return 1;
    }

    int lua_getModVersion(lua_State* L)
    {
        ModContext* mod = resolveCallerMod(L);
        if (!mod)
        {
            lua_pushnil(L);
            return 0;
        } 
        else lua_pushstring(L, mod->version.c_str());
        return 1;
    }

    int lua_getModInfo(lua_State* L)
    {
        ModContext* mod = resolveCallerMod(L);
        if (!mod)
        {
            lua_pushnil(L);
            return 0;
        } 

        lua_newtable(L);

        lua_pushstring(L, mod->id.c_str());
        lua_setfield(L, -2, "id");

        lua_pushstring(L, mod->name.c_str());
        lua_setfield(L, -2, "name");

        lua_pushstring(L, mod->author.c_str());
        lua_setfield(L, -2, "author");

        lua_pushstring(L, mod->version.c_str());
        lua_setfield(L, -2, "version");

        return 1;
    }

    void RegisterModInfo(lua_State* L, ModLoader* modLoader)
    {
        lua_pushcfunction(L, lua_getModName);
        lua_setfield(L, -2, "getModName");
        lua_pushcfunction(L, lua_getModAuthor);
        lua_setfield(L, -2, "getModAuthor");
        lua_pushcfunction(L, lua_getModVersion);
        lua_setfield(L, -2, "getModVersion");
        lua_pushcfunction(L, lua_getModInfo);
        lua_setfield(L, -2, "getModInfo");
    }
}