# Fields and Structures

When you wrap a __Terraria__ object using either `picka.class` or `picka.wrap` you can use their fields directly through Lua. This allows you to use some fields for your mod or set them to specific values. However,  you should always know the _data type_ of the field that you are working with. 

Example of reading and setting values:
``` Lua
local Main = picka.class("Terraria", "Main")
local player = picka.wrap(Main.player[Main.get_myPlayer()]) -- getting player instance and wrapping it at the same time

-- here we are reading value of field statLife
picka.log("What's my player current HP? Player current HP: " .. player.statLife)

-- and now we can set some value to it
picka.log("Now the player's HP will be increased!")
player.statLifeMax = 999 -- set value 999 to field statLifeMax, now player will have maximum hp of 999
player.statLife = player.statLifeMax -- and statLife we are setting to statLifeMax
```

### Structure fields
Through API we also have access to fields-structures, like Vector2 fields of Player, NPC or Projectile. We can utilize it like this:
``` Lua
local Main = picka.class("Terraria", "Main")
local player = picka.wrap(Main.player[Main.get_myPlayer()])

-- let's read .X and .Y fields of player's position
picka.log("Player position by x: " .. player.position.X .. " and by Y: " .. player.position.Y) 

-- and let's make a `teleport` effect
picka.log("Player will be teleported to - X: 228 and Y: 1337")
local playerPos = player.position

playerPos.X = 228
playerPos.Y = -1337

player.position = playerPos
```

In this example, i use temporary variable `playerPos`, because Picka returns copy of field player.position, so modifying fields directly like this: 
``` Lua
player.position.X = 228
player.position.Y = -1337
```
will NOT modify `player.position`. This is only for fields-structures, this does not affect regular fields like `player.statLife`.

So, we always need to copy structure to temporary variable, and then assign it to original field. This way, the player's position will be modified correctly.

