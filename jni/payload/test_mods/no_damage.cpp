#include "no_damage.h"
#include "log.h"
#include "../SDK/hook.h"
#include "SDK/SDK.h"
#include <stdbool.h>
#include <dlfcn.h>

Player_Hurt_fn original_Player_Hurt = NULL; 

double my_Player_Hurt(void* self, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable)
{
    LOGI("Player.Hurt called! damage=%d hitDirection=%d crit=%d", damage, hitDirection, crit);

    damage = 0;
    
    SDK::Chat("Blocked damage!", { 0, 255, 0 });

    double result = original_Player_Hurt(self, damageSource, damage, hitDirection, pvp, quiet, crit, cooldownCounter, dodgeable);
    LOGI("Player.Hurt result=%f", result);
    return result;
}