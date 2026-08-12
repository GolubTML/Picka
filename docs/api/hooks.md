# Hooks

One of the main parts of modding is ___Hooks___. Without _hooks_ it will be impossible to make content mods. Picka provides two ways to hook methods: `picka.hook` which is low-level API and `Method:hook` which is high-level API. It's recommended to use the second one, because for `picka.hook` you also need to get _method pointer_ (which is explained in [Low-Level API](low-level_API.md) section).
So, with hook we can do actual mods, because it gives us opportunity to change behaviour of methods.
## Syntax

`Method:hook` has two overloads. 
Signature for both overloads:
``` Lua
-- first overload
Class.Method:hook(function(original, ...)
	-- do some stuff
	
	picka.callNative(original, ...)
end)

-- second overload
Class.Method:hook(argc, function(original, ...)
	-- also do some stuff
	
	picka.callNative(original, ...)
end)
```
So, as we can see, `hook` contains with one main argument _Lua_ `function()`. Inside `function()`, you  define the arguments that the hooked method receives.

If the method you want to hook is non _static_ (which means it is an instance method), the following argument after `original` should be `instance` on which the method was called:
``` Lua
Class.NotStaticMethod:hook(function(original, instance, ...)
	picka.callNative(original, instance, ...)
end)
```
Static methods do not have `instance` argument. You can find signature of specific method [here](https://docs.tmodloader.net/docs/stable/annotated.html).

In second overload, we see the argument `argc` which stands for _argument count_. This argument allows you to select a specific overload of a method, for example, if you want to hook `Main.NewText(string, color)` instead of `Main.NewText(string, r, g, b)` you should specify in `argc` number __2__, so picka can find this specific overload. 
(__Important to know, here the same issue as in Methods, Picka selects overload only by amount of arguments you specify, not by their _type_.__ )

For **information**, `picka.callNative` is an old way to call methods, but, you should use it in hooks, because this is currently the way to call the original function from a hook.
Also, for most methods you want to call original function, but it's not __required__. If you want to change behavior entirely, you can simply omit the call:
``` Lua
Main.NewText:hook(function(original, string, r, g, b)
	-- do not call original
	-- The original implementation will not be executed
	
	picka.log("New 'Main.NewText'") 
end)
```

## What can we do with hooks?
This is very interesting question. In general, you could make mods like changing stats and behavior of some weapon, or change AI of some boss. Here you can see example of simple mod:
``` Lua
local Item = picka.class("Terraria", "Item")

Item.SetDefaults:hook(function(original, instance, Type, variant)
	picka.callNative(original, instance, Type, variant)
	
	local item = picka.wrap(instance)
	
	if item.type == 4956 then -- id 4956 is Zenith
		item.damage = 1 
		item.expert = true
		item.SetNameOverride("Htinez") -- setting new name
	end
end)
```
We call original function __before__ we change stats of Zenith, because we need to wait for every item to initialize, so we can change stats.

And also, example with overload:
``` Lua
local Main = picka.class("Terraria", "Main")

Main.NewText:hook(2, function(original, string, color) 
	picka.log("Hooked NewText with 2 arguments! (string, color)")
	picka.callNative(original, string, color)
end)
```

You can make pretty much __anything__ with this, including loading completely new _modded content_ - you just need to figure out how to build it. 

This is all for High Level API, you can see example mods [here](../../LuaScripts/example/).
Also, you can see [Utility API](../home.md#utility-api) and [Low Level API](low-level_API.md) for more understanding, how all of this works.

__Enjoy__