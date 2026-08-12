# Mod Structure

All mods for _Picka_ and _Picka launcher_ need to have 3 basic files:
- `main.lua`: Entry point of your mod. 
Example of `main.lua`:
``` Lua
picka.log("Hi from my mod!")
```
- `config.json`: Configuration of your mod. This json contains variables like:
	- `name`: Name of your mod. If you create mod in ___Picka Launcher___, and did not specify the name, default name will be `Unknown Mod`
	- `version`: Version of your mod. By default it `1.0`
	- `description`: Description of your mod, this is for the _launcher_. And, like a `name`, if you did not specify the description, default description will be `No description provided for this mod...`
	- `author`: Your name, nickname or initials. By default it will be: "Unknown Author"
Example of `config.json`:
``` Json
{
    "name": "My super cool mod",
    "version": "1.0",
    "description": "This mod is changing everything",
    "author": "GolubTML"
}
```
- `icon.png`: Icon for your mod. I recommend use icons 128 by 128 pixels for launcher, but you can use even 32 by 32 pixel icon. Animation is not supported for now.
## How to create mod with structure?
Use [Picka Launcher](https://github.com/GolubTML/Picka-Laucher.git) for it. Picka will give you ability to easily create mods.