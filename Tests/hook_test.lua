picka.log("Hello in first hook test!")

-- So, here, we need to see how hook works inside of Lua
-- Let's, for example, hook Main.Initialize

local init = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "Initialize", 0)

if init then
    picka.log("Main.Initialize addres at: " .. string.format("0x%X", init))
    picka.log("Trying to hook!")

    picka.hook(init, 0, function (original, instance)
        picka.log("U see me?")

        picka.callNative(original, instance)

        picka.log("Hello from Hook (in Lua!)")
    end)
else
    picka.log("ERR: Cannot find Main.Initialize!")
end
