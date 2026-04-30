/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
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
 *      Status bar code.
 *      Does the face/direction indicator animatin.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "deh/strings.h"
#include "m_random.h"
#include "i_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "st_font.h"
#include "r_main.h"
#include "am_map.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "s_sound.h"
#include "sounds.h"
#include "dstrings.h"
#include "p_pspr.h"
#include "r_draw.h"
#include "e6y.h"//e6y

#include "dsda/settings.h"
#include "dsda/stretch.h"
#include "dsda/text_color.h"
#include "dsda/animinfo.h"
#include "dsda/library.h"
#include "dsda/exhud.h"
#include "dsda/pause.h"
#include "dsda/hud_components/base.h"

#include "heretic/sb_bar.h"

//
// STATUS BAR DATA
//

// berserk icon type from config
berserk_icon_t berserk_icon;

// armor icon type from config
armor_icon_t armor_icon;

int ST_SCALED_HEIGHT;

// Check if STBAR exists
static dboolean stbar_exists;

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// Location of status bar
#define ST_X                    0
#define ST_X2                   104

// proff 08/18/98: Changed for high-res
#define ST_FX                   (ST_X+143)
#define ST_FY                   (ST_Y+1)
//#define ST_FX                   143
//#define ST_FY                   169

// Should be set to patch width
//  for tall numbers later on
//#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

// proff 08/18/98: Changed for high-res
#define ST_FACESX               (ST_X+143)
#define ST_FACESY               (ST_Y)
//#define ST_FACESX               143
//#define ST_FACESY               168

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?
// I dunno, why don't you go and find out!!!  killough

// AMMO number pos.
#define ST_AMMOWIDTH            3
// proff 08/18/98: Changed for high-res
#define ST_AMMOX                (ST_X+44)
#define ST_AMMOY                (ST_Y+3)
//#define ST_AMMOX                44
//#define ST_AMMOY                171

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
// proff 08/18/98: Changed for high-res
#define ST_HEALTHX              (ST_X+90)
#define ST_HEALTHY              (ST_Y+3)
//#define ST_HEALTHX              90
//#define ST_HEALTHY              171

#define ST_ICONX               (ST_X+143)
#define ST_ICONY               (ST_Y)

#define ST_BERSERKX             (ST_ICONX+28)
#define ST_BERSERKY             (ST_ICONY+25)
#define ST_ARMORICONX           (ST_ICONX+28)
#define ST_ARMORICONY           (ST_ICONY+2)

// Weapon pos.
// proff 08/18/98: Changed for high-res
#define ST_ARMSX                (ST_X+111)
#define ST_ARMSY                (ST_Y+4)
#define ST_ARMSBGX              (ST_X+104)
#define ST_ARMSBGY              (ST_Y)
//#define ST_ARMSX                111
//#define ST_ARMSY                172
//#define ST_ARMSBGX              104
//#define ST_ARMSBGY              168
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
// proff 08/18/98: Changed for high-res
#define ST_FRAGSX               (ST_X+138)
#define ST_FRAGSY               (ST_Y+3)
//#define ST_FRAGSX               138
//#define ST_FRAGSY               171
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
// proff 08/18/98: Changed for high-res
#define ST_ARMORX               (ST_X+221)
#define ST_ARMORY               (ST_Y+3)
//#define ST_ARMORX               221
//#define ST_ARMORY               171

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
// proff 08/18/98: Changed for high-res
#define ST_KEY0X                (ST_X+239)
#define ST_KEY0Y                (ST_Y+3)
//#define ST_KEY0X                239
//#define ST_KEY0Y                171
#define ST_KEY1WIDTH            ST_KEY0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_KEY1X                (ST_X+239)
#define ST_KEY1Y                (ST_Y+13)
//#define ST_KEY1X                239
//#define ST_KEY1Y                181
#define ST_KEY2WIDTH            ST_KEY0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_KEY2X                (ST_X+239)
#define ST_KEY2Y                (ST_Y+23)
//#define ST_KEY2X                239
//#define ST_KEY2Y                191

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
// proff 08/18/98: Changed for high-res
#define ST_AMMO0X               (ST_X+288)
#define ST_AMMO0Y               (ST_Y+5)
//#define ST_AMMO0X               288
//#define ST_AMMO0Y               173
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO1X               (ST_X+288)
#define ST_AMMO1Y               (ST_Y+11)
//#define ST_AMMO1X               288
//#define ST_AMMO1Y               179
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO2X               (ST_X+288)
#define ST_AMMO2Y               (ST_Y+23)
//#define ST_AMMO2X               288
//#define ST_AMMO2Y               191
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO3X               (ST_X+288)
#define ST_AMMO3Y               (ST_Y+17)
//#define ST_AMMO3X               288
//#define ST_AMMO3Y               185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO0X            (ST_X+314)
#define ST_MAXAMMO0Y            (ST_Y+5)
//#define ST_MAXAMMO0X            314
//#define ST_MAXAMMO0Y            173
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO1X            (ST_X+314)
#define ST_MAXAMMO1Y            (ST_Y+11)
//#define ST_MAXAMMO1X            314
//#define ST_MAXAMMO1Y            179
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO2X            (ST_X+314)
#define ST_MAXAMMO2Y            (ST_Y+23)
//#define ST_MAXAMMO2X            314
//#define ST_MAXAMMO2Y            191
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO3X            (ST_X+314)
#define ST_MAXAMMO3Y            (ST_Y+17)
//#define ST_MAXAMMO3X            314
//#define ST_MAXAMMO3Y            185

// killough 2/8/98: weapon info position macros UNUSED, removed here

// main player in game
static player_t *plyr;

// used to execute ST_Init() only once
static int veryfirsttime = 1;

// CPhipps - no longer do direct PLAYPAL handling here

// used for timing
static unsigned int st_clock;

// whether left-side main status bar is active
static dboolean st_statusbaron;

// Armor icon types
static patchnum_t armorpatch[ARMOR_ICON_END];
static patchnum_t armorstyle1[ARMOR_ICON_END];
static patchnum_t armorstyle2[ARMOR_ICON_END];

// Berserk icon types
static patchnum_t berserkpatch[BERSERK_ICON_END];

// face status patches
static patchnum_t faces[ST_NUMFACES];

// face background
static patchnum_t faceback; // CPhipps - single background, translated for different players

//e6y: status bar background
patchnum_t stbarbg;
patchnum_t grnrock;
patchnum_t brdr_t, brdr_b, brdr_l, brdr_r;
patchnum_t brdr_tl, brdr_tr, brdr_bl, brdr_br;

// stdisk stuff
dboolean stdisk_exists;
dboolean stcdrom_exists;
patchnum_t stdisk_icon;
patchnum_t stcdrom_icon;

// main bar right
static patchnum_t armsbg;

// weapon ownership patches
static patchnum_t arms[6][2];

// smooth health / armor values
int sts_animated_count;
int st_health = 0;
int st_armor = 0;

 // number of frags so far in deathmatch
static int      st_fragscount;

// used to use appopriately pained face
static int      st_oldhealth = -1;

// used for evil grin
static dboolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int      st_facecount = 0;

// current face index, used by w_faces
static int      st_faceindex = 0;

// [crispy] blinking key or skull in the status bar
int             st_keyorskull[3];
int             st_keytype[6];

// a random number per tick
static int      st_randomnumber;

static int st_priority;
static int st_lastattackcount = -1;
dboolean st_face_reset = true;

extern char     *mapnames[];

static int cr_health_bad;
static int cr_health_warning;
static int cr_health_ok;
static int cr_health_super;
static int cr_armor_zero;
static int cr_armor_one;
static int cr_armor_two;
static int cr_ammo_out;
static int cr_ammo_bad;
static int cr_ammo_warning;
static int cr_ammo_ok;
static int cr_ammo_full;

//
// STATUS BAR CODE
//

static void ST_Stop(void);

void ST_LoadTextColors(void)
{
  cr_health_bad = dsda_TextCR(dsda_tc_stbar_health_bad);
  cr_health_warning = dsda_TextCR(dsda_tc_stbar_health_warning);
  cr_health_ok = dsda_TextCR(dsda_tc_stbar_health_ok);
  cr_health_super = dsda_TextCR(dsda_tc_stbar_health_super);
  cr_armor_zero = dsda_TextCR(dsda_tc_stbar_armor_zero);
  cr_armor_one = dsda_TextCR(dsda_tc_stbar_armor_one);
  cr_armor_two = dsda_TextCR(dsda_tc_stbar_armor_two);
  cr_ammo_out = dsda_TextCR(dsda_tc_stbar_ammo_out);
  cr_ammo_bad = dsda_TextCR(dsda_tc_stbar_ammo_bad);
  cr_ammo_warning = dsda_TextCR(dsda_tc_stbar_ammo_warning);
  cr_ammo_ok = dsda_TextCR(dsda_tc_stbar_ammo_ok);
  cr_ammo_full = dsda_TextCR(dsda_tc_stbar_ammo_full);
}

int sts_solid_bg_color;

static void ST_refreshBackground(void)
{
  int y = ST_Y;
  enum patch_translation_e flags = VPT_ALIGN_LEFT_TOP;

  if (st_statusbaron)
    {
      flags = VPT_ALIGN_BOTTOM;

      // Draw Normal stbar if it exists
      if (stbar_exists)
        V_DrawNamePatchAnimateFS(ST_X, y, stbar, CR_DEFAULT, flags);

      // Draw Doom v1.1 two part statusbar (if stbar not found)
      if (doom_v11 && !stbar_exists)
      {
        V_DrawNamePatch(ST_X, y, "STMBARL", CR_DEFAULT, flags);
        V_DrawNamePatch(ST_ARMSBGX, y, "STMBARR", CR_DEFAULT, flags);
      }

      if (!deathmatch)
      {
        V_DrawNamePatchAnimate(ST_ARMSBGX, y, starms, CR_DEFAULT, flags);
      }
    }
}


// Respond to keyboard input events,
//  intercept cheats.
dboolean ST_Responder(event_t *ev)
{
  return M_CheatResponder(ev);
}

static int ST_GetPainOffset(void)
{
  static int lastPainOffset;
  static int oldhealth = -1;
  int maxhealth = 100;
  int health = CLAMP(plyr->health, 0, maxhealth);

  if (health != oldhealth)
  {
    oldhealth = health;
    lastPainOffset = ST_FACESTRIDE * (((maxhealth - health) * ST_NUMPAINFACES) / (maxhealth + 1));
  }

  return lastPainOffset;
}

static int ST_BaseFace(int offset)
{
  return ST_GetPainOffset() + offset;
}

static int ST_OuchHealth(void)
{
    // haleyjd 10/12/03: classic DOOM problem of missing OUCH face
    // was due to inversion of this test:
    // if(plyr->health - st_oldhealth > ST_MUCHPAIN)
    // e6y: compatibility optioned
    return comp[comp_ouchface] ?
            (plyr->health - st_oldhealth) :
            (st_oldhealth - plyr->health);
}

static void ST_DeathFace()
{
  if (!plyr->health)
  {
    st_priority = 9;
    st_faceindex = ST_DEADFACE;
    st_facecount = 1;
  }
}

static void ST_NewWeaponFace()
{
  int         i;
  dboolean    doevilgrin;

  if (plyr->bonuscount)
  {
    // picking up bonus
    doevilgrin = false;

    // check if just picked up weapon
    for (i=0;i<NUMWEAPONS;i++)
    {
      if (oldweaponsowned[i] != plyr->weaponowned[i])
      {
        doevilgrin = true;
        oldweaponsowned[i] = plyr->weaponowned[i];
      }
    }

    // evil grin if just picked up weapon
    if (doevilgrin)
    {
      st_priority = 8;
      st_faceindex = ST_BaseFace(ST_EVILGRINOFFSET);
      st_facecount = ST_EVILGRINCOUNT;
    }
  }
}

static void ST_EnemyDamageFace()
{
  angle_t     playerangle;
  angle_t     badguyangle;
  dboolean    right;

  if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
  {
    // being attacked
    st_priority = 7;

    if(ST_OuchHealth() > ST_MUCHPAIN)
    {
      // e6y
      // There are TWO bugs in the ouch face code.
      // Not only was the condition reversed, but the priority system is
      // broken in a way that makes the face not work with monster damage.
      if(!comp[comp_ouchface])
        st_priority = 8;

      st_facecount = ST_TURNCOUNT;
      st_faceindex = ST_BaseFace(ST_OUCHOFFSET);
    }
    else
    {
      playerangle = plyr->mo->angle;
      badguyangle = R_PointToAngle2(plyr->mo->x,
                                    plyr->mo->y,
                                    plyr->attacker->x,
                                    plyr->attacker->y);

      if (badguyangle > playerangle)
      {
        badguyangle -= playerangle;
        right = badguyangle > ANG180;
      }
      else
      {
        badguyangle = playerangle - badguyangle;
        right = badguyangle <= ANG180;
      }

      st_facecount = ST_TURNCOUNT;

      if (badguyangle < ANG45)  // head-on
        st_faceindex = ST_RAMPAGEOFFSET;
      else if (right)     // turn face right
        st_faceindex = ST_TURNOFFSET;
      else                // turn face left
        st_faceindex = ST_TURNOFFSET + 1;

      st_faceindex = ST_BaseFace(st_faceindex);
    }
  }
}

static void ST_EnvironmentDamageFace()
{
  if (plyr->damagecount)
  {
    st_facecount = ST_TURNCOUNT;

    if(ST_OuchHealth() > ST_MUCHPAIN)
    {
      st_priority = 7;
      st_faceindex = ST_BaseFace(ST_OUCHOFFSET);
    }
    else
    {
      st_priority = 6;
      st_faceindex = ST_BaseFace(ST_RAMPAGEOFFSET);
    }
  }
}

static dboolean ST_AttackCheck()
{
  dboolean attack_raw = plyr->cmd.buttons & BT_ATTACK;
  dboolean angry_face_fix = dsda_IntConfig(dsda_config_doomguy_angry_face_fix);

  if (st_face_reset)
  {
    if (attack_raw)
      st_face_reset = false;

    // Ignore G_PlayerReborn attackdown
    if (angry_face_fix)
      return false;
  }

  return plyr->attackdown;
}

static void ST_AttackHoldFace()
{
  // rapid firing
  if (ST_AttackCheck())
  {
    if (st_lastattackcount == -1)
    {
      st_lastattackcount = ST_RAMPAGEDELAY;
    }
    else if (--st_lastattackcount == 0)
    {
      st_priority = 5;
      st_faceindex = ST_BaseFace(ST_RAMPAGEOFFSET);
      st_facecount = 1;
      st_lastattackcount = 1;
    }
  }
  else
  {
    st_lastattackcount = -1;
  }
}

static void ST_InvulnerableFace()
{
  // invulnerability
  if ((plyr->cheats & CF_GODMODE)
      || plyr->powers[pw_invulnerability])
  {
    st_priority = 4;
    st_faceindex = ST_GODFACE;
    st_facecount = 1;
  }
}

static void ST_LookingFace()
{
  // look left or look right if the facecount has timed out
  st_faceindex = ST_BaseFace(st_randomnumber % 3);
  st_facecount = ST_STRAIGHTFACECOUNT;
  st_priority = 0;
}

void ST_updateFaceWidget()
{
  if (st_priority < 10)
    ST_DeathFace();

  if (st_priority < 9)
    ST_NewWeaponFace();

  if (st_priority < 8)
    ST_EnemyDamageFace();

  if (st_priority < 7)
    ST_EnvironmentDamageFace();

  if (st_priority < 6)
    ST_AttackHoldFace();

  if (st_priority < 5)
    ST_InvulnerableFace();

  if (!st_facecount)
    ST_LookingFace();

  st_facecount--;
}

static void ST_updateNyanIcons(void)
{
  int i;

  for (i = 0; i < ARMOR_ICON_END; i++)
  {
    int pwad_icon = W_PWADLumpNumExists2(armorstyle1[i].lumpnum);

    if (pwad_icon)
      armorpatch[i] = armorstyle1[i];
    else
    {
      if (armor_icon == ARMOR_ICON_1)
        armorpatch[i] = armorstyle1[i];
      else if (armor_icon == ARMOR_ICON_2)
        armorpatch[i] = armorstyle2[i];
    }
  }

  // nothing here for now....
  // for (i = 0; i < BERSERK_ICON_END; i++) { }
}

int sts_traditional_keys; // killough 2/28/98: traditional status bar keys
int sts_blink_keys; // [crispy] blinking key or skull in the status bar

void ST_SetKeyBlink(player_t* player, int blue, int yellow, int red)
{
  int i;
  // Init array with args to iterate through
  const int keys[3] = { blue, yellow, red };

  //if (raven) return;

  player->keyblinktics = KEYBLINKTICS;

  for (i = 0; i < 3; i++)
  {
    if (   ((keys[i] == KEYBLINK_EITHER) && !(player->cards[i] || player->cards[i+3]))
        || ((keys[i] == KEYBLINK_CARD)   && !(player->cards[i]))
        || ((keys[i] == KEYBLINK_SKULL)  && !(player->cards[i+3]))
        || ((keys[i] == KEYBLINK_BOTH)   && !(player->cards[i] && player->cards[i+3])))
    {
      player->keyblinkkeys[i] = keys[i];
    }
    else
    {
      player->keyblinkkeys[i] = KEYBLINK_NONE;
    }
  }
}

int ST_BlinkKey(player_t* player, int index)
{
  const keyblink_t keyblink = player->keyblinkkeys[index];

  if (!keyblink)
    return KEYBLINK_NONE;

  if (player->keyblinktics & KEYBLINKMASK)
  {
    if (keyblink == KEYBLINK_EITHER || (keyblink == KEYBLINK_BOTH && sts_traditional_keys))
    {
      if (st_keyorskull[index] && st_keyorskull[index] != KEYBLINK_BOTH)
      {
        return st_keyorskull[index];
      }
      else if ( (player->keyblinktics & (2*KEYBLINKMASK)) &&
               !(player->keyblinktics & (4*KEYBLINKMASK)))
      {
        return KEYBLINK_SKULL;
      }
      else
      {
        return KEYBLINK_CARD;
      }
    }
    else
    {
      return keyblink;
    }
  }

  return -1;
}

void ST_updateBlinkingKeys(player_t* plyr)
{
  if (hexen) return;

  // [crispy] blinking key or skull in the status bar
  if (plyr->keyblinktics)
  {
    int i;
    static int last_gametic = -1;
    int do_blink = sts_blink_keys && !dsda_StrictMode() && ((R_StatusBarVisible() && !inventory) || dsda_CheckExHudKeys());

    // Tie blinking to gametic (fixes Heretic key blinking)
    if (gametic != last_gametic)
    {
      last_gametic = gametic;

      if (do_blink)
      {
        if (!(plyr->keyblinktics & (2*KEYBLINKMASK - 1)))
          S_StartVoidSound(g_sfx_itemup);

        plyr->keyblinktics--;
      }
      else
        plyr->keyblinktics = 0;
    }

    if (!do_blink) return;

    // Always render key blinks (fixes Heretic key blinking)
    // Always render key blinks directly for Heretic
    if (heretic && R_StatusBarVisible())
    {
      for (i = 0; i < 3; i++)
      {
        switch (ST_BlinkKey(plyr, i))
        {
          case KEYBLINK_CARD:
            if (i == 0)
              V_DrawNamePatch(153, 180, "bkeyicon", CR_DEFAULT, VPT_STRETCH);
            else if (i == 1)
              V_DrawNamePatch(153, 164, "ykeyicon", CR_DEFAULT, VPT_STRETCH);
            else if (i == 2)
              V_DrawNamePatch(153, 172, "gkeyicon", CR_DEFAULT, VPT_STRETCH);
            break;

          case KEYBLINK_NONE:
          case KEYBLINK_SKULL:
          case KEYBLINK_BOTH:
          default:
            break;
        }
      }
    }
  }
}

static void ST_updateWidgets(void)
{
  // refresh nyan icons
  ST_updateNyanIcons();

  // refresh everything if this is him coming back to life
  ST_updateFaceWidget();

  // refresh blinking keys
  ST_updateBlinkingKeys(plyr);  
}

static void ST_DrawFaceBack(int x, int y, int vpt, dboolean exhud)
{
  int shadow;

  if (raven) return;

  shadow = exhud ? SHADOW_EXTRA : SHADOW_OFF;

  // killough 3/7/98: make face background change with displayplayer
  if (netgame)
  {
    V_DrawShadowedNumPatchAdv(x, y, faceback.lumpnum,
        shadow,
        displayplayer ? CR_LIMIT+displayplayer : CR_DEFAULT,
        displayplayer ? (VPT_COLOR | vpt) : vpt);
  }
}

static void ST_DrawFaceNyanExt(int x, int y, int vpt, dboolean exhud)
{
  int shadow;

  if (raven || exhud)
    return;

  shadow = (netgame || exhud) ? SHADOW_OFF : SHADOW_EXTRA;

  if (armor_icon)
  {
    int arm_x = x + 28;
    int arm_y = y + 2;
    int armor_type = CLAMP(plyr->armortype, 0, 2);
    int color = CR_DEFAULT;

    V_DrawShadowedNumPatchAdv(arm_x, arm_y, armorpatch[armor_type].lumpnum, shadow, color, vpt);
  }

  if (berserk_icon)
  {
    int berserk_x = x + 28;
    int berserk_y = y + 25;
    dboolean berserk_on = !!plyr->powers[pw_strength];
    int color = berserk_on ? unityedition ? CR_GREEN : CR_RED : CR_DEFAULT;

    // PWAD lumps found - disable colouring
    if (W_PWADLumpNumExists2(berserkpatch[berserk_on].lumpnum) || W_PWADLumpNumExists2(berserkpatch[!berserk_on].lumpnum))
      color = CR_DEFAULT;

    V_DrawShadowedNumPatchAdv(berserk_x, berserk_y, berserkpatch[berserk_on].lumpnum, shadow, color, vpt);
  }
}

void ST_DrawFaceWidget(int x, int y, int vpt, dboolean exhud)
{
  int shadow;

  if (raven) return;

  shadow = (exhud && !netgame) ? SHADOW_EXTRA : SHADOW_OFF;

  if (netgame)
    ST_DrawFaceBack(x, y, vpt, exhud);

  // Singleplayer
  V_DrawShadowedNumPatchAdv(x, y, faces[st_faceindex].lumpnum, shadow, CR_DEFAULT, vpt);
  ST_DrawFaceNyanExt(x, y, vpt, exhud);
}

static void ST_DrawArmsNum(int x, int y, int vpt, dboolean exhud)
{
  int i;
  dboolean ssg_arms_config;
  int st_shotguns;

  if (deathmatch)
    return;

  // SSG on ARMS config
  ssg_arms_config = dsda_IntConfig(dsda_config_ssg_on_arms) && (gamemode == commercial);

  // [crispy] show SSG availability in the Shotgun slot of the arms widget (only if config)
  st_shotguns = ssg_arms_config ? (plyr->weaponowned[wp_shotgun] || plyr->weaponowned[wp_supershotgun]) :
                                  (plyr->weaponowned[wp_shotgun]);

  // weapons owned
  for(i=0;i<6;i++)
  {
      int arm_x = x+(i%3)*ST_ARMSXSPACE;
      int arm_y = y+(i/3)*ST_ARMSYSPACE;
      dboolean owned = plyr->weaponowned[i+1];

      // [crispy] show SSG availability in the Shotgun slot of the arms widget
      if (i == wp_shotgun-1)
        owned = st_shotguns;

      V_DrawNumPatch(arm_x, arm_y, arms[i][owned].lumpnum, CR_DEFAULT, vpt);
  }
}

// Based off Raven Animated Numbers
int SmoothCount(int smoothval, int realval, int override)
{
    int delta;
    int curval;
    int ticker_delta_cap = 8;

    if (!sts_animated_count && !override)
    {
      return realval;
    }

    curval = realval;
    if (curval < 0)
    {
        curval = 0;
    }
    if (curval < smoothval)
    {
        delta = (smoothval - curval) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > ticker_delta_cap)
        {
            delta = ticker_delta_cap;
        }
        smoothval -= delta;
    }
    else if (curval > smoothval)
    {
        delta = (curval - smoothval) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > ticker_delta_cap)
        {
            delta = ticker_delta_cap;
        }
        smoothval += delta;
    }

    return smoothval;
}

int st_palette = 0;

static void ST_doPaletteStuff(void)
{
  int         palette;
  int damagecount = dsda_PainPalette() ? plyr->damagecount : 0;

  if (dsda_PowerPalette() && plyr->powers[pw_strength])
    {
      // slowly fade the berzerk out
      int berzerkcount = 12 - (plyr->powers[pw_strength]>>6);
      if (berzerkcount > damagecount)
        damagecount = berzerkcount;
    }

  if (damagecount)
    {
      // In Chex Quest, the player never sees red.  Instead, the
      // radiation suit palette is used to tint the screen green,
      // as though the player is being covered in goo by an
      // attacking flemoid.
      if (chex_exe)
      {
        palette = RADIATIONPAL;
      }
      else
      {
        palette = (damagecount + 7) >> 3;
        if (palette >= NUMREDPALS)
          palette = NUMREDPALS - 1;

        /* cph 2006/08/06 - if in the menu, reduce the red tint - navigating to
         * load a game can be tricky if the screen is all red */
        if (menuactive || dsda_Paused() || dsda_PainPaletteReduced())
          palette /= 2;

        palette += STARTREDPALS;
      }
    }
  else
    if (dsda_PickupPalette() && plyr->bonuscount)
      {
        palette = (plyr->bonuscount+7)>>3;
        if (palette >= NUMBONUSPALS)
          palette = NUMBONUSPALS-1;
        if (dsda_PickupPaletteReduced())
          palette /= 2;
        palette += STARTBONUSPALS;
      }
    else
      if (dsda_PowerPalette() && (plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet] & 8))
        palette = RADIATIONPAL;
      else
        palette = 0;

  if (palette != st_palette) {
    V_SetPalette(st_palette = palette); // CPhipps - use new palette function
  }
}

void M_ChangeApplyPalette(void)
{
  st_palette = -1;

  if (in_game && gamestate == GS_LEVEL)
  {
    if (raven)
      SB_PaletteFlash(true);
    else
      ST_doPaletteStuff();
  }
  else
    V_SetPalette(0);
}

void ST_Ticker(void)
{
  if (raven) RETURN(SB_Ticker());

  st_health = SmoothCount(st_health, plyr->health, false);
  st_armor  = SmoothCount(st_armor, plyr->armorpoints[ARMOR_ARMOR], false);

  st_clock++;
  st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = plyr->health;
}

int ST_HealthColor(int health)
{
  if (health < hud_health_red)
    return cr_health_bad;
  else if (health < hud_health_yellow)
    return cr_health_warning;
  else if (health <= hud_health_green)
    return cr_health_ok;
  else
    return cr_health_super;
}

int ST_ArmorColor(void)
{
  // armor color dictated by type (Status Bar)
  if (plyr->armortype >= 2)
    return cr_armor_two;
  else if (plyr->armortype == 1)
    return cr_armor_one;
  else
    return cr_armor_zero;
}

static const char* doom_keynames[9] = {
  "STKEYS0", "STKEYS1", "STKEYS2",
  "STKEYS3", "STKEYS4", "STKEYS5",
  "STKEYS6", "STKEYS7", "STKEYS8"
};

const char* ST_GetKeyName(player_t* player, int slot)
{
  int card, skull, combo;

  if (heretic)
  {
    int color;
    
    color = (slot == 0 ? key_yellow :
             slot == 1 ? key_green  :
                         key_blue);

    if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
    {
      switch (ST_BlinkKey(player, color))
      {
        case KEYBLINK_NONE: return NULL;
        case KEYBLINK_CARD:
        case KEYBLINK_BOTH:
          return color == key_yellow ? "ykeyicon" :
                 color == key_green  ? "gkeyicon" :
                                       "bkeyicon";
        default: return NULL;
          break;
      }
    }

    return player->cards[color] ? (color == key_yellow ? "ykeyicon" :
                                   color == key_green  ? "gkeyicon" :
                                                         "bkeyicon") :
                                                         NULL;
  }

  card  = slot;
  skull = slot + 3;
  combo = slot + 6;

  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
    switch (ST_BlinkKey(player, slot))
    {
      case KEYBLINK_NONE:
        return NULL;

      case KEYBLINK_CARD:
        return doom_keynames[card];

      case KEYBLINK_SKULL:
        return doom_keynames[skull];

      case KEYBLINK_BOTH:
        return doom_keynames[sts_traditional_keys ? skull : combo];

      default:
        return NULL;
    }
  }

  if (player->cards[skull])
    return doom_keynames[(!player->cards[card] || sts_traditional_keys) ? skull : combo];

  if (player->cards[card])
    return doom_keynames[card];

  return NULL;
}

static void ST_DrawKeys(int x, int y, int spacing, int vpt)
{
  int i;
  const char* keyname;

  for (i = 0; i < 3; i++)
  {
    keyname = ST_GetKeyName(plyr, i);

    if (keyname)
      V_DrawNamePatch(x, y, keyname, CR_DEFAULT, vpt);

    y += spacing;
  }
}

static void ST_DrawReadyWeaponText(int x, int y, int count, int vpt)
{
  //jff 2/16/98 make color of ammo depend on amount
  int cm = dsda_TextCR(dsda_AmmoColorBig(plyr));
  ammotype_t ammo_type = weaponinfo[plyr->readyweapon].ammo;
  int ammo;

  if (ammo_type == am_noammo || !plyr->maxammo[ammo_type])
    return;

  ammo = plyr->ammo[ammo_type];

  dsda_DrawBigNumber(x, y, 0, cm, vpt, count, ammo, true, ANCHOR_RIGHT, false);
}

static void ST_DrawFullAmmoNum(int x, int y, int vpt)
{
  int i;
  static const int row_y[4] = { 0, 6, 18, 12 };
  static const int max_indent = 26;

  // Full Ammo
  for (i = 0; i < 4; i++)
  {
    int current_ammo = plyr->ammo[i];
    int max_ammo = plyr->maxammo[i];

    // ammo count (all four kinds)
    dsda_DrawSmallNumber(x, y + row_y[i], 0, CR_DEFAULT, vpt, ST_AMMO0WIDTH, current_ammo, true, ANCHOR_RIGHT);   //jff 2/16/98 no xlation

    // max ammo count (all four kinds)
    dsda_DrawSmallNumber(x + max_indent, y + row_y[i], 0, CR_DEFAULT, vpt, ST_MAXAMMO0WIDTH, max_ammo, true, ANCHOR_RIGHT);
  }
}

static void ST_DrawFragsText(int x, int y, int count, int vpt)
{
  int i;

  if (!deathmatch)
    return;

  st_fragscount = 0;

  for (i = 0; i < g_maxplayers; i++)
  {
    if (i != displayplayer)            // killough 3/7/98
      st_fragscount += plyr->frags[i];
    else
      st_fragscount -= plyr->frags[i];
  }

  dsda_DrawBigNumber(x, y, 0, CR_DEFAULT, vpt, count, st_fragscount, true, ANCHOR_RIGHT, false);
}

static void ST_DrawHealthPercent(int x, int y, int count, int vpt)
{
  int health = st_health; // Animated health
  int cm = ST_HealthColor(health); //jff 2/16/98 make color of health depend on amount

  dsda_DrawBigNumber(x, y, 0, cm, vpt, count, health, true, ANCHOR_PERCENT, true);
}

static void ST_DrawArmorPercent(int x, int y, int count, int vpt)
{
  int armor = st_armor; // Animated armor
  int cm = ST_ArmorColor();

  dsda_DrawBigNumber(x, y, 0, cm, vpt, count, armor, true, ANCHOR_PERCENT, true);
}

static void ST_drawWidgets(void)
{
  enum patch_translation_e flags = VPT_ALIGN_BOTTOM;

  // ready weapon ammo
  ST_DrawReadyWeaponText(ST_AMMOX, ST_AMMOY, ST_AMMOWIDTH, flags);

  // Health Percent
  ST_DrawHealthPercent(ST_HEALTHX, ST_HEALTHY, ST_HEALTHWIDTH, flags);

  // in deathmatch only, summary of frags stats
  if (deathmatch)
    ST_DrawFragsText(ST_FRAGSX, ST_FRAGSY, ST_FRAGSWIDTH, flags);

  // arms numbers
  if (!deathmatch)
    ST_DrawArmsNum(ST_ARMSX, ST_ARMSY, flags, false);

  // Doomguy Face
  ST_DrawFaceWidget(ST_FACESX, ST_FACESY, flags, false);

  // Armor Percent
  ST_DrawArmorPercent(ST_ARMORX, ST_ARMORY, ST_ARMORWIDTH, flags);

  // Keys
  ST_DrawKeys(ST_KEY0X, ST_KEY0Y, 10, flags);

  // Full Ammo
  ST_DrawFullAmmoNum(ST_AMMO0X, ST_AMMO0Y, flags);
}

void ST_SetResolution(void)
{
  R_FillBackScreen();
}

dboolean drawdisk = false;
int drawdisktics;
const int DRAWDISKTICS = (TICRATE / 2);

void ST_DrawDisk(int x, int y, int vpt)
{
  if (drawdisk)
  {
    if (dsda_ShowDataDisk() && !raven && (stdisk_exists || stcdrom_exists) && drawdisktics)
    {
      dboolean stcdrom = dsda_ShowDataDisk() == 2 && stcdrom_exists;
      patchnum_t icon = stcdrom ? stcdrom_icon : stdisk_icon;

      V_DrawMenuNumPatch(x, y, icon.lumpnum, CR_DEFAULT, vpt);

      if (!--drawdisktics)
        drawdisk = false;
    }
  }
}

void ST_Drawer(void)
{
  dboolean statusbaron = R_StatusBarVisible();

  V_BeginUIDraw();

  if (raven)
  {
    SB_Drawer(statusbaron);
    V_EndUIDraw();
    return;
  }

  /* cph - let status bar on be controlled
   * completely by the call from D_Display
   * proff - really do it
   */

  ST_doPaletteStuff();  // Do red-/gold-shifts from damage/items

  if (statusbaron) {
    ST_refreshBackground(); // draw status bar background to off-screen buff
    ST_drawWidgets(); // draw widgets
  }

  V_EndUIDraw();
}


static void ST_loadDoomStbar(void)
{
  stbar_exists = W_LumpNameExists(stbar);

  // Main stbar
  if (stbar_exists)         R_SetPatchNum(&stbarbg, stbar);
  else if (doom_v11)        R_SetPatchNum(&stbarbg, "STMBARR");
}

static void ST_loadStdisk(void)
{
  if (W_LumpNameExists(s_stcdrom))
  {
    R_SetPatchNum(&stcdrom_icon, s_stcdrom);
    stcdrom_exists = true;
  }

  if (W_LumpNameExists(s_stdisk))
  {
    R_SetPatchNum(&stdisk_icon, s_stdisk);
    stdisk_exists = true;
  }
}

//
// ST_loadGraphics
//
// CPhipps - Loads graphics needed for status bar
//
static void ST_loadGraphics(void)
{
  int  i, facenum;
  char namebuf[9];
  // cph - macro that either acquires a pointer and lock for a lump, or
  // unlocks it. var is referenced exactly once in either case, so ++ in arg works

  // Armor Icon
  for (i=0;i<ARMOR_ICON_END;i++)
    {
      if (chex)
      {
        snprintf(namebuf, sizeof(namebuf), "CHXARMS%d", i);
        R_SetPatchNum(&armorstyle1[i], namebuf);
        R_SetPatchNum(&armorstyle2[i], namebuf);
      }
      else
      {
        snprintf(namebuf, sizeof(namebuf), "STFARMS%d", i);
        R_SetPatchNum(&armorstyle1[i], namebuf);
        snprintf(namebuf, sizeof(namebuf), "STFARM2%d", i);
        R_SetPatchNum(&armorstyle2[i], namebuf);
      }
    }

  // Berserk Icon
  for (i=0;i<BERSERK_ICON_END;i++)
    {
      if (chex)
        snprintf(namebuf, sizeof(namebuf), "CHXPSTR%d", i);
      else
        snprintf(namebuf, sizeof(namebuf), "STFPSTR%d", i);
      R_SetPatchNum(&berserkpatch[i], namebuf);
    }

  //e6y: status bar background
  ST_loadDoomStbar();
  R_SetPatchNum(&brdr_t, "brdr_t");
  R_SetPatchNum(&brdr_b, "brdr_b");
  R_SetPatchNum(&brdr_l, "brdr_l");
  R_SetPatchNum(&brdr_r, "brdr_r");
  R_SetPatchNum(&brdr_tl, "brdr_tl");
  R_SetPatchNum(&brdr_tr, "brdr_tr");
  R_SetPatchNum(&brdr_bl, "brdr_bl");
  R_SetPatchNum(&brdr_br, "brdr_br");

  // data access disk
  ST_loadStdisk();

  // arms background
  R_SetPatchNum(&armsbg, "STARMS");

  // arms ownership widgets
  for (i=0;i<6;i++)
    {
      // gray #
      snprintf(namebuf, sizeof(namebuf), "STGNUM%d", i+2);
      R_SetPatchNum(&arms[i][0], namebuf);

      // yellow #
      snprintf(namebuf, sizeof(namebuf), "STYSNUM%d", i + 2);
      R_SetPatchNum(&arms[i][1], namebuf);
    }

  // face backgrounds for different color players
  // killough 3/7/98: add better support for spy mode by loading all
  // player face backgrounds and using displayplayer to choose them:
  R_SetPatchNum(&faceback, "STFB0");

  // face states
  facenum = 0;
  for (i=0;i<ST_NUMPAINFACES;i++)
    {
      int j;
      for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
          snprintf(namebuf, sizeof(namebuf), "STFST%d%d", i, j);
          R_SetPatchNum(&faces[facenum++], namebuf);
        }
      snprintf(namebuf, sizeof(namebuf), "STFTR%d0", i);        // turn right
      R_SetPatchNum(&faces[facenum++], namebuf);
      snprintf(namebuf, sizeof(namebuf), "STFTL%d0", i);        // turn left
      R_SetPatchNum(&faces[facenum++], namebuf);
      snprintf(namebuf, sizeof(namebuf), "STFOUCH%d", i);       // ouch!
      R_SetPatchNum(&faces[facenum++], namebuf);
      snprintf(namebuf, sizeof(namebuf), "STFEVL%d", i);        // evil grin ;)
      R_SetPatchNum(&faces[facenum++], namebuf);
      snprintf(namebuf, sizeof(namebuf), "STFKILL%d", i);       // pissed off
      R_SetPatchNum(&faces[facenum++], namebuf);
    }
  R_SetPatchNum(&faces[facenum++], "STFGOD0");
  R_SetPatchNum(&faces[facenum++], "STFDEAD0");

  ST_LoadTextColors();
}

static void ST_initData(void)
{
  int i;

  plyr = &players[displayplayer];            // killough 3/7/98

  st_clock = 0;

  st_statusbaron = true;

  st_lastattackcount = -1;
  st_faceindex = 0;
  st_palette = -1;

  st_oldhealth = -1;

  for (i=0;i<NUMWEAPONS;i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

  dsda_InitPatchNumbers();
}

static dboolean st_stopped = true;

void ST_Start(void)
{
  if (raven) return;

  if (!st_stopped)
    ST_Stop();
  ST_initData();
  st_stopped = false;
}

static void ST_Stop(void)
{
  if (st_stopped)
    return;
  V_SetPalette(0);
  st_stopped = true;
}

void ST_Init(void)
{
  if (raven) RETURN(SB_Init());

  veryfirsttime = 0;
  ST_loadGraphics();
}
