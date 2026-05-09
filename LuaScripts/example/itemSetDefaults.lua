-- in this example, we are going to undestand, how to hook Item.SetDefaults
-- so, let's say, for example, we want to change stats of copper short sword
-- ID of copper short sword is 3507
-- so, we will just change damage of sword

-- we need wrap instance for it
-- let's just copy paste it

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

-- so, we need exact overload for hook
-- we will use this `void SetDefaults(int Type, ItemVariant variant)`
-- let's get class
local ItemClass = picka.getClass("Assembly-CSharp", "Terraria", "Item")

if ItemClass == nil then
    picka.log("Cannot find Item class!")
end

-- and here, let's get method
local iSetDefaults = picka.getMethodAddr(ItemClass, "SetDefaults", -1)

if iSetDefaults == nil then
    picka.log("Cannot get Item.SetDefaults!")
end

-- let's hook
-- we need 2 arguments: 
--      int Type
--      ItemVariant variant

-- let's cache wrapped item outside of hook
local item = nil

picka.hook(iSetDefaults, 2, function (original, instance, Type, variant)
    picka.log("We inside of Item.SetDefaults!")

    -- we need to call original immediately
    picka.callNative(original, instance, Type, variant)

    item = wrapInstance(instance, "Terraria", "Item")
    if item == nil then picka.log("Cannot get wrap of instance!") end

    if Type == 3507 then
        item.damage = 999
        picka.log("Changed stats of copper short sword!")
    end

end)