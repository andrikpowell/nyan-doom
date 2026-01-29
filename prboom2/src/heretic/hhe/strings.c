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
//	HHE Strings
//

#include "doomstat.h"
#include "doomdef.h"
#include "heretic/dstrings.h"  // to get the initial values
#include "hexen/dstrings.h"

#include "deh/func.h"
#include "deh/strings.h"

// #include "heretic/dstrings.h" -- we don't do that here but we declare the
// variables.  This externalizes everything that there is a string
// set for in the language files.  See heretic/dstrings.h for detailed comments,
// original English values etc.  These are set to the macro values,
// which are set by D_ENGLSH.H or D_FRENCH.H(etc).  REX files are a
// better way of changing these strings globally by language.

// ====================================================================
// Any of these can be changed using the rex extensions

// Artifacts
const char *s_HERETIC_TXT_ARTIHEALTH          = HERETIC_TXT_ARTIHEALTH;
const char *s_HERETIC_TXT_ARTIFLY             = HERETIC_TXT_ARTIFLY;
const char *s_HERETIC_TXT_ARTIINVULNERABILITY = HERETIC_TXT_ARTIINVULNERABILITY;
const char *s_HERETIC_TXT_ARTITOMEOFPOWER     = HERETIC_TXT_ARTITOMEOFPOWER;
const char *s_HERETIC_TXT_ARTIINVISIBILITY    = HERETIC_TXT_ARTIINVISIBILITY;
const char *s_HERETIC_TXT_ARTIEGG             = HERETIC_TXT_ARTIEGG;
const char *s_HERETIC_TXT_ARTISUPERHEALTH     = HERETIC_TXT_ARTISUPERHEALTH;
const char *s_HERETIC_TXT_ARTITORCH           = HERETIC_TXT_ARTITORCH;
const char *s_HERETIC_TXT_ARTIFIREBOMB        = HERETIC_TXT_ARTIFIREBOMB;
const char *s_HERETIC_TXT_ARTITELEPORT        = HERETIC_TXT_ARTITELEPORT;

// Items
const char *s_HERETIC_TXT_ITEMHEALTH          = HERETIC_TXT_ITEMHEALTH;
const char *s_HERETIC_TXT_ITEMBAGOFHOLDING    = HERETIC_TXT_ITEMBAGOFHOLDING;
const char *s_HERETIC_TXT_ITEMSHIELD1         = HERETIC_TXT_ITEMSHIELD1;
const char *s_HERETIC_TXT_ITEMSHIELD2         = HERETIC_TXT_ITEMSHIELD2;
const char *s_HERETIC_TXT_ITEMSUPERMAP        = HERETIC_TXT_ITEMSUPERMAP;

// Ammo
const char *s_HERETIC_TXT_AMMOGOLDWAND1       = HERETIC_TXT_AMMOGOLDWAND1;
const char *s_HERETIC_TXT_AMMOGOLDWAND2       = HERETIC_TXT_AMMOGOLDWAND2;
const char *s_HERETIC_TXT_AMMOMACE1           = HERETIC_TXT_AMMOMACE1;
const char *s_HERETIC_TXT_AMMOMACE2           = HERETIC_TXT_AMMOMACE2;
const char *s_HERETIC_TXT_AMMOCROSSBOW1       = HERETIC_TXT_AMMOCROSSBOW1;
const char *s_HERETIC_TXT_AMMOCROSSBOW2       = HERETIC_TXT_AMMOCROSSBOW2;
const char *s_HERETIC_TXT_AMMOBLASTER1        = HERETIC_TXT_AMMOBLASTER1;
const char *s_HERETIC_TXT_AMMOBLASTER2        = HERETIC_TXT_AMMOBLASTER2;
const char *s_HERETIC_TXT_AMMOSKULLROD1       = HERETIC_TXT_AMMOSKULLROD1;
const char *s_HERETIC_TXT_AMMOSKULLROD2       = HERETIC_TXT_AMMOSKULLROD2;
const char *s_HERETIC_TXT_AMMOPHOENIXROD1     = HERETIC_TXT_AMMOPHOENIXROD1;
const char *s_HERETIC_TXT_AMMOPHOENIXROD2     = HERETIC_TXT_AMMOPHOENIXROD2;

// Weapons
const char *s_HERETIC_TXT_WPNMACE             = HERETIC_TXT_WPNMACE;
const char *s_HERETIC_TXT_WPNCROSSBOW         = HERETIC_TXT_WPNCROSSBOW;
const char *s_HERETIC_TXT_WPNBLASTER          = HERETIC_TXT_WPNBLASTER;
const char *s_HERETIC_TXT_WPNSKULLROD         = HERETIC_TXT_WPNSKULLROD;
const char *s_HERETIC_TXT_WPNPHOENIXROD       = HERETIC_TXT_WPNPHOENIXROD;
const char *s_HERETIC_TXT_WPNGAUNTLETS        = HERETIC_TXT_WPNGAUNTLETS;

// cheats
const char *s_HERETIC_TXT_CHEATGODON          = HERETIC_TXT_CHEATGODON;
const char *s_HERETIC_TXT_CHEATGODOFF         = HERETIC_TXT_CHEATGODOFF;
const char *s_HERETIC_TXT_CHEATNOCLIPON       = HERETIC_TXT_CHEATNOCLIPON;
const char *s_HERETIC_TXT_CHEATNOCLIPOFF      = HERETIC_TXT_CHEATNOCLIPOFF;
const char *s_HERETIC_TXT_CHEATWEAPONS        = HERETIC_TXT_CHEATWEAPONS;
const char *s_HERETIC_TXT_CHEATFLIGHTON       = HERETIC_TXT_CHEATFLIGHTON;
const char *s_HERETIC_TXT_CHEATFLIGHTOFF      = HERETIC_TXT_CHEATFLIGHTOFF;
const char *s_HERETIC_TXT_CHEATPOWERON        = HERETIC_TXT_CHEATPOWERON;
const char *s_HERETIC_TXT_CHEATPOWEROFF       = HERETIC_TXT_CHEATPOWEROFF;
const char *s_HERETIC_TXT_CHEATHEALTH         = HERETIC_TXT_CHEATHEALTH;
const char *s_HERETIC_TXT_CHEATKEYS           = HERETIC_TXT_CHEATKEYS;
const char *s_HERETIC_TXT_CHEATSOUNDON        = HERETIC_TXT_CHEATSOUNDON;
const char *s_HERETIC_TXT_CHEATSOUNDOFF       = HERETIC_TXT_CHEATSOUNDOFF;
const char *s_HERETIC_TXT_CHEATTICKERON       = HERETIC_TXT_CHEATTICKERON;
const char *s_HERETIC_TXT_CHEATTICKEROFF      = HERETIC_TXT_CHEATTICKEROFF;
const char *s_HERETIC_TXT_CHEATARTIFACTS1     = HERETIC_TXT_CHEATARTIFACTS1;
const char *s_HERETIC_TXT_CHEATARTIFACTS2     = HERETIC_TXT_CHEATARTIFACTS2;
const char *s_HERETIC_TXT_CHEATARTIFACTS3     = HERETIC_TXT_CHEATARTIFACTS3;
const char *s_HERETIC_TXT_CHEATARTIFACTSFAIL  = HERETIC_TXT_CHEATARTIFACTSFAIL;
const char *s_HERETIC_TXT_CHEATWARP           = HERETIC_TXT_CHEATWARP;
const char *s_HERETIC_TXT_CHEATSCREENSHOT     = HERETIC_TXT_CHEATSCREENSHOT;
const char *s_HERETIC_TXT_CHEATCHICKENON      = HERETIC_TXT_CHEATCHICKENON;
const char *s_HERETIC_TXT_CHEATCHICKENOFF     = HERETIC_TXT_CHEATCHICKENOFF;
const char *s_HERETIC_TXT_SHOWFPSON           = HERETIC_TXT_SHOWFPSON;
const char *s_HERETIC_TXT_SHOWFPSOFF          = HERETIC_TXT_SHOWFPSOFF;
const char *s_HERETIC_TXT_CHEATMASSACRE       = HERETIC_TXT_CHEATMASSACRE;
const char *s_HERETIC_TXT_CHEATIDDQD          = HERETIC_TXT_CHEATIDDQD;
const char *s_HERETIC_TXT_CHEATIDKFA          = HERETIC_TXT_CHEATIDKFA;

// keys
const char *s_HERETIC_TXT_GOTBLUEKEY          = HERETIC_TXT_GOTBLUEKEY;
const char *s_HERETIC_TXT_GOTGREENKEY         = HERETIC_TXT_GOTGREENKEY;
const char *s_HERETIC_TXT_GOTYELLOWKEY        = HERETIC_TXT_GOTYELLOWKEY;
const char *s_HERETIC_TXT_NEEDBLUEKEY         = HERETIC_TXT_NEEDBLUEKEY;
const char *s_HERETIC_TXT_NEEDGREENKEY        = HERETIC_TXT_NEEDGREENKEY;
const char *s_HERETIC_TXT_NEEDYELLOWKEY       = HERETIC_TXT_NEEDYELLOWKEY;

// automap
const char *s_HERETIC_AUTOPAGE                = HERETIC_AUTOPAGE;
const char *s_HERETIC_AMSTR_FOLLOWON          = HERETIC_AMSTR_FOLLOWON;
const char *s_HERETIC_AMSTR_FOLLOWOFF         = HERETIC_AMSTR_FOLLOWOFF;
const char *s_HERETIC_AMSTR_GRIDON            = HERETIC_AMSTR_GRIDON;
const char *s_HERETIC_AMSTR_GRIDOFF           = HERETIC_AMSTR_GRIDOFF;
const char *s_HERETIC_AMSTR_MARKEDSPOT        = HERETIC_AMSTR_MARKEDSPOT;
const char *s_HERETIC_AMSTR_MARKSCLEARED      = HERETIC_AMSTR_MARKSCLEARED;

// messages
const char *s_HERETIC_TXT_GAMESAVED           = HERETIC_TXT_GAMESAVED;
const char *s_HERETIC_MSGON                   = HERETIC_MSGON;
const char *s_HERETIC_MSGOFF                  = HERETIC_MSGOFF;
const char *s_HERETIC_SWSTRING                = HERETIC_SWSTRING;
const char *s_HERETIC_RAVENQUITMSG            = HERETIC_RAVENQUITMSG;
const char *s_HERETIC_ENDGAME                 = HERETIC_ENDGAME;

// main menu
const char *s_HERETIC_MNU_NEW_GAME            = HERETIC_MNU_NEW_GAME;
const char *s_HERETIC_MNU_OPTIONS             = HERETIC_MNU_OPTIONS;
const char *s_HERETIC_MNU_GAME_FILES          = HERETIC_MNU_GAME_FILES;
const char *s_HERETIC_MNU_INFO                = HERETIC_MNU_INFO;
const char *s_HERETIC_MNU_QUIT_GAME           = HERETIC_MNU_QUIT_GAME;
const char *s_HERETIC_MNU_SAVE_GAME           = HERETIC_MNU_SAVE_GAME;
const char *s_HERETIC_MNU_LOAD_GAME           = HERETIC_MNU_LOAD_GAME;
const char *s_HERETIC_MNU_MUSIC_VOL           = HERETIC_MNU_MUSIC_VOL;
const char *s_HERETIC_MNU_SFX_VOL             = HERETIC_MNU_SFX_VOL;
const char *s_HERETIC_MNU_CHOOSESKILL         = HERETIC_MNU_CHOOSESKILL;

// episode names
const char *s_HERETIC_EPISODE_1               = HERETIC_EPISODE_1;
const char *s_HERETIC_EPISODE_2               = HERETIC_EPISODE_2;
const char *s_HERETIC_EPISODE_3               = HERETIC_EPISODE_3;
const char *s_HERETIC_EPISODE_4               = HERETIC_EPISODE_4;
const char *s_HERETIC_EPISODE_5               = HERETIC_EPISODE_5;

// level names
const char *s_HERETIC_HUSTR_E1M1       = HERETIC_HUSTR_E1M1;
const char *s_HERETIC_HUSTR_E1M2       = HERETIC_HUSTR_E1M2;
const char *s_HERETIC_HUSTR_E1M3       = HERETIC_HUSTR_E1M3;
const char *s_HERETIC_HUSTR_E1M4       = HERETIC_HUSTR_E1M4;
const char *s_HERETIC_HUSTR_E1M5       = HERETIC_HUSTR_E1M5;
const char *s_HERETIC_HUSTR_E1M6       = HERETIC_HUSTR_E1M6;
const char *s_HERETIC_HUSTR_E1M7       = HERETIC_HUSTR_E1M7;
const char *s_HERETIC_HUSTR_E1M8       = HERETIC_HUSTR_E1M8;
const char *s_HERETIC_HUSTR_E1M9       = HERETIC_HUSTR_E1M9;
const char *s_HERETIC_HUSTR_E2M1       = HERETIC_HUSTR_E2M1;
const char *s_HERETIC_HUSTR_E2M2       = HERETIC_HUSTR_E2M2;
const char *s_HERETIC_HUSTR_E2M3       = HERETIC_HUSTR_E2M3;
const char *s_HERETIC_HUSTR_E2M4       = HERETIC_HUSTR_E2M4;
const char *s_HERETIC_HUSTR_E2M5       = HERETIC_HUSTR_E2M5;
const char *s_HERETIC_HUSTR_E2M6       = HERETIC_HUSTR_E2M6;
const char *s_HERETIC_HUSTR_E2M7       = HERETIC_HUSTR_E2M7;
const char *s_HERETIC_HUSTR_E2M8       = HERETIC_HUSTR_E2M8;
const char *s_HERETIC_HUSTR_E2M9       = HERETIC_HUSTR_E2M9;
const char *s_HERETIC_HUSTR_E3M1       = HERETIC_HUSTR_E3M1;
const char *s_HERETIC_HUSTR_E3M2       = HERETIC_HUSTR_E3M2;
const char *s_HERETIC_HUSTR_E3M3       = HERETIC_HUSTR_E3M3;
const char *s_HERETIC_HUSTR_E3M4       = HERETIC_HUSTR_E3M4;
const char *s_HERETIC_HUSTR_E3M5       = HERETIC_HUSTR_E3M5;
const char *s_HERETIC_HUSTR_E3M6       = HERETIC_HUSTR_E3M6;
const char *s_HERETIC_HUSTR_E3M7       = HERETIC_HUSTR_E3M7;
const char *s_HERETIC_HUSTR_E3M8       = HERETIC_HUSTR_E3M8;
const char *s_HERETIC_HUSTR_E3M9       = HERETIC_HUSTR_E3M9;
const char *s_HERETIC_HUSTR_E4M1       = HERETIC_HUSTR_E4M1;
const char *s_HERETIC_HUSTR_E4M2       = HERETIC_HUSTR_E4M2;
const char *s_HERETIC_HUSTR_E4M3       = HERETIC_HUSTR_E4M3;
const char *s_HERETIC_HUSTR_E4M4       = HERETIC_HUSTR_E4M4;
const char *s_HERETIC_HUSTR_E4M5       = HERETIC_HUSTR_E4M5;
const char *s_HERETIC_HUSTR_E4M6       = HERETIC_HUSTR_E4M6;
const char *s_HERETIC_HUSTR_E4M7       = HERETIC_HUSTR_E4M7;
const char *s_HERETIC_HUSTR_E4M8       = HERETIC_HUSTR_E4M8;
const char *s_HERETIC_HUSTR_E4M9       = HERETIC_HUSTR_E4M9;
const char *s_HERETIC_HUSTR_E5M1       = HERETIC_HUSTR_E5M1;
const char *s_HERETIC_HUSTR_E5M2       = HERETIC_HUSTR_E5M2;
const char *s_HERETIC_HUSTR_E5M3       = HERETIC_HUSTR_E5M3;
const char *s_HERETIC_HUSTR_E5M4       = HERETIC_HUSTR_E5M4;
const char *s_HERETIC_HUSTR_E5M5       = HERETIC_HUSTR_E5M5;
const char *s_HERETIC_HUSTR_E5M6       = HERETIC_HUSTR_E5M6;
const char *s_HERETIC_HUSTR_E5M7       = HERETIC_HUSTR_E5M7;
const char *s_HERETIC_HUSTR_E5M8       = HERETIC_HUSTR_E5M8;
const char *s_HERETIC_HUSTR_E5M9       = HERETIC_HUSTR_E5M9;

// Extra maps
const char *s_HERETIC_HUSTR_E6M1       = HERETIC_HUSTR_E6M1;
const char *s_HERETIC_HUSTR_E6M2       = HERETIC_HUSTR_E6M2;
const char *s_HERETIC_HUSTR_E6M3       = HERETIC_HUSTR_E6M3;
const char *s_HERETIC_HUSTR_E6M4       = HERETIC_HUSTR_E6M4;
const char *s_HERETIC_HUSTR_E6M5       = HERETIC_HUSTR_E6M5;
const char *s_HERETIC_HUSTR_E6M6       = HERETIC_HUSTR_E6M6;
const char *s_HERETIC_HUSTR_E6M7       = HERETIC_HUSTR_E6M7;
const char *s_HERETIC_HUSTR_E6M8       = HERETIC_HUSTR_E6M8;
const char *s_HERETIC_HUSTR_E6M9       = HERETIC_HUSTR_E6M9;

// textscreens
const char *s_HERETIC_E1TEXT           = HERETIC_E1TEXT;
const char *s_HERETIC_E2TEXT           = HERETIC_E2TEXT;
const char *s_HERETIC_E3TEXT           = HERETIC_E3TEXT;
const char *s_HERETIC_E4TEXT           = HERETIC_E4TEXT;
const char *s_HERETIC_E5TEXT           = HERETIC_E5TEXT;

// textscreen backgrounds
const char *s_HERETIC_BGFLATE1        = HERETIC_BGFLATE1;
const char *s_HERETIC_BGFLATE2        = HERETIC_BGFLATE2;
const char *s_HERETIC_BGFLATE3        = HERETIC_BGFLATE3;
const char *s_HERETIC_BGFLATE4        = HERETIC_BGFLATE4;
const char *s_HERETIC_BGFLATE5        = HERETIC_BGFLATE5;

// killers
const char *s_HERETIC_IN_KILLERS1     = HERETIC_IN_KILLERS1;
const char *s_HERETIC_IN_KILLERS2     = HERETIC_IN_KILLERS2;
const char *s_HERETIC_IN_KILLERS3     = HERETIC_IN_KILLERS3;
const char *s_HERETIC_IN_KILLERS4     = HERETIC_IN_KILLERS4;
const char *s_HERETIC_IN_KILLERS5     = HERETIC_IN_KILLERS5;
const char *s_HERETIC_IN_KILLERS6     = HERETIC_IN_KILLERS6;

// Intermission
const char *s_HERETIC_IN_BGFLAT       = HERETIC_IN_BGFLAT;
const char *s_HERETIC_IN_FINISHED     = HERETIC_IN_FINISHED;
const char *s_HERETIC_IN_ENTERING     = HERETIC_IN_ENTERING;
const char *s_HERETIC_IN_KILLS        = HERETIC_IN_KILLS;
const char *s_HERETIC_IN_ITEMS        = HERETIC_IN_ITEMS;
const char *s_HERETIC_IN_SECRETS      = HERETIC_IN_SECRETS;
const char *s_HERETIC_IN_TIME         = HERETIC_IN_TIME;
const char *s_HERETIC_IN_TOTAL        = HERETIC_IN_TOTAL;
const char *s_HERETIC_IN_VICTIMS      = HERETIC_IN_VICTIMS;

const char *s_HERETIC_COOP_BONUS      = HERETIC_COOP_BONUS;
const char *s_HERETIC_COOP_SECRET     = HERETIC_COOP_SECRET;

// skills
const char *s_HERETIC_SKILL_1         = HERETIC_SKILL_1;
const char *s_HERETIC_SKILL_2         = HERETIC_SKILL_2;
const char *s_HERETIC_SKILL_3         = HERETIC_SKILL_3;
const char *s_HERETIC_SKILL_4         = HERETIC_SKILL_4;
const char *s_HERETIC_SKILL_5         = HERETIC_SKILL_5;

// obituaries
// Melee
const char *s_OB_DSPARIL1HIT          = OB_DSPARIL1HIT;
const char *s_OB_DSPARIL2HIT          = OB_DSPARIL2HIT;
const char *s_OB_HERETICIMPHIT        = OB_HERETICIMPHIT;
const char *s_OB_IRONLICHHIT          = OB_IRONLICHHIT;
const char *s_OB_BONEKNIGHTHIT        = OB_BONEKNIGHTHIT;
const char *s_OB_MINOTAURHIT          = OB_MINOTAURHIT;
const char *s_OB_WIZARDHIT            = OB_WIZARDHIT;

// Range
const char *s_OB_CHICKEN              = OB_CHICKEN;
const char *s_OB_BEAST                = OB_BEAST;
const char *s_OB_CLINK                = OB_CLINK;
const char *s_OB_DSPARIL1             = OB_DSPARIL1;
const char *s_OB_DSPARIL2             = OB_DSPARIL2;
const char *s_OB_HERETICIMP           = OB_HERETICIMP;
const char *s_OB_IRONLICH             = OB_IRONLICH;
const char *s_OB_BONEKNIGHT           = OB_BONEKNIGHT;
const char *s_OB_MINOTAUR             = OB_MINOTAUR;
const char *s_OB_MUMMY                = OB_MUMMY;
const char *s_OB_MUMMYLEADER          = OB_MUMMYLEADER;
const char *s_OB_SNAKE                = OB_SNAKE;
const char *s_OB_WIZARD               = OB_WIZARD;

// Player Weapons
const char *s_OB_MPSTAFF              = OB_MPSTAFF;
const char *s_OB_MPGAUNTLETS          = OB_MPGAUNTLETS;
const char *s_OB_MPGOLDWAND           = OB_MPGOLDWAND;
const char *s_OB_MPCROSSBOW           = OB_MPCROSSBOW;
const char *s_OB_MPBLASTER            = OB_MPBLASTER;
const char *s_OB_MPSKULLROD           = OB_MPSKULLROD;
const char *s_OB_MPPHOENIXROD         = OB_MPPHOENIXROD;
const char *s_OB_MPMACE               = OB_MPMACE;

// Player Weapons (Tome)
const char *s_OB_MPPSTAFF             = OB_MPPSTAFF;
const char *s_OB_MPPGAUNTLETS         = OB_MPPGAUNTLETS;
const char *s_OB_MPPGOLDWAND          = OB_MPPGOLDWAND;
const char *s_OB_MPPCROSSBOW          = OB_MPPCROSSBOW;
const char *s_OB_MPPBLASTER           = OB_MPPBLASTER;
const char *s_OB_MPPSKULLROD          = OB_MPPSKULLROD;
const char *s_OB_MPPPHOENIXROD        = OB_MPPPHOENIXROD;
const char *s_OB_MPPMACE              = OB_MPPMACE;

//
// Hexen stuff (probably shouldn't be here, but meh)
//

// Artifacts
const char *s_TXT_ARTIINVULNERABILITY = TXT_ARTIINVULNERABILITY;
const char *s_TXT_ARTIHEALTH          = TXT_ARTIHEALTH;
const char *s_TXT_ARTISUPERHEALTH     = TXT_ARTISUPERHEALTH;
const char *s_TXT_ARTIHEALINGRADIUS   = TXT_ARTIHEALINGRADIUS;
const char *s_TXT_ARTISUMMON          = TXT_ARTISUMMON;
const char *s_TXT_ARTITORCH           = TXT_ARTITORCH;
const char *s_TXT_ARTIEGG             = TXT_ARTIEGG;
const char *s_TXT_ARTIFLY             = TXT_ARTIFLY;
const char *s_TXT_ARTIBLASTRADIUS     = TXT_ARTIBLASTRADIUS;
const char *s_TXT_ARTIPOISONBAG       = TXT_ARTIPOISONBAG;
const char *s_TXT_ARTITELEPORTOTHER   = TXT_ARTITELEPORTOTHER;
const char *s_TXT_ARTISPEED           = TXT_ARTISPEED;
const char *s_TXT_ARTIBOOSTMANA       = TXT_ARTIBOOSTMANA;
const char *s_TXT_ARTIBOOSTARMOR      = TXT_ARTIBOOSTARMOR;
const char *s_TXT_ARTITELEPORT        = TXT_ARTITELEPORT;

// Puzzle artifacts
const char *s_TXT_ARTIPUZZSKULL       = TXT_ARTIPUZZSKULL;
const char *s_TXT_ARTIPUZZGEMBIG      = TXT_ARTIPUZZGEMBIG;
const char *s_TXT_ARTIPUZZGEMRED      = TXT_ARTIPUZZGEMRED;
const char *s_TXT_ARTIPUZZGEMGREEN1   = TXT_ARTIPUZZGEMGREEN1;
const char *s_TXT_ARTIPUZZGEMGREEN2   = TXT_ARTIPUZZGEMGREEN2;
const char *s_TXT_ARTIPUZZGEMBLUE1    = TXT_ARTIPUZZGEMBLUE1;
const char *s_TXT_ARTIPUZZGEMBLUE2    = TXT_ARTIPUZZGEMBLUE2;
const char *s_TXT_ARTIPUZZBOOK1       = TXT_ARTIPUZZBOOK1;
const char *s_TXT_ARTIPUZZBOOK2       = TXT_ARTIPUZZBOOK2;
const char *s_TXT_ARTIPUZZSKULL2      = TXT_ARTIPUZZSKULL2;
const char *s_TXT_ARTIPUZZFWEAPON     = TXT_ARTIPUZZFWEAPON;
const char *s_TXT_ARTIPUZZCWEAPON     = TXT_ARTIPUZZCWEAPON;
const char *s_TXT_ARTIPUZZMWEAPON     = TXT_ARTIPUZZMWEAPON;
const char *s_TXT_ARTIPUZZGEAR        = TXT_ARTIPUZZGEAR;

// end heretic/dstrings.h variable declarations
// ====================================================================
