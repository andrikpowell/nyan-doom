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
//	DSDA Obituaries
//

#include <string.h>

#include "doomtype.h"
#include "doomstat.h"
#include "deh/strings.h"
#include "m_misc.h"
#include "p_mobj.h"
#include "p_inter.h"
#include "heretic/dstrings.h"

#include "dsda/utility.h"
#include "dsda/messenger.h"
#include "dsda/text_color.h"
#include "hu_lib.h"

#include "hu_obituary.h"

// gender-neutral pronouns

struct
{
    const char *const from;
    const char *const to;
} static const pronouns[] = {
    { "%g", "they"    },
    { "%h", "them"    },
    { "%p", "their"   },
    { "%s", "theirs"  },
    { "%r", "they're" },
};

static const char *playerstr[] = {
    "Player 1",
    "Player 2",
    "Player 3",
    "Player 4",
};

static void dsda_AssignObituary(const int type, const char *ob, const char *ob_m)
{
  if (ob && !mobjinfo[type].obituary)
  {
    mobjinfo[type].obituary = ob;
  }
  if (ob_m && !mobjinfo[type].obituary_melee)
  {
    mobjinfo[type].obituary_melee = ob_m;
  }
}

static void dsda_SetObituaryDefaults(void)
{
  if (heretic)
  {
    dsda_AssignObituary(HERETIC_MT_IMP,              OB_HERETICIMP,  OB_HERETICIMPHIT);
    dsda_AssignObituary(HERETIC_MT_IMPLEADER,        OB_HERETICIMP,  OB_HERETICIMPHIT);
    dsda_AssignObituary(HERETIC_MT_MUMMY,            OB_MUMMY,       NULL);
    dsda_AssignObituary(HERETIC_MT_MUMMYGHOST,       OB_MUMMY,       NULL);
    dsda_AssignObituary(HERETIC_MT_MUMMYLEADER,      OB_MUMMYLEADER, NULL);
    dsda_AssignObituary(HERETIC_MT_MUMMYLEADERGHOST, OB_MUMMYLEADER, NULL);
    dsda_AssignObituary(HERETIC_MT_KNIGHT,           OB_BONEKNIGHT,  OB_BONEKNIGHTHIT);
    dsda_AssignObituary(HERETIC_MT_KNIGHTGHOST,      OB_BONEKNIGHT,  OB_BONEKNIGHTHIT);
    dsda_AssignObituary(HERETIC_MT_WIZARD,           OB_WIZARD,      OB_WIZARDHIT);
    dsda_AssignObituary(HERETIC_MT_BEAST,            OB_BEAST,       NULL);
    dsda_AssignObituary(HERETIC_MT_CLINK,            OB_CLINK,       NULL);
    dsda_AssignObituary(HERETIC_MT_SNAKE,            OB_SNAKE,       NULL);
    dsda_AssignObituary(HERETIC_MT_HEAD,             OB_IRONLICH,    OB_IRONLICHHIT);
    dsda_AssignObituary(HERETIC_MT_MINOTAUR,         OB_MINOTAUR,    OB_MINOTAURHIT);
    dsda_AssignObituary(HERETIC_MT_SORCERER1,        OB_DSPARIL1,    OB_DSPARIL1HIT);
    dsda_AssignObituary(HERETIC_MT_SORCERER2,        OB_DSPARIL2,    OB_DSPARIL2HIT);
    dsda_AssignObituary(HERETIC_MT_CHICKEN,          OB_CHICKEN,     NULL);
  }
  else if (hexen)
  {
    dsda_AssignObituary(HEXEN_MT_ETTIN,          OB_ETTIN,       NULL);
    dsda_AssignObituary(HEXEN_MT_FIREDEMON,      OB_FIREDEMON,   NULL);
    dsda_AssignObituary(HEXEN_MT_DEMON,          OB_DEMON1,      OB_DEMON1HIT);
    dsda_AssignObituary(HEXEN_MT_DEMON2,         OB_DEMON2,      OB_DEMON2HIT);
    dsda_AssignObituary(HEXEN_MT_CENTAUR,        OB_CENTAUR,     NULL);
    dsda_AssignObituary(HEXEN_MT_CENTAURLEADER,  OB_SLAUGHTAUR,  OB_SLAUGHTAURHIT);
    dsda_AssignObituary(HEXEN_MT_BISHOP,         OB_BISHOP,      NULL);
    dsda_AssignObituary(HEXEN_MT_ICEGUY,         OB_ICEGUY,      NULL);
    dsda_AssignObituary(HEXEN_MT_SERPENT,        OB_SERPENT,     OB_SERPENTHIT);
    dsda_AssignObituary(HEXEN_MT_SERPENTLEADER,  OB_SERPENT,     OB_SERPENTHIT);
    dsda_AssignObituary(HEXEN_MT_WRAITH,         OB_WRAITH,      OB_WRAITHHIT);
    dsda_AssignObituary(HEXEN_MT_DRAGON,         OB_DRAGON,      NULL);
    dsda_AssignObituary(HEXEN_MT_SORCBOSS,       OB_HERESIARCH,  NULL);
    dsda_AssignObituary(HEXEN_MT_FIGHTER_BOSS,   OB_FBOSS,       NULL);
    dsda_AssignObituary(HEXEN_MT_CLERIC_BOSS,    OB_CBOSS,       NULL);
    dsda_AssignObituary(HEXEN_MT_MAGE_BOSS,      OB_MBOSS,       NULL);
    dsda_AssignObituary(HEXEN_MT_KORAX,          OB_KORAX,       NULL);
  }
  else
  {
    dsda_AssignObituary(MT_POSSESSED, s_OB_ZOMBIE,   NULL);
    dsda_AssignObituary(MT_SHOTGUY,   s_OB_SHOTGUY,  NULL);
    dsda_AssignObituary(MT_VILE,      s_OB_VILE,     NULL);
    dsda_AssignObituary(MT_UNDEAD,    s_OB_UNDEAD,   s_OB_UNDEADHIT);
    dsda_AssignObituary(MT_FATSO,     s_OB_FATSO,    NULL);
    dsda_AssignObituary(MT_CHAINGUY,  s_OB_CHAINGUY, NULL);
    dsda_AssignObituary(MT_SKULL,     s_OB_SKULL,    NULL);
    dsda_AssignObituary(MT_TROOP,     s_OB_IMP,      s_OB_IMPHIT);
    dsda_AssignObituary(MT_HEAD,      s_OB_CACO,     s_OB_CACOHIT);
    dsda_AssignObituary(MT_SERGEANT,  NULL,          s_OB_DEMONHIT);
    dsda_AssignObituary(MT_SHADOWS,   NULL,          s_OB_SPECTREHIT);
    dsda_AssignObituary(MT_BRUISER,   s_OB_BARON,    s_OB_BARONHIT);
    dsda_AssignObituary(MT_KNIGHT,    s_OB_KNIGHT,   s_OB_KNIGHTHIT);
    dsda_AssignObituary(MT_SPIDER,    s_OB_SPIDER,   NULL);
    dsda_AssignObituary(MT_BABY,      s_OB_BABY,     NULL);
    dsda_AssignObituary(MT_CYBORG,    s_OB_CYBORG,   NULL);
    dsda_AssignObituary(MT_WOLFSS,    s_OB_WOLFSS,   NULL);
  }
}

void dsda_InitObituaries(void)
{
  dsda_SetObituaryDefaults();

  // [FG] TODO only the server knows the names of all clients,
  //           but at least we know ours...

  playerstr[consoleplayer] = dsda_StringConfig(dsda_config_player_name);
}

static const char *dsda_GetWeaponObituary(const char *ob, mobj_t *source, method_t mod)
{
  if (heretic)
  {
    switch (source->player->readyweapon)
    {
      case wp_staff:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPSTAFF : OB_MPSTAFF;
        break;
      case wp_goldwand:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPGOLDWAND : OB_MPGOLDWAND;
        break;
      case wp_crossbow:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPCROSSBOW : OB_MPCROSSBOW;
        break;
      case wp_blaster:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPBLASTER : OB_MPBLASTER;
        break;
      case wp_skullrod:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPSKULLROD : OB_MPSKULLROD;
        break;
      case wp_phoenixrod:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPPHOENIXROD : OB_MPPHOENIXROD;
        break;
      case wp_mace:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPMACE : OB_MPMACE;
        break;
      case wp_gauntlets:
        ob = source->player->powers[pw_weaponlevel2] ? OB_MPPGAUNTLETS : OB_MPGAUNTLETS;
        break;
      case wp_beak:
        ob = OB_CHICKEN;
        break;
      default:
        ob = s_OB_MPDEFAULT;
        break;
    }
  }
  else if (hexen)
  {
    switch (source->player->pclass)
    {
      case PCLASS_FIGHTER:
        switch (source->player->readyweapon)
        {
          case wp_first:
            ob = OB_MPFWEAPFIST;
            break;
          case wp_second:
            ob = OB_MPFWEAPAXE;
            break;
          case wp_third:
            ob = (mod == MOD_Melee) ? OB_MPFWEAPHAMMERM : OB_MPFWEAPHAMMERR;
            break;
          case wp_fourth:
            ob = OB_MPFWEAPQUIETUS;
            break;
          default:
            ob = s_OB_MPDEFAULT;
            break;
        }
      case PCLASS_CLERIC:
        switch (source->player->readyweapon)
        {
          case wp_first:
            ob = OB_MPCWEAPMACE;
            break;
          case wp_second:
            ob = (mod == MOD_Melee) ? OB_MPCWEAPSTAFFM : OB_MPCWEAPSTAFFR;
            break;
          case wp_third:
            ob = OB_MPCWEAPFLAME;
            break;
          case wp_fourth:
            ob = OB_MPCWEAPWRAITHVERGE;
            break;
          default:
            ob = s_OB_MPDEFAULT;
            break;
        }
      case PCLASS_MAGE:
        switch (source->player->readyweapon)
        {
          case wp_first:
            ob = OB_MPMWEAPWAND;
            break;
          case wp_second:
            ob = OB_MPMWEAPFROST;
            break;
          case wp_third:
            ob = OB_MPMWEAPLIGHTNING;
            break;
          case wp_fourth:
            ob = OB_MPMWEAPBLOODSCOURGE;
            break;
          default:
            ob = s_OB_MPDEFAULT;
            break;
        }
      default:
        ob = s_OB_MPDEFAULT;
        break;
    }
  }
  else // Doom
  {
    switch (source->player->readyweapon)
    {
      case wp_fist:
        ob = s_OB_MPFIST;
        break;
      case wp_pistol:
        ob = s_OB_MPPISTOL;
        break;
      case wp_shotgun:
        ob = s_OB_MPSHOTGUN;
        break;
      case wp_chaingun:
        ob = s_OB_MPCHAINGUN;
        break;
      case wp_missile:
        ob = s_OB_MPROCKET;
        break;
      case wp_plasma:
        ob = s_OB_MPPLASMARIFLE;
        break;
      case wp_bfg:
        ob = s_OB_MPBFG_BOOM;
        break;
      case wp_chainsaw:
        ob = s_OB_MPCHAINSAW;
        break;
      case wp_supershotgun:
        ob = s_OB_MPSSHOTGUN;
        break;
      default:
        ob = s_OB_MPDEFAULT;
        break;
    }
  }

  return ob;
}

static const char *dsda_GetObituary(mobj_t *target, mobj_t *inflictor, mobj_t *source, method_t mod)
{
  const char *ob = s_OB_DEFAULT;

  if (target->player->mo != target)
  {
    ob = s_OB_VOODOO;
  }
  else if (target == source)
  {
    ob = s_OB_KILLEDSELF;

    if (inflictor && inflictor != target && mobjinfo[inflictor->type].obituary_self)
    {
      ob = mobjinfo[inflictor->type].obituary_self;
    }
  }
  else if (source)
  {
    if (mod == MOD_Telefrag)
    {
      ob = (source->player) ? s_OB_MPTELEFRAG : s_OB_MONTELEFRAG;
    }
    else if (source->player)
    {
      ob = dsda_GetWeaponObituary(ob, source, mod);
    }
    else
    {
      const int type = source->type;

      if (type >= 0 && type < num_mobj_types)
      {
        if (mod == MOD_Melee && mobjinfo[type].obituary_melee)
        {
          ob = mobjinfo[type].obituary_melee;
        }
        else if (mobjinfo[type].obituary)
        {
          ob = mobjinfo[type].obituary;
        }
      }
    }
  }
  else
  {
    switch (mod)
    {
      case MOD_Slime:
        ob = s_OB_SLIME;
        break;
      case MOD_Lava:
        ob = s_OB_LAVA;
        break;
      case MOD_Crush:
        ob = s_OB_CRUSH;
        break;
      case MOD_Falling:
        ob = s_OB_FALLING;
      default:
        break;
    }
  }

  return ob;
}

static void dsda_ExpandObituary(dsda_string_t *out, const char *tmp, mobj_t *target, mobj_t *source)
{
  const char *from = (source && source->player) ? playerstr[source->player - players] : NULL;
  const char *to = playerstr[target->player - players];

  // NULL or ""
  if (!to || !*to) to = "Player";

  dsda_InitString(out, NULL);

  for (const char *p = tmp; *p; )
  {
    if (*p == '%' && p[1])
    {
      const char code = p[1];
      p += 2;

      // %k = killer
      if (code == 'k')
      {
        dsda_StringCat(out, from ? from : "");
        continue;
      }

      // %o = victim
      if (code == 'o')
      {
        dsda_StringCat(out, to ? to : "");
        continue;
      }

      // pronoun lookup (%g %h %p %s %r)
      {
        char key[3] = { '%', code, '\0' };
        dboolean matched = false;

        for (int i = 0; i < (int)(sizeof(pronouns) / sizeof(pronouns[0])); i++)
        {
          if (!strcmp(pronouns[i].from, key))
          {
            dsda_StringCat(out, pronouns[i].to);
            matched = true;
            break;
          }
        }

        if (matched)
          continue;
      }

      dsda_AppendChar(out, '%');
      dsda_AppendChar(out, code);
      continue;
    }

    dsda_AppendChar(out, *p++);
  }
}

void dsda_Obituary(mobj_t *target, struct mobj_s *inflictor, struct mobj_s *source, method_t mod)
{
  int i;

  const char *ob = dsda_GetObituary(target, inflictor, source, mod);

  for (i = 0; i < g_maxplayers; i++)
  {
    if (!playeringame[i])
      break;

    {
      dsda_string_t obituary_message;
      dsda_ExpandObituary(&obituary_message, ob, target, source);
      dsda_StringPrintF(&obituary_message, "%s%s", HU_ColorFromConfig(dsda_config_obituaries_color), obituary_message.string);
      dsda_AddPlayerObituary(obituary_message.string, &players[i]);
      dsda_FreeString(&obituary_message);
    }
  }
}
