local path = "storage/emulated/0/Mods/Texture_test/icon.png"

local Main = picka.class("Terraria", "Main")
local Vector2 = picka.class("Microsoft.Xna.Framework", "Vector2")
local Color = picka.class("Microsoft.Xna.Framework.Graphics", "Color")

_G.__picka_textures = _G.__picka_textures or {}

local graphicsManager = picka.wrap(Main.graphics)
picka.log("Got graphics manager: " .. tostring(graphicsManager))

local graphicsDevice = graphicsManager.get_GraphicsDevice()
picka.log("Got graphics device: " .. tostring(graphicsDevice))

local testTexture = picka.loadTexture(path, graphicsDevice);
picka.log("Created and pushed to GPU texture: " .. tostring(testTexture));
table.insert(_G.__picka_textures, testTexture)

local textureWrap = picka.wrap(testTexture)
picka.log("Texture width: " .. textureWrap.get_UnityTextureWidth() .. ", texture height: " .. textureWrap.get_UnityTextureHeight())

local spriteBatch = picka.wrap(Main.spriteBatch)
picka.log("Get spriteBatch: " .. tostring(spriteBatch))

local newPos = Vector2.new(500, 700)
picka.log(type(newPos))
picka.log("Vector X before hook: " .. newPos.X .. " Y: " .. newPos.Y)
local told = false;

Main.DrawItem:hook(function (original, item, whoami)
    picka.callNative(original, item, whoami)

    if not told then
        picka.log("Vector X: " .. newPos.X .. " Y: " .. newPos.Y)
        told = true
    end
    
    spriteBatch.Draw(testTexture, newPos, Color.White)

    newPos.X = newPos.X + 0.01
    newPos.Y = newPos.Y - 0.01
end)
