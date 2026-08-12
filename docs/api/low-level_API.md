# Low-Level API

Picka's Low-Level API provides direct access to the underlying Il2Cpp objects and methods used by Terraria.

Unlike [High-Level API](../home.md), which is designed to hide most of the _Il2Cpp_-specific details, the Low-Level API expose them directly. This gives you more control over how classes, methods, fields and memory are accessed, but also requires a better understanding of Il2Cpp and Terraria's internal structures.

It's recommended to use High-level API in mods whenever possible. 
The Low-Level API is mainly useful when: 
- a feature is not available through the High-Level API; 
- you need direct access to a method pointer or `MethodInfo`; 
- you need to work with fields or structures that are not supported by the High-Level API; 
- you are developing your own systems or loaders on top of Picka. 
 
**Warning:** Low-Level API functions operate much closer to the game's internals. Passing an incorrect object, pointer, argument count or data type may cause crashes or undefined behaviour.

## Before you start

The low-level API uses several concepts that are hidded by the high-level API.

- `C# Class` - a pointer/reference to Il2Cpp class.
- `Instances` - an object created from `C# Class`
- `Method pointer` - native addres of method
- `MethodInfo` - Il2Cpp metadata describing a method
- `Field offsets` - the location of fields inside object or structure

If you are nor familiar with this concepts, it's better to start with High-Level API, instead of low-level.
# API overview:

1. [Classes](#classes):
	- `picka.getClass(assemblies, namespace, className)`
	- `picka.getStaticField(class, fieldName)`
	- `picka.setStaticField(class, fieldName, value)`
	- `picka.getField(instance, fieldName)`
	- `picka.setField(instance, fieldName, value)`
2. [Methods](#methods):
- Method Address:
	 1.`picka.getMethodAddr(assemblies, namespace, className, methodName, args)`
	 2.`picka.getMethodAddr(class, methodName, args)`
- MethodInfo:
	- 1.`picka.getMethodInfo(assemblies, namespace, className, methodName, args)`
	- 2.`picka.getMethodInfo(class, methodName, args)`
- Method calling and hooking:
	- `picka.callNative(methodPointer, args)`
	- `picka.callMethod(methodPointer, args)`
	- `picka.hook(methodPtr, args, func)`
3. [Arrays](#arrays):
	- `picka.getArrayLength(array)`
	- `picka.getArrayElement(array, index)`
4. [Memory manipulation](#memory-manipulation):
	- `picka.readFloat(instance, fieldOffset)`
	- `picka.getFieldOffset(instance, fieldName)`
## Classes
### picka.getClass:
So, `picka.getClass` returns pointer to some class in memory. 
``` Lua
picka.getClass(assembly, namespace, className) -> pointer
```
With it, we can create some small mods. As arguments, `getClass` takes `assembly` - where we need to find namespace (for all Terraria methods it's `Assembly-CSharp`), `namespace` - where our class is (more often, you will use `Terraria` as a namespace, but i recommend to check, in what namespace, you class located.

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
-- also, you have acces to Unity classes as well. Assemblies of Unity often is Unity-Engine (and this namespace too)
```

### picka.getStaticField:
With this method, we can get value of static(!) field in class. As arguments, takes `class` pointer and `field` name. 

Example:
``` Lua
local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")

local dayTime = picka.getStaticFiled(MainClass, "dayTime") -- return dayTime bool
picka.log("Day time is: " .. tostring(dayTime))
```

### picka.setStaticField:
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

### picka.getField & picka.setField:
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

## Methods

Here, we have two overloads, for two methods: `getMethodAddr` and `getMethodInfo`. Let's start with `getMethodAddr`.

### picka.getMethodAddr:
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

### picka.getMethodInfo:
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

### picka.callNative & picka.callMethod:
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
Remember, when i told, we would need to use `picka.getMethodInfo` in future? Here it is. Main different from `callNative`, we can use and call methods which required as arguments __C# Structures__. For example: `Projectile.NewProjectile(IEntitySource spawnSource, float x, float y, float vx, float vy, int Type, int damage, float knockBack, int owner, float ai0, float ai1, float ai2, NewProjectileModifier modifer)`, or, for example, something much easier: `Main.NewText(System.String string, Color color)`

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

