# Arrays

Picka allows you to work with C# arrays directly through Lua. If field of some __C# Class__ contains an _array_, you can get its length using default Lua operator - `#`  and access its elements using `[index]`. 
__Important to know__, that unlike Lua tables, which index starts with `1`, C# Arrays index starts with `0`. Therefore, to get the _first_ element of a C# array, use `0` instead of `1`. 

We can utilize this feature in the following example:
``` Lua
local Main = picka.class("Terraria", "Main")
local npcs = Main.npc -- this is array of NPCs

picka.log("NPC count: " .. #npcs) -- getting length of array

local firstNPC = npcs[0] 
if firstNPC then
	local npc = picka.wrap(firstNPC) -- and we can wrap it
	
	picka.log("NPC type: " .. npc.type)
	picka.log("NPC active: " .. tostring(npc.active))
end
```

In this example, `Main.npc[0]` gives us an existing __NPC instance__ from the `Main.npc` array. We can pass this instance to `picka.wrap()` to access its fields and methods. 
This is especially useful when you need to work with an existing object  
that you obtained from an array or another object field.