picka.log("Welcome in no damage mod")
-- also, you can write print("Welcome in no damage mod"), they work the same, i fix this
-- let's hook Player.Hurt

-- we need to write full path to Player.Hurt
-- it always in Assembly-CSharp, Terraria
-- But if, you want for example hook Unity method, you should write UnityEngine as assemblie

-- if method with overloads, you need to write exact amount of arguments, 
-- but if it doesnt, you can just write -1 as argument in picka.getMethodAddr 
local pHurt = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Player", "Hurt", -1) -- like here

-- but here you cant, because NewText has 3 overloads, we use with 4 arguments
local newText = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "NewText", 4)

-- let's check, is it real?
if pHurt then
    picka.log("Find Player.Hurt at: " .. string.format("0x%X", pHurt))

    -- so, let's hook our method
    -- but, we need to know how many arguments in method. 
    -- In Player.Hurt it is - 10
    picka.hook(pHurt, 10, function (original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
        -- if we need to use string in game, like, for example in chat, we always need to make il2cpp_string
        -- so, use picka.newString("..."), for it
        local damageStr = picka.newString("Damage taken: " .. tostring(damage))
        
        -- if we need to call methods from Terraria, or Unity, use this
        -- and else, we need to know how mane arguments in method
        picka.callNative(newText, damageStr, 0, 255, 0)

        -- for immortality, we need to call default functions, but with damage = 0

        return picka.callNative(original, instance, damageSource, 0, hitDir, pvp, quiet, crit, cooldown, dodgeable)
    end)

else
    picka.log("ERR: Cannot find Player.Hurt")
end