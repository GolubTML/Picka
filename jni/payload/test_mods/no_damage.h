#pragma once
#include <stdio.h>
#include <stdbool.h>

typedef double (*Player_Hurt_fn)(void* self, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable);
extern Player_Hurt_fn original_Player_Hurt;

double my_Player_Hurt(void* self, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable);