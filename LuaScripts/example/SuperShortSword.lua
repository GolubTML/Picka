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
]]    

-- here we are declaring all class that we will need
local Item = picka.class("Terraria", "Item")
local Player = picka.class("Terraria", "Player")
local Projectile = picka.class("Terraria", "Projectile")
local Main = picka.class("Terraria", "Main")

local item = nil

Item.SetDefaults:hook(function (original, instance, Type, variant)
    picka.callNative(original, instance, Type, variant)

    item = picka.wrap(instance)

    if Type == 3507 then
        item.damage = 500 -- let's damage will be 500 for example
        item.shootSpeed = 10 -- and shoot speed for example 10
        item.expert = true
        item.useStyle = 5
    end
end)

local player = nil

Player.ItemCheck_Shoot:hook(function (original, instance, i, sItem, weaponDamage, withAudioVisualFeedback)
    -- so, here we need to get item instance
    -- eventually, we have sItem here
    item = picka.wrap(sItem)

    player = picka.wrap(instance)

    if item.type == 3507 then
        if player.itemAnimation == player.itemAnimationMax - 1 then
            local px = player.position.X
            local py = player.position.Y

            local screenWidth = Main.get_screenWidth() 
            local screenHeight = Main.get_screenHeight()

            local mX = Main.get_mouseX()
            local mY = Main.get_mouseY()

            local dx = mX - (screenWidth / 2)
            local dy = mY - (screenHeight / 2)
            local dist = math.sqrt(dx*dx + dy*dy)

            if dist < 1 then dist = 1 end

            local speed = item.shootSpeed
            if speed == 0 then speed = 12.0 end

            local source = Projectile.GetNoneSource()

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
                    79,
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