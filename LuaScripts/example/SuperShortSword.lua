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


-- yes, for test, i use FUCKING lighning
local LightingClass = picka.getClass("Assembly-CSharp", "Terraria", "Lighting")
local addLightMethod = picka.getMethodInfo(LightingClass, "AddLight", 5)

local player = nil

picka.hook(itemCheck_Shoot, 4, function (original, instance, i, sItem, weaponDamage, withAudioVisualFeedback)
    -- so, here we need to get item instance
    -- eventually, we have sItem here
    item = wrapInstance(sItem, "Terraria", "Item")
    if item == nil then picka.log("Cannot get Item instance!") end

    if item.type == 3507 then
        -- AND we need player instance too
        player = wrapInstance(instance, "Terraria", "Player")
        if player == nil then picka.log("Cannot get Player instance!") end

        --[[
        local px = picka.readFloat(instance, 20)
        picka.log("Type of instance: " .. type(instance))
        ]]

        local px = picka.readFloat(instance, positionOffset)
        local py = picka.readFloat(instance, positionOffset + 4)
        
        picka.log("Extracted Pos: " .. tostring(px) .. ", " .. tostring(py))

        local vel = { x = 0, y = 0 } -- let's say, it's zero for now

        local source = Projectile.GetNoneSource()
        picka.log("Step 2: Source obtained: " .. tostring(source))
        
        picka.log("Step 3: Calling NewProjectile")
        Projectile.NewProjectile(0, 
            px, py,
            0.0, -10.0,
            9,
            10,
            0.0,
            player.whoAmI,
            0.0,
            0.0,
            0.0,
            0 -- maybe, here shouldn't be nil?
        )
        
        local tileX = math.floor(px / 16)
        local tileY = math.floor(py / 16)

        picka.callMethod(addLightMethod, tileX, tileY, 1.0, 0.0, 0.0)
        picka.log("AddLight (tile) called!")

        picka.log("Projectile spawned at: " .. px .. " " .. py .. " Source: " .. tostring(source))

        --[[ for i = -1, 1 do
            local angle = i * 15
            local velocity = Utils.RotatedBy(vel, pos, angle)

            Projectile.NewProjectile(Projectile.GetNoneSource(), 
                pos, velocity,
                item.shoot,
                50,
                0.2,
                player.whoAmI,
                0,
                0,
                0,
                nil
            )

            -- oh fuck
        end ]]

        return
    end

    picka.callNative(original, instance, i, sItem, weaponDamage, withAudioVisualFeedback)
end)