picka.log("Hello in native call test!")

-- lets call Main.NewText
-- it will be with 4 argumnents
local addr = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "NewText", 4) 

if addr then
    picka.log("Find Main.NewText with 4 argumnents: " .. string.format("0x%X", addr))

    local il2cpp_str = picka.newString("Hello from lua!")
    picka.callNative(addr, il2cpp_str, 255, 0, 0)

    -- while true do
    --    picka.callNative(addr, il2cpp_str, 255, 0, 0)
    --end 
else
    picka.log("ERR: Cannot find Main.NewText with 4 args!")
end