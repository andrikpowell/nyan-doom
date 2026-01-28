//
// Copyright(C) 1999-2004 by Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright(C) 2005-2006 by Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
//	DEH Compatibility
//

#include "doomdef.h"
#include "doomstat.h"
#include "doomdef.h"
#include "p_inter.h"
#include "p_tick.h"
#include "p_enemy.h"

#include "deh/misc.h"

#include "dsda/settings.h"
#include "dsda/sfx.h"
#include "dsda/configuration.h"

void deh_changeCompTranslucency(void)
{
  extern byte* edited_mobjinfo_bits;
  int i;
  int boom_translucent_sprites, vanilla_translucent_sprites, translucency_active;
  int config_trans_missile, config_trans_fx, config_trans_powerup;

  int predefined_translucency[] = {
    MT_FIRE, MT_SMOKE, MT_FATSHOT, MT_BRUISERSHOT, MT_SPAWNFIRE,
    MT_TROOPSHOT, MT_HEADSHOT, MT_PLASMA, MT_BFG, MT_ARACHPLAZ, MT_PUFF,
    MT_TFOG, MT_IFOG, MT_MISC12, MT_INV, MT_INS, MT_MEGA
  };
  int missile_transucency[] = { MT_FATSHOT, MT_BRUISERSHOT, MT_TROOPSHOT, MT_HEADSHOT, MT_PLASMA, MT_BFG, MT_ARACHPLAZ };
  int fx_translucency[] = { MT_FIRE, MT_SMOKE, MT_SPAWNFIRE, MT_PUFF, MT_TFOG, MT_IFOG };
  int powerup_translucency[] = { MT_MISC12, MT_INV, MT_INS, MT_MEGA };

  if (raven) return;

  boom_translucent_sprites = dsda_IntConfig(dsda_config_translucent_sprites);
  vanilla_translucent_sprites = !dsda_StrictMode() && boom_translucent_sprites > 1;
  translucency_active = (compatibility_level >= boom_compatibility_compatibility) ? !comp[comp_translucency] : vanilla_translucent_sprites;

  config_trans_missile = dsda_IntConfig(dsda_config_translucent_missiles);
  config_trans_fx = dsda_IntConfig(dsda_config_translucent_effects);
  config_trans_powerup = dsda_IntConfig(dsda_config_translucent_powerups);

  // Reset translucency
  for (i = 0; (size_t)i < sizeof(predefined_translucency) / sizeof(predefined_translucency[0]); i++)
    if (!edited_mobjinfo_bits[predefined_translucency[i]])
      mobjinfo[predefined_translucency[i]].flags &= ~MF_TRANSLUCENT;

  // Set translucency
  if (translucency_active)
  {
    // Only if translucency config is active
    if (boom_translucent_sprites)
    {
      // Set projectiles translucency
      if (config_trans_missile)
      {
        for (i = 0; (size_t)i < sizeof(missile_transucency) / sizeof(missile_transucency[0]); i++)
          if (!edited_mobjinfo_bits[missile_transucency[i]])
            mobjinfo[missile_transucency[i]].flags |= MF_TRANSLUCENT;
      }

      // Set effects translucency
      if (config_trans_fx)
      {
        for (i = 0; (size_t)i < sizeof(fx_translucency) / sizeof(fx_translucency[0]); i++)
          if (!edited_mobjinfo_bits[fx_translucency[i]])
            mobjinfo[fx_translucency[i]].flags |= MF_TRANSLUCENT;
      }

      // Set powerups translucency
      if (config_trans_powerup)
      {
        for (i = 0; (size_t)i < sizeof(powerup_translucency) / sizeof(powerup_translucency[0]); i++)
          if (!edited_mobjinfo_bits[powerup_translucency[i]])
            mobjinfo[powerup_translucency[i]].flags |= MF_TRANSLUCENT;
      }
    }
  }

  // This updates the existing things in the map.
  if (in_game)
  {
    thinker_t *th, *start_th;
    int i;
    th = &thinkercap;
    start_th = th;

    do
    {
      th = th->next;
      if (th->function == P_MobjThinker)
      {
        mobj_t *mobj;
        mobj = (mobj_t *) th;

        for (i = 0; (size_t)i < sizeof(predefined_translucency) / sizeof(predefined_translucency[0]); i++)
          if (mobj->type == predefined_translucency[i])
            mobj->flags = mobjinfo[predefined_translucency[i]].flags;
      }
    } while (th != start_th);
  }
}

void deh_applyCompatibility(void)
{
  extern byte* edited_mobjinfo_bits;
  int comp_max = (compatibility_level == doom_12_compatibility ? 199 : 200);

  max_soul = (IsDehMaxSoul ? deh_max_soul : comp_max);
  mega_health = (IsDehMegaHealth ? deh_mega_health : comp_max);

  if (comp[comp_maxhealth])
  {
    maxhealth = 100;
    maxhealthbonus = (IsDehMaxHealth ? deh_maxhealth : comp_max);
  }
  else
  {
    maxhealth = (IsDehMaxHealth ? deh_maxhealth : 100);
    maxhealthbonus = maxhealth * 2;
  }

  if (raven) return;

  if (!edited_mobjinfo_bits[MT_SKULL])
  {
    if (compatibility_level == doom_12_compatibility)
      mobjinfo[MT_SKULL].flags |= (MF_COUNTKILL);
    else
      mobjinfo[MT_SKULL].flags &= ~(MF_COUNTKILL);
  }

  if (doom_v11)
  {
    int i;

    // Add rad suits to item count for Doom v1.1
    if(!edited_mobjinfo_bits[MT_MISC14])
      mobjinfo[MT_MISC14].flags |= (MF_COUNTITEM);

    // Doom v1.1 - link missing sounds
    doom_S_sfx[sfx_pdiehi].link = &doom_S_sfx[sfx_pldeth];
    doom_S_sfx[sfx_tink].link   = &doom_S_sfx[sfx_stnmov];
    doom_S_sfx[sfx_itmbk].link  = &doom_S_sfx[sfx_itemup];
    doom_S_sfx[sfx_getpow].link = &doom_S_sfx[sfx_itemup];

    // Following changes actually also apply to 1.2, but I'm not gonna add them to that right now
    //
    // Boss spider is not fullbright when attacking
    if (!edited_mobjinfo_bits[MT_SPIDER])
      if (states[S_SPID_ATK1].frame == FF_FULLBRIGHT)
        for (i = S_SPID_ATK1; i <= S_SPID_ATK4; ++i)
          states[i].frame &= ~FF_FULLBRIGHT;

    // Powerups are not fullbright
    // soulsphere
    if (!edited_mobjinfo_bits[MT_MISC12])
      if (states[S_SOUL].frame == FF_FULLBRIGHT)
        for (i = S_SOUL; i <= S_SOUL6; ++i)
          states[i].frame &= ~FF_FULLBRIGHT;

    // invulnerability
    if (!edited_mobjinfo_bits[MT_INV])
      if (states[S_PINV].frame == FF_FULLBRIGHT)
        for (i = S_PINV; i <= S_PINV4; ++i)
          states[i].frame &= ~FF_FULLBRIGHT;

    // berserk
    if (!edited_mobjinfo_bits[MT_MISC13])
      if (states[S_PSTR].frame == FF_FULLBRIGHT)
        states[S_PSTR].frame &= ~FF_FULLBRIGHT;

    // partial invisibility
    if (!edited_mobjinfo_bits[MT_INS])
      if (states[S_PINS].frame == FF_FULLBRIGHT)
        for (i = S_PINS; i <= S_PINS4; ++i)
          states[i].frame &= ~FF_FULLBRIGHT;

    // radiation suit
    if (!edited_mobjinfo_bits[MT_MISC14])
      if (states[S_SUIT].frame == FF_FULLBRIGHT)
        states[S_SUIT].frame &= ~FF_FULLBRIGHT;

    // computer map
    if (!edited_mobjinfo_bits[MT_MISC15])
      if (states[S_PMAP].frame == FF_FULLBRIGHT)
        for (i = S_PMAP; i <= S_PMAP6; ++i)
          states[i].frame &= ~FF_FULLBRIGHT;
  }

  deh_changeCompTranslucency();
}

static dboolean CheckSafeState(statenum_t state)
{
    int count = 0;

    for (statenum_t s = state; s != S_NULL; s = states[s].nextstate)
    {
        // [FG] recursive/nested states
        if (count++ >= 100)
        {
            return false;
        }

        // [crispy] a state with -1 tics never changes
        if (states[s].tics == -1)
        {
            break;
        }

        if (states[s].action)
        {
            // [FG] A_Light*() considered harmless
            if (states[s].action == A_Light0 ||
                states[s].action == A_Light1 ||
                states[s].action == A_Light2)
            {
                continue;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

void dsda_SSGFrameFix(void)
{
  // [FG] fix desyncs by SSG-flash correction
  if (CheckSafeState(S_DSGUNFLASH1) && states[S_DSGUNFLASH1].tics == 5)
  {
    states[S_DSGUNFLASH1].tics = 4;
  }
}
