picka.log("Hello in method test!")

local newText_addr = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "NewText", 2);

if newText_addr then 
    picka.log("Found Main.NewText at: " .. string.format("0x%X", newText_addr))
else
    picka.log("ERR: Cannot find Main.NewText.\n Is problem in function?")
end

picka.log("Let's test other methods too")

local mainUpdate_addr = picka.getMethodAddr("Asssembly-CSharp", "Terraria", "Main", "Update", 1);

if mainUpdate_addr then
    picka.log("Found Main.Update at: " .. string.format("0x%X", mainUpdate_addr))
else
    picka.log("ERR: Cannot find Main.Update!")
end

-- So, this working with 2 methods now. But what will be with overloads?