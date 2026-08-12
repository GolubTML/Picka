# Classes and Instances

Picka API provides two main methods for working with Terraria objects:
1. `picka.class()` which return __wrapped C# class__
2. `picka.wrap()` which returns a wrapper for an existing C# object instance.

## Signature:
``` Lua
picka.class(namespace, name) -> C# Class wrapper
```
First argument is a `namespace`. It can be: `Terraria`, `Microsoft.Xna.Framework`, `Microsoft.Xna.Framework.Graphics` and etc. 
Second argument is a name, specifically __class name__. For example: `Main`, `Player`, `Projectile`, `NPC` and etc.

`picka.class` give you ability to find and set static fields of the class and also, call static methods of class. Example mini mod:
``` Lua
local Main = picka.class("Terraria", "Main")

picka.log("Is day time? " .. tostring(Main.dayTime))
Main.NewText("Hi!", 255, 255, 255)
```

As we can see, we can get static field `dayTime` and also call static method `NewText` with specific overload (it's important to know some overloads for methods, so, you should check [this](https://docs.tmodloader.net/docs/stable/annotated.html) website to see all methods, fields and classes). Well, now we move on next method.

``` Lua
picka.wrap(instance) -> object wrapper
```
There is only one argument - `instance`. Instance is a object of some class, for example, when you already have `Player` object.

Like `picka.class`, `picka.wrap` give you ability to find and set fields and call methods for existing object. Example:
``` Lua
local Main = picka.class("Terraria", "Main")
-- we get existing NPC object from array of NPCs
local npc = picka.wrap(Main.npc[0])

npc.scale = 3
npc.boss = true
npc.lifeMax = 100000
npc.life = npc.lifeMax
```
And also, `picka.wrap` does not create a new class object, it only wrap existing object instance!

