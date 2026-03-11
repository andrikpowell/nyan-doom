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
 *      Game completion, final screen animation.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomstat.h"
#include "d_event.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_random.h"
#include "p_enemy.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "deh/strings.h"  // Ty 03/22/98 - externalizations

#include "heretic/f_finale.h"
#include "hexen/f_finale.h"

#include "dsda/font.h"
#include "dsda/mapinfo.h"
#include "dsda/map_format.h"
#include "dsda/preferences.h"
#include "dsda/configuration.h"
#include "dsda/animinfo.h"
#include "dsda/library.h"
#include "dsda/input.h"

#include "f_finale.h" // CPhipps - hmm...

// defines for the end mission display text                     // phares

#define TEXTSPEED    3     // original value                    // phares
#define TEXTWAIT     250   // original value                    // phares
#define NEWTEXTSPEED 0.01f // new value                         // phares
#define NEWTEXTWAIT  1000  // new value                         // phares

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
int finalestage;
int finalecount;
const char*   finaletext;
const char*   finaleflat;
const char*   finalepatch;

// defines for the end mission display text                     // phares

// CPhipps - removed the old finale screen text message strings;
// they were commented out for ages already
// Ty 03/22/98 - ... the new s_WHATEVER extern variables are used
// in the code below instead.

int UMAPINFO_Text;

static int dsda_SkipInterText(void)
{
  if (allow_incompatibility && dsda_IntConfig(nyan_config_skip_default_text))
    return dsda_CheckInterText();
  
  return false;
}

void    F_CastTicker (void);
dboolean F_CastResponder (event_t *ev);
void    F_CastDrawer (void);

void WI_checkForAccelerate(void);    // killough 3/28/98: used to
extern int acceleratestage;          // accelerate intermission screens
int midstage;                 // whether we're in "mid-stage"

//
// F_StartFinale
//
void F_StartFinale (void)
{
  int mnum;
  int muslump;
  int SkipText;

  if (heretic) return Heretic_F_StartFinale();
  if (hexen) return Hexen_F_StartFinale();

  SkipText = dsda_SkipInterText();

  // Arsinikk
  // If iwad story text and option, move forward in Doom 2 / Plutonia / TNT
  if (SkipText && (gamemode == commercial) && !F_ShowCast())
  {
    gameaction = ga_worlddone;
    return;
  }

  gameaction = ga_nothing;
  gamestate = GS_FINALE;
  automap_active = false;

  // killough 3/28/98: clear accelerative text flags
  acceleratestage = midstage = 0;

  finaletext = NULL;
  finaleflat = NULL;
  finalepatch = NULL;

  dsda_InterMusic(&mnum, &muslump);

  if (muslump >= 0)
  {
    S_ChangeMusInfoMusic(muslump, true);
  }
  else
  {
    S_ChangeMusic(mnum, true);
  }

  // Arsinikk
  // If iwad story text and option, do actions for Doom 1
  // Music change above is needed for Doom 1
  // If Doom 2 cast, skip text and do cast
  if (SkipText)
  {
    if (gamemode != commercial)
    {
      if (gameepisode == 3)
        F_StartScroll(NULL, NULL, NULL, true);
      else
        F_StartPostFinale();
    }
    else if (F_ShowCast())
      F_StartCast(NULL, NULL, true); // cast of Doom 2 characters
    return;
  }

  // Okay - IWAD dependend stuff.
  // This has been changed severly, and
  //  some stuff might have changed in the process.
  switch ( gamemode )
  {
    // DOOM 1 - E1, E3 or E4, but each nine missions
    case shareware:
    case registered:
    case retail:
    {
      switch (gameepisode)
      {
        case 1:
             finaleflat = bgflatE1; // Ty 03/30/98 - new externalized bg flats
             finaletext = s_E1TEXT; // Ty 03/23/98 - Was e1text variable.
             break;
        case 2:
             finaleflat = bgflatE2;
             finaletext = s_E2TEXT; // Ty 03/23/98 - Same stuff for each
             break;
        case 3:
             finaleflat = bgflatE3;
             finaletext = s_E3TEXT;
             break;
        case 4:
             finaleflat = bgflatE4;
             finaletext = s_E4TEXT;
             break;
        default:
             // Ouch.
             break;
      }
      break;
    }

    // DOOM II and missions packs with E1, M34
    case commercial:
    {
      // Ty 08/27/98 - added the gamemission logic
      switch (gamemap)
      {
        case 6:
             finaleflat = bgflat06;
             finaletext = (gamemission==pack_tnt)  ? s_T1TEXT :
                          (gamemission==pack_plut) ? s_P1TEXT : s_C1TEXT;
             break;
        case 11:
             finaleflat = bgflat11;
             finaletext = (gamemission==pack_tnt)  ? s_T2TEXT :
                          (gamemission==pack_plut) ? s_P2TEXT : s_C2TEXT;
             break;
        case 20:
             finaleflat = bgflat20;
             finaletext = (gamemission==pack_tnt)  ? s_T3TEXT :
                          (gamemission==pack_plut) ? s_P3TEXT : s_C3TEXT;
             break;
        case 30:
             finaleflat = bgflat30;
             finaletext = (gamemission==pack_tnt)  ? s_T4TEXT :
                          (gamemission==pack_plut) ? s_P4TEXT : s_C4TEXT;
             break;
        case 15:
             finaleflat = bgflat15;
             finaletext = (gamemission==pack_tnt)  ? s_T5TEXT :
                          (gamemission==pack_plut) ? s_P5TEXT : s_C5TEXT;
             break;
        case 31:
             finaleflat = bgflat31;
             finaletext = (gamemission==pack_tnt)  ? s_T6TEXT :
                          (gamemission==pack_plut) ? s_P6TEXT : s_C6TEXT;
             break;
        default:
             // Ouch.
             break;
      }
      if (gamemission == pack_nerve && gamemap == 8)
      {
        finaleflat = bgflat06;
        finaletext = s_C6TEXT;
      }
      break;
      // Ty 08/27/98 - end gamemission logic
    }

    // Indeterminate.
    default:  // Ty 03/30/98 - not externalized
         finaleflat = "F_SKY1"; // Not used anywhere else.
         finaletext = s_C1TEXT;  // FIXME - other text, music?
         break;
  }

  if (dsda_FinaleShortcut()) {
    switch (gamemode) {
      case shareware:
      case registered:
      case retail:
        switch (gameepisode) {
          case 1:
              finaleflat = bgflatE1;
              finaletext = s_E1TEXT;
              break;
          case 2:
              finaleflat = bgflatE2;
              finaletext = s_E2TEXT;
              break;
          case 3:
              finaleflat = bgflatE3;
              finaletext = s_E3TEXT;
              break;
          case 4:
          default:
              finaleflat = bgflatE4;
              finaletext = s_E4TEXT;
              break;
        }
        break;
      case commercial:
        if (gamemission == pack_nerve) {
          finaleflat = bgflat06;
          finaletext = s_C6TEXT;
        }
        else {
          finaleflat = bgflat30;
          finaletext = (gamemission == pack_tnt)  ? s_T4TEXT :
                       (gamemission == pack_plut) ? s_P4TEXT : s_C4TEXT;
        }
        break;
      case indetermined:
        break;
    }
  }

  dsda_StartFinale();

  finalestage = 0;
  finalecount = 0;
}



dboolean F_Responder (event_t *event)
{
  if (heretic) return Heretic_F_Responder(event);
  if (hexen) return Hexen_F_Responder(event);

  if (finalestage == 2)
    return F_CastResponder (event);

  return false;
}

// Get_TextSpeed() returns the value of the text display speed  // phares
// Rewritten to allow user-directed acceleration -- killough 3/28/98

float Get_TextSpeed(void)
{
  return midstage ? NEWTEXTSPEED : (midstage=acceleratestage) ?
    acceleratestage=0, NEWTEXTSPEED : TEXTSPEED;
}


//
// F_Ticker
//
// killough 3/28/98: almost totally rewritten, to use
// player-directed acceleration instead of constant delays.
// Now the player can accelerate the text display by using
// the fire/use keys while it is being printed. The delay
// automatically responds to the user, and gives enough
// time to read.
//
// killough 5/10/98: add back v1.9 demo compatibility
//

dboolean F_ShowCast(void)
{
  return gamemap == 30 ||
         (gamemission == pack_nerve && allow_incompatibility && gamemap == 8) ||
         dsda_FinaleShortcut();
}

void F_Ticker(void)
{
  int i;
  int next_level = false;

  if (heretic) return Heretic_F_Ticker();
  if (hexen) return Hexen_F_Ticker();

  if (dsda_FTicker())
  {
    return;
  }

  if (!demo_compatibility || allow_incompatibility) // Allow for textscreen skip for Doom 1 + wait for Doom 2
    WI_checkForAccelerate();  // killough 3/28/98: check for acceleration
  else
    if (gamemode == commercial && finalecount > 50) // check for skipping
      for (i = 0; i < g_maxplayers; i++)
        if (players[i].cmd.buttons)
          next_level = true;      // go on to the next level

  // advance animation
  finalecount++;

  if (finalestage == 2)
    F_CastTicker();

  if (!finalestage)
    {
      float speed = demo_compatibility ? TEXTSPEED : Get_TextSpeed();
      /* killough 2/28/98: changed to allow acceleration */
      if (finalecount > strlen(finaletext)*speed +
          (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
          (midstage && acceleratestage)) {
        if (gamemode != commercial)       // Doom 1 / Ultimate Doom episode end
          {                               // with enough time, it's automatic
            if (gameepisode == 3)
              F_StartScroll(NULL, NULL, NULL, true);
            else
              F_StartPostFinale();
          }
        else   // you must press a button to continue in Doom 2
          if ((!demo_compatibility || allow_incompatibility) && midstage)
            {
              next_level = true;
            }
      }
    }

  if (next_level)
  {
    if (F_ShowCast())
      F_StartCast(NULL, NULL, true); // cast of Doom 2 characters
    else
      gameaction = ga_worlddone; // next level, e.g. MAP07
  }
}

//
// F_TextWrite
//
// This program displays the background and text at end-mission     // phares
// text time. It draws both repeatedly so that other displays,      //   |
// like the main menu, can be drawn over it dynamically and         //   V
// erased dynamically. The TEXTSPEED constant is changed into
// the Get_TextSpeed function so that the speed of writing the      //   ^
// text can be increased, and there's still time to read what's     //   |
// written.                                                         // phares
// CPhipps - reformatted

#include "hu_stuff.h"

void F_TextWrite (void)
{
  if (finalepatch)
  {
    V_ClearBorder(finalepatch);
    V_DrawNamePatchAnimateFS(0, 0, finalepatch, CR_DEFAULT, VPT_STRETCH);
  }
  else
    V_DrawBackgroundName(finaleflat);

  { // draw some of the text onto the screen
    int         cx = 10;
    int         cy = 10;
    const char* ch = finaletext; // CPhipps - const
    int         count = (int)((float)(finalecount - 10)/Get_TextSpeed()); // phares
    int         w;

    if (count < 0)
      count = 0;

    for ( ; count ; count-- ) {
      int       c = *ch++;

      if (!c)
        break;

      if (c == '\n') {
        cx = 10;
        cy += 11;
        continue;
      }

      c = toupper(c) - HU_FONTSTART;
      if (c < 0 || c> HU_FONTSIZE) {
        cx += 4;
        continue;
      }

      w = hud_font.font[c].width;
      if (cx+w > SCREENWIDTH)
        break;

      // CPhipps - patch drawing updated
      V_DrawMenuNumPatch(cx, cy, hud_font.font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);
      cx+=w;
    }
  }
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
  const char **name; // CPhipps - const**
  mobjtype_t   type;
} castinfo_t;

static const castinfo_t castorder_d2[] = {
  { &s_CC_ZOMBIE,  MT_POSSESSED },
  { &s_CC_SHOTGUN, MT_SHOTGUY },
  { &s_CC_HEAVY,   MT_CHAINGUY },
  { &s_CC_IMP,     MT_TROOP },
  { &s_CC_DEMON,   MT_SERGEANT },
  { &s_CC_LOST,    MT_SKULL },
  { &s_CC_CACO,    MT_HEAD },
  { &s_CC_HELL,    MT_KNIGHT },
  { &s_CC_BARON,   MT_BRUISER },
  { &s_CC_ARACH,   MT_BABY },
  { &s_CC_PAIN,    MT_PAIN },
  { &s_CC_REVEN,   MT_UNDEAD },
  { &s_CC_MANCU,   MT_FATSO },
  { &s_CC_ARCH,    MT_VILE },
  { &s_CC_SPIDER,  MT_SPIDER },
  { &s_CC_CYBER,   MT_CYBORG },
  { &s_CC_HERO,    MT_PLAYER },
  { NULL,          0 }
};

static const castinfo_t castorder_d1[] = {
  { &s_CC_ZOMBIE,  MT_POSSESSED },
  { &s_CC_SHOTGUN, MT_SHOTGUY },
  { &s_CC_IMP,     MT_TROOP },
  { &s_CC_DEMON,   MT_SERGEANT },
  { &s_CC_LOST,    MT_SKULL },
  { &s_CC_CACO,    MT_HEAD },
  { &s_CC_BARON,   MT_BRUISER },
  { &s_CC_SPIDER,  MT_SPIDER },
  { &s_CC_CYBER,   MT_CYBORG },
  { &s_CC_HERO,    MT_PLAYER },
  { NULL,          0 }
};

static const castinfo_t *castorder = castorder_d2;
static int castorder_count;

static int castnum;
static int casttics;
static state_t* caststate;
static dboolean castdeath;
static int castframes;
static int castonmelee;
static dboolean castattacking;
static uint64_t castflags;
static uint64_t castflags2;
static signed char	castangle; // [crispy] turnable cast
static signed char	castskip; // [crispy] skippable cast
static const char *castbackground;

//
// F_StartCast
//

static void F_StartCastMusic(const char* music, dboolean loop_music)
{
  if (music)
  {
    if (!S_ChangeMusicByName(music, loop_music))
      lprintf(LO_WARN, "Finale cast music not found: %s\n", music);
  }
  else if (gamemode == commercial)
  {
    S_ChangeMusic(mus_evil, loop_music);
  }
  else
  {
    lprintf(LO_WARN, "Finale cast music unspecified\n");
    S_StopMusic();
  }
}

// [crispy] randomize seestate and deathstate sounds in the cast
static int F_RandomizeSound (int sound)
{
	switch (sound)
	{
		// [crispy] actor->info->seesound, from p_enemy.c:A_Look()
		case sfx_posit1:
		case sfx_posit2:
		case sfx_posit3:
			return sfx_posit1 + Nyan_Random()%3;
			break;

		case sfx_bgsit1:
		case sfx_bgsit2:
			return sfx_bgsit1 + Nyan_Random()%2;
			break;

		// [crispy] actor->info->deathsound, from p_enemy.c:A_Scream()
		case sfx_podth1:
		case sfx_podth2:
		case sfx_podth3:
			return sfx_podth1 + Nyan_Random()%3;
			break;

		case sfx_bgdth1:
		case sfx_bgdth2:
			return sfx_bgdth1 + Nyan_Random()%2;
			break;

		default:
			return sound;
			break;
	}
}

typedef struct
{
	void *const action;
	const int sound;
	const dboolean early;
} actionsound_t;

static const actionsound_t actionsounds[] =
{
	{A_PosAttack,   sfx_pistol, false},
	{A_SPosAttack,  sfx_shotgn, false},
	{A_CPosAttack,  sfx_shotgn, false},
	{A_CPosRefire,  sfx_shotgn, false},
	{A_VileTarget,  sfx_vilatk, true},
	{A_SkelWhoosh,  sfx_skeswg, false},
	{A_SkelFist,    sfx_skepch, false},
	{A_SkelMissile, sfx_skeatk, true},
	{A_FatAttack1,  sfx_firsht, false},
	{A_FatAttack2,  sfx_firsht, false},
	{A_FatAttack3,  sfx_firsht, false},
	{A_HeadAttack,  sfx_firsht, true},
	{A_BruisAttack, sfx_firsht, true},
	{A_TroopAttack, sfx_claw,   false},
	{A_SargAttack,  sfx_sgtatk, true},
	{A_SkullAttack, sfx_sklatk, false},
	{A_PainAttack,  sfx_sklatk, true},
	{A_BspiAttack,  sfx_plasma, false},
	{A_CyberAttack, sfx_rlaunc, false},
};

// [crispy] play attack sound based on state action function (instead of state number)
static int F_SoundForState (int st)
{
	void *const castaction = (void *) caststate->action;
	void *const nextaction = (void *) (&states[caststate->nextstate])->action;

	// [crispy] fix Doomguy in casting sequence
	if (castaction == NULL)
	{
		if (st == S_PLAY_ATK2)
			return sfx_dshtgn;
		else
			return 0;
	}
	else
	{
		int i;

		for (i = 0; i < arrlen(actionsounds); i++)
		{
			const actionsound_t *const as = &actionsounds[i];

			if ((!as->early && castaction == as->action) ||
			    (as->early && nextaction == as->action))
			{
				return as->sound;
			}
		}
	}

	return 0;
}

void F_StartCast (const char* background, const char* music, dboolean loop_music)
{
  castorder = (gamemode == commercial ? castorder_d2 : castorder_d1);
  castorder_count = (gamemode == commercial ? arrlen(castorder_d2) : arrlen(castorder_d1));
  castbackground = (background ? background : bgcastcall);

  wipegamestate = -1;         // force a screen wipe
  castnum = 0;
  caststate = &states[mobjinfo[castorder[castnum].type].seestate];
  casttics = caststate->tics;
  castdeath = false;
  finalestage = 2;
  castframes = 0;
  castonmelee = 0;
  castattacking = false;

  F_StartCastMusic(music, loop_music);
}

//
// F_CastTicker
//
void F_CastTicker (void)
{
  int st;
  int sfx;

  if (--casttics > 0)
    return;                 // not time to change state yet

  if (caststate->tics == -1 || caststate->nextstate == S_NULL || castskip) // [crispy] skippable cast
  {
    if (castskip)
    {
        castnum += castskip;
        castskip = 0;
    }
    else
    {
      // switch from deathstate to next monster
      castnum++;
    }

    castdeath = false;
    if (castorder[castnum].name == NULL)
      castnum = 0;
    if (mobjinfo[castorder[castnum].type].seesound)
      S_StartVoidSound(F_RandomizeSound(mobjinfo[castorder[castnum].type].seesound));
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    castflags  = mobjinfo[castorder[castnum].type].flags;
    castflags2 = mobjinfo[castorder[castnum].type].flags2;
    castframes = 0;
    castangle = 0; // [crispy] turnable cast
  }
  else
  {
    // just advance to next state in animation

    // [crispy] Allow A_RandomJump() in deaths in cast sequence
    if (caststate->action == A_RandomJump && Nyan_Random() < caststate->misc2)
    {
        st = caststate->misc1;
    }
    else
    {
      // [crispy] fix Doomguy in casting sequence
      if (!castdeath && caststate == &states[S_PLAY_ATK1])
          st = S_PLAY_ATK2;
      else
      if (!castdeath && caststate == &states[S_PLAY_ATK2])
          goto stopattack;    // Oh, gross hack!
      else
        st = caststate->nextstate;
    }

    caststate = &states[st];
    castframes++;

    // [nyan] allow flags to be altered in cast sequence
    if (caststate->action == A_AddFlags)
    {
      castflags  |= caststate->args[0];
      castflags2 |= caststate->args[1];
    }
    else if (caststate->action == A_RemoveFlags)
    {
      castflags  &= ~caststate->args[0];
      castflags2 &= ~caststate->args[1];
    }

    sfx = F_SoundForState(st);
/*
    // sound hacks....
    switch (st)
    {
      case S_PLAY_ATK2:     sfx = sfx_dshtgn; break; // [crispy] fix Doomguy in casting sequence
      case S_POSS_ATK2:     sfx = sfx_pistol; break;
      case S_SPOS_ATK2:     sfx = sfx_shotgn; break;
      case S_VILE_ATK2:     sfx = sfx_vilatk; break;
      case S_SKEL_FIST2:    sfx = sfx_skeswg; break;
      case S_SKEL_FIST4:    sfx = sfx_skepch; break;
      case S_SKEL_MISS2:    sfx = sfx_skeatk; break;
      case S_FATT_ATK8:
      case S_FATT_ATK5:
      case S_FATT_ATK2:     sfx = sfx_firsht; break;
      case S_CPOS_ATK2:
      case S_CPOS_ATK3:
      case S_CPOS_ATK4:     sfx = sfx_shotgn; break;
      case S_TROO_ATK3:     sfx = sfx_claw; break;
      case S_SARG_ATK2:     sfx = sfx_sgtatk; break;
      case S_BOSS_ATK2:
      case S_BOS2_ATK2:
      case S_HEAD_ATK2:     sfx = sfx_firsht; break;
      case S_SKULL_ATK2:    sfx = sfx_sklatk; break;
      case S_SPID_ATK2:
      case S_SPID_ATK3:     sfx = sfx_shotgn; break;
      case S_BSPI_ATK2:     sfx = sfx_plasma; break;
      case S_CYBER_ATK2:
      case S_CYBER_ATK4:
      case S_CYBER_ATK6:    sfx = sfx_rlaunc; break;
      case S_PAIN_ATK3:     sfx = sfx_sklatk; break;
      default: sfx = 0; break;
    }
*/
    if (sfx)
      S_StartVoidSound(sfx);
  }

  if (!castdeath && castframes == 12)
  {
    // go into attack frame
    castattacking = true;
    if (castonmelee)
      caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
    else
      caststate=&states[mobjinfo[castorder[castnum].type].missilestate];
    castonmelee ^= 1;
    if (caststate == &states[S_NULL])
    {
      if (castonmelee)
        caststate=
          &states[mobjinfo[castorder[castnum].type].meleestate];
      else
        caststate=
          &states[mobjinfo[castorder[castnum].type].missilestate];
    }
  }

  if (castattacking)
  {
    if (castframes == 24
       ||  caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
    {
      stopattack:
      castattacking = false;
      castframes = 0;
      caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    }
  }

  casttics = caststate->tics;

  if (casttics == -1)
  {
    // [crispy] Allow A_RandomJump() in deaths in cast sequence
    if (caststate->action == A_RandomJump)
    {
        if (Nyan_Random() < caststate->misc2)
          caststate = &states[caststate->misc1];
        else
          caststate = &states[caststate->nextstate];

        casttics = caststate->tics;
    }

    if (casttics == -1)
        casttics = 15;
  }
}


//
// F_CastResponder
//

dboolean F_CastResponder (event_t* ev)
{
  dboolean xdeath = false;

  if (ev->type != ev_keydown)
    return false;

  // [crispy] make monsters turnable in cast ...
  if (dsda_InputActivated(dsda_input_turnleft))
  {
    if (++castangle > 7)
        castangle = 0;
    return false;
  }
  else if (dsda_InputActivated(dsda_input_turnright))
  {
    if (--castangle < 0)
        castangle = 7;
    return false;
  }
  else
  // [crispy] ... and allow to skip through them ..
  if (dsda_InputActivated(dsda_input_strafeleft))
  {
    castskip = castnum ? -1 : castorder_count - 2;
    return false;
  }
  else
  if (dsda_InputActivated(dsda_input_straferight))
  {
    castskip = +1;
    return false;
  }

  // [crispy] ... and finally turn them into gibbs
  if (dsda_InputActivated(dsda_input_speed))
    xdeath = true;

  if (castdeath)
    return true;                    // already in dying frames

  // go into death frame
  castdeath = true;
  if (xdeath && mobjinfo[castorder[castnum].type].xdeathstate)
	  caststate = &states[mobjinfo[castorder[castnum].type].xdeathstate];
  else
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
  casttics = caststate->tics;

  // [crispy] Allow A_RandomJump() in deaths in cast sequence
  if (casttics == -1 && caststate->action == A_RandomJump)
  {
      if (Nyan_Random() < caststate->misc2)
      {
          caststate = &states [caststate->misc1];
      }
      else
      {
          caststate = &states [caststate->nextstate];
      }
      casttics = caststate->tics;
  }

  castframes = 0;
  castattacking = false;
  if (xdeath && mobjinfo[castorder[castnum].type].xdeathstate)
      S_StartSound (NULL, sfx_slop);
  else
    if (mobjinfo[castorder[castnum].type].deathsound)
      S_StartVoidSound(F_RandomizeSound(mobjinfo[castorder[castnum].type].deathsound));

  return true;
}


static void F_CastPrint (const char* text) // CPhipps - static, const char*
{
  const char* ch; // CPhipps - const
  int         c;
  int         cx;
  int         w;
  int         width;

  // find width
  ch = text;
  width = 0;

  while (ch)
  {
    c = *ch++;
    if (!c)
      break;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      width += 4;
      continue;
    }

    w = hud_font.font[c].width;
    width += w;
  }

  // draw it
  cx = 160-width/2;
  ch = text;
  while (ch)
  {
    c = *ch++;
    if (!c)
      break;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      cx += 4;
      continue;
    }

    w = hud_font.font[c].width;
    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, 180, hud_font.font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);
    cx+=w;
  }
}


//
// F_CastDrawer
//

void F_CastDrawer (void)
{
  spritedef_t*        sprdef;
  spriteframe_t*      sprframe;
  int                 lump;
  dboolean            flip;
  int                 rot;
  int                 cm;
  int                 exflags;

  // e6y: wide-res
  V_ClearBorder(castbackground);
  // erase the entire screen to a background
  // CPhipps - patch drawing updated
  V_DrawNamePatchAnimateFS(0,0, castbackground, CR_DEFAULT, VPT_STRETCH); // Ty 03/30/98 bg texture extern

  F_CastPrint (*(castorder[castnum].name));

  // draw the current frame in the middle of the screen
  sprdef = &sprites[caststate->sprite];
  sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
  rot = castangle * 2;
  lump = sprframe->lump[rot]; // [crispy] turnable cast
  flip = (dboolean)sprframe->flip[rot]; // [crispy] turnable cast

  // set defaults
  cm = CR_DEFAULT;
  exflags = 0;

  // [nyan] allow colour translation
  if (castflags & MF_TRANSLATION)
  {
    cm = CR_LIMIT + ((castflags & MF_TRANSLATION) >> MF_TRANSSHIFT);
    exflags |= VPT_COLOR;
  }

  // [nyan] allow translucency
  if (castflags & MF_TRANSLUCENT)
  {
    exflags |= VPT_TRANSMAP;
  }

  // [nyan] allow fuzz
  if (castflags & MF_SHADOW)
  {
    exflags |= VPT_FUZZ;
  }

  // CPhipps - patch drawing updated
  V_DrawNumPatch(160, 170, lump+firstspritelump, cm,
     VPT_STRETCH | (flip ? VPT_FLIP : 0) | exflags);
}

//
// F_BunnyScroll
//

static const char* scrollpic1;
static const char* scrollpic2;

static void F_StartScrollMusic(const char* music, dboolean loop_music)
{
  if (music) {
    if (!S_ChangeMusicByName(music, loop_music))
      lprintf(LO_WARN, "Finale scroll music not found: %s\n", music);
  }
  else if (W_LumpNameExists("D_BUNNY"))
    S_ChangeMusic(mus_bunny, loop_music);
  else {
    lprintf(LO_WARN, "Finale scroll music unspecified\n");
    S_StopMusic();
  }
}

static dboolean end_patches_exist;

void F_StartScroll (const char* right, const char* left, const char* music, dboolean loop_music)
{
  wipegamestate = -1; // force a wipe
  scrollpic1 = right ? right : e3bunny1;
  scrollpic2 = left ? left : e3bunny2;
  finalecount = 0;
  finalestage = 1;

  end_patches_exist = W_CheckNumForName("END0") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END1") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END2") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END3") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END4") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END5") != LUMP_NOT_FOUND &&
                      W_CheckNumForName("END6") != LUMP_NOT_FOUND;

  F_StartScrollMusic(music, loop_music);
}

static void F_DrawEndPatches (void)
{
  char        name[10];
  int         stage;
  static int  laststage;

  if (!end_patches_exist)
    return;

  if (finalecount < 1130)
    return;
  if (finalecount < 1180)
  {
    // CPhipps - patch drawing updated
    V_DrawNamePatch((320-13*8)/2, (200-8*8)/2, "END0", CR_DEFAULT, VPT_STRETCH);
    laststage = 0;
    return;
  }

  stage = (finalecount-1180) / 5;
  if (stage > 6)
    stage = 6;
  if (stage > laststage)
  {
    S_StartVoidSound(sfx_pistol);
    laststage = stage;
  }

  snprintf(name, sizeof name, "END%i", stage);
  // CPhipps - patch drawing updated
  V_DrawNamePatch((320-13*8)/2, (200-8*8)/2, name, CR_DEFAULT, VPT_STRETCH);
}

#define R_PatchAnimateByName(name) (R_PatchByNum(N_GetPatchAnimateNum(name, false)))

void F_BunnyScroll (void)
{
  const rpatch_t *p1, *p2;
  int p1offset, p2width;
  float scrolled;

  // Get patch sizes (if widescreen or not)
  p1 = R_PatchAnimateByName(scrollpic1);
  p2 = R_PatchAnimateByName(scrollpic2);

  // Set patch offsets
  p1offset = (p1->width == 320) ? ((p2->width - 320) / 2) : 0;
  p2width = p2->width;

  // Start scrolling!
  scrolled = 320 - ((float)finalecount-230)/2;
  V_ClearBorder(scrollpic1);

  if (scrolled <= 0) {
    V_DrawNamePatchAnimateFS(0, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
  } else if (scrolled >= 320) {
    V_DrawNamePatchAnimateFS(p1offset, 0, scrollpic1, CR_DEFAULT, VPT_STRETCH);
    if (p1offset > 0)
      V_DrawNamePatchAnimateFS(-320, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
  } else {
    V_DrawNamePatchAnimatePreciseFS((float)(p1offset + 320) - scrolled, 0, scrollpic1, CR_DEFAULT, VPT_STRETCH);
    V_DrawNamePatchAnimatePreciseFS(-scrolled, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
  }
  if (p2width == 320)
    V_ClearBorderNoFill(scrollpic1);

  F_DrawEndPatches();
}

void F_StartPostFinale (void)
{
  finalecount = 0;
  finalestage = 1;
  wipegamestate = -1; // force a wipe
}

//
// F_Drawer
//
void F_Drawer (void)
{
  if (heretic) return Heretic_F_Drawer();
  if (hexen) return Hexen_F_Drawer();

  if (dsda_FDrawer())
  {
    return;
  }

  if (finalestage == 2)
  {
    F_CastDrawer ();
    return;
  }

  if (!finalestage)
    F_TextWrite ();
  else
  {
    const char* finalelump = NULL;

    // allows use of HELP2 screen for PWADs under DOOM 1
    dboolean showhelp2 = (gamemode == retail && pwad_help2_check) || gamemode <= registered;

    switch (gameepisode)
    {
      // CPhipps - patch drawing updated
      case 1:
        finalelump = showhelp2 ? help2 : credit;
        break;
      case 2:
        finalelump = e2victory;
        break;
      case 3:
        F_BunnyScroll ();
        break;
      case 4:
        finalelump = e4endpic;
        break;
    }

    if (finalelump)
    {
      V_ClearBorder(finalelump); // e6y: wide-res
      V_DrawNamePatchAnimateFS(0, 0, finalelump, CR_DEFAULT, VPT_STRETCH);
    }
  }
}
