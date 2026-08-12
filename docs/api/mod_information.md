# Mod Information

Picka gives 4 methods with which you can get several infomations about mod. Here are list of all of them:
- `picka.getModName()`
- `picka.getModAuthor()`
- `picka.getModVersion()`
- `picka.getModInfo()`

## picka.getModName

Which is name suggest, you can get _mod name_ through this method. Signature:
``` Lua
picka.getModName() -> string
```

For example, my mod name is "Cool Mod", i can get this information, and print it in game:
``` Lua
local Main = picka.class("Terraria", "Main")

local myMod = "My mod name is: " .. picka.getModName() 
Main.NewText(myMod, 255, 255, 255)
```

In result i will get in chat "My mod name is: Cool Mod".

## picka.getModAuthor and picka.getModVersion

Similar to `picka.getModName`, i can get _author name_ and _mod verison_. Example:
``` Lua
local Main = picka.class("Terraria", "Main")

local myMod = "My mod name is: " .. picka.getModName()
Main.NewText(myMod, 255, 0, 0)

local modAuthor = "Mod author name is: " .. picka.getModAuthor()
Main.NewText(modAuthor, 0, 255, 0)

local modVersion = "Mod version is: " .. picka.getModVersion()
Main.NewText(modVersion, 0, 0, 255)
```

## picka.getModInfo

This is combination of `getModName`, `getModAuthor` and `getModVersion`, but, it returns all this information in table. Example:
``` Lua
local Main = picka.class("Terraria", "Main")

local modInfo = picka.getModInfo()

local myMod = "My mod name is: " .. modInfo.name
Main.NewText(myMod, 255, 0, 0)

local modAuthor = "Mod author name is: " .. modInfo.author
Main.NewText(modAuthor, 0, 255, 0)

local modVersion = "Mod version is: " .. modInfo.version
Main.NewText(modVersion, 0, 0, 255)

local modId = "Mod id is: " .. modInfo.id -- id is name of folder where mod is located
Main.NewText(modId, 255, 255, 255)
```