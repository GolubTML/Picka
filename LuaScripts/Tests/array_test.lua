picka.log("Hello in array test!")

-- let's get the array of active NPC's from Main class
-- let's just copy wrapper here

local function wrapClass(namespace, klassName, assemblies)
    local class = picka.getClass(assemblies or "Assembly-CSharp", namespace, klassName)
    if not class then return nil end

    local cache = {}
    local wrapper = {}

    setmetatable(wrapper, {
        __index = function (t, key)
            if cache[key] then
                return cache[key]
            end

            local value = picka.getFieldStatic(class, key)

            if value ~= nil then
                return value
            end

            local methodWrapper = function (...)
                local args = {...} 
                local method = picka.getMethodAddr(class, key, #args)

                if method then
                    return picka.callNative(method, ...)
                else
                    picka.log("ERR: Class method " .. key .. " with amount of arguments " .. #args .. " not found!")
                end
                
            end

            cache[key] = methodWrapper
            return methodWrapper
        end,
        __newindex = function (t, key, v)
            return picka.setFieldStatic(class, key, v)
        end
    })
    
    return wrapper
end

local Main = wrapClass("Terraria", "Main")
-- so, here are the array of NPCs
local npcs = Main.npc

if npcs then
    -- firstly, let's get length of the array
    -- it should be 200
    local len = picka.getArrayLength(npcs) 
    picka.log("NPC Array Length: " .. tostring(len))

    -- and, let's try get first NPC from array
    local firstNPC = picka.getArrayElement(npcs, 0)
    picka.log("First NPC Ptr: " .. tostring(firstNPC))
end
