local path = "storage/emulated/0/Mods/Texture_test/icon.png"

local Main = picka.class("Terraria", "Main")
local Vector2 = picka.class("Microsoft.Xna.Framework", "Vector2")
local Color = picka.class("Microsoft.Xna.Framework.Graphics", "Color")

local graphicsManager = picka.wrap(Main.graphics)
picka.log("Got graphics manager: " .. tostring(graphicsManager))

local graphicsDevice = graphicsManager.get_GraphicsDevice()
picka.log("Got graphics device: " .. tostring(graphicsDevice))

local testTexture = picka.loadTexture(path, graphicsDevice);
picka.log("Created and pushed to GPU texture: " .. tostring(testTexture));

local textureWrap = picka.wrap(testTexture)
picka.log("Texture width: " .. textureWrap.get_UnityTextureWidth() .. ", texture height: " .. textureWrap.get_UnityTextureHeight())

local spriteBatch = picka.wrap(Main.spriteBatch)
picka.log("Get spriteBatch: " .. tostring(spriteBatch))

local newPos = Vector2.new(500, 700)

Main.DrawItems:hook(function (original)
    picka.callNative(original)

    spriteBatch.Draw(testTexture, newPos, Color.White)
end)
