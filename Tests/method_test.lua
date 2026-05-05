picka.log("Hello in method test!")

local addr = picka.getMethodAddr("Asssembly-CSharp", "Terraria", "Main", "NewText", 2);

if addr then 
    picka.log("Found Main.NewText at: " .. string.format("0x%X", addr))
else
    picka.log("ERR: Cannot find Main.NewText.\n Is problem in function?")
end