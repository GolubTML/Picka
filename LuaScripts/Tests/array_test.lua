picka.log("Hello in array test!")

-- let's get the array of active NPC's from Main class

local Main = picka.class("Terraria", "Main")
-- so, here are the array of NPCs
local npcs = Main.npc

if npcs then
    local len = #npcs
    picka.log("NPC Array Length: " .. tostring(len))

    -- and, let's try get first NPC from array
    local firstNPC = Main.npc[0];
    picka.log("First NPC Ptr: " .. tostring(firstNPC))

    local npc = picka.wrap(firstNPC)
    picka.log("Trying to set size for first npc...\n Is npc active: " .. tostring(npc.active) .. ", type: " .. tostring(npc.type))
    -- id of first npc which is loaded always will be Old Man!


    npc.scale = 4
    npc.boss = true
    npc.lifeMax = 100000
    npc.life = npc.lifeMax
    picka.log("Size set!")
end

local player = picka.wrap(Main.player[Main.get_myPlayer()])
-- local pos = player.position
player.name = "Cool name"

local playerName = "Player name: " .. player.name
Main.NewText(playerName, 255, 255, 255)

-- local newPosition = {
--     X = 228,
--     Y = -1337
-- }
-- 
-- player.Teleport(newPosition, 0, 0)
-- pos.X = 228
-- pos.Y = -1337
-- 
-- player.position = pos