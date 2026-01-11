/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2002 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Cheat sequence checking.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "am_map.h"
#include "g_game.h"
#include "r_data.h"
#include "p_inter.h"
#include "p_tick.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "s_advsound.h"
#include "s_random.h"
#include "sounds.h"
#include "dstrings.h"
#include "r_main.h"
#include "p_map.h"
#include "d_deh.h"  // Ty 03/27/98 - externalized strings
/* cph 2006/07/23 - needs direct access to thinkercap */
#include "p_tick.h"
#include "w_wad.h"
#include "p_setup.h"
#include "p_user.h"
#include "lprintf.h"

#include "heretic/def.h"
#include "heretic/sb_bar.h"

#include "dsda.h"
#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/excmd.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/input.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/messenger.h"
#include "dsda/settings.h"
#include "dsda/skill_info.h"

#define plyr (players+consoleplayer)     /* the console player */

// Allow cheats from dehacked files in-game
#define allow_deh_cheats (dsda_IntConfig(dsda_config_deh_change_cheats) && !dsda_Flag(dsda_arg_nocheats))
#define WHICH_CHEAT(x) (allow_deh_cheats ? (x)->deh_cheat : (x)->cheat)

//-----------------------------------------------------------------------------
//
// CHEAT SEQUENCE PACKAGE
//
//-----------------------------------------------------------------------------

static void cheat_mus(char buf[3]);
static void cheat_musrr();
static void cheat_camera();
static void cheat_choppers();
static void cheat_god_raven();
static void cheat_god();
static void cheat_fa();
static void cheat_k();
static void cheat_kfa_raven();
static void cheat_kfa();
static void cheat_noclip();
static void cheat_pw(int pw);
static void cheat_behold();
static void cheat_clev();
static void cheat_clevx(char buf[3]);
static void cheat_mypos();
static void cheat_rate();
static void cheat_comp();
static void cheat_compx(char buf[3]);
static void cheat_skill();
static void cheat_skillx(char buf[1]);
static void cheat_friction();
static void cheat_pushers();
static void cheat_massacre();
static void cheat_ddt();
static void cheat_reveal_secret();
static void cheat_reveal_kill();
static void cheat_reveal_item();
static void cheat_reveal_weapon();
static void cheat_reveal_exit();
static void cheat_reveal_key();
static void cheat_reveal_keyx();
static void cheat_reveal_doom_key(int key);
static void cheat_reveal_heretic_key(int key);
static void cheat_hom();
static void cheat_fast();
static void cheat_nuke();
static void cheat_key();
static void cheat_keyx();
static void cheat_keyxx(int key);
static void cheat_key_doom(int key);
static void cheat_key_heretic(int key);
static void cheat_weap();
static void cheat_weapx(char buf[3]);
static void cheat_ammo();
static void cheat_ammox(char buf[1]);
static void cheat_smart();
static void cheat_pitch();
static void cheat_megaarmour();
static void cheat_health();
static void cheat_notarget();
static void cheat_freeze();
static void cheat_killonsight();
static void cheat_fly();

// heretic
static void cheat_reset_health();
static void cheat_tome();
static void cheat_chicken();
static void cheat_artifact(char buf[3]);

// hexen
static void cheat_inventory();
static void cheat_puzzle();
static void cheat_classx(char buf[2]);
static void cheat_class();
static void cheat_init();
static void cheat_script(char buf[3]);
static void cheat_hexen_suicide();

// nyan
static void cheat_nut();
static void cheat_suicide();
static void cheat_strip();

//-----------------------------------------------------------------------------
//
// List of cheat codes, functions, and special argument indicators.
//
// The first argument is the cheat code.
//
// The second argument is the DEH cheat code, which is used for first copying
// the vanilla cheat codes, but then replaced DEH cheats if found.
//
// The third argument is its DEH name, or NULL if it's not supported by -deh.
//
// The fourth argument is a combination of the bitmasks,
// which excludes the cheat during certain modes of play.
//
// The fifth argument is the handler function.
//
// The sixth argument is passed to the handler function if it's non-negative;
// if negative, then its negative indicates the number of extra characters
// expected after the cheat code, which are passed to the handler function
// via a pointer to a buffer (after folding any letters to lowercase).
//
//-----------------------------------------------------------------------------

cheatseq_t cheat[] = {
  CHEAT("idmus",      NULL,   "Change music",     cht_always, cht_any, cheat_mus, -2, false),
  CHEAT("idchoppers", NULL,   "Chainsaw",         not_demo, cht_doom, cheat_choppers, 0, false),
  CHEAT("iddqd",      NULL,   "God mode",         not_classic_demo, cht_doom, cheat_god, 0, false),
  CHEAT("idkfa",      NULL,   "Ammo & Keys",      not_demo, cht_doom, cheat_kfa, 0, false),
  CHEAT("idfa",       NULL,   "Ammo",             not_demo, cht_doom, cheat_fa, 0, false),
  CHEAT("idspispopd", NULL,   "No Clipping 1",    not_classic_demo, cht_any, cheat_noclip, 0, false),
  CHEAT("idclip",     NULL,   "No Clipping 2",    not_classic_demo, cht_any, cheat_noclip, 0, false),
  CHEAT("idbehold",   NULL,   "BEHOLD menu",      cht_always, cht_doom, cheat_behold, 0, false),
  CHEAT("idbeholdh",  NULL,   "Invincibility",    not_demo, cht_doom, cheat_health, 0, false),
  CHEAT("idbeholdm",  NULL,   "Invincibility",    not_demo, cht_doom, cheat_megaarmour, 0, false),
  CHEAT("idbeholdv",  NULL,   "Invincibility",    not_demo, cht_doom, cheat_pw, pw_invulnerability, false),
  CHEAT("idbeholds",  NULL,   "Berserk",          not_demo, cht_doom, cheat_pw, pw_strength, false),
  CHEAT("idbeholdi",  NULL,   "Invisibility",     not_demo, cht_doom, cheat_pw, pw_invisibility, false),
  CHEAT("idbeholdr",  NULL,   "Radiation Suit",   not_demo, cht_doom, cheat_pw, pw_ironfeet, false),
  CHEAT("idbeholda",  NULL,   "Auto-map",         cht_always, cht_doom, cheat_pw, pw_allmap, false),
  CHEAT("idbeholdl",  NULL,   "Lite-Amp Goggles", cht_always, cht_doom, cheat_pw, pw_infrared, false),
  CHEAT("idclev",     NULL,   "Level Warp",       not_demo | not_menu, cht_any, cheat_clevx, -2, false),
  CHEAT("idclev",     NULL,   "Level Warp",       not_demo | not_menu, cht_any, cheat_clev, 0, false),
  CHEAT("idmypos",    NULL,   NULL,               cht_always, cht_any, cheat_mypos, 0, false),
  CHEAT("idrate",     NULL,   "Frame rate",       cht_always, cht_any, cheat_rate, 0, false),

  // phares
  CHEAT("comp",       NULL,   NULL,               not_demo, cht_doom, cheat_compx, -2, false),
  CHEAT("comp",       NULL,   NULL,               not_demo, cht_doom, cheat_comp, 0, false),
  CHEAT("skill",      NULL,   NULL,               not_demo, cht_any, cheat_skillx, -1, false),
  CHEAT("skill",      NULL,   NULL,               not_demo, cht_any, cheat_skill, 0, false),
  CHEAT("tntem",      NULL,   NULL,               not_demo, cht_any, cheat_massacre, 0, false),  // jff 2/01/98 kill all monsters
  CHEAT("killem",     NULL,   NULL,               not_demo, cht_any, cheat_massacre, 0, false),  // jff 2/01/98 kill all monsters

  // killough 2/07/98: moved from am_map.c
  CHEAT("iddt",       NULL,   "Map cheat",        cht_always, cht_any, cheat_ddt, 0, true),
  CHEAT("iddst",      NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_secret, 0, true),
  CHEAT("iddkt",      NULL,   NULL,               cht_always, cht_any, cheat_reveal_kill, 0, true),
  CHEAT("iddit",      NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_item, 0, true),
  CHEAT("iddet",      NULL,   NULL,               cht_always, cht_any, cheat_reveal_exit, 0, true),

  // find weapon cheats
  CHEAT("iddwt",      NULL,   NULL,               cht_always, cht_any, cheat_reveal_weapon, 0, false),
  CHEAT("iddwt2",     NULL,   NULL,               cht_always, cht_hexen, cheat_reveal_weaponx, wp_pistol+1, true),
  CHEAT("iddwt3",     NULL,   NULL,               cht_always, cht_any, cheat_reveal_weaponx, wp_shotgun+1, true),
  CHEAT("iddwt4",     NULL,   NULL,               cht_always, cht_any, cheat_reveal_weaponx, wp_chaingun+1, true),
  CHEAT("iddwt5",     NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_weaponx, wp_missile+1, true),
  CHEAT("iddwt6",     NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_weaponx, wp_plasma+1, true),
  CHEAT("iddwt7",     NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_weaponx, wp_bfg+1, true),
  CHEAT("iddwt8",     NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_weaponx, wp_chainsaw+1, true),
  CHEAT("iddwt9",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_weaponx, wp_supershotgun+1, true),

  // key cheats
  CHEAT("iddfrc",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_RKEY, true),
  CHEAT("iddfyc",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_YKEY, true),
  CHEAT("iddfbc",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_BKEY, true),
  CHEAT("iddfrs",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_RSKU, true),
  CHEAT("iddfys",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_YSKU, true),
  CHEAT("iddfbs",     NULL,   NULL,               cht_always, cht_doom, cheat_reveal_doom_key, SPR_BSKU, true),
  CHEAT("iddfg",      NULL,   NULL,               cht_always, cht_heretic, cheat_reveal_heretic_key, HERETIC_SPR_AKYY, true), // heretic
  CHEAT("iddfy",      NULL,   NULL,               cht_always, cht_heretic, cheat_reveal_heretic_key, HERETIC_SPR_CKYY, true), // heretic
  CHEAT("iddfb",      NULL,   NULL,               cht_always, cht_heretic, cheat_reveal_heretic_key, HERETIC_SPR_BKYY, true), // heretic
  CHEAT("iddf",       NULL,   NULL,               cht_always, cht_doom | cht_heretic, cheat_reveal_key, 0, false),
  CHEAT("iddfr",      NULL,   NULL,               cht_always, cht_doom, cheat_reveal_keyx, 0, false),
  CHEAT("iddfy",      NULL,   NULL,               cht_always, cht_doom, cheat_reveal_keyx, 0, false),
  CHEAT("iddfb",      NULL,   NULL,               cht_always, cht_doom, cheat_reveal_keyx, 0, false),

  // killough 2/16/98: generalized key cheats
  CHEAT("keyrc",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_redcard, false),
  CHEAT("keyyc",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_yellowcard, false),
  CHEAT("keybc",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_bluecard, false),
  CHEAT("keyrs",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_redskull, false),
  CHEAT("keyys",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_yellowskull, false),
  CHEAT("keybs",      NULL,   NULL,               not_demo, cht_doom, cheat_key_doom, it_blueskull, false),
  CHEAT("keyg",       NULL,   NULL,               not_demo, cht_heretic, cheat_key_heretic, key_green, false),  // heretic
  CHEAT("keyy",       NULL,   NULL,               not_demo, cht_heretic, cheat_key_heretic, key_yellow, false), // heretic
  CHEAT("keyb",       NULL,   NULL,               not_demo, cht_heretic, cheat_key_heretic, key_blue, false),   // heretic
  CHEAT("key",        NULL,   NULL,               not_demo, cht_doom | cht_heretic, cheat_key, 0, false),
  CHEAT("keyr",       NULL,   NULL,               not_demo, cht_doom, cheat_keyx, 0, false),
  CHEAT("keyy",       NULL,   NULL,               not_demo, cht_doom, cheat_keyx, 0, false),
  CHEAT("keyb",       NULL,   NULL,               not_demo, cht_doom, cheat_keyx, 0, false),

  // killough 2/16/98: generalized weapon cheats
  CHEAT("weap",       NULL,   NULL,               not_demo, cht_any, cheat_weapx, -1, false),
  CHEAT("weap",       NULL,   NULL,               not_demo, cht_any, cheat_weap, 0, false),
  CHEAT("ammo",       NULL,   NULL,               not_demo, cht_any, cheat_ammox, -1, false),
  CHEAT("ammo",       NULL,   NULL,               not_demo, cht_any, cheat_ammo, 0, false),
  // removed "TNTAMO" due to "AMMO" being short

  // Obscure TNT cheats
  CHEAT("tntka",      NULL,   NULL,               not_demo,   cht_any, cheat_k, 0, false),         // Ty 04/11/98 - Added TNTKA
  CHEAT("hom",        NULL,   NULL,               cht_always, cht_any, cheat_hom, 0, false),     // killough 2/07/98: HOM autodetector
  CHEAT("smart",      NULL,   NULL,               not_demo,   cht_any, cheat_smart, 0, false),     // killough 2/21/98: smart monster toggle
  CHEAT("pitch",      NULL,   NULL,               cht_always, cht_any, cheat_pitch, 0, false),   // killough 2/21/98: pitched sound toggle
  CHEAT("fast",       NULL,   NULL,               not_demo,   cht_any, cheat_fast, 0, false),      // killough 3/6/98: -fast toggle
  CHEAT("nuke",       NULL,   NULL,               not_demo,   cht_any, cheat_nuke, 0, false),      // killough 12/98: disable nukage damage
  CHEAT("ice",        NULL,   NULL,               not_demo,   cht_any, cheat_friction, 0, false),  // phares 3/10/98: toggle variable friction effects
  CHEAT("push",       NULL,   NULL,               not_demo,   cht_any, cheat_pushers, 0, false),   // phares 3/10/98: toggle pushers

  // dsda cheats
  CHEAT("notarget",   NULL,   NULL,               not_demo, cht_any, cheat_notarget, 0, false),  // [RH] Monsters don't target
  CHEAT("fly",        NULL,   NULL,               not_demo, cht_any, cheat_fly, 0, false),       // fly mode is active

  // heretic
  CHEAT("quicken", NULL, NULL, not_classic_demo, cht_heretic, cheat_god, 0, false),
  CHEAT("ponce", NULL, NULL, not_demo, cht_heretic, cheat_reset_health, 0, false),
  CHEAT("kitty", NULL, NULL, not_classic_demo, cht_heretic, cheat_noclip, 0, false),
  CHEAT("massacre", NULL, NULL, not_demo, cht_heretic, cheat_massacre, 0, false),
  CHEAT("rambo", NULL, NULL, not_demo, cht_heretic, cheat_fa, 0, false),
  CHEAT("skel", NULL, NULL, not_demo, cht_heretic, cheat_k, 0, false),
  CHEAT("gimme", NULL, NULL, not_demo, cht_heretic, cheat_artifact, -2, false),
  CHEAT("shazam", NULL, NULL, not_demo, cht_heretic, cheat_tome, 0, false),
  CHEAT("engage", NULL, NULL, not_demo | not_menu, cht_heretic, cheat_clevx, -2, false),
  CHEAT("engage", NULL, NULL, not_demo | not_menu, cht_heretic, cheat_clev, 0, false),
  CHEAT("ravmap", NULL, NULL, cht_always, cht_heretic, cheat_ddt, 0, true),
  CHEAT("cockadoodledoo", NULL, NULL, not_demo, cht_heretic, cheat_chicken, 0, false),

  // raven joke cheats
  CHEAT("iddqd", NULL, NULL, not_demo, cht_heretic | cht_hexen, cheat_god_raven, 0, false),
  CHEAT("idkfa", NULL, NULL, not_demo, cht_heretic | cht_hexen, cheat_kfa_raven, 0, false),
  CHEAT("conan", NULL, NULL, not_demo, cht_hexen, cheat_kfa_raven, 0, false),
  CHEAT("martek", NULL, NULL, not_demo, cht_hexen, cheat_hexen_suicide, 0, false),
  CHEAT("quicken", NULL, NULL, not_demo, cht_hexen, cheat_hexen_suicide, 0, false), // from Hexen beta

  // hexen
  CHEAT("satan", NULL, NULL, not_classic_demo, cht_hexen, cheat_god, 0, false),
  CHEAT("clubmed", NULL, NULL, not_demo, cht_hexen, cheat_reset_health, 0, false),
  CHEAT("butcher", NULL, NULL, not_demo, cht_hexen, cheat_massacre, 0, false),
  CHEAT("nra", NULL, NULL, not_demo, cht_hexen, cheat_fa, 0, false),
  CHEAT("indiana", NULL, NULL, not_demo, cht_hexen, cheat_inventory, 0, false),
  CHEAT("locksmith", NULL, NULL, not_demo, cht_hexen, cheat_k, 0, false),
  CHEAT("sherlock", NULL, NULL, not_demo, cht_hexen, cheat_puzzle, 0, false),
  CHEAT("casper", NULL, NULL, not_classic_demo, cht_hexen, cheat_noclip, 0, false),
  CHEAT("shadowcaster", NULL, NULL, not_demo, cht_hexen, cheat_classx, -1, false),
  CHEAT("shadowcaster", NULL, NULL, not_demo, cht_hexen, cheat_class, 0, false),
  CHEAT("visit", NULL, NULL, not_demo | not_menu, cht_hexen, cheat_clevx, -2, false),
  CHEAT("visit", NULL, NULL, not_demo | not_menu, cht_hexen, cheat_clev, 0, false),
  CHEAT("init", NULL, NULL, not_demo, cht_hexen, cheat_init, 0, false),
  CHEAT("puke", NULL,  NULL, not_demo, cht_hexen, cheat_script, -2, false),
  CHEAT("mapsco", NULL, NULL, cht_always, cht_hexen, cheat_ddt, 0, true),
  CHEAT("deliverance", NULL, NULL, not_demo, cht_hexen, cheat_chicken, 0, false),

  // H+H aliases
  CHEAT("healthyself", NULL, NULL, not_demo, cht_heretic | cht_hexen, cheat_reset_health, 0, false),
  CHEAT("gunsgunsguns", NULL, NULL, not_demo, cht_heretic | cht_hexen, cheat_fa, 0, false),
  CHEAT("ghost", NULL, NULL, not_classic_demo, cht_hexen, cheat_noclip, 0, false),
  CHEAT("squeal", NULL, NULL, not_demo, cht_hexen, cheat_chicken, 0, false),
  CHEAT("whipit", NULL, NULL, not_demo, cht_hexen, cheat_inventory, 0, false),

  // nyan
  CHEAT("idnut", NULL, NULL, cht_always, cht_doom, cheat_nut, 0, false),
  CHEAT("camera", NULL, NULL, not_demo, cht_any, cheat_camera, 0, false),
  CHEAT("strip", NULL, NULL, not_demo, cht_any, cheat_strip, 0, false),
  CHEAT("killme", NULL, NULL, not_demo, cht_any, cheat_suicide, 0, false),
  CHEAT("basilisk", NULL, NULL, not_demo, cht_any, cheat_killonsight, 0, false),

  // end-of-list marker
  {NULL}
};

//-----------------------------------------------------------------------------

static void cheat_mus(char buf[3])
{
  int epsd, map;

  // Check for random
  if (buf[0] == 'r' && buf[1] == 'r')
  {
    cheat_musrr();
    return;
  }

  //jff 3/20/98 note: this cheat allowed in netgame/demorecord

  //jff 3/17/98 avoid musnum being negative and crashing
  if (!isdigit(buf[0]) || !isdigit(buf[1]))
    return;

  if (gamemode == commercial)
  {
    epsd = 1; //jff was 0, but espd is 1-based
    map = (buf[0] - '0') * 10 + buf[1] - '0';
  }
  else
  {
    epsd = buf[0] - '0';
    map = buf[1] - '0';
  }

  dsda_ChangeMusic(epsd, map, false, true);
}

static void cheat_musrr(void)
{
  S_PlayRandomMusic(true);
}

// 'choppers' invulnerability & chainsaw
static void cheat_choppers()
{
  plyr->weaponowned[wp_chainsaw] = true;
  plyr->powers[pw_invulnerability] = true;
  dsda_AddMessage(s_STSTR_CHOPPERS);
}

void M_CheatGod(void)
{
  // dead players are first respawned at the current position
  if (plyr->playerstate == PST_DEAD)
  {
    signed int an;
    mapthing_t mt = {0};

    P_MapStart();
    mt.x = plyr->mo->x;
    mt.y = plyr->mo->y;
    mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
    mt.type = consoleplayer + 1;
    mt.options = 1; // arbitrary non-zero value
    P_SpawnPlayer(consoleplayer, &mt);

    // reset view to center (heretic / hexen)
    plyr->lookdir = 0;

    // spawn a teleport fog
    an = plyr->mo->angle >> ANGLETOFINESHIFT;
    P_SpawnMobj(plyr->mo->x + 20*finecosine[an],
                plyr->mo->y + 20*finesine[an],
                plyr->mo->z + g_telefog_height,
                g_mt_tfog);
    S_StartMobjSound(plyr->mo, g_sfx_revive);
    P_MapEnd();
  }

  plyr->cheats ^= CF_GODMODE;
  if (plyr->cheats & CF_GODMODE)
  {
    if (plyr->mo)
      plyr->mo->health = god_health;  // Ty 03/09/98 - deh

    plyr->health = god_health;
    dsda_AddMessage(s_STSTR_DQDON);
  }
  else
    dsda_AddMessage(s_STSTR_DQDOFF);
}

int hexen_suicide_seq = 0;

static void cheat_hexen_suicide(void)
{
  hexen_suicide_seq++;
  switch (hexen_suicide_seq)
  {
    case 1:
      P_SetMessage(plyr, HEXEN_TXT_CHEATQUICKEN1, true);
      break;
    case 2:
      P_SetMessage(plyr, HEXEN_TXT_CHEATQUICKEN2, true);
      break;
    case 3:
      P_DamageMobj(plyr->mo, NULL, plyr->mo, 10000);
      P_SetMessage(plyr, HEXEN_TXT_CHEATQUICKEN3, true);
      hexen_suicide_seq = 0;
      break;
    default:
      break;
  }
}

static void cheat_god_raven(void)
{
  P_DamageMobj(plyr->mo, NULL, plyr->mo, 10000);
  dsda_AddMessage(HERETIC_TXT_CHEATIDDQD);
}

static void cheat_suicide()
{
  P_DamageMobj(plyr->mo, NULL, plyr->mo, 10000);
  dsda_AddMessage("Fuck my life");
}

static void cheat_god()
{                                    // 'dqd' cheat for toggleable god mode
  if (demorecording)
  {
    dsda_QueueExCmdGod();
    return;
  }

  M_CheatGod();
}

void cheat_killonsight(void)
{
  if (!allow_incompatibility) return;

  plyr->cheats ^= CF_BASILISK;
  if (plyr->cheats & CF_BASILISK)
  {
    dsda_AddMessage("Strike fear into your enemies!");
  }
  else
    dsda_AddMessage("Enemies no longer fear you.");
}

void cheat_nut(void)
{

  if (raven) return;

  plyr->cheats ^= CF_NUT;
  if (plyr->cheats & CF_NUT)
  {
    dsda_AddMessage("Feeling Nutty?");
  }
  else
    dsda_AddMessage("Go live your nut-less life.");
}

// CPhipps - new health and armour cheat codes
static void cheat_health()
{
  if (!(plyr->cheats & CF_GODMODE)) {
    if (plyr->mo)
      plyr->mo->health = mega_health;
    plyr->health = mega_health;
    dsda_AddMessage(s_STSTR_BEHOLDX);
  }
}

static void cheat_megaarmour()
{
  plyr->armorpoints[ARMOR_ARMOR] = idfa_armor;      // Ty 03/09/98 - deh
  plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh
  dsda_AddMessage(s_STSTR_BEHOLDX);
}

static void cheat_fa()
{
  int i;

  if (hexen)
  {
    for (i = 0; i < NUMARMOR; i++)
    {
        plyr->armorpoints[i] = pclass[plyr->pclass].armor_increment[i];
    }
    for (i = 0; i < HEXEN_NUMWEAPONS; i++)
    {
        plyr->weaponowned[i] = true;
    }
    for (i = 0; i < NUMMANA; i++)
    {
        plyr->ammo[i] = MAX_MANA;
    }
  }
  else
  {
    if (!plyr->backpack)
    {
      for (i=0 ; i<NUMAMMO ; i++)
        plyr->maxammo[i] *= 2;
      plyr->backpack = true;
    }

    plyr->armorpoints[ARMOR_ARMOR] = idfa_armor;      // Ty 03/09/98 - deh
    plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh

    // You can't own weapons that aren't in the game // phares 02/27/98
    for (i=0;i<NUMWEAPONS;i++)
      if (!(((i == wp_plasma || i == wp_bfg) && gamemode == shareware) ||
            (i == wp_supershotgun && gamemode != commercial)))
        plyr->weaponowned[i] = true;

    for (i=0;i<NUMAMMO;i++)
      if (i!=am_cell || gamemode!=shareware)
        plyr->ammo[i] = plyr->maxammo[i];

    dsda_AddMessage(s_STSTR_FAADDED);
  }
}

static void cheat_k()
{
  int i;
  for (i=0;i<NUMCARDS;i++)
    if (!plyr->cards[i])     // only print message if at least one key added
      {                      // however, caller may overwrite message anyway
        plyr->cards[i] = true;
        dsda_AddMessage("Keys Added");
      }
}

static void cheat_dostrip(int idkfa_raven)
{
  int i, startweapon, endweapon;

  if (hexen)
  {
    for (i = wp_second; i < HEXEN_NUMWEAPONS; i++)
      plyr->weaponowned[i] = false;
  }

  if (heretic && plyr->chickenTics) return;

  // Set weapons per game
  startweapon = heretic ? wp_goldwand : wp_pistol;
  endweapon = heretic ? wp_gauntlets : wp_supershotgun;

  // Take weapons away
  for(i = startweapon; i <= endweapon; i++)
  {
    if ((i==wp_bfg || i==wp_plasma) && gamemode==shareware)
      break;

    plyr->weaponowned[i] = false;
  }

  // Force weapon switch
  plyr->pendingweapon = wp_staff;
  dsda_AddMessage(idkfa_raven ? HERETIC_TXT_CHEATIDKFA : "Stripped down to the bone");
  return;
}

static void cheat_kfa_raven()
{
  // take away weapons... skip keys
  cheat_dostrip(raven);
}

static void cheat_strip()
{
  // take away weapons... skip keys
  cheat_dostrip(false);
}

static void cheat_kfa()
{
  cheat_k();
  cheat_fa();
  dsda_AddMessage(s_STSTR_KFAADDED);
}

void M_CheatNoClip(void)
{
  dsda_AddMessage((plyr->cheats ^= CF_NOCLIP) & CF_NOCLIP ? s_STSTR_NCON : s_STSTR_NCOFF);
}

static void cheat_noclip()
{
  if (demorecording)
  {
    dsda_QueueExCmdNoClip();
    return;
  }

  M_CheatNoClip();
}

// 'behold?' power-up cheats (modified for infinite duration -- killough)
static void cheat_pw(int pw)
{
  if (pw == pw_allmap)
    dsda_TrackFeature(uf_automap);

  if (pw == pw_infrared)
    dsda_TrackFeature(uf_liteamp);

  if (plyr->powers[pw])
    plyr->powers[pw] = pw!=pw_strength && pw!=pw_allmap;  // killough
  else
    {
      P_GivePower(plyr, pw);
      if (pw != pw_strength)
        plyr->powers[pw] = -1;      // infinite duration -- killough
    }
  dsda_AddMessage(s_STSTR_BEHOLDX);
}

// 'behold' power-up menu
static void cheat_behold()
{
  dsda_AddMessage(s_STSTR_BEHOLD);
}

// check 'clev' change-level cheat
static void cheat_clev()
{
  int epsd, map;
  char *cur, *next;
  cur = Z_Strdup(VANILLA_MAP_LUMP_NAME(gameepisode, gamemap));

  dsda_NextMap(&epsd, &map);
  next = VANILLA_MAP_LUMP_NAME(epsd, map);

  if (W_LumpNameExists(next))
    doom_printf("Current: %s, next: %s", cur, next);
  else
    doom_printf("Current: %s", cur);

  Z_Free(cur);
}

// 'clev' change-level cheat
static void cheat_clevx(char buf[3])
{
  int epsd, map;

  if (gamemode == commercial)
  {
    epsd = 1; //jff was 0, but espd is 1-based
    map = (buf[0] - '0') * 10 + buf[1] - '0';
  }
  else
  {
    epsd = buf[0] - '0';
    map = buf[1] - '0';
  }

  if (dsda_ResolveCLEV(&epsd, &map))
  {
    dsda_AddMessage(s_STSTR_CLEV);

    G_DeferedInitNew(gameskill, epsd, map);
  }
}

// 'mypos' for player position
static void cheat_mypos()
{
  dsda_ToggleConfig(dsda_config_coordinate_display, false);
}

// cph - cheat to toggle frame rate/rendering stats display
static void cheat_rate()
{
  dsda_ToggleRenderStats();
}

// check compatibility cheat
static void cheat_comp()
{
  if (raven)
    return doom_printf("Cheat disabled for %s", heretic ? "Heretic" : "Hexen");

  if (doom_v11)
    return doom_printf("Cheat disabled");

  doom_printf("Complevel: %i - %s", compatibility_level, comp_lev_str[compatibility_level]);
}

// compatibility cheat
static void cheat_compx(char buf[3])
{
  if (raven)
    return doom_printf("Cheat disabled for %s", heretic ? "Heretic" : "Hexen");

  if (doom_v11)
    return doom_printf("Cheat disabled");

  {
    int compinput = (buf[0] - '0') * 10 + buf[1] - '0';

    if (compinput < 0 ||
        compinput >= MAX_COMPATIBILITY_LEVEL ||
        (compinput > 17 && compinput < 21))
    {
      return; //doom_printf("Invalid complevel");
    }
    else
    {
      compatibility_level = compinput;
      G_Compatibility(); // this is missing options checking
      doom_printf("New Complevel: %i - %s", compatibility_level, comp_lev_str[compatibility_level]);
    }
  }
}

// Get skill strings
static const char* dsda_skill_str(void)
{
  if (hexen)
  {
    // Hexen Skill Strings
    extern const char * hexen_skills[4][5];

    return hexen_skills[PlayerClass[consoleplayer]][gameskill];
  }

  return skill_infos[gameskill].name;
}

// Check skill cheat
static void cheat_skill()
{
  if (!tc_game)
    doom_printf("Skill: %i - %s", gameskill + 1, dsda_skill_str());
  else
    doom_printf("Skill: %i", gameskill + 1);
}

// Skill cheat
static void cheat_skillx(char buf[1])
{
  int skill = buf[0] - '0';

  if (skill >= 1 && skill <= num_og_skills)
  {
    gameskill = skill - 1;

    if (!tc_game)
      doom_printf("Next Level Skill: %i - %s", gameskill + 1, dsda_skill_str());
    else
      doom_printf("Next Level Skill: %i", gameskill + 1);

    dsda_UpdateGameSkill(gameskill);
  }
}

// variable friction cheat
static void cheat_friction()
{
  dsda_AddMessage((variable_friction = !variable_friction) ? "Variable Friction enabled" :
                                                             "Variable Friction disabled");
}


// Pusher cheat
// phares 3/10/98
static void cheat_pushers()
{
  dsda_AddMessage((allow_pushers = !allow_pushers) ? "Pushers enabled" : "Pushers disabled");
}

static void cheat_massacre()    // jff 2/01/98 kill all monsters
{
  // jff 02/01/98 'em' cheat - kill all monsters
  // partially taken from Chi's .46 port
  //
  // killough 2/7/98: cleaned up code and changed to use dprintf;
  // fixed lost soul bug (LSs left behind when PEs are killed)

  int killcount=0;
  thinker_t *currentthinker = NULL;
  extern void A_PainDie(mobj_t *);

  // killough 7/20/98: kill friendly monsters only if no others to kill
  uint64_t mask = MF_FRIEND;
  P_MapStart();
  do
    while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL)
    if (currentthinker->function == P_MobjThinker &&
  !(((mobj_t *) currentthinker)->flags & mask) && // killough 7/20/98
        (((mobj_t *) currentthinker)->flags & MF_COUNTKILL ||
         ((mobj_t *) currentthinker)->type == MT_SKULL))
      { // killough 3/6/98: kill even if PE is dead
        if (((mobj_t *) currentthinker)->health > 0)
          {
            killcount++;
            P_DamageMobj((mobj_t *)currentthinker, NULL, NULL, 10000);
          }
        if (((mobj_t *) currentthinker)->type == MT_PAIN)
          {
            A_PainDie((mobj_t *) currentthinker);    // killough 2/8/98
            P_SetMobjState ((mobj_t *) currentthinker, S_PAIN_DIE6);
          }
      }
  while (!killcount && mask ? mask=0, 1 : 0); // killough 7/20/98
  P_MapEnd();
  // killough 3/22/98: make more intelligent about plural
  // Ty 03/27/98 - string(s) *not* externalized
  doom_printf("%d Monster%s Killed", killcount, killcount==1 ? "" : "s");
}

void M_CheatIDDT(void)
{
  extern int dsda_reveal_map;

  dsda_TrackFeature(uf_iddt);

  dsda_reveal_map = (dsda_reveal_map+1) % 3;
}

// killough 2/7/98: move iddt cheat from am_map.c to here
// killough 3/26/98: emulate Doom better
static void cheat_ddt()
{
  if (automap_input)
    M_CheatIDDT();
}

static void cheat_reveal_secret()
{
  static int last_secret = -1;
  dboolean found = false;

  if (automap_input)
  {
    int i, start_i;

    dsda_TrackFeature(uf_iddt);

    i = last_secret + 1;
    if (i >= numsectors)
      i = 0;
    start_i = i;

    do
    {
      sector_t *sec = &sectors[i];

      if (P_IsSecret(sec))
      {
        dsda_UpdateIntConfig(dsda_config_automap_follow, false, true);

        // This is probably not necessary
        if (sec->lines && sec->lines[0] && sec->lines[0]->v1)
        {
          found = true;
          AM_SetMapCenter(sec->lines[0]->v1->x, sec->lines[0]->v1->y);
          last_secret = i;
          break;
        }
      }

      i++;
      if (i >= numsectors)
        i = 0;
    } while (i != start_i);

    if (!found)
      dsda_AddMessage("No Secrets found");
  }
}

static void cheat_cycle_mobj(mobj_t **last_mobj, int *last_count, int flags, int alive, const char* notfound_msg)
{
  extern int init_thinkers_count;
  thinker_t *th, *start_th;
  dboolean found = false;

  // If the thinkers have been wiped, addresses are invalid
  if (*last_count != init_thinkers_count)
  {
    *last_count = init_thinkers_count;
    *last_mobj = NULL;
  }

  if (*last_mobj)
    th = &(*last_mobj)->thinker;
  else
    th = &thinkercap;

  start_th = th;

  do
  {
    th = th->next;
    if (th->function == P_MobjThinker)
    {
      mobj_t *mobj;

      mobj = (mobj_t *) th;

      if ((!alive || mobj->health > 0) && mobj->flags & flags)
      {
        found = true;
        dsda_UpdateIntConfig(dsda_config_automap_follow, false, true);
        AM_SetMapCenter(mobj->x, mobj->y);
        P_SetTarget(last_mobj, mobj);
        break;
      }
    }
  } while (th != start_th);

  if (!found)
    dsda_AddMessage(notfound_msg);
}

dboolean cheat_get_hexen_piece(int num)
{
  int weapon_piece_1 = 0, weapon_piece_2 = 0, weapon_piece_3 = 0;
  dboolean fighter = PlayerClass[consoleplayer] == PCLASS_FIGHTER;
  dboolean cleric = PlayerClass[consoleplayer] == PCLASS_CLERIC;
  dboolean mage = PlayerClass[consoleplayer] == PCLASS_MAGE;

  weapon_piece_1 = fighter ? HEXEN_SPR_WFR1 : cleric ? HEXEN_SPR_WCH1 : mage ? HEXEN_SPR_WMS1 : false;
  weapon_piece_2 = fighter ? HEXEN_SPR_WFR2 : cleric ? HEXEN_SPR_WCH2 : mage ? HEXEN_SPR_WMS2 : false;
  weapon_piece_3 = fighter ? HEXEN_SPR_WFR3 : cleric ? HEXEN_SPR_WCH3 : mage ? HEXEN_SPR_WMS3 : false;

  return ((num == weapon_piece_1) || (num == weapon_piece_2) || (num == weapon_piece_3));
}

static void cheat_cycle_mobj_spr(mobj_t **last_mobj, int *last_count, int sprite_num, int weapon_num, int flags, int rflags, const char* notfound_msg)
{
  extern int init_thinkers_count;
  thinker_t *th, *start_th;
  dboolean found_num;
  dboolean found = false;

  // If the thinkers have been wiped, addresses are invalid
  if (*last_count != init_thinkers_count)
  {
    *last_count = init_thinkers_count;
    *last_mobj = NULL;
  }

  if (*last_mobj)
    th = &(*last_mobj)->thinker;
  else
    th = &thinkercap;

  start_th = th;

  do
  {
    th = th->next;
    if (th->function == P_MobjThinker)
    {
      mobj_t *mobj;

      mobj = (mobj_t *) th;

      found_num = (mobj->sprite == sprite_num);

      // Hexen 4th Weapon Logic
      if (hexen && sprite_num == 666) // dummy value
      {
        found_num = cheat_get_hexen_piece(mobj->sprite);
        notfound_msg = "Weapon 4 parts not found";
      }

      if (found_num && (flags ? mobj->flags & flags : true) && (rflags ? !(mobj->flags & rflags) : true))
      {
        found = true;
        dsda_UpdateIntConfig(dsda_config_automap_follow, false, true);
        AM_SetMapCenter(mobj->x, mobj->y);
        P_SetTarget(last_mobj, mobj);
        break;
      }
    }
  } while (th != start_th);

  if (found)
  {
    if (weapon_num != -1)
    {
      char found_msg[64];

      if (hexen && sprite_num == 666)
        snprintf(found_msg, sizeof found_msg, "Weapon 4 part found");
      else
        snprintf(found_msg, sizeof found_msg, "Weapon %d Found", weapon_num);

      dsda_AddMessage(found_msg);
    }
  }
  else
  {
    if (weapon_num != -1 && !(hexen && weapon_num == 4))
    {
      char weapon_notfound_msg[64];
      snprintf(weapon_notfound_msg, sizeof weapon_notfound_msg, "Weapon %d Not Found", weapon_num);
      dsda_AddMessage(weapon_notfound_msg);
    }
    else
      dsda_AddMessage(notfound_msg);
  }
}

int cheat_get_weapon(int num)
{
  if (hexen)
  {
    int HEXEN_SPR_4WPN = 666; // dummy value

    if (PlayerClass[consoleplayer] == PCLASS_FIGHTER)
      switch (num)
      {
        case 2: return HEXEN_SPR_WFAX; // timon's axe
        case 3: return HEXEN_SPR_WFHM; // hammer of retribution
        case 4: return HEXEN_SPR_4WPN; // quietus (3 parts)
        default: return false;
      }
    else if (PlayerClass[consoleplayer] == PCLASS_CLERIC)
      switch (num)
      {
        case 2: return HEXEN_SPR_WCSS; // serpent staff
        case 3: return HEXEN_SPR_WCFM; // firestorm
        case 4: return HEXEN_SPR_4WPN; // wraithverge (3 parts)
        default: return false;
      }
    else if (PlayerClass[consoleplayer] == PCLASS_MAGE)
      switch (num)
      {
        case 2: return HEXEN_SPR_WMCS; // frost shards
        case 3: return HEXEN_SPR_WMLG; // arc of death
        case 4: return HEXEN_SPR_4WPN; // bloodscourge (3 parts)
        default: return false;
      }
  }
  else if (heretic)
  {
    switch (num)
    {
      case 3: return HERETIC_SPR_WBOW; // crossbow
      case 4: return HERETIC_SPR_WBLS; // dragon claw
      case 5: return HERETIC_SPR_WSKL; // hellstaff
      case 6: return HERETIC_SPR_WPHX; // phoenix rod
      case 7: return HERETIC_SPR_WMCE; // firemace
      case 8: return HERETIC_SPR_WGNT; // gauntlets
      default: return false;
    }
  }
  else
    switch (num)
    {
      case 3: return SPR_SHOT; // shotgun
      case 4: return SPR_MGUN; // chaingun
      case 5: return SPR_LAUN; // rocket launcher
      case 6: return SPR_PLAS; // plasma gun
      case 7: return SPR_BFUG; // bfg
      case 8: return SPR_CSAW; // chainsaw
      case 9: return SPR_SGN2; // ssg
      default: return false;
    }

  return false;
}

static void cheat_reveal_kill()
{
  if (automap_input)
  {
    static int last_count;
    static mobj_t *last_mobj;

    dsda_TrackFeature(uf_iddt);

    cheat_cycle_mobj(&last_mobj, &last_count, MF_COUNTKILL, true, "No Monsters Found");
  }
}

static void cheat_reveal_item()
{
  if (automap_input)
  {
    static int last_count;
    static mobj_t *last_mobj;

    dsda_TrackFeature(uf_iddt);

    cheat_cycle_mobj(&last_mobj, &last_count, MF_COUNTITEM, false, "No Items Found");
  }
}

// Check weapon cheat
static void cheat_reveal_weapon()
{
  if (automap_input)
  {
    if (raven)
      dsda_AddMessage(heretic ? "Weapon number 3-8" : "Weapon number 2-4");
    else
      dsda_AddMessage(gamemode == commercial ? "Weapon number 3-9" : "Weapon number 3-8");
  }
}

// Reveal weapon cheat
void cheat_reveal_weaponx(int weapon)
{
  if (automap_input)
  {
    int sprite_num = cheat_get_weapon(weapon);

    if (sprite_num != false)
    {
      static int last_count;
      static mobj_t *last_mobj;

      dsda_TrackFeature(uf_iddt);

      cheat_cycle_mobj_spr(&last_mobj, &last_count, sprite_num, weapon, MF_SPECIAL, MF_DROPPED, "Weapon Not Found");
    }
  }
}

// Exit finder [Nugget]
static void cheat_reveal_exit(void)
{
  if (automap_input)
  {

    static int last_exit_line = -1;
    int i, start_i;
    int found_exit;
    int exit_lines, secret_lines;

    dsda_TrackFeature(uf_iddt);

    i = last_exit_line + 1;

    if (i >= numlines) { i = 0; }

    start_i = i;

    // (found_exit & 1) == normal
    // (found_exit & 2) == secret
    // Can be both
    found_exit = 0;

    do {
      const line_t *const line = &lines[i];
      const short special = line->special;

      if (hexen)
      {
        exit_lines = (special ==  74 ||
                      special ==  75);
        
        secret_lines = false;
      }
      else if (heretic)
      {
        exit_lines = (special ==  11 ||
                      special ==  51 ||
                      special ==  52 ||
                      special == 105);
        
        secret_lines = (special ==  51 ||
                        special == 105);
      }
      else
      {
        exit_lines = (special ==  11 ||
                      special ==  51 ||
                      special ==  52 ||
                      special == 124 ||
                      special == 197 ||
                      special == 198);
        
        secret_lines = (special ==  51 ||
                        special == 124 ||
                        special == 198);
      }

      if (exit_lines)
        found_exit |= 1 + secret_lines;

      if (!raven)
      {
        for (int j = 0;  j < 2;  j++)
        {
          sector_t *const sector = j ? line->backsector : line->frontsector;

          if (!raven && sector && P_IsDeathExit(sector))
          {
            found_exit |= 1 + ((sector->special & DEATH_MASK) &&
                               (sector->special & DAMAGE_MASK) == DAMAGE_MASK);
          }
        }
      }

      if (found_exit)
      {
        dsda_UpdateIntConfig(dsda_config_automap_follow, false, true);
        AM_SetMapCenter(line->v1->x, line->v1->y);

        doom_printf(
          "Exit Finder: found %s",
            (found_exit == 3) ? "normal and secret exits"
          : (found_exit == 2) ? "secret exit"
          :                     "normal exit"
        );

        last_exit_line = i;
        break;
      }

      i++;
      if (i >= numlines)
        i = 0;
    } while (i != start_i);

    if (!found_exit)
      doom_printf("Exit Finder: no exits found");
  }
}

// killough 2/7/98: HOM autodetection
static void cheat_hom()
{
  dsda_AddMessage(dsda_ToggleConfig(dsda_config_flashing_hom, true) ? "HOM Detection On"
                                                                    : "HOM Detection Off");
}

// killough 3/6/98: -fast parameter toggle
static void cheat_fast()
{
  dsda_AddMessage(dsda_ToggleConfig(dsda_config_fast_monsters, true) ? "Fast Monsters On"
                                                                     : "Fast Monsters Off");
  dsda_RefreshGameSkill(); // refresh fast monsters
}

static void cheat_nuke(void)
{
  dsda_AddMessage((disable_nuke = !disable_nuke) ?
                  "Nukage Disabled" : "Nukage Enabled");
}

// killough 2/16/98: keycard/skullkey cheat functions
static void cheat_key()
{
  if (hexen) return;

  doom_printf("Add key: %s, Yellow, Blue", !heretic ? "Red" : "Green");
}

static void cheat_keyx()
{
  if (raven) return;

  dsda_AddMessage("Add key: Card, Skull");
}

static void cheat_key_doom(int key)
{
  if (raven) return;

  cheat_keyxx(key);
}

static void cheat_key_heretic(int key)
{
  if (!heretic) return;

  cheat_keyxx(key);
}

static void cheat_keyxx(int key)
{
  dsda_AddMessage((plyr->cards[key] = !plyr->cards[key]) ? "Key Added" : "Key Removed");
}

// Key Finder [Nugget]
static void cheat_reveal_key(void)
{
  if (hexen) return;

  if (automap_input)
    doom_printf("Key Finder: %s, Yellow, Blue", !heretic ? "Red" : "Green");
}

static void cheat_reveal_keyx(void)
{
  if (raven) return;

  if (automap_input)
    dsda_AddMessage("Key Finder: Card, Skull");
}

void cheat_reveal_keyxx(int key)
{
  if (hexen) return;

  if (automap_input)
  {
    static int last_count;
    static mobj_t *last_mobj;

    dsda_TrackFeature(uf_iddt);

    cheat_cycle_mobj_spr(&last_mobj, &last_count, key, -1, MF_SPECIAL, false, "Key Finder: key not found");
  }
}

static void cheat_reveal_doom_key(int key)
{
  if (raven) return;

  if (automap_input)
    cheat_reveal_keyxx(key);
}

static void cheat_reveal_heretic_key(int key)
{
  if (!heretic) return;

  if (automap_input)
    cheat_reveal_keyxx(key);
}

// killough 2/16/98: generalized weapon cheats

static void cheat_weap()
{
  dsda_AddMessage(gamemode == commercial || heretic ? "Weapon number 1-9" : "Weapon number 1-8");
}

static void cheat_weapx(char buf[3])
{
  int w = *buf - '1';

  if ((w==wp_supershotgun && gamemode!=commercial) ||      // killough 2/28/98
      ((w==wp_bfg || w==wp_plasma) && gamemode==shareware))
    return;

  if (w==wp_fist)           // make '1' apply beserker strength toggle
    cheat_pw(pw_strength);
  else
    if (w >= 0 && w < NUMWEAPONS) {
      if ((plyr->weaponowned[w] = !plyr->weaponowned[w]))
        dsda_AddMessage("Weapon Added");
      else
        {
          dsda_AddMessage("Weapon Removed");
          if (w==plyr->readyweapon)         // maybe switch if weapon removed
            plyr->pendingweapon = P_SwitchWeapon(plyr);
        }
    }
}

// killough 2/16/98: generalized ammo cheats
static void cheat_ammo()
{
  int ammo_max = hexen ? NUMMANA : NUMAMMO;
  const char* backpack = !hexen ? ", Backpack" : "";
  doom_printf("Ammo 1-%i%s", ammo_max, backpack);
}

static void cheat_hexen_ammox(char buf[1])
{
  int a = *buf - '1';
  if (a>=0 && a<NUMMANA)  // Ty 03/27/98 - *not* externalized
      dsda_AddMessage((plyr->ammo[a] = !plyr->ammo[a]) ?
                       plyr->ammo[a] = MAX_MANA, "Mana Added" : "Mana Removed");
}

static void cheat_ammox(char buf[1])
{
  int a = *buf - '1';

  if (hexen)
    return cheat_hexen_ammox(buf);

  if (*buf == 'b')  // Ty 03/27/98 - strings *not* externalized
    if ((plyr->backpack = !plyr->backpack))
    {
      dsda_AddMessage("Backpack Added");
      for (a = 0; a < NUMAMMO; a++)
        plyr->maxammo[a] <<= 1;
    }
    else
    {
      dsda_AddMessage("Backpack Removed");
      for (a = 0; a < NUMAMMO; a++)
        if (plyr->ammo[a] > (plyr->maxammo[a] >>= 1))
          plyr->ammo[a] = plyr->maxammo[a];
    }
  else
    if (a>=0 && a<NUMAMMO)  // Ty 03/27/98 - *not* externalized
      { // killough 5/5/98: switch plasma and rockets for now -- KLUDGE
        if (!heretic)
          a = a==am_cell ? am_misl : a==am_misl ? am_cell : a;  // HACK
        dsda_AddMessage((plyr->ammo[a] = !plyr->ammo[a]) ?
                        plyr->ammo[a] = plyr->maxammo[a], "Ammo Added" : "Ammo Removed");
      }
}

static void cheat_smart()
{
  dsda_AddMessage((monsters_remember = !monsters_remember) ?
                  "Smart Monsters Enabled" : "Smart Monsters Disabled");
}

static void cheat_pitch()
{
  dsda_AddMessage(dsda_ToggleConfig(dsda_config_pitched_sounds, true) ? "Pitch Effects Enabled"
                                                                      : "Pitch Effects Disabled");
}

static void cheat_notarget()
{
  plyr->cheats ^= CF_NOTARGET;
  if (plyr->cheats & CF_NOTARGET)
    dsda_AddMessage("Notarget Mode ON");
  else
    dsda_AddMessage("Notarget Mode OFF");
}

static void cheat_camera()
{
  if (!allow_incompatibility)
    return dsda_AddMessage("Camera Mode Not Allowed");

  plyr->cheats ^= CF_CAMERA;
  plyr->cheats ^= CF_GODMODE;
  plyr->cheats ^= CF_NOTARGET;
  plyr->cheats ^= CF_FLY;

  if (plyr->cheats & CF_CAMERA)
    {
      plyr->mo->flags |= CF_GODMODE;
      plyr->mo->flags |= CF_NOTARGET;

      if (raven)
      {
        if (!plyr->powers[pw_flight])
        {
          P_GivePower(plyr, pw_flight);
          plyr->powers[pw_flight] = INT_MAX;
        }
      }
      else
      {
        plyr->mo->flags |= MF_NOGRAVITY;
        plyr->mo->flags |= MF_FLY;
      }

      dsda_AddMessage("Camera Mode ON");
    }
    else
    {
      plyr->mo->flags &= ~CF_GODMODE;
      plyr->mo->flags &= ~CF_NOTARGET;

      if (raven)
      {
        if (plyr->powers[pw_flight])
        {
          P_PlayerEndFlight(plyr);
          plyr->powers[pw_flight] = 0;
        }
      }
      else
      {
        plyr->mo->flags &= ~MF_NOGRAVITY;
        plyr->mo->flags &= ~MF_FLY;
      }

      dsda_AddMessage("Camera Mode OFF");
    }
}

static void cheat_freeze()
{
  dsda_ToggleFrozenMode();
  dsda_AddMessage(dsda_FrozenMode() ? "FREEZE ON" : "FREEZE OFF");
}

static void cheat_fly()
{
  if (plyr->mo != NULL)
  {
    if (raven)
    {
      if (plyr->powers[pw_flight])
      {
        P_PlayerEndFlight(plyr);
        plyr->powers[pw_flight] = 0;
        dsda_AddMessage("Fly mode OFF");
      }
      else
      {
        P_GivePower(plyr, pw_flight);
        plyr->powers[pw_flight] = INT_MAX;
        dsda_AddMessage("Fly mode ON");
      }
    }
    else
    {
      plyr->cheats ^= CF_FLY;
      if (plyr->cheats & CF_FLY)
      {
        plyr->mo->flags |= MF_NOGRAVITY;
        plyr->mo->flags |= MF_FLY;
        dsda_AddMessage("Fly mode ON");
      }
      else
      {
        plyr->mo->flags &= ~MF_NOGRAVITY;
        plyr->mo->flags &= ~MF_FLY;
        dsda_AddMessage("Fly mode OFF");
      }
    }
  }
}

static dboolean M_ClassicDemo(void)
{
  return (demorecording || demoplayback) && !dsda_AllowCasualExCmdFeatures();
}

dboolean M_CheatAllowed(int when)
{
  return !dsda_StrictMode() &&
         !(when & not_demo         && (demorecording || demoplayback)) &&
         !(when & not_classic_demo && M_ClassicDemo()) &&
         !(when & not_menu         && menuactive);
}

dboolean M_CheatGame(int game)
{
  return (game & cht_any) ||
         (game & cht_doom    && !raven) ||
         (game & cht_heretic && heretic) ||
         (game & cht_hexen   && hexen);
}

static int update_cheats = true;

static void cht_InitCheats(void)
{
  if (update_cheats)
  {
    cheatseq_t* cht;

    update_cheats = false;

    for (cht = cheat; WHICH_CHEAT(cht); cht++)
    {
      cht->sequence_len = strlen(WHICH_CHEAT(cht));
    }
  }
}

void cht_UpdateCheats(void)
{
  update_cheats = true;
  cht_InitCheats();
}

int cheat_in_progress = false;

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
static int M_FindCheats(int key)
{
  int rc = 0;
  cheatseq_t* cht;
  char char_key;

  cht_InitCheats();

  char_key = (char)key;
  cheat_in_progress = false;

  for (cht = cheat; WHICH_CHEAT(cht); cht++)
  {
    if (M_CheatAllowed(cht->when) && M_CheatGame(cht->game))
    {
      if (cht->chars_read < cht->sequence_len)
      {
        // still reading characters from the cheat code
        // and verifying.  reset back to the beginning
        // if a key is wrong

        if (char_key == WHICH_CHEAT(cht)[cht->chars_read])
          ++cht->chars_read;
        else if (char_key == WHICH_CHEAT(cht)[0])
          cht->chars_read = 1;
        else
          cht->chars_read = 0;

        cht->param_chars_read = 0;

        // mark cheat as "in progress" after 3 characters
        if (cht->chars_read >= 3 && cht->chars_read < cht->sequence_len)
          cheat_in_progress = true;
      }
      else if (cht->param_chars_read < -cht->arg)
      {
        // we have passed the end of the cheat sequence and are
        // entering parameters now

        cht->parameter_buf[cht->param_chars_read] = char_key;

        ++cht->param_chars_read;

        // affirmative response
        rc = 1;
      }

      if (cht->chars_read >= cht->sequence_len &&
          cht->param_chars_read >= -cht->arg)
      {
          // mark cheat progress "complete"
          cheat_in_progress = false;

        if (cht->param_chars_read)
        {
          static char argbuf[CHEAT_ARGS_MAX + 1];

          // process the arg buffer
          memcpy(argbuf, cht->parameter_buf, -cht->arg);

          cht->func(argbuf);
        }
        else
        {
          // call cheat handler
          cht->func(cht->arg);

          if (cht->repeatable)
          {
            --cht->chars_read;
          }
        }

        if (!cht->repeatable)
          cht->chars_read = cht->param_chars_read = 0;
        rc = 1;
      }
    }
  }

  return rc;
}

typedef struct cheat_input_s {
  int input;
  const cheat_when_t when;
  void (*const func)();
  const int arg;
} cheat_input_t;

static cheat_input_t cheat_input[] = {
  { dsda_input_iddqd, not_classic_demo, cheat_god, 0 },
  { dsda_input_idkfa, not_demo, cheat_kfa, 0 },
  { dsda_input_idfa, not_demo, cheat_fa, 0 },
  { dsda_input_idclip, not_classic_demo, cheat_noclip, 0 },
  { dsda_input_idbeholdh, not_demo, cheat_health, 0 },
  { dsda_input_idbeholdm, not_demo, cheat_megaarmour, 0 },
  { dsda_input_idbeholdv, not_demo, cheat_pw, pw_invulnerability },
  { dsda_input_idbeholds, not_demo, cheat_pw, pw_strength },
  { dsda_input_idbeholdi, not_demo, cheat_pw, pw_invisibility },
  { dsda_input_idbeholdr, not_demo, cheat_pw, pw_ironfeet },
  { dsda_input_idbeholda, cht_always, cheat_pw, pw_allmap },
  { dsda_input_idbeholdl, cht_always, cheat_pw, pw_infrared },
  { dsda_input_idmypos, cht_always, cheat_mypos, 0 },
  { dsda_input_idrate, cht_always, cheat_rate, 0 },
  { dsda_input_iddt, cht_always, cheat_ddt, 0 },
  { dsda_input_ponce, not_demo, cheat_reset_health, 0 },
  { dsda_input_shazam, not_demo, cheat_tome, 0 },
  { dsda_input_chicken, not_demo, cheat_chicken, 0 },
  { dsda_input_notarget, not_demo, cheat_notarget, 0 },
  { dsda_input_freeze, not_demo, cheat_freeze, 0 },
  { dsda_input_idmusrr, not_demo, cheat_musrr, 0 },
  { dsda_input_camera, not_demo, cheat_camera, 0 },
  { dsda_input_basilisk, not_demo, cheat_killonsight, 0 },
  { 0 }
};

dboolean M_CheatResponder(event_t *ev)
{
  cheat_input_t* cheat_i;

  if (dsda_ProcessCheatCodes() &&
      ev->type == ev_keydown &&
      M_FindCheats(ev->data1.i))
    return true;

  for (cheat_i = cheat_input; cheat_i->input; cheat_i++)
  {
    if (dsda_InputActivated(cheat_i->input))
    {
      if (M_CheatAllowed(cheat_i->when))
        cheat_i->func(cheat_i->arg);

      return true;
    }
  }

  if (M_CheatAllowed(not_demo) && dsda_InputActivated(dsda_input_avj))
  {
    plyr->mo->momz = 1000 * FRACUNIT / plyr->mo->info->mass;

    return true;
  }

  return false;
}

dboolean M_CheatEntered(const char* element, const char* value)
{
  cheatseq_t* cheat_i;

  for (cheat_i = cheat; WHICH_CHEAT(cheat_i); cheat_i++)
  {
    if (!strcmp(WHICH_CHEAT(cheat_i), element) && M_CheatAllowed(cheat_i->when & ~not_menu))
    {
      if (cheat_i->arg >= 0)
        cheat_i->func(cheat_i->arg);
      else
        cheat_i->func(value);
      return true;
    }
  }
  return false;
}

// heretic

#include "p_user.h"

static void cheat_reset_health(void)
{
  if (heretic && plyr->chickenTics)
  {
    plyr->health = plyr->mo->health = MAXCHICKENHEALTH;
  }
  else if (hexen && plyr->morphTics)
  {
    plyr->health = plyr->mo->health = MAXMORPHHEALTH;
  }
  else
  {
    plyr->health = plyr->mo->health = MAXHEALTH;
  }
  dsda_AddMessage(HERETIC_TXT_CHEATHEALTH);
}

static void cheat_artifact(char buf[3])
{
  int i;
  int j;
  int type;
  int count;

  if (!heretic) return;

  type = buf[0] - 'a' + 1;
  count = buf[1] - '0';
  if (type == 26 && count == 0)
  { // All artifacts
    for (i = arti_none + 1; i < NUMARTIFACTS; i++)
    {
      if (gamemode == shareware && (i == arti_superhealth || i == arti_teleport))
      {
        continue;
      }
      for (j = 0; j < 16; j++)
      {
        P_GiveArtifact(plyr, i, NULL);
      }
    }
    dsda_AddMessage(HERETIC_TXT_CHEATARTIFACTS3);
  }
  else if (type > arti_none && type < NUMARTIFACTS && count > 0 && count < 10)
  {
    if (gamemode == shareware && (type == arti_superhealth || type == arti_teleport))
    {
      dsda_AddMessage(HERETIC_TXT_CHEATARTIFACTSFAIL);
      return;
    }
    for (i = 0; i < count; i++)
    {
      P_GiveArtifact(plyr, type, NULL);
    }
    dsda_AddMessage(HERETIC_TXT_CHEATARTIFACTS3);
  }
  else
  {
    dsda_AddMessage(HERETIC_TXT_CHEATARTIFACTSFAIL);
  }
}

static void cheat_tome(void)
{
  if (!heretic) return;

  if (plyr->powers[pw_weaponlevel2])
  {
    plyr->powers[pw_weaponlevel2] = 0;
    dsda_AddMessage(HERETIC_TXT_CHEATPOWEROFF);
  }
  else
  {
    P_UseArtifact(plyr, arti_tomeofpower);
    dsda_AddMessage(HERETIC_TXT_CHEATPOWERON);
  }
}

static void cheat_chicken(void)
{
  if (!raven) return;

  P_MapStart();
  if (heretic)
  {
    if (plyr->chickenTics)
    {
      if (P_UndoPlayerChicken(plyr))
      {
          dsda_AddMessage(HERETIC_TXT_CHEATCHICKENOFF);
      }
    }
    else if (P_ChickenMorphPlayer(plyr))
    {
      dsda_AddMessage(HERETIC_TXT_CHEATCHICKENON);
    }
  }
  else
  {
    if (plyr->morphTics)
    {
      P_UndoPlayerMorph(plyr);
    }
    else
    {
      P_MorphPlayer(plyr);
    }
    dsda_AddMessage(HEXEN_TXT_CHEATPIG);
  }
  P_MapEnd();
}

// hexen

#include "hexen/p_acs.h"

static void cheat_init(void)
{
  if (dsda_ResolveINIT())
  {
    P_SetMessage(plyr, HERETIC_TXT_CHEATWARP, true);
  }
}

static void cheat_inventory(void)
{
  int i, j;
  int start, end;

  if (!raven) return;

  if (heretic)
  {
    start = arti_none + 1;
    end = NUMARTIFACTS;
  }
  else
  {
    start = hexen_arti_none + 1;
    end = hexen_arti_firstpuzzitem;
  }

  for (i = start; i < end; i++)
  {
    for (j = 0; j < g_arti_limit; j++)
    {
      P_GiveArtifact(plyr, i, NULL);
    }
  }
  P_SetMessage(plyr, HEXEN_TXT_CHEATARTIFACTS, true);
}

static void cheat_puzzle(void)
{
  int i;

  if (!hexen) return;

  for (i = hexen_arti_firstpuzzitem; i < HEXEN_NUMARTIFACTS; i++)
  {
    P_GiveArtifact(plyr, i, NULL);
  }
  P_SetMessage(plyr, HEXEN_TXT_CHEATPUZZLES, true);
}

static void cheat_class()
{
  P_SetMessage(plyr, HEXEN_TXT_CHEATCLASS1, true);
}

static void cheat_classx(char buf[2])
{
  int i;
  int new_class;

  if (!hexen) return;

  if (plyr->morphTics)
  {                           // don't change class if the player is morphed
      return;
  }

  new_class = 1 + (buf[0] - '0');
  if (new_class > PCLASS_MAGE || new_class < PCLASS_FIGHTER)
  {
    P_SetMessage(plyr, HEXEN_TXT_CHEATCLASSFAIL, true);
    return;
  }
  plyr->pclass = new_class;
  for (i = 0; i < NUMARMOR; i++)
  {
    plyr->armorpoints[i] = 0;
  }
  PlayerClass[consoleplayer] = new_class;
  P_PostMorphWeapon(plyr, wp_first);
  SB_SetClassData();
  P_SetMessage(plyr, HEXEN_TXT_CHEATCLASS2, true);
}

static void cheat_script(char buf[3])
{
  int script;
  byte script_args[3];
  int tens, ones;
  static char textBuffer[40];

  if (!map_format.acs) return;

  tens = buf[0] - '0';
  ones = buf[1] - '0';
  script = tens * 10 + ones;
  if (script < 1)
      return;
  if (script > 99)
      return;
  script_args[0] = script_args[1] = script_args[2] = 0;

  if (P_StartACS(script, 0, script_args, plyr->mo, NULL, 0))
  {
    snprintf(textBuffer, sizeof(textBuffer), "RUNNING SCRIPT %.2d", script);
    P_SetMessage(plyr, textBuffer, true);
  }
}
