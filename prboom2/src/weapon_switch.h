//
// Copyright(C) 2026 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Weapon Switch
//

#ifndef __WEAPON_SWITCH__
#define __WEAPON_SWITCH__

#include "d_player.h"

dboolean G_WeaponSelectable(weapontype_t weapon);
weapontype_t G_AdjustWeaponSelection(const player_t *player, weapontype_t weapon, weapontype_t current_weapon);
void G_NextWeaponUpdate(void);
dboolean G_NextWeaponActivate(void);
dboolean G_NextWeaponDeactivate(void);
weapontype_t G_NextWeaponTranslate(weapontype_t weapon);
void G_NextWeaponResendCmd(void);
void G_NextWeaponReset(weapontype_t weapon);


#endif
