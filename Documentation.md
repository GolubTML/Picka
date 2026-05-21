All mods for Picka written in Lua via specific API. Currently, Picka API only give 16 functions for mods, but for now, it's more than enough. For better understanding of modding with Lua, I recommend to learn Lua itself. 

Picka API currently provides 16 methods., and here are list of it:
1. [Log](#log):
	`picka.log(str)`
2. [Il2Cpp Structures](#il2cpp-structures):
	`picka.newString(str)`
3. [Classes](#classes):
	`picka.getClass(assemblies, namespace, className)`
	`picka.getStaticField(class, fieldName)`
	`picka.setStaticField(class, fieldName, value)`
	`picka.getField(instance, fieldName)`
	`picka.setField(instance, fieldName, value)`
4. [Methods](#methods):
	Following methods will have overloads:
	Method Address:
		1.`picka.getMethodAddr(assemblies, namespace, className, methodName, args)`
		2.`picka.getMethodAddr(class, methodName, args)`
	MethodInfo:
		1.`picka.getMethodInfo(assemblies, namespace, className, methodName, args)`
		2.`picka.getMethodInfo(class, methodName, args)`
	Method calling and hooking:
		(callNative and callMethod have big differens, later on, i will show it)
		`picka.callNative(methodPointer, args)`
		`picka.callMethod(methodPointer, args)`
		`picka.hook(methodPtr, args, func)`
5. [Arrays](#arrays):
	`picka.getArrayLength(array)`
	`picka.getArrayElement(array, index)`
6. [Memory manipulation](#memory-manipulation):
	`picka.readFloat(instance, fieldOffset)`
	`picka.getFieldOffset(instance, fieldName)`

Also, you might want to see useful examples of mods. Click here -> [Some more information](#some-more-information)


# Examples:
#### Log
The most easiest method in API, just print log in `logcat` (use adb logcat | grep "Payload" via USB debugging).

Example:
``` Lua
picka.log("Hi from my Mod!")
-- also, you can use print as well, API works with this too
print("And hi from here too!")
```

### Il2Cpp Structures
For now, Picka has only one method to work with Il2Cpp structure. `picka.newString` uses for cases, where we need to cast C# strint (`System.String`) to a method. Example of it, can be `Main.NewText` in Terraria, it uses `System.String` from C#.

Example:
``` Lua
local il2cppString = picka.newString("Il2Cpp string was created!")
-- just create il2Cpp string
```

# Classes
## picka.getClass:
So, `picka.getClass` returns pointer to some class in memory. 
``` Lua
picka.getClass(assembly, namespace, className) -> pointer
```
With it, we can create some small mods. As arguments, `getClass` takes `assembly` - where we need to find namespace (for all Terraria methods it's `Assembly-CSharp`), `namespace` - where our class is (more often, you will use `Terraria` as a namespace, but i recommend to check, in what namespace, you class located. For better research, you can use this [website](https://docs.tmodloader.net/docs/1.4-preview/), but it's for Terraria 1.4)

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
-- also, you have acces to Unity classes as well. Assemblies of Unity often is Unity-Engine (and this namespace too)
```

## picka.getStaticField:
With this method, we can get value of static(!) field in class. As arguments, takes `class` pointer and `field` name. 

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")

local dayTime = picka.getStaticFiled(MainClass, "dayTime") -- return dayTime bool
picka.log("Day time is: " .. tostring(dayTime))
```

## picka.setStaticField:
Opposite of `getStaticField`. Set specific value to field (will not work, if field - class or structure!). As arguments, takes `class` pointer, `field` name and `value`. 

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")

local dayTime = picka.getStaticFiled(MainClass, "dayTime") 

if dayTime ~= 0 then
	picka.setStaticField(MainClass, "dayTime", 1) -- 1 is True
	picka.log("Now it's day!")
end
```

## picka.getField & picka.setField:
It's work's the same as `getStaticField` and `setStaticField`, but! as first argument, these methods get *non static classes* or *instance* in other words. It uses in case, if you want to increase player health for example. Other arguments are the same.

Example:
``` Lua
-- i really don't want to use hooks here, but, i should
-- let's say, we hooks Player.Hurt, and we want to get statLife and statLifeMax

picka.hook(pHurt, 10, function (original, instance, ...)
	local statLifeMax = picka.getField(instance, "statLifeMax")
	picka.log("Player statLifeMax is: " .. tostring(statLifeMax))
	
	-- and let's here set statLifeMax to statLife
	picka.setField(instance, "statLife", statLifeMax)
	picka.log("Hp restored!")
	
	picka.callNative(original, instance, ...) -- i will explain this later
end)
```


# Methods

Here, we have two overloads, for two methods: `getMethodAddr` and `getMethodInfo`. Let's start with `getMethodAddr`.

## picka.getMethodAddr:
This function returns pointer to specific method.
``` Lua
picka.getMethodAddr(assembly, namespace, className, methodName, args) -> pointer
-- or
picka.getMethodAddr(class, methodName, args) -> pointer
```
As we see, this function has 2 overloads. In first,  we have 5 arguments in total: `assembly`, `namespace`, `className`, `methodName` - name of our method we need, `args` - how many argumnets this method takes (*note*: if method doesn't have any overloads, we can cast `-1` as args value. It will automatically find it). With this, we can get methods, and call it!

Example
``` Lua
-- i will use second overload, because it is easier to understand
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local NewTextAddr = picka.getMethodAddr(MainClass, "NewText", 4) -- Main.NewText has 3 overloads, i will use this one: NewText(str, r, g, b)

if NewTextAddr then
	picka.log("Find NewText addr: " .. string.format("0x%X", NewTextAddr))
else
	picka.log("Cannot find NewText with 4 argumetns!")
end
```

## picka.getMethodInfo:
As `getMethodAddr`, this function work the same, but(!), it's return MethodInfo (Il2Cpp struct), instead of pointer to memory.
``` Lua
picka.getMethodInfo(assembly, namespace, className, methodName, args) -> MethodInfo

picka.getMethodInfo(class, methodName, args) -> MethodInfo
```

For what we need this? It wil later uses in `picka.callMethod` function. Soon, you will see.

Example
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local NewText = picka.getMethodInfo(MainClass, "NewText", 2) -- let's here use overload with 2 arguments: NewText(str, Color)

picka.log("Found NewText with 2 overloads! " .. tostring(NewText))
```

## picka.callNative & picka.callMethod:
And *here*, starts interesting part. Now, we can call our methods via these methods. 
Arguments for both the same, but have one different:

``` Lua
picka.callNative(methodPtr, args) -- you put as many arguments as the method requires
picka.callMethod(methodInfo, args) -- the same as callNative, but first argument is MethodInfo
```

Let's firstly understand, what the difference between this two. `picka.callNative` - it's an old API for calling methods. `callNative` didn't care much about what data types you give as an argument. It can cause to crushes, so, use it, only if you use ***simple*** method (by ***simple*** method, i assume methods, that didn's use complex structures and class. Example of *simple* method can be: `Main.NewText(System.String, byte r, byte g, byte b)` - because it's only use `byte` and `string` as data types).
Also, `picka.callNative` used it `picka.hook`, because.. i don't optimezed `hook` for `callMethod` (i'm lazy tbh).

Example
``` Lua
-- let's get method (NewText)
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local NewText = picka.getMethodAddr(MainClass, "NewText", 4)

-- And else, NewText uses System.String, so, let's use picka.newString!
local hiString = picka.newString("Hi from picka.callNative!")
-- and here we call it
-- we need provide 4 arguments it in the same order as in the signature of the original method
picka.callNative(NewText, hiString, 255, 0, 0) -- red message 
```

#### And now, picka.callMethod
Remember, when i told, we would need to use `picka.getMethodInfo` in future? Here it is. Main different from `callNative`, we can use and call ***complex*** methods with it (by ***complex*** methods, i assume methods, which uses `float/double` types, *structures* and classes. For example: `Projectile.NewProjectile(IEntitySource spawnSource, float x, float y, float vx, float vy, int Type, int damage, float knockBack, int owner, float ai0, float ai1, float ai2, NewProjectileModifier modifer)`, or, for example, something much easier: `Main.NewText(System.String string, Color color)`, where `Color` is a struct)

Example
``` Lua
-- let's get Main class as usual
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local NewText = picka.getMethodInfo(MainClass, "NewText", 2) -- second argument - Color

local hiString = picka.newString("Hello from callMethod!")
local color = {R = 0, G = 255, B = 0} -- yeah, we create table, with signature similar to Microsoft.Xna.Framework.Graphics.Color

-- let's call the method
picka.callMethod(NewText, hiString, color)
```

## picka.hook:
The most interesting part is here! `Hook` provides you with a method, to modify internal functions or methods of Terraria, just by using Lua. As arguments, `picka.hook` uses only 3 arguments:
``` Lua
picka.hook(method, argsCount, function(original, ...)
	picka.callNative(original, ...)
end)
```

`method` - pointer (yes, pointer, so you should use `picka.getMethodAddr` for it), `argsCount` - how many arguments in signature of func and.. `function` it self. Mention for `picka.hook`: if method is non static (or uses `intance`) you must define it in hook `function` signature. For example:
``` Lua
picka.hook(playerMethod, someArgs, function(original, instance, ...) --here!
	picka.callNative(original, instance, ...)
end)
```

But, if method is static, you musn't define `instance`. 

Example of small mod:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local Initialize = picka.getMethodAddr(MainClass, "Initialize", 0) -- Main.Initialize doen't have any arguments

picka.hook(Initialize, 0, function (original, instance)
	picka.log("You see me?")
	
	picka.callNative(original, instance) -- calling original method
	
	picka.log("Hello from hook!")
end)
```

# Arrays
And, with picka, we can work with arrays too. We have 2 methods for this only, but, it's enough for now. `picka.getArrayLength` and `picka.getArrayElement`
``` Lua
picka.getArrayLength(array) -> int
picka.getArrayElement(array, index) -> Object
```
`picka.getArrayElement` can return `Objects` like Classes or Structures as well. 

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local npcs = picka.getFieldStatic(MainClass, "npc") -- array of NPCs

if npcs then
	local len = picka.getArrayLength(npcs) -- let's get array length
	picka.log("NPC Array Length: " .. tostring(len)) -- it should be 201
	
	local firstNPC = picka.getArrayElement(npcs, 0) -- and here, let's get first NPC in array (pointer)
	picka.log("First NPC Ptr: " .. tostring(firstNPC))
end
```


# Memory manipulation
So, here, we also have only 2 methods: `picka.readFloat(instance, fieldOffset)` and `picka.getFieldOffset(instance, fieldName)`.

## picka.readFloat & picka.getFieldOffset
`picka.fieldOffset` used in cases, where we just need get offset of field, for example, in `Structures`. Signature:
``` Lua
picka.getFieldOffset(class, fieldName) -> address
```

This specific method used in cases, where you need to get acces to fields of **Structure**, because, `getStaticField` or `getField` doesn't work with structures. For example, if you want to get `Player` postion, you need firstly get `position` field, and then use `readField`
Signature:
``` Lua
picka.readFloat(instance, offset) -> float
```

Example
``` Lua
local PlayerClass = picka.getClass("Assembly-CSharp", "Terraria", "Player")
local positionOffset = picka.getFieldOffset(PlayerClass, "position")

-- let's imagine, that we are in some player method hook
local playerHurt = picka.getMethodAddr(...)

picka.hook(playerHurt, 10, function(original, instance, ...)
	local playerX = picka.readFloat(instance, positionOffset)
	local playerY = picka.readFloat(instance, positionOffset + 4) 
	
	picka.log("Player X: " .. playerX .. " , Player Y: " .. playerY)
end)
```


# Some more information
If you want to see some mods example (or tests), you can just look for `LuaScripts/example or LuaScripts/Tests`. Good luck! :D
