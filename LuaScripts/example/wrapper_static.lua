-- So, here, will be example of wrapper.
-- Wrapper is a functions, with metatable in it.
-- Function of wrapper, is making clean code:
-- for example, instead of writing local someField = picka.getFieldStatic(klass, name)
-- we can just type klass.someField, like in C# or C++ for example

-- in wrapper, we need just 2 arguments: namespace and class name 
-- (also, we can add 3 argument - assemblies, if we need methods/fields from Unity)

local function wrapClass(namespace, klassName, assemblies)
    -- in wrap we need get class, so, here we are getting it
    local class = picka.getClass(assemblies or "Assembly-CSharp", namespace, klassName)
    if not class then return nil end

    -- else, adding cache for speed
    local cache = {}
    local wrapper = {}

    -- and here, we use metatable
    setmetatable(wrapper, {
        -- in __index we read the field
        __index = function (t, key)
            -- let's check, if field or method already in cache
            if cache[key] then
                return cache[key]
            end

            -- firstly, we need to check field
            -- if it exist, return it immedietly
            local value = picka.getFieldStatic(class, key)

            if value ~= nil then
                return value
            end

            -- but if there is no field, it's method
            local methodWrapper = function (...)
                local args = {...} -- we will need this for overloads
                local method = picka.getMethodAddr(class, key, #args)

                if method then
                    return picka.callNative(method, ...)
                else
                    picka.log("ERR: Class method " .. key .. " with amount of arguments " .. #args .. " not found!")
                end
                
            end

            -- and here, we add method to cache
            cache[key] = methodWrapper
            return methodWrapper
        end,
        -- and so, in __newindex we set new value to fiels
        __newindex = function (t, key, v)
            return picka.setFieldStatic(class, key, v)
        end
    })
    
    return wrapper
end

-- so, let's test it, for example, on Main.dayTime
-- so, here we get wrapped class of Terraria.Main, and now, we can get fields
local Main = wrapClass("Terraria", "Main")

-- let's print into adb logcat, valur of Main.dayTime
picka.log(Main.dayTime) -- and here, called __index

-- let's call NewText from Main
local il2cpp_string = picka.newString("Hello from wrapper!")
Main.NewText(il2cpp_string, 255, 0, 255)

-- and now, let's set new value to it
Main.dayTime = (Main.dayTime == 0) -- and here, called __newindex

-- and now, we can change from day to night easily