picka.log("Hello in get class test")

-- let't get class Main for example
local main = picka.getClass("Assembly-CSharp", "Terraria", "Main")

if main then
    picka.log("Find Main class!")

    -- let's try to get some values 

    local dayTime = picka.getFieldStatic(main, "dayTime")
    picka.log("Got dayTime! " .. dayTime)

    picka.log("Trying to set value to dayTime..")
    picka.setFieldStatic(main, "dayTime", 0);
    picka.log("It works!")

    -- local get_myPlayer = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Main", "get_myPlayer", 0)
    -- local myIdx = picka.callNative(get_myPlayer)
    -- picka.log("My Player Index from getter: " .. myIdx)
else
    picka.log("ERR: Cannot find Main class!")
end