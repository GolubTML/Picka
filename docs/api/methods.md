# Methods

As you can see from past chaptes (especially in __Classes and Instances__), I said, that you can `call` method of _wrapped object_ through `picka.class` or `picka.wrap`. This allows you to call methods from wrapped object such as: `Main.NewText`, `Player.AddBuff`, `Item.NewItem` and etc. 

Let's use it in following example:
``` Lua
local Main = picka.class("Terraria", "Main")

Main.NewText("Hello there!", 255, 255, 255)
```
Here Picka API will automatically find appropriate method `NewText` with correct _overload_.
(__But you should know, that Picka select overload only by the number of arguments, not by their types. Because of this, some overloads may be impossible to call!__. This will be fixed in future updateds)

## Method overloads
Several methods in Terraria Classses has _overloads_. For example, we have two overloads of method `NewText`:
``` C#
static void Main.NewText(System.String message, byte r, byte g, byte b);
static void Main.NewText(System.String message, Color color);
```
Diffents here only in arguments, so, when you use `NewText` from example, Picka uses first _overload_ of method. But, you still can use second overload like this:
``` Lua
Main.NewText("Hello!", { R = 255, G = 255, B = 255 })
```
Result will be similar to first example, the only exception, you use struct `Color` via Lua _table_. 

## Passing structures as arguments
Some methods require _C# structures_ as argument. For example, we have method:
``` C#
void Player.Teleport(Vector2 newPos, int Style, int extraInfo);
```
To call this method, we need to provide `Vector2` value.
Picka allows supported C# structures to be represented using Lua tables:
``` Lua
local Main = picka.class("Terraria", "Main")
local player = picka.wrap(Main.player[Main.get_myPlayer()])

local newPosition = {
	X = 228,
	Y = -1337
} 

-- we can pass it like argument
player.Teleport(newPosition, 0, 0)
```
Here, `newPosition` represents `Vector2` structure and can be passed directly to the method.