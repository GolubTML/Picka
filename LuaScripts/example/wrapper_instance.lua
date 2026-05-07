-- so, here the main differens in how wrappers works
-- now, we need get field and methods from instance, not from class it self
-- like, if we try use methods of Player, we get crash, because Player needs instance to work with
-- in wrapper, we have one addition argument - instance

local function wrapInstance(instance, namespace, klassName, assemblies)
    if not instance or instance == 0 then return nil end

    local class = picka.getClass(assemblies or "Assembly-CSharp", namespace, klassName)
    if not class then return nil end
    
    -- we will else use cache here
    local cache = {}
    -- and, in wrapper, we meed store the value of instance, so
    local wrapper = { _ptr = instance } -- pointer to instance

    -- all work will be the same as in wrapper_static.lua
    setmetatable(wrapper, {
        __index = function (t, k)
            if cache[k] then return cache[k] end

            local value = picka.getField(t._ptr, k)
            if value ~= nil then return value end

            local methodWrapper = function (...)
                local args = {...}
                local method = picka.getMethodAddr(class, k, #args)

                -- and here, we need to put instance as first argument
                -- because all instance methods have `this` as first argument
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

-- and yeah, it's big problem of copy-pasting code. But for test it's good
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

-- so, let's try to use it on practice
-- let's hook 

local pHurt = picka.getMethodAddr("Assembly-CSharp", "Terraria", "Player", "Hurt", -1) 
local Main = wrapClass("Terraria", "Main") -- we will need it, for Main.NewText

-- and, let's define player instance here, for optimization
local player = nil

if pHurt then
    picka.hook(pHurt, 10, function (original, instance, damageSource, damage, hitDir, pvp, quiet, crit, cooldown, dodgeable)
        -- so, let's wrap instance here, and save it
        if not player or player._ptr ~= instance then
            player = wrapInstance(instance, "Terraria", "Player")
        end

        if player.statLife < 100 then
            player.statLife = player.statLifeMax2

            -- let's use picka.newString inside of Main.NewText
            -- (yeah, i'm just too lazy creating new variable)
            Main.NewText(picka.newString("Auto heal"), 129, 49, 0)
        end

        return picka.callNative(original, instance, damageSource, 0, hitDir, pvp, quiet, crit, cooldown, dodgeable)
    end)
end