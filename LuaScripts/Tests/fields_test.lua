picka.log("Hello in instance fields test")

-- so, here, we will testing, how we should get fields of instance
-- let's take Player fields for example

local pHurt = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Player", "Hurt", -1) 
local newText = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "NewText", 4)

if pHurt then
    picka.hook(pHurt, 10, function (original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
        picka.log("Debug instance type: " .. type(instance))
        picka.log("Debug instance value: " .. tostring(instance))

        -- let's get statLifeMax value
        local statLifeMax = picka.getField(instance, "statLifeMax")
        if statLifeMax then
            picka.log("Find statLifeMax! " .. tostring(statLifeMax))
        else
            picka.log("Field statLifeMax not found!")
        end

        -- and here, we will change statLife value with statLifeMax
        picka.setField(instance, "statLife", statLifeMax)
        picka.log("All hp restored!")

        local succesStr = picka.newString("All hp restored!")
        picka.callNative(newText, succesStr, 255, 255, 255)

        return picka.callNative(original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
    end)
end