# Picka Lua API

Picka provides a Lua API for writing mods for mobile Terraria.

The API is designed to hide most IL2CPP internals from mod developers. With this API you can work with Terraria classes, instances, objects, methods, hooks and fields using Lua-style scripting.
For example, here are small mod:

``` Lua
local Main = picka.class("Terraria", "Main")
local player = picka.wrap(Main.player[Main.get_myPlayer()])

player.statLifeMax = 9999
player.statLife = player.statLifeMax

picka.log("Player position by x: " .. player.position.X)
picka.log("Player position by y: " .. player.position.Y)
```

Before exploring the API, it is important to understand how Picka mods are structured.
See [Mod Structure](mod_structure.md) before continue with the API overview.

## API overview

### High-level API:
- [C# classes and instances](api/classes_and_instances.md)
- [Fields and Structures](api/fields_and_structures.md)
- [Strings](api/strings.md)
- [Arrays](api/arrays.md)
- [Methods](api/methods.md)
- [Hooks](api/hooks.md)
### Utility API:
- [Logging](api/logging.md)
- [Mod information](api/mod_information.md)
### Low-level API:

The low-level API provides direct access to Picka's internal Il2Cpp functionality. It is mainly intended for advanced modding, debugging, and cases where the High-level API is not enough.

For detailed documentation, see the [Low-Level API](api/low-level_API.md)