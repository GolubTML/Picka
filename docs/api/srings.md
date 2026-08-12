# Strings

Lua and C# strings are two different types. Picka automatically converts Lua strings to C# `System.String` when they are used in methods or fields.

For example, Lua strings can be passed directly to methods:
``` Lua
local Main = picka.class("Terraria", "Main")

Main.NewText("Hi!", 255, 255, 255)
```

They can also be assign directly to fields. Example can be:
``` Lua
local Main = picka.class("Terraria", "Main")
local player = picka.wrap(Main.player[Main.get_myPlayer()])

player.name = "CoolName"

Main.NewText(player.name, 255, 255, 255)
```