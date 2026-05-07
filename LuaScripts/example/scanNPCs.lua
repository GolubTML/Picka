-- so here, we will see, how we can work with arrays
-- for example, let's just scan all NPCs from Main.npc
-- for this, let's just copy wrapClass and Instance

local function wrapInstance(instance, namespace, klassName, assemblies)
    if not instance or instance == 0 then return nil end

    local class = picka.getClass(assemblies or "Assembly-CSharp", namespace, klassName)
    if not class then return nil end
    
    local cache = {}
    local wrapper = { _ptr = instance }

    setmetatable(wrapper, {
        __index = function (t, k)
            if cache[k] then return cache[k] end

            local value = picka.getField(t._ptr, k)
            if value ~= nil then return value end

            local methodWrapper = function (...)
                local args = {...}
                local method = picka.getMethodAddr(class, k, #args)

                if method then
                    picka.callNative(method, t._ptr, ...) 
                else
                    picka.log("ERR: instance method " .. key .. " with amount of arguments " .. #args .. " not found!")
                end

            end

            cache[k] = methodWrapper
            return methodWrapper
        end,

        __newindex = function (t, k, v)
            picka.setField(t._ptr, k, v)
        end
    })

    return wrapper
end

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

-- let's wrap Main class
local Main = wrapClass("Terraria", "Main")
-- and get all npc
local npcs = Main.npc

if npcs then
    -- now, we need length of array
    local len = picka.getArrayLength(npcs) 
    picka.log("Scanning throught " .. len .. " NPC slots in array!")

    for i = 0, len - 1 do
        -- and let's get every npc
        local npcPtr = picka.getArrayElement(npcs, i)

        if npcPtr then
            local npc = wrapInstance(npcPtr, "Terraria", "NPC")

            if npc.active ~= 0 then
                -- let's just log every npc into logcat

                picka.log("NPC["..i.."] is active. Hp: " .. npc.life .. "/" .. npc.lifeMax)
            end

            -- and here, we can kill every npc
            -- like this:
            -- npc.life = 0;  it will kill every npc

            -- but if not frendly, so:
            if npc.friendly == 0 then npc.life = 0 end
        end
    end
end
