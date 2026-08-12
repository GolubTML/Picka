# Logging

Picka provides one method for debugging your mods - `picka.log`, you have already seen this method before, in previous chapters. `picka.log` write a _message_ to the file `picka.log` in Mods directory (in /storage/emulated/0/Mods/ directory). 

## Signature
It's one of the most important method, besides of `Main.NewText` for _in-game_ debugging. Signature:
``` Lua
picka.log(message)
```

With this we can create one of the simples mods:
``` Lua
picka.log("Hi from my mod!")
```

Also, as you have seen before, you can display some useful information here, like player position, HP, velocity and etc. Example:
``` Lua
picka.log("Player HP: " .. player.statLife)
```


This is very useful for hooks when you want to check whether a hook is working:
``` Lua
Player.AddBuff:hook(function(original, type, timeToAdd, quiet, foodHack)
	picka.log("We are inside of Player.AddBuff")
	
	picka.callNative(original, type, timeToAdd, quiet, foodHack)
end)
```