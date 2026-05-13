-- In this mod, we are going to make super copper short sword
--[[ 
    Plan:
        We need to hook Item.SetDefaults as usual, for incresing the damage
        And we need to hook Player.ItemCheck_Shoot for triple projectile spawn
            Signature: "void Terraria_Player__ItemCheck_Shoot (Terraria_Player_o* __this, 
                int32_t i, Terraria_Item_o* sItem, 
                int32_t weaponDamage, bool withAudioVisualFeedback, 
            const MethodInfo* method);
        
        Also, we need method Utils.RotateBy for spawn
            Signature Microsoft_Xna_Framework_Vector2_o Terraria_Utils__RotatedBy 
            (Microsoft_Xna_Framework_Vector2_o spinningpoint, 
                double radians, Microsoft_Xna_Framework_Vector2_o center, 
            const MethodInfo* method);

        In addition, for projectile spawn we need method NewProjectile
            Signature int32_t Terraria_Projectile__NewProjectile (
                Terraria_DataStructures_IEntitySource_o* spawnSource, 
                float X, float Y, 
                float SpeedX, float SpeedY, 
                int32_t Type, 
                int32_t Damage, 
                float KnockBack, 
                int32_t Owner, 
                float ai0, float ai1, float ai2, 
                Terraria_NewProjectileModifier_o* modifer, 
            const MethodInfo* method);

            Oh fuck..
]]    

-- let's copy wrapInstane

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
                local method = picka.getMethodInfo(class, k, #args)

                if method then
                    picka.callMethod(method, t._ptr, ...) 
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
    local wrapper = { _ptr = class }

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

                -- picka.log("Calling " .. key .. " with " .. #args .. " arguments") -- ТУТ МЫ УВИДИМ ПРАВДУ
                -- for i, v in ipairs(args) do
                --     picka.log("Arg " .. i .. ": " .. tostring(v))
                -- end

                local method = picka.getMethodInfo(class, key, #args)

                if method then
                    return picka.callMethod(method, ...)
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

local ItemClass = picka.getClass("Assembly-CSharp", "Terraria", "Item")
if ItemClass == nil then picka.log("Cannot find Item class!") end

local iSetDefaults = picka.getMethodAddr(ItemClass, "SetDefaults", -1)
if iSetDefaults == nil then picka.log("Cannot get Item.SetDefaults!") end

local item = nil

picka.hook(iSetDefaults, 2, function (original, instance, Type, variant)
    picka.callNative(original, instance, Type, variant)

    item = wrapInstance(instance, "Terraria", "Item")
    if item == nil then picka.log("Cannot get wrap of instance!") end

    if Type == 3507 then
        item.damage = 500 -- let's damage will be 500 for example
        item.shootSpeed = 10 -- and shoot speed for example 10
    end
end)

-- and now, we need to hook Player.ItemCheck_Shoot
local PlayerClass = picka.getClass("Assembly-CSharp", "Terraria", "Player")
if PlayerClass == nil then picka.log("Cannot find Plater class!") end
local positionOffset = picka.getFieldOffset(PlayerClass, "position")

local itemCheck_Shoot = picka.getMethodAddr(PlayerClass, "ItemCheck_Shoot", -1)
if itemCheck_Shoot == nil then picka.log("Cannot get Player.ItemCheck_Shoot!") end

-- also, we need RotateBy method, as i sad before
local Utils = wrapClass("Terraria", "Utils")
if Utils == nil then picka.log("Cannot get wrapped Utils class!") end

local Projectile = wrapClass("Terraria", "Projectile")
if Projectile == nil then picka.log("Cannot get wrapped Projectile class!") end

local MainClass = picka.getClass("Assembly-CSharp", "Terraria", "Main")
local Main = wrapClass("Terraria", "Main")
local screenPosOffset = picka.getFieldOffset(MainClass, "screenPosition")

local player = nil

picka.hook(itemCheck_Shoot, 4, function (original, instance, i, sItem, weaponDamage, withAudioVisualFeedback)
    -- so, here we need to get item instance
    -- eventually, we have sItem here
    item = wrapInstance(sItem, "Terraria", "Item")
    if item == nil then picka.log("Cannot get Item instance!") end

    player = wrapInstance(instance, "Terraria", "Player")
    if player == nil then picka.log("Cannot get Player instance!") end

    if item.type == 3507 then
        if player.itemAnimation == player.itemAnimationMax - 1 then
            local px = picka.readFloat(instance, positionOffset)
            local py = picka.readFloat(instance, positionOffset + 4)

            local screenWidth = Main.get_screenWidth() 
            local screenHeight = Main.get_screenHeight()

            local mX = Main.get_mouseX()
            local mY = Main.get_mouseY()

            local vel = { x = 0, y = 0 } -- let's say, it's zero for now

            local dx = mX - (screenWidth / 2)
            local dy = mY - (screenHeight / 2)
            local dist = math.sqrt(dx*dx + dy*dy)

            if dist < 1 then dist = 1 end

            local speed = item.shootSpeed
            if speed == 0 then speed = 12.0 end

            local velX = (dx / dist) * speed
            local velY = (dy / dist) * speed

            local source = Projectile.GetNoneSource()

            picka.log("Projectile spawned at: " .. px .. " " .. py .. " Source: " .. tostring(source))
            
            picka.log("Extracted Pos: " .. tostring(px) .. ", " .. tostring(py))

            picka.log(string.format("[DEBUG] Player: %.1f,%.1f | Mouse: %d,%d", px, py, mX, mY))
            picka.log(string.format("[DEBUG] Final Vector: %.2f, %.2f", velX, velY))

            local baseAngle = math.atan(dy, dx)

            for i = -1, 1 do
                -- so, we can't use Utils.RotateBy, and so we will make our own
                local currentAngle = baseAngle + (i * 0.15) -- 0.25 - angle
                local vX = math.cos(currentAngle) * speed
                local vY = math.sin(currentAngle) * speed
                
                -- let's make more interesting result

                Projectile.NewProjectile(source, 
                    px, py,
                    vX, vY,
                    math.random(1000), -- use ALL 1000 projectiles in game 
                    item.damage,
                    0.4,
                    player.whoAmI,
                    0.0,
                    0.0,
                    0.0,
                    0
                )

                -- oh fuck
            end
        end
        
        return
    end

    picka.callNative(original, instance, i, sItem, weaponDamage, withAudioVisualFeedback)
end)