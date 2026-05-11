-- let's test, how we can get MethodInfo

local PlayerClass = picka.getClass("Assembly-CSharp", "Terraria", "Player")
if PlayerClass == nil then print("Cannot get Player class!") end

local someMethodInfo = picka.getMethodInfo(PlayerClass, "Hurt", -1)
print("Find methodInfo! " .. tostring(someMethodInfo))

local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
if MainClass == nil then print("Cannot get Player class!") end

local anotherMethodInfo = picka.getMethodInfo(MainClass, "NewText", 4)
print("Find methodInfo! " .. tostring(anotherMethodInfo))


local pHurt = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Player", "Hurt", -1) -- like here

if pHurt then
    picka.log("Find Player.Hurt at: " .. string.format("0x%X", pHurt))
    
    picka.hook(pHurt, 10, function (original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
        
        local il2cpp_str = picka.newString("Hi from picka.callMethod!")
        picka.callMethod(anotherMethodInfo, il2cpp_str, 0, 255, 0)
        
        print("NewText called successfully!")
        return picka.callNative(original, instance, damageSource, 0, hitDir, pvp, quiet, crit, cooldown, dodgeable)
    end)

else
    picka.log("ERR: Cannot find Player.Hurt")
end