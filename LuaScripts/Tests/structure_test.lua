-- let's see how we can use structures
-- let's call Main.NewText but with structure: Color

local PlayerClass = picka.getClass("Assembly-CSharp", "Terraria", "Player")
if PlayerClass == nil then print("Cannot get Player class!") end

local someMethodInfo = picka.getMethodInfo(PlayerClass, "Hurt", -1)

local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")

local anotherMethodInfo = picka.getMethodInfo(MainClass, "NewText", 2)
picka.log("Found NewText with 2 overloads! " .. tostring(anotherMethodInfo))

local pHurt = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Player", "Hurt", -1) -- like here

if pHurt then
    picka.log("Find Player.Hurt at: " .. string.format("0x%X", pHurt))
    
    picka.hook(pHurt, 10, function (original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
        
        local il2cpp_str = picka.newString("Using structure Color!")

        local color = {R = 120, G = 255, B = 20, A = 255}

        picka.callMethod(anotherMethodInfo, il2cpp_str, color)
        
        print("NewText called successfully!")
        return picka.callNative(original, instance, damageSource, 0, hitDir, pvp, quiet, crit, cooldown, dodgeable)
    end)

else
    picka.log("ERR: Cannot find Player.Hurt")
end